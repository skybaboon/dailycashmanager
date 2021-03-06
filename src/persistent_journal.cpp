/*
 * Copyright 2013 Matthew Harvey
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "persistent_journal.hpp"
#include "account.hpp"
#include "entry.hpp"
#include "journal.hpp"
#include "dcm_database_connection.hpp"
#include "dcm_exceptions.hpp"
#include <jewel/decimal.hpp>
#include <jewel/exception.hpp>
#include <jewel/log.hpp>
#include <jewel/optional.hpp>
#include <sqloxx/handle.hpp>
#include <sqloxx/id.hpp>
#include <sqloxx/next_auto_key.hpp>
#include <sqloxx/sql_statement.hpp>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using jewel::Log;
using jewel::Decimal;
using jewel::DecimalAdditionException;
using jewel::DecimalRangeException;
using jewel::UninitializedOptionalException;
using jewel::value;
using sqloxx::next_auto_key;
using sqloxx::Handle;
using sqloxx::Id;
using sqloxx::SQLStatement;
using std::ostream;
using std::unordered_map;
using std::unordered_set;
using std::string;
using std::vector;

namespace dcm
{

string
PersistentJournal::exclusive_table_name()
{
    return "journals";
}

string
PersistentJournal::primary_key_name()
{
    return "journal_id";
}

void
PersistentJournal::setup_tables(DcmDatabaseConnection& dbc)
{
    dbc.execute_sql
    (   "create table transaction_types"
        "("
            "transaction_type_id integer primary key"
        ");"
    );
    for
    (   int i = 0;
        i != static_cast<int>(TransactionType::num_transaction_types);
        ++i
    )
    {
        SQLStatement statement
        (   dbc,
            "insert into transaction_types(transaction_type_id) values(:p)"
        );
        statement.bind(":p", i);
        statement.step_final();
    }
    dbc.execute_sql
    (   "create table journals"
        "("
            "journal_id integer primary key autoincrement, "
            "transaction_type_id integer not null references "
                "transaction_types, "
            "comment text"
        ");"
    );
    return;
}

PersistentJournal::PersistentJournal
(   IdentityMap& p_identity_map,
    IdentityMap::Signature const& p_signature
):
    PersistentObject(p_identity_map),
    Journal()
{
    (void)p_signature;  // silence compiler re. unused parameter
}

PersistentJournal::PersistentJournal
(   IdentityMap& p_identity_map,
    Id p_id,
    IdentityMap::Signature const& p_signature
):
    PersistentObject(p_identity_map, p_id),
    Journal()
{
    (void)p_signature;  // silence compiler re. unused parameter
}

PersistentJournal::~PersistentJournal() = default;

PersistentJournal::PersistentJournal(PersistentJournal const& rhs):
    PersistentObject(rhs),
    Journal(rhs)
{
}

void
PersistentJournal::swap(PersistentJournal& rhs)
{
    Journal::swap(rhs);
    PersistentObject::swap(rhs);
    return;
}

Id
PersistentJournal::save_new_journal_core()
{
    JEWEL_LOG_TRACE();
    ensure_pl_only_budget();
    if (would_cause_overflow())
    {
        JEWEL_THROW
        (   JournalOverflowException,
            "Posting of PersistentJournal would cause overflow or precision "
            "loss in Account balances."
        );
    }
    if (!is_balanced())
    {
        JEWEL_THROW
        (   UnbalancedJournalException,
            "Cannot save journal core in unbalanced state."
        );
    }
    Id const journal_id = next_auto_key
    <    DcmDatabaseConnection,
        Id
    >    (database_connection(), primary_table_name());
    SQLStatement statement
    (   database_connection(),
        "insert into journals(transaction_type_id, comment) "
        "values(:transaction_type_id, :comment)"
    );
    statement.bind
    (   ":transaction_type_id",
        static_cast<int>(Journal::do_get_transaction_type())
    );
    statement.bind(":comment", wx_to_std8(Journal::do_get_comment()));
    statement.step_final();
    for (Handle<Entry> const entry: entries())
    {
        entry->set_journal_id(journal_id);
        entry->save();
    }
    return journal_id;
}

void
PersistentJournal::save_existing_journal_core()
{
    JEWEL_LOG_TRACE();
    ensure_pl_only_budget();
    if (would_cause_overflow())
    {
        JEWEL_THROW
        (   JournalOverflowException,
            "Posting of PersistentJournal would cause overflow or precision "
            "loss in Account balances."
        );
    }
    if (!is_balanced())
    {
        JEWEL_THROW
        (   UnbalancedJournalException,
            "Cannot save journal core in unbalanced state."
        );
    }
    SQLStatement updater
    (   database_connection(),
        "update journals "
        "set comment = :comment, "
        "transaction_type_id = :transaction_type_id "
        "where journal_id = :id"
    );
    updater.bind
    (   ":transaction_type_id",
        static_cast<int>(Journal::do_get_transaction_type())
    );
    updater.bind(":comment", wx_to_std8(Journal::do_get_comment()));
    updater.bind(":id", id());
    updater.step_final();
    unordered_set<Id> saved_entry_ids;
    for (Handle<Entry> const& entry: Journal::do_get_entries())
    {
        entry->save();
        JEWEL_ASSERT (entry->has_id());
        saved_entry_ids.insert(entry->id());
    }
    // Remove any entries in the database with this journal's journal_id, that
    // no longer exist in the in-memory journal
    SQLStatement entry_finder
    (   database_connection(),    
        "select entry_id from entries where journal_id = :journal_id"
    );
    entry_finder.bind(":journal_id", id());
    unordered_set<Id>::const_iterator const saved_entries_end =
        saved_entry_ids.end();
    while (entry_finder.step())
    {
        Id const entry_id = entry_finder.extract<Id>(0);
        if (saved_entry_ids.find(entry_id) == saved_entries_end)
        {
            Handle<Entry> const doomed_entry(database_connection(), entry_id);
            // This entry is in the database but no longer in the in-memory
            // journal, so should be deleted.
            doomed_entry->remove();
            // Note it's OK even if the last entry is deleted. Another
            // entry will never be reassigned its id - SQLite makes sure
            // of that - providing we let SQLite assign all the ids
            // automatically.
        }
    }
    JEWEL_LOG_TRACE();
    return;
}

void
PersistentJournal::load_journal_core()
{
    SQLStatement statement
    (   database_connection(),
        "select transaction_type_id, comment "
        "from journals where journal_id = :p"
    );
    statement.bind(":p", id());
    statement.step();
    Journal temp(*this);
    SQLStatement entry_finder
    (   database_connection(),    
        "select entry_id from entries where journal_id = :jid "
        "order by entry_id"
    );
    entry_finder.bind(":jid", id());
    while (entry_finder.step())
    {
        Id const entr_id = entry_finder.extract<Id>(0);
        Handle<Entry> const entry(database_connection(), entr_id);
        temp.push_entry(entry);
    }
    temp.set_transaction_type
    (   static_cast<TransactionType>
        (   statement.extract<int>(0)
        )
    );
    temp.set_comment(std8_to_wx(statement.extract<string>(1)));
    Journal::swap(temp);    
    return;
}

void
PersistentJournal::ghostify_journal_core()
{
    for (Handle<Entry> const& entry: entries())
    {
        entry->ghostify();
    }
    clear_core();
    return;
}

std::vector<Handle<Entry> > const&
PersistentJournal::do_get_entries()
{
    load();
    return Journal::do_get_entries();
}

void
PersistentJournal::do_set_transaction_type(TransactionType p_transaction_type)
{
    load();
    Journal::do_set_transaction_type(p_transaction_type);
    return;
}

void
PersistentJournal::do_set_comment(wxString const& p_comment)
{
    load();
    Journal::do_set_comment(p_comment);
    return;
}

void
PersistentJournal::do_push_entry(Handle<Entry> const& p_entry)
{
    load();
    Journal::do_push_entry(p_entry);
    if (has_id())
    {
        p_entry->set_journal_id(id());
    }
    return;
}

void
PersistentJournal::do_remove_entry(Handle<Entry> const& p_entry)
{
    load();
    Journal::do_remove_entry(p_entry);
    return;
}

void
PersistentJournal::do_clear_entries()
{
    load();
    Journal::do_clear_entries();
    return;
}

wxString
PersistentJournal::do_get_comment()
{
    load();
    return Journal::do_get_comment();
}

TransactionType
PersistentJournal::do_get_transaction_type()
{
    load();
    return Journal::do_get_transaction_type();
}

void
PersistentJournal::ensure_pl_only_budget()
{
    if (transaction_type() != TransactionType::envelope)
    {
        return;
    }
    JEWEL_ASSERT (transaction_type() == TransactionType::envelope);
    for (auto const& entry: entries())
    {
        auto const ast = entry->account()->account_super_type();
        if (ast != AccountSuperType::pl)
        {
            JEWEL_THROW
            (   InvalidJournalException,
                "Budget Journal contains an Entry to a non-P&L Account."
            );
        }
    }
    return;
}

bool
PersistentJournal::would_cause_overflow()
{
    JEWEL_LOG_TRACE();
    unordered_map<Id, Decimal> prospective_balances;
    for (auto const& entry: entries())
    {
        Handle<Account> account;
        try
        {
            account = entry->account();
        }
        catch (UninitializedOptionalException&)
        {
            // do nothing
        }
        if (static_cast<bool>(account) && account->has_id())
        {
            auto const aid = account->id();
            if (prospective_balances.find(aid) == prospective_balances.end())
            {
                prospective_balances[aid] = account->technical_balance();
            }
            try
            {
                JEWEL_LOG_TRACE();
                prospective_balances[aid] += entry->amount();    
                JEWEL_LOG_TRACE();
            }
            catch (DecimalAdditionException&)
            {
                JEWEL_LOG_TRACE();
                return true;
            }
            catch (DecimalRangeException&)
            {
                JEWEL_LOG_TRACE();
                return true;
            }
        }
    }
    JEWEL_LOG_TRACE();
    return false;
}

bool
has_entry_with_id(PersistentJournal& journal, Id entry_id)
{
    for (Handle<Entry> const& entry: journal.entries())
    {
        if (entry->has_id() && (entry->id() == entry_id))
        {
            return true;
        }
    }
    return false;
}

Id
max_journal_id(DcmDatabaseConnection& dbc)
{
    SQLStatement s(dbc, "select max(journal_id) from journals");
    s.step();
    return s.extract<Id>(0);
}

Id
min_journal_id(DcmDatabaseConnection& dbc)
{
    SQLStatement s(dbc, "select min(journal_id) from journals");
    s.step();
    return s.extract<Id>(0);
}

bool
journal_id_exists(DcmDatabaseConnection& dbc, Id id)
{
    SQLStatement s
    (   dbc,
        "select journal_id from journals where journal_id = :p"
    );
    s.bind(":p", id);
    return s.step();
}

bool
journal_id_is_draft
(   DcmDatabaseConnection& dbc,
    Id id
)
{
    JEWEL_ASSERT (journal_id_exists(dbc, id));
    SQLStatement s
    (   dbc,
        "select journal_id from draft_journal_detail where "
        "journal_id = :p"
    );
    s.bind(":p", id);
    return s.step();
}
    
}  // namespace dcm

