#include "decimal_text_ctrl.hpp"
#include "decimal_validator.hpp"
#include <jewel/debug_log.hpp>
#include <jewel/decimal.hpp>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/textctrl.h>
#include <wx/window.h>
#include <iostream>

using jewel::Decimal;
using std::endl;

namespace phatbooks
{
namespace gui
{


BEGIN_EVENT_TABLE(DecimalTextCtrl, wxTextCtrl)
	EVT_KILL_FOCUS(DecimalTextCtrl::on_kill_focus)
END_EVENT_TABLE()


DecimalTextCtrl::DecimalTextCtrl
(	wxWindow* p_parent,
	unsigned int p_id,
	wxSize const& p_size,
	Decimal::places_type p_precision
):
	wxTextCtrl
	(	p_parent,
		p_id,
		wxEmptyString,
		wxDefaultPosition,
		p_size,
		wxALIGN_RIGHT,
		DecimalValidator(Decimal(0, p_precision), p_precision)
	),
	m_precision(p_precision)
{
}

void
DecimalTextCtrl::on_kill_focus(wxFocusEvent& event)
{
	// Unfortunately if we call Validate() and TransferDataToWindow()
	// directly on the DecimalTextCtrl, it doesn't work. We have to call
	// through parent instead.
	GetParent()->Validate();
	GetParent()->TransferDataToWindow();
	event.Skip();
}

}  // namespace gui
}  // namespace phatbooks