// Copyright (c) 2013, Matthew Harvey. All rights reserved.


/** \file entry_impl.cpp
 *
 * \brief Source file pertaining to Entry class.
 *
 * \author Matthew Harvey
 * \date 04 July 2012.
 *
 * Copyright (c) 2012, Matthew Harvey. All rights reserved.
 */


#include "entry.hpp"
#include "entry_data.hpp"
#include "account_handle.hpp"
#include "date.hpp"
#include "draft_journal.hpp"
#include "string_conv.hpp"
#include "commodity_handle.hpp"
#include "ordinary_journal.hpp"
#include "phatbooks_database_connection.hpp"
#include "string_conv.hpp"
#include "transaction_side.hpp"
#include "transaction_type.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <sqloxx/database_connection.hpp>
#include <sqloxx/general_typedefs.hpp>
#include <sqloxx/handle.hpp>
#include <sqloxx/sql_statement.hpp>
#include <boost/optional.hpp>
#include <jewel/log.hpp>
#include <jewel/decimal.hpp>
#include <jewel/optional.hpp>
#include <wx/string.h>
#include <memory>
#include <string>
#include <utility>

using boost::optional;
using jewel::clear;
using jewel::Decimal;
using jewel::value;
using sqloxx::Id;
using sqloxx::SQLStatement;
using std::move;
using std::string;
using std::unique_ptr;

namespace gregorian = boost::gregorian;

