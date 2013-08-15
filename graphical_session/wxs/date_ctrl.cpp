// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "date_ctrl.hpp"
#include "date_validator.hpp"
#include "locale.hpp"
#include "phatbooks_exceptions.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>
#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/window.h>
#include <cassert>

using boost::optional;
namespace gregorian = boost::gregorian;

namespace phatbooks
{
namespace gui
{

BEGIN_EVENT_TABLE(DateCtrl, wxTextCtrl)
	EVT_KILL_FOCUS(DateCtrl::on_kill_focus)
END_EVENT_TABLE()

DateCtrl::DateCtrl
(	wxWindow* p_parent,
	unsigned int p_id,
	wxSize const& p_size,
	gregorian::date const& p_date,
	bool p_allow_blank,
	optional<gregorian::date> const& p_min_date
):
	wxTextCtrl
	(	p_parent,
		p_id,
		date_format_wx(p_date),
		wxDefaultPosition,
		p_size,
		wxALIGN_RIGHT,
		DateValidator(p_date, p_allow_blank, p_min_date)
	)
{
}
	
optional<gregorian::date>
DateCtrl::date()
{
	DateValidator const* const validator =
		dynamic_cast<DateValidator const*>(GetValidator());
	assert (validator);
	return validator->date();
}

void
DateCtrl::on_kill_focus(wxFocusEvent& event)
{
	// Unfortunately if we call Validate() and TransferDataToWindow()
	// directly on the DateCtrl, it doesn't work. We have to call
	// through parent instead.
	GetParent()->Validate();
	GetParent()->TransferDataToWindow();
	event.Skip();
	return;
}

}  // namespace gui
}  // namespace phatbooks
