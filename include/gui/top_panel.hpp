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

#ifndef GUARD_top_panel_hpp_7915960996372607
#define GUARD_top_panel_hpp_7915960996372607

#include "account_type.hpp"
#include "reconciliation_list_panel.hpp"
#include "sizing.hpp"
#include "transaction_ctrl.hpp"
#include <jewel/assert.hpp>
#include <sqloxx/handle_fwd.hpp>
#include <sqloxx/id.hpp>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <type_traits>
#include <vector>

namespace dcm
{

// Begin forward declarations

class Account;
class DraftJournal;
class Entry;
class OrdinaryJournal;
class DcmDatabaseConnection;
class ProtoJournal;

namespace gui
{

class AccountListCtrl;
class DraftJournalListCtrl;
class EntryListPanel;
class Frame;
class ReportPanel;
class TransactionCtrl;

// End forward declarations

/**
 * Top level panel intended as immediate child of Frame.
 *
 * @todo LOW PRIORITY The various "update_for_..." functions each contain calls
 * to analogous "update_for_..." functions for each of the sub-widgets
 * in TopPanel. This makes for repetitive code. We could maybe streamline
 * this and make it more maintainable either by using wxWidgets' event
 * system, or else by having an abstract base class for the various
 * sub-widget classes, and then storing pointers to these in a vector
 * in TopPanel, and running through the vector calling "update_for_..." on
 * each.
 *
 * @todo MEDIUM PRIORITY Put some space between the two AccountListCtrls, or do
 * some other thing, to clearly indicate that these are conceptually separate
 * controls, and that one is not, say, a subset of the other. Put a similar kind
 * of "divider" between the envelope list and the TransactionCtrl to the right.
 */
class TopPanel: public wxPanel
{
public:

    TopPanel
    (   Frame* parent,
        DcmDatabaseConnection& p_database_connection
    );

    TopPanel(TopPanel const&) = delete;
    TopPanel(TopPanel&&) = delete;
    TopPanel& operator=(TopPanel const&) = delete;
    TopPanel& operator=(TopPanel&&) = delete;
    ~TopPanel() = default;

    /**
     * @returns a vector populated with handles to all the balance sheet
     * Accounts currently selected by the user in the main window.
     */
    std::vector<sqloxx::Handle<Account> >
    selected_balance_sheet_accounts() const;

    /**
     * @returns a vector populated with handles to all the P&L Accounts
     * currently selected by the user in the main window.
     */
    std::vector<sqloxx::Handle<Account> >
    selected_pl_accounts() const;

    /**
     * @returns a vector populated with all the OrdinaryJournals currently
     * selected by the user in the main window.
     */
    std::vector<sqloxx::Handle<OrdinaryJournal> >
    selected_ordinary_journals() const;

    /**
     * @returns a vector populated with all the DraftJournals currently
     * selected by the user in the main window.
     */
    std::vector<sqloxx::Handle<DraftJournal> >
    selected_draft_journals() const;

    /**
     * Update the display to reflect current state of database, after
     * saving of p_saved_object, where p_saved_object is a newly saved
     * object that was not already in the database.
     */
    void update_for_new
    (   sqloxx::Handle<DraftJournal> const& p_saved_object
    );
    void update_for_new
    (   sqloxx::Handle<OrdinaryJournal> const& p_saved_object
    );
    void update_for_new(sqloxx::Handle<Account> const& p_saved_object);

    /**
     * Update the display to reflect current state of database, after
     * saving of p_saved_object, where p_saved_object already existed
     * in the database, but has just been amended and the amendments
     * saved.
     *
     * NOTE these do not provide any information to TopPanel about any
     * Entries that have been deleted from a DraftJournal or
     * OrdinaryJournal represented by \e p_saved_object. Information
     * about deleted Entries should be provided to TopPanel separately
     * via update_for_deleted_ordinary_entries(...) or
     * update_for_deleted_draft_entries(...) (as appropriate) (which
     * may in turn be done via appropriate PersistentObjectEvent(s) rather
     * than by calling these functions directly). It is better to notify
     * re. the deleted Entries prior to notifying re. the (other) Journal
     * edits.
     */
    void update_for_amended(sqloxx::Handle<DraftJournal> const& p_saved_object);
    void update_for_amended
    (   sqloxx::Handle<OrdinaryJournal> const& p_saved_object
    );
    void update_for_amended(sqloxx::Handle<Account> const& p_saved_object);

    /**
     * Update the display to reflect that the object with
     * p_doomed_id (or the objects with p_doomed_ids) have just
     * been removed from the database.
     */
    void update_for_deleted_ordinary_journal(sqloxx::Id p_doomed_id);
    void update_for_deleted_draft_journal(sqloxx::Id p_doomed_id);
    void update_for_deleted_ordinary_entries
    (   std::vector<sqloxx::Id> const& p_doomed_ids
    );
    void update_for_deleted_draft_entries
    (   std::vector<sqloxx::Id> const& p_doomed_ids
    );

