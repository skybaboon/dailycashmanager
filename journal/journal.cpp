#include "b_string.hpp"
#include "journal.hpp"
#include "entry.hpp"
#include <consolixx/table.hpp>
#include <jewel/debug_log.hpp>
#include <jewel/decimal.hpp>
#include <jewel/output_aux.hpp>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

using consolixx::Table;
using jewel::Decimal;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

namespace phatbooks
{

Journal::~Journal()
{
}

void
Journal::set_whether_actual(bool p_is_actual)
{
	do_set_whether_actual(p_is_actual);
	return;
}

void
Journal::set_comment(BString const& p_comment)
{
	do_set_comment(p_comment);
	return;
}

void
Journal::push_entry(Entry& entry)
{
	do_push_entry(entry);
	return;
}

void
Journal::remove_entry(Entry& entry)
{
	do_remove_entry(entry);
	return;
}

std::vector<Entry> const&
Journal::entries() const
{
	return do_get_entries();
}

BString
Journal::comment() const
{
	return do_get_comment();
}

bool
Journal::is_actual() const
{
	return do_get_whether_actual();
}

Decimal
Journal::balance() const
{
	typedef vector<Entry>::const_iterator Iter;
	Decimal ret(0, 0);
	for (Iter it = entries().begin(), end = entries().end(); it != end; ++it)
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
Journal::do_output(ostream& os) const
{
	namespace alignment	= consolixx::alignment;
	if (is_actual()) os << "ACTUAL TRANSACTION";
	else os << "BUDGET TRANSACTION";
	os << endl;
	if (!comment().empty()) os << comment() << endl;
	os << endl;
	Table<Entry> table;
	table.push_column(Entry::create_id_column());
	table.push_column(Entry::create_account_name_column());
	table.push_column(Entry::create_comment_column());
#	ifdef PHATBOOKS_EXPOSE_COMMODITY
		table.push_column(Entry::create_commodity_abbreviation_column());
#	endif
	bool const change_signs = !is_actual();
	table.push_column
	(	change_signs?
		Entry::create_amount_column():
		Entry::create_reversed_amount_column()
	);
	table.populate(entries().begin(), entries().end());
	os << table;
	return;
}

void
Journal::output_journal_aux(ostream& os, Journal const& journal)
{
	journal.do_output(os);
	return;
}


ostream&
operator<<(ostream& os, Journal const& journal)
{
	jewel::output_aux(os, journal, Journal::output_journal_aux);
	return os;
}



}  // namespace phatbooks