namespace phatbooks
{

void Entry::setup_tables(PhatbooksDatabaseConnection& dbc)
{
	dbc.execute_sql
	(	"create table transaction_sides"
		"("
			"transaction_side_id integer primary key"
		");"
	);
	for
	(	int i = 0;
		i != static_cast<int>(TransactionSide::num_transaction_sides);
		++i
	)
	{
		SQLStatement statement
		(	dbc,
			"insert into transaction_sides(transaction_side_id) values(:p)"
		);
		statement.bind(":p", i);
		statement.step_final();
	}
	dbc.execute_sql
	(	"create table entries"
		"("
			"entry_id integer primary key autoincrement, "
			"journal_id not null references journals, "
			"comment text, "
			"account_id not null references accounts, "
			"amount integer not null, "
			"is_reconciled not null references booleans, "
			"transaction_side_id not null references transaction_sides"
		");"
	);
	dbc.execute_sql
	(	"create index entry_journal_index on entries(journal_id); "
	);
	return;
}

Entry::Entry
(	IdentityMap& p_identity_map
):
	PersistentObject(p_identity_map),
	m_data(new EntryData)
{
}


Entry::Entry
(	IdentityMap& p_identity_map,
	Id p_id
):
	PersistentObject(p_identity_map, p_id),
	m_data(new EntryData)
{
}

Entry::~Entry() = default;

void
Entry::set_journal_id(Id p_journal_id)
{
	load();
	m_data->journal_id = p_journal_id;
	return;
}


void
Entry::set_account(AccountHandle const& p_account)
{
	load();
	m_data->account = p_account;
	return;
}


void
Entry::set_comment(wxString const& p_comment)
{
	load();
	m_data->comment = p_comment;
	return;
}


void
Entry::set_amount(Decimal const& p_amount)
{
	load();
	m_data->amount = p_amount;
	return;
}

void
Entry::set_whether_reconciled(bool p_is_reconciled)
{
	load();
	m_data->is_reconciled = p_is_reconciled;
	return;
}

void
Entry::set_transaction_side
(	TransactionSide p_transaction_side
)
{
	load();
	m_data->transaction_side = p_transaction_side;
}

AccountHandle
Entry::account()
{
	load();
	return value(m_data->account);
}

wxString
Entry::comment()
{
	load();
	return value(m_data->comment);
}


jewel::Decimal
Entry::amount()
{
	load();
	return value(m_data->amount);
}

bool
Entry::is_reconciled()
{
	load();
	return value(m_data->is_reconciled);
}

TransactionSide
Entry::transaction_side()
{
	load();
	return value(m_data->transaction_side);
}

void
Entry::swap(Entry& rhs)
{
	swap_base_internals(rhs);
	using std::swap;
	swap(m_data, rhs.m_data);
	return;
}

Entry::Entry(Entry const& rhs):
	PersistentObject(rhs),
	m_data(new EntryData(*(rhs.m_data)))
{
}

void
Entry::do_load()
{
	Entry temp(*this);
	SQLStatement statement
	(	database_connection(),
		"select account_id, comment, amount, journal_id, is_reconciled, "
		"transaction_side_id "
		" from entries where "
		"entry_id = :p"
	);
	statement.bind(":p", id());
	statement.step();
	AccountHandle const acct
	(	database_connection(),
		statement.extract<sqloxx::Id>(0)
	);
	Decimal const amt
	(	statement.extract<Decimal::int_type>(2),
		acct->commodity()->precision()
	);

	temp.m_data->account = acct;
	temp.m_data->comment = std8_to_wx(statement.extract<string>(1));
	temp.m_data->amount = amt;
	temp.m_data->journal_id = statement.extract<Id>(3);
	temp.m_data->is_reconciled =
		static_cast<bool>(statement.extract<int>(4));
	temp.m_data->transaction_side =
		static_cast<TransactionSide>
		(	statement.extract<int>(5)
		);
	
	swap(temp);
	return;
}

void
Entry::process_saving_statement(SQLStatement& statement)
{
	statement.bind(":journal_id", value(m_data->journal_id));
	statement.bind(":comment", wx_to_std8(value(m_data->comment)));
	statement.bind(":account_id", value(m_data->account)->id());
	statement.bind(":amount", m_data->amount->intval());
	statement.bind
	(	":is_reconciled",
		static_cast<int>(value(m_data->is_reconciled))
	);
	statement.bind
	(	":transaction_side_id",
		static_cast<int>(value(m_data->transaction_side))
	);
	statement.step_final();
	return;
}

void
Entry::do_save_existing()
{
	// We need to get the old Account so we can mark it as stale
	SQLStatement old_account_capturer
	(	database_connection(),
		"select account_id from entries where entry_id = :p"
	);
	old_account_capturer.bind(":p", id());
	old_account_capturer.step();
	PhatbooksDatabaseConnection::BalanceCacheAttorney::mark_as_stale
	(	database_connection(),
		old_account_capturer.extract<sqloxx::Id>(0)
	);
	old_account_capturer.step_final();

	// And we also need to mark the new Account as stale
	PhatbooksDatabaseConnection::BalanceCacheAttorney::mark_as_stale
	(	database_connection(),
		account()->id()
	);

	// And now we can update the Entry itself
	SQLStatement updater
	(	database_connection(),
		"update entries set "
		"journal_id = :journal_id, "
		"comment = :comment, "
		"account_id = :account_id, "
		"amount = :amount, "
		"is_reconciled = :is_reconciled, "
		"transaction_side_id = :transaction_side_id "
		"where entry_id = :entry_id"
	);
	updater.bind(":entry_id", id());
	process_saving_statement(updater);
	return;
}


void
Entry::do_save_new()
{
	PhatbooksDatabaseConnection::BalanceCacheAttorney::mark_as_stale
	(	database_connection(),
		account()->id()
	);
	SQLStatement inserter
	(	database_connection(),
		"insert into entries"
		"("
			"journal_id, "
			"comment, "
			"account_id, "
			"amount, "
			"is_reconciled, "
			"transaction_side_id"
		") "
		"values"
		"("
			":journal_id, "
			":comment, "
			":account_id, "
			":amount, "
			":is_reconciled, "
			":transaction_side_id"
		")"
	);
	process_saving_statement(inserter);
	return;
}

void
Entry::do_ghostify()
{
	clear(m_data->journal_id);
	clear(m_data->account);
	clear(m_data->comment);
	clear(m_data->amount);
	clear(m_data->is_reconciled);
	clear(m_data->transaction_side);
	return;
}

void
Entry::do_remove()
{
	PhatbooksDatabaseConnection::BalanceCacheAttorney::mark_as_stale
	(	database_connection(),
		account()->id()
	);
	std::string const statement_text =
		"delete from " + primary_table_name() + " where " +
		primary_key_name() + " = :p";
	SQLStatement statement(database_connection(), statement_text);
	statement.bind(":p", id());
	statement.step_final();
}

std::string
Entry::primary_table_name()
{
	return "entries";
}

std::string
Entry::exclusive_table_name()
{
	return primary_table_name();
}

std::string
Entry::primary_key_name()
{
	return "entry_id";
}

void
Entry::mimic(Entry& rhs)
{
	load();
	Entry temp(*this);
	temp.set_account(rhs.account());
	temp.set_comment(rhs.comment());
	temp.set_amount(rhs.amount());
	temp.set_whether_reconciled(rhs.is_reconciled());
	temp.set_transaction_side(rhs.transaction_side());
	swap(temp);
	return;
}
	
gregorian::date
Entry::date()
{
	OrdinaryJournal const oj(database_connection(), journal_id());
	return oj.date();
}

sqloxx::Id
Entry::journal_id()
{
	load();
	return value(m_data->journal_id);
}

unique_ptr<SQLStatement>
create_date_ordered_actual_ordinary_entry_selector
(	PhatbooksDatabaseConnection& p_database_connection,
	optional<gregorian::date> const& p_maybe_min_date,
	optional<gregorian::date> const& p_maybe_max_date,
	optional<AccountHandle> const& p_maybe_account
)
{
	// TODO Factor out duplicated code between here and
	// "entry_table_iterator.cpp".
#	ifndef NDEBUG
		// Ensure we are picking all and only the
		// actual transactions.
		int const target_non_actual_type = 3;
		int i = 0;
		int const lim =
			static_cast<int>(TransactionType::num_transaction_types);
		for ( ; i != lim; ++i)
		{
			TransactionType const ttype = static_cast<TransactionType>(i);
			if (ttype == static_cast<TransactionType>(target_non_actual_type))
			{
				JEWEL_ASSERT (!transaction_type_is_actual(ttype));
			}
			else
			{
				JEWEL_ASSERT (transaction_type_is_actual(ttype));
			}
		}
#	endif

	string text =
		"select entry_id from entries join ordinary_journal_detail "
		"using(journal_id) join journals using(journal_id) where "
		"transaction_type_id != 3";
	if (p_maybe_min_date) text += " and date >= :min_date";
	if (p_maybe_max_date) text += " and date <= :max_date";
	if (p_maybe_account) text += " and account_id = :account_id";
	unique_ptr<SQLStatement> ret
	(	new SQLStatement(p_database_connection, text)
	);
	if (p_maybe_min_date)
	{
		ret->bind(":min_date", julian_int(*p_maybe_min_date));
	}
	if (p_maybe_max_date)
	{
		ret->bind(":max_date", julian_int(*p_maybe_max_date));
	}
	if (p_maybe_account && (*p_maybe_account)->has_id())
	{
		ret->bind(":account_id", (*p_maybe_account)->id());
	}
	return move(ret);
}

}  // namespace phatbooks