    /**
     * Update the display to reflect current state of database, after
     * BudgetItems pertaining to p_account have been added, deleted
     * and/or amended.
     * 
     * Precondition: p_account should be of AccountSuperType::pl.
     */
    void update_for_amended_budget(sqloxx::Handle<Account> const& p_account);

    /**
     * Update the display to reflect the current state of \e p_entry
     * with respect just to whether it is reconciled.
     *
     * NOTE This is messy and "coupled", but: this intentionally does
     * \e not update the
     * ReconciliationListPanel / ReconciliationEntryListCtrl, as it
     * is assumed these are the \e source of the change - we don't update
     * these \e again, on pain of circularity.
     *
     * @todo LOW PRIORITY Make this less messy and "coupled" (see note
     * above).
     */
    void update_for_reconciliation_status(sqloxx::Handle<Entry> const& p_entry);

    /**
     * @returns a ProtoJournal containing two Entries, with blank
     * comments, and with Accounts based either on the
     * most commonly used Accounts in the database, or on Accounts
     * currently selected by the user in the EntryListCtrls.
     */
    ProtoJournal make_proto_journal() const;

    /**
     * @returns a ProtoJournal containing two Entries, with blank
     * comments, and with Accounts already set to some default P&L
     * Account (client should not rely on any particular Account here).
     * The ProtoJournal will be of TransactionType::envelope.
     */
    ProtoJournal make_proto_envelope_transfer() const;

    /**
     * Configure the TransactionCtrl to reflect the currently selected
     * Accounts (if any).
     */
    void configure_transaction_ctrl();

    /**
     * Configure the TransactionCtrl to reflect an existing OrdinaryJournal
     * or DraftJournal.
     * \e JournalType should be either sqloxx::Handle<OrdinaryJournal> or
     * sqloxx::Handle<DraftJournal>.
     * Compilation will break if this is not the case.
     */
    template <typename JournalType>
    void configure_transaction_ctrl(JournalType const& p_journal);

    void configure_draft_journal_list_ctrl();

    /**
     * @returns \e true if and only if we will now be showing
     * hidden Accounts of AccountSuperType \e p_account_super_type.
     */
    bool toggle_show_hidden_accounts
    (   AccountSuperType p_account_super_type
    );

private:

    void configure_account_lists();
    void configure_entry_list();
    void configure_reconciliation_page();
    void configure_report_page();

    DcmDatabaseConnection& m_database_connection;
    wxBoxSizer* m_top_sizer;
    wxNotebook* m_notebook;
    wxPanel* m_notebook_page_accounts;
    wxPanel* m_notebook_page_transactions;
    wxPanel* m_notebook_page_reconciliations;
    wxPanel* m_notebook_page_reports;
    wxBoxSizer* m_right_column_sizer;
    AccountListCtrl* m_bs_account_list;
    AccountListCtrl* m_pl_account_list;
    EntryListPanel* m_entry_list_panel;
    ReconciliationListPanel* m_reconciliation_panel;
    ReportPanel* m_report_panel;
    TransactionCtrl* m_transaction_ctrl;
    DraftJournalListCtrl* m_draft_journal_list;
};


// IMPLEMENT MEMBER FUNCTION TEMPLATE

template <typename JournalType>
void
TopPanel::configure_transaction_ctrl(JournalType const& p_journal)
{
    static_assert
    (   std::is_same<JournalType, sqloxx::Handle<DraftJournal> >::value ||
        std::is_same<JournalType, sqloxx::Handle<OrdinaryJournal> >::value,
        "Top::Panel::configure_transaction_ctrl was passed a type other than "
        "sqloxx::Handle<DraftJournal> or sqloxx::Handle<OrdinaryJournal>."
    );
    TransactionCtrl* old = nullptr;
    JEWEL_ASSERT (m_right_column_sizer);
    if (m_transaction_ctrl)
    {
        m_right_column_sizer->Detach(m_transaction_ctrl);
        old = m_transaction_ctrl;
    }
    m_transaction_ctrl = new TransactionCtrl
    (   this,
        wxSize(GetClientSize().x, 10000),
        p_journal
    );
    m_right_column_sizer->Insert
    (   0,
        m_transaction_ctrl,
        wxSizerFlags(6).Expand().
            Border(wxNORTH | wxSOUTH | wxWEST | wxEAST, standard_border() * 2)
    );
    if (old)
    {
        old->Destroy();
        old = nullptr;
    }
    Layout();
    JEWEL_ASSERT (m_transaction_ctrl);
    // m_transaction_ctrl->SetFocus();  // This doesn't seem to have any effect
    return;
}


}  // namespace gui
}  // namespace dcm


#endif  // GUARD_top_panel_hpp_7915960996372607
