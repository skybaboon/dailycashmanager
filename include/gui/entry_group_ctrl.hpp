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

#ifndef GUARD_entry_group_ctrl_hpp_08374422745953511
#define GUARD_entry_group_ctrl_hpp_08374422745953511

#include "account_type.hpp"
#include "decimal_text_ctrl.hpp"
#include "entry.hpp"
#include "transaction_side.hpp"
#include "transaction_type.hpp"
#include <boost/optional.hpp>
#include <jewel/decimal.hpp>
#include <sqloxx/handle.hpp>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <memory>
#include <vector>

namespace dcm
{

// Begin forward declarations

class Account;
class Journal;
class DcmDatabaseConnection;

namespace gui
{

class AccountCtrl;
class Button;
class TextCtrl;
class TransactionCtrl;

// End forward declarations

/**
 * Widget for entering details regarding a single "side" of a transaction.
 * This could comprise multiple Entries.
 * 
 * @see TransactionSide
 */
class EntryGroupCtrl: public wxPanel
{
public:

    /**
     * Precondition: p_journal should have all the basic attributes of a
     * ProtoJournal initialized. Also all the
     * Entries in p_journal must be have all their attributes initialized,
     * except they may or may not have an ID.
     */
    EntryGroupCtrl
    (   TransactionCtrl* p_parent,
        wxSize const& p_text_ctrl_size,
        Journal& p_journal,
        TransactionSide p_transaction_side,
        DcmDatabaseConnection& p_database_connection
    );

    EntryGroupCtrl(EntryGroupCtrl const&) = delete;
    EntryGroupCtrl(EntryGroupCtrl&&) = delete;
    EntryGroupCtrl& operator=(EntryGroupCtrl const&) = delete;
    EntryGroupCtrl& operator=(EntryGroupCtrl&&) = delete;
    
    ~EntryGroupCtrl() = default;

    void refresh_for_transaction_type
    (   TransactionType p_transaction_type
    );

    jewel::Decimal primary_amount() const;

    jewel::Decimal total_amount() const;

    /**
     * @returns a vector of handles to newly created Entries (WITHOUT ids),
     * based on the data currently in the EntryGroupCtrl. The Entries will not
     * be associated with any Journal.
     */
    std::vector<sqloxx::Handle<Entry> > make_entries() const;

    bool is_all_zero() const;

    void update_for_new(sqloxx::Handle<Account> const& p_saved_object);

    void update_for_amended(sqloxx::Handle<Account> const& p_saved_object);

    /**
     * Go through each row. If that row corresponds to an entry which
     * is reconciled, then disable it and set a tooltip explaining why it
     * is disabled. Otherwise enable it and remove any tooltip. Returns
     * \e true if at least one Entry is reconciled; otherwise, returns
     * \e false.
     */
    bool reflect_reconciliation_statuses();

private:

    class EntryDecimalTextCtrl: public DecimalTextCtrl
    {
    public:
        EntryDecimalTextCtrl(EntryGroupCtrl* p_parent, wxSize const& p_size);
        virtual ~EntryDecimalTextCtrl();
    private:
        void on_left_double_click(wxMouseEvent& event);
        DECLARE_EVENT_TABLE()
    };

    friend class EntryDecimalTextCtrl;

    void configure_available_account_types();
    void configure_top_row(bool p_include_split_button);
    void pop_row();
    void push_row
    (   sqloxx::Handle<Entry> const& p_entry,
        boost::optional<jewel::Decimal> const& p_previous_row_amount,
        bool p_multiple_entries
    );
    void adjust_layout_for_new_number_of_rows();
    wxString side_description() const;

    void on_unsplit_button_click(wxCommandEvent& event);
    void on_split_button_click(wxCommandEvent& event);

    bool is_source() const;

    /**
     * Adjusts the amount in \e p_target on the basis of
     * such that the EntryGroupCtrl becomes balanced.
     */
    void autobalance(EntryDecimalTextCtrl* p_target);

    size_t num_rows() const;

    DcmDatabaseConnection& m_database_connection;

    static unsigned int const s_unsplit_button_id = wxID_HIGHEST + 1;
    static unsigned int const s_split_button_id = s_unsplit_button_id + 1;

    TransactionSide m_transaction_side;
    TransactionType m_transaction_type;

    wxSize m_text_ctrl_size;

    typedef std::vector<AccountType> ATypeVec;
    std::unique_ptr<ATypeVec> m_available_account_types;

    wxGridBagSizer* m_top_sizer;
    wxStaticText* m_side_descriptor;
    Button* m_unsplit_button;
    Button* m_split_button;

    struct EntryRow
    {
        EntryRow(sqloxx::Handle<Entry> const& p_entry);
        AccountCtrl* account_ctrl;
        TextCtrl* comment_ctrl;
        EntryDecimalTextCtrl* amount_ctrl;
        sqloxx::Handle<Entry> entry;
    };
    friend struct EntryRow;

    std::vector<EntryRow> m_entry_rows;

    size_t m_current_row;

    DECLARE_EVENT_TABLE()

};  // class EntryGroupCtrl


}  // namespace gui
}  // namespace dcm

#endif  // GUARD_entry_group_ctrl_hpp_08374422745953511
