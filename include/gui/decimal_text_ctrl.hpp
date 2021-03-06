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

#ifndef GUARD_decimal_text_ctrl_hpp_19653581837030865
#define GUARD_decimal_text_ctrl_hpp_19653581837030865

#include "gui/text_ctrl.hpp"
#include <jewel/decimal.hpp>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/window.h>

namespace dcm
{
namespace gui
{

/**
 * A subclass of TextCtrl specifically for the entry and validation
 * of wxString representations of jewel::Decimal.
 */
class DecimalTextCtrl: public TextCtrl
{
public:
    DecimalTextCtrl
    (   wxWindow* p_parent,
        unsigned int p_id,
        wxSize const& p_size,
        jewel::Decimal::places_type p_precision,
        bool p_print_dash_for_zero = true
    );

    DecimalTextCtrl(DecimalTextCtrl const&) = delete;
    DecimalTextCtrl(DecimalTextCtrl&&) = delete;
    DecimalTextCtrl& operator=(DecimalTextCtrl const&) = delete;
    DecimalTextCtrl& operator=(DecimalTextCtrl&&) = delete;
    virtual ~DecimalTextCtrl();

    /**
     * NOTE As a side-effect of calling this function, the
     * \e parent window of the DecimalTextCtrl will have
     * Validate() called on it.
     * This is an unfortunate workaround for odd behaviour whereby
     * wxWidgets does not seem to support these functions being
     * called directly on the DecimalTextCtrl itself.
     */
    void set_amount(jewel::Decimal const& p_amount);

    jewel::Decimal amount();

private:
    virtual void on_kill_focus(wxFocusEvent& event);
    virtual void on_set_focus(wxFocusEvent& event);
    bool m_print_dash_for_zero;
    jewel::Decimal::places_type m_precision;
    DECLARE_EVENT_TABLE()

};  // class DecimalTextCtrl

}  // namespace gui
}  // namespace dcm

#endif  // GUARD_decimal_text_ctrl_hpp_19653581837030865
