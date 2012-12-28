
/** \file journal.cpp
 *
 * \brief Source file relating to Journal class.
 *
 * \author Matthew Harvey
 * \date 04 July 2012.
 *
 * Copyright (c) 2012, Matthew Harvey. All rights reserved.
 */



#include "journal.hpp"
#include "account.hpp"
#include "commodity.hpp"
#include "entry.hpp"
#include "phatbooks_database_connection.hpp"
#include "consolixx/table.hpp"
#include <jewel/output_aux.hpp>
#include <sqloxx/next_auto_key.hpp>
#include <sqloxx/sql_statement.hpp>
#include <jewel/decimal.hpp>
#include <jewel/optional.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace alignment = consolixx::alignment;

using sqloxx::next_auto_key;
using sqloxx::SQLStatement;
using boost::numeric_cast;
using boost::scoped_ptr;
using boost::shared_ptr;
using boost::unordered_set;
using consolixx::Table;
using jewel::clear;
using jewel::Decimal;
using jewel::output_aux;
using jewel::value;
using std::ios_base;
using std::logic_error;
using std::ostream;
using std::ostringstream;
using std::runtime_error;
using std::string;
using std::vector;

#include <jewel/debug_log.hpp>
#include <iostream>
using std::endl;


namespace phatbooks
{

void
Journal::setup_tables(PhatbooksDatabaseConnection& dbc)
{
	dbc.execute_sql
	(	"create table journals"
		"("
			"journal_id integer primary key autoincrement, "
			"is_actual integer not null references booleans, "
			"comment text"
		");"
	);
	return;
}


Journal::Journal():
	m_data(new JournalData)
{
}

Journal::Journal(Journal const& rhs):
	m_data(new JournalData(*(rhs.m_data)))
{
}

Journal::~Journal()
{
	/* If m_data is a smart pointer, this is not required.
	delete m_data;
	m_data = 0;
	*/
}

vector<Entry> const& 
Journal::entries() const
{
	return m_data->entries;
}

void
Journal::set_whether_actual(bool p_is_actual)
{
	m_data->is_actual = p_is_actual;
	return;
}

void
Journal::set_comment(string const& p_comment)
{
	m_data->comment = p_comment;
	return;
}

void
Journal::add_entry(Entry& entry)
{
	/*
	JEWEL_DEBUG_LOG << "Calling Journal::add_entry " << endl;
	*/
	m_data->entries.push_back(entry);
	return;
}

string
Journal::comment() const
{
	return value(m_data->comment);
}

bool
Journal::is_actual() const
{
	return value(m_data->is_actual);
}

Decimal
Journal::balance() const
{
	Decimal ret(0, 0);
	for
	(	vector<Entry>::const_iterator it = entries().begin(),
			end = entries().end();
		it != end;
		++it
	)
	{
		ret += it->amount();
	}
	return ret;
}

bool
Journal::is_balanced() const
{
	return balance() == Decimal(0, 0);
}

void
Journal::swap(Journal& rhs)
{
	using std::swap;
	swap(m_data, rhs.m_data);
	return;
}

Journal::Id
Journal::do_save_new_journal_base
(	PhatbooksDatabaseConnection& dbc
)
{
	Id const journal_id = next_auto_key
	<	PhatbooksDatabaseConnection,
		Id
	>	(dbc, "journals");
	SQLStatement statement
	(	dbc,
		"insert into journals(is_actual, comment) "
		"values(:is_actual, :comment)"
	);
	statement.bind(":is_actual", static_cast<int>(value(m_data->is_actual)));
	statement.bind(":comment", value(m_data->comment));
	statement.step_final();
	typedef vector<Entry>::iterator EntryIter;
	EntryIter const endpoint = m_data->entries.end();
	for (EntryIter it = m_data->entries.begin(); it != endpoint; ++it)
	{
		it->set_journal_id(journal_id);
		it->save();
	}
	return journal_id;
}

void
Journal::do_save_existing_journal_base
(	PhatbooksDatabaseConnection& dbc,
	Journal::Id id
)
{
	SQLStatement updater
	(	dbc,
		"update journals set is_actual = :is_actual, comment = :comment "
		"where journal_id = :id"
	);
	updater.bind(":is_actual", static_cast<int>(value(m_data->is_actual)));
	updater.bind(":comment", value(m_data->comment));
	updater.bind(":id", id);
	updater.step_final();
	typedef vector<Entry>::iterator EntryIter;
	EntryIter const endpoint = m_data->entries.end();
	unordered_set<Entry::Id> saved_entry_ids;
	for (EntryIter it = m_data->entries.begin(); it != endpoint; ++it)
	{
		it->save();
		saved_entry_ids.insert(it->id());
	}
	// Remove any entries in the database with this journal's journal_id, that
	// no longer exist in the in-memory journal
	SQLStatement entry_finder
	(	dbc,	
		"select entry_id from entries where journal_id = :journal_id"
	);
	entry_finder.bind(":journal_id", id);
	unordered_set<Entry::Id>::const_iterator const saved_entries_end =
		saved_entry_ids.end();
	while (entry_finder.step())
	{
		Entry::Id const entry_id = entry_finder.extract<Entry::Id>(0);
		if (saved_entry_ids.find(entry_id) == saved_entries_end)
		{
			Entry doomed_entry(dbc, entry_id);
			// This entry is in the database but no longer in the in-memory
			// journal, so should be deleted.
			doomed_entry.remove();
			// Note it's OK even if the last entry is deleted. Another
			// entry will never be reassigned its id - SQLite makes sure
			// of that - providing we let SQLite assign all the ids
			// automatically.
		}
	}
	return;
}


void
Journal::do_load_journal_base
(	PhatbooksDatabaseConnection& dbc,
	Journal::Id id
)
{
	SQLStatement statement
	(	dbc,
		"select is_actual, comment from journals where journal_id = :p"
	);
	statement.bind(":p", id);
	statement.step();
	Journal temp(*this);
	SQLStatement entry_finder
	(	dbc,	
		"select entry_id from entries where journal_id = :jid"
	);
	entry_finder.bind(":jid", id);
	while (entry_finder.step())
	{
		Entry::Id const entr_id = entry_finder.extract<Entry::Id>(0);
		Entry entry(dbc, entr_id);
		temp.m_data->entries.push_back(entry);
	}
	temp.m_data->is_actual = static_cast<bool>(statement.extract<int>(0));
	temp.m_data->comment = statement.extract<string>(1);
	swap(temp);	
	return;
}

void
Journal::do_ghostify_journal_base()
{
	clear(m_data->is_actual);
	clear(m_data->comment);
	typedef vector<Entry>::iterator EntryIter;
	EntryIter endpoint = m_data->entries.end();
	for (EntryIter it = m_data->entries.begin(); it != endpoint; ++it)
	{
		it->ghostify();
	}
	m_data->entries.clear();
	return;
}

string
Journal::primary_table_name()
{
	return "journals";
}

string
Journal::primary_key_name()
{
	return "journal_id";
}

void
Journal::clear_entries()
{
	(m_data->entries).clear();
	return;
}


Journal::Id
max_journal_id(PhatbooksDatabaseConnection& dbc)
{
	SQLStatement s(dbc, "select max(journal_id) from journals");
	s.step();
	return s.extract<Journal::Id>(0);
}

Journal::Id
min_journal_id(PhatbooksDatabaseConnection& dbc)
{
	SQLStatement s(dbc, "select min(journal_id) from journals");
	s.step();
	return s.extract<Journal::Id>(0);
}

bool
journal_id_exists(PhatbooksDatabaseConnection& dbc, Journal::Id id)
{
	SQLStatement s
	(	dbc,
		"select journal_id from journals where journal_id = :p"
	);
	s.bind(":p", id);
	return s.step();
}

bool
journal_id_is_draft(PhatbooksDatabaseConnection& dbc, Journal::Id id)
{
	if (!journal_id_exists(dbc, id))
	{
		throw std::runtime_error("Journal with id does not exist.");
	}
	SQLStatement s
	(	dbc,
		"select journal_id from draft_journal_detail where "
		"journal_id = :p"
	);
	s.bind(":p", id);
	return s.step();
}
	

ostream&
operator<<(ostream& os, Journal const& journal)
{
	output_aux(os, journal, output_journal_aux);
	return os;
}
		


}  // namespace phatbooks
