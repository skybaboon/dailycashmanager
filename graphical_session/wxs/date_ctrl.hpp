#ifndef GUARD_date_ctrl_hpp
#define GUARD_date_ctrl_hpp

#include "date.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/textctrl.h>
#include <wx/window.h>

namespace phatbooks
{
namespace gui
{

/**
 * Widget with which the user may select a date.
 */
class DateCtrl: public wxTextCtrl
{
public:
	DateCtrl
	(	wxWindow* p_parent,
		unsigned int p_id,
		wxSize const& p_size,
		boost::gregorian::date const& p_date = today()
	);

	/**
	 * @returns a date from the control.
	 *
	 * @throws phatbooks::InvalidDateException if the text in the
	 * control cannot be converted to a valid date.
	 */
	boost::gregorian::date date() const;

private:
	void on_kill_focus(wxFocusEvent& event);
	boost::gregorian::date m_date;
	DECLARE_EVENT_TABLE()

};  // class DateCtrl

}  // namespace gui
}  // namespace phatbooks

#endif  // GUARD_date_ctrl_hpp
