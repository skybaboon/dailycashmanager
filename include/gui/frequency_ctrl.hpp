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

#ifndef GUARD_frequency_ctrl_hpp_5481597355325519
#define GUARD_frequency_ctrl_hpp_5481597355325519

#include "frequency.hpp"
#include "gui/combo_box.hpp"
#include <boost/optional.hpp>
#include <wx/gdicmn.h>
#include <wx/window.h>
#include <wx/windowid.h>
#include <vector>

namespace dcm
{

// Begin forward declarations

class DcmDatabaseConnection;

namespace gui
{

// End forward declarations

/**
 * Widget for the the user to select a Frequency (for a DraftJournal), or
 * else to select no Frequency (for an OrdinaryJournal).
 */
class FrequencyCtrl: public ComboBox
{
public:

    /**
     * @param p_parent parent window
     *
     * @param p_id id
     *
     * @param p_size size
     *
     * @param p_database_connection connection to database
     *
     * @param p_supports_ordinary_journal pass \e true to support
     * "Frequencies" that are  to an OrdinaryJournal (as opposed to
     * DraftJournal). The only such Frequency is "once-off" (which is
     * not actually represented by a Frequency at all).
     *
     * @param p_supports_draft_journal pass \e true to support Frequencies
     * that are required for repeating DraftJournals (i.e. DraftJournals
     * with at least one Repeater).
     *
     * If both p_supports_ordinary_journal and p_supports_draft_journal are
     * passed e\ false, then the FrequencyCtrl will display text appropriate
     * to selected a Frequency for the purpose of a BudgetItem (as opposed
     * to a Journal).
     */
    FrequencyCtrl
    (   wxWindow* p_parent,
        wxWindowID p_id,
        wxSize const& p_size,
        DcmDatabaseConnection& p_database_connection,
        bool p_supports_ordinary_journal = false,
        bool p_supports_draft_journal = false
    );

    FrequencyCtrl(FrequencyCtrl const&) = delete;
    FrequencyCtrl(FrequencyCtrl&&) = delete;
    FrequencyCtrl& operator=(FrequencyCtrl const&) = delete;
    FrequencyCtrl& operator=(FrequencyCtrl&&) = delete;
    virtual ~FrequencyCtrl();

    /**
     * If no Frequency is selected (i.e. "Once off" or equivalent is selected)
     * then this returns an uninitialized boost::optional<Frequency>.
     * If a Frequency is selected, then this returns a
     * boost::optional<Frequency> that has been initialized with that
     * Frequency.
     */
    boost::optional<Frequency> frequency() const;

    /**
     * If p_maybe_frequency is passed an uninitialized optional, then
     * the FrequencyCtrl will display "Once off" or equivalent, or else
     * will throw InvalidFrequencyException if OrdinaryJournals are not
     * supported.
     *
     * If p_maybe_frequency is passed an initialized optional, then
     * the FrequencyCtrl will display text describing the Frequency with
     * which it has been initialized, or else will throw
     * InvalidFrequencyException if DraftJournals are not supported.
     */
    void set_frequency(boost::optional<Frequency> const& p_maybe_frequency);
    
private:
    bool supports_ordinary_journal() const;
    bool supports_draft_journal() const;
    bool supports_budget_item() const;

    std::vector<Frequency> m_frequencies;

    DcmDatabaseConnection& m_database_connection;
    bool const m_supports_ordinary_journal;
    bool const m_supports_draft_journal;

};  // class FrequencyCtrl




}  // namespace gui
}  // namespace dcm

#endif  // GUARD_frequency_ctrl_hpp_5481597355325519
