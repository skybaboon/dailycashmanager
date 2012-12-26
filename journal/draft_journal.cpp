#include "draft_journal.hpp"
#include "draft_journal_impl.hpp"
#include "draft_journal_reader.hpp"
#include "entry.hpp"
#include "journal.hpp"
#include "phatbooks_database_connection.hpp"
#include <jewel/output_aux.hpp>
#include <sqloxx/handle.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <ostream>
#include <string>

using boost::shared_ptr;
using jewel::output_aux;
using std::endl;
using std::ostream;
using std::string;
using std::vector;
using sqloxx::Handle;

namespace phatbooks
{

void
DraftJournal::setup_tables(PhatbooksDatabaseConnection& dbc)
{
	DraftJournalImpl::setup_tables(dbc);
	return;
}

DraftJournal::DraftJournal
(	PhatbooksDatabaseConnection& p_database_connection
):
	m_impl(Handle<DraftJournalImpl>(p_database_connection))
{
}

DraftJournal::DraftJournal
(	PhatbooksDatabaseConnection& p_database_connection,
	Id p_id
):
	m_impl(Handle<DraftJournalImpl>(p_database_connection, p_id))
{
}


DraftJournal
DraftJournal::create_unchecked
(	PhatbooksDatabaseConnection& p_database_connection,
	Id p_id
)
{
	return DraftJournal
	(	Handle<DraftJournalImpl>::create_unchecked
		(	p_database_connection,
			p_id
		)
	);
}


// TODO There is similar code in OrdinaryJournal for this constructor.
// Factor out.
DraftJournal::DraftJournal
(	Journal& p_journal,
	PhatbooksDatabaseConnection& p_database_connection
):
	m_impl(Handle<DraftJournalImpl>(p_database_connection))
{
	m_impl->set_whether_actual(p_journal.is_actual());
	m_impl->set_comment(p_journal.comment());
	for
	(	vector<Entry>::const_iterator it = p_journal.entries().begin();
		it != p_journal.entries().end();
		++it
	)
	{
		Entry entry(*it);
		m_impl->add_entry(entry);
	}
}


void
DraftJournal::set_whether_actual(bool p_is_actual)
{
	m_impl->set_whether_actual(p_is_actual);
	return;
}

void
DraftJournal::set_comment(string const& p_comment)
{
	m_impl->set_comment(p_comment);
	return;
}

void
DraftJournal::set_name(string const& p_name)
{
	m_impl->set_name(p_name);
	return;
}

void
DraftJournal::add_entry(Entry& entry)
{
	m_impl->add_entry(entry);
	return;
}

void
DraftJournal::add_repeater(Repeater& repeater)
{
	m_impl->add_repeater(repeater);
	return;
}

bool
DraftJournal::is_actual() const
{
	return m_impl->is_actual();
}

string
DraftJournal::comment() const
{
	return m_impl->comment();
}

bool
DraftJournal::is_balanced() const
{
	return m_impl->is_balanced();
}
	
string
DraftJournal::name() const
{
	return m_impl->name();
}

vector<Entry> const&
DraftJournal::entries() const
{
	return m_impl->entries();
}

void
DraftJournal::save()
{
	m_impl->save();
	return;
}

DraftJournal::Id
DraftJournal::id() const
{
	return m_impl->id();
}

string
DraftJournal::repeater_description() const
{
	return m_impl->repeater_description();
}

DraftJournal::DraftJournal
(	sqloxx::Handle<DraftJournalImpl> const& p_handle
):
	m_impl(p_handle)
{
}


namespace
{
	void output_draft_journal_aux(ostream& os, DraftJournal const& dj)
	{
		os << "JOURNAL NAME " << dj.name() << " ";
		output_journal_aux(os, dj);
		os << endl << dj.repeater_description() << endl;
		return;
	}
}  // End anonymous namespace




ostream&
operator<<(ostream& os, DraftJournal const& draft_journal)
{
	output_aux(os, draft_journal, output_draft_journal_aux);
	return os;
}


}  // namespace phatbooks
