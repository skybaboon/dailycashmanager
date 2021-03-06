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

#include "gui/date_ctrl.hpp"
#include "date.hpp"
#include "dcm_exceptions.hpp"
#include "gui/date_validator.hpp"
#include "gui/locale.hpp"
#include "gui/text_ctrl.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>
#include <jewel/assert.hpp>
#include <jewel/optional.hpp>
#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/window.h>

using boost::optional;
using jewel::value;
namespace gregorian = boost::gregorian;

namespace dcm
{
namespace gui
{

BEGIN_EVENT_TABLE(DateCtrl, TextCtrl)
    EVT_KILL_FOCUS(DateCtrl::on_kill_focus)
    EVT_SET_FOCUS(DateCtrl::on_set_focus)
END_EVENT_TABLE()

DateCtrl::DateCtrl
(   wxWindow* p_parent,
    unsigned int p_id,
    wxSize const& p_size,
    gregorian::date const& p_date,
    bool p_allow_blank,
    optional<gregorian::date> const& p_min_date
):
    TextCtrl
    (   p_parent,
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
    JEWEL_ASSERT (validator);
    return validator->date();
}

void
DateCtrl::on_kill_focus(wxFocusEvent& event)
{
    event.Skip();
    auto const orig = date();
    auto* const validator = GetValidator();
    JEWEL_ASSERT (validator);
    if
    (   !validator->Validate(static_cast<wxWindow*>(this)) ||
        !validator->TransferToWindow()
    )
    {
        if (orig)
        {
            SetValue(date_format_wx(value(orig)));
            validator->Validate(static_cast<wxWindow*>(this));
        }
    }
    return;
}

void
DateCtrl::on_set_focus(wxFocusEvent& event)
{
    event.Skip();
    SelectAll();
    return;
}

}  // namespace gui
}  // namespace dcm
