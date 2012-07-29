#include "journal.hpp"
#include "date.hpp"
#include "entry.hpp"
#include "general_typedefs.hpp"
#include "entry.hpp"
#include "repeater.hpp"
#include <list>

using std::list;

namespace phatbooks
{

Journal::Journal(bool p_is_actual):
	m_is_actual(p_is_actual),
	m_date((null_date())),
	m_comment(""),
	m_entry_list((list<Entry>())),
	m_repeater_list((list<Repeater>()))
{
}


	







}  // namespace phatbooks
