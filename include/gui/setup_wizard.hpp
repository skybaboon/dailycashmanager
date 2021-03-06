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

#ifndef GUARD_setup_wizard_hpp_8623281646810137
#define GUARD_setup_wizard_hpp_8623281646810137

#include "account_type.hpp"
#include "commodity.hpp"
#include "gui/text_ctrl.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <sqloxx/handle.hpp>
#include <wx/dataview.h>
#include <wx/filedlg.h>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/treelist.h>
#include <wx/validate.h>
#include <wx/window.h>
#include <wx/wizard.h>
#include <memory>
#include <set>
#include <vector>

namespace dcm
{

// Begin forward declarations

struct AugmentedAccount;
class DcmDatabaseConnection;

namespace gui
{

class Button;
class ComboBox;
class DateCtrl;
class Frame;
class MultiAccountPanel;

// End forward declarations


/**
 * Represents the wizard that is presented to GUI users when they first
 * start up the application.
 *
 * @todo MEDIUM PRIORITY In the FilepathPage, the controls for the directory
 * and the filepath look like you can write in them, but actually you can't.
 * This misleads the user and is bad (which is pointed out in the "GUI Bloopers"
 * book). Fix it.
 */
class SetupWizard: public wxWizard
{
public:
    
    /**
     * Precondition: p_database_connection is not valid (that's the whole
     * reason we are calling the wizard...).
     */
    SetupWizard(DcmDatabaseConnection& p_database_connection);

    SetupWizard(SetupWizard const&) = delete;
    SetupWizard(SetupWizard&&) = delete;
    SetupWizard& operator=(SetupWizard const&) = delete;
    SetupWizard& operator=(SetupWizard&&) = delete;
    ~SetupWizard();

    void run();

    /**
     * @returns a standard size to be used for text controls
     * (TextCtrl, ComboBox) in the pages of this wizard.
     * The returned wxSize is intended to be used with wxDLG_UNIT macro
     * when initializing the controls' sizes.
     */
    static wxSize standard_text_box_size();
    
    sqloxx::Handle<Commodity> selected_currency() const;
    boost::gregorian::date selected_start_date() const;
    void set_assumed_currency(sqloxx::Handle<Commodity> const& p_commodity);
    jewel::Decimal total_opening_balance() const;

    /**
     * Set the total "Account names already recorded" to p_names. Used
     * to ensure Account names not reused between different pages.
     */
    void set_account_names_already_taken(std::set<wxString> const& p_names);

    std::set<wxString> const& account_names_already_taken() const;
    
    class AccountPage;

private:

    std::vector<AugmentedAccount> augmented_accounts() const;

    class FilepathValidator;

    class FilepathPage;

    void render_account_pages();

    /**
     * Set the default Commodity for m_database_connection, based on
     * the currency selected by the user.
     *
     * Should be called only after calling RunWizard().
     */
    void configure_default_commodity();

    /**
     * Set the entity creation date for m_database_connection, based on
     * the start date selected by the user.
     *
     * Should be called only after calling RunWizard().
     */
    void configure_entity_creation_date();

    /**
     * Open m_database_connection to filepath selected by the user
     * in the wizard, creating the file in the process.
     *
     * Should be called only after calling RunWizard(), and after calling
     * configure_default_commodity() and configure_entity_creation_date().
     */
    void create_file();

    void delete_file();

    /**
     * Reflect in the database the Accounts selected by user in
     * the wizard.
     *
     * Should be called only after calling RunWizard(), and after calling
     * configure_default_commodity() and create_file().
     */
    void configure_accounts();

    DcmDatabaseConnection& m_database_connection;
    std::set<wxString> m_account_names_already_taken;
    FilepathPage* m_filepath_page;
    AccountPage* m_balance_sheet_account_page;
    AccountPage* m_pl_account_page;
    
};  // SetupWizard


/**
 * To facilitate validation of filepath entered in
 * SetupWizard::FilepathPage.
 */
class SetupWizard::FilepathValidator: public wxValidator
{
public:
    FilepathValidator(boost::filesystem::path* p_filepath);
    FilepathValidator(FilepathValidator const& rhs);

    FilepathValidator(FilepathValidator&&) = delete;
    FilepathValidator& operator=(FilepathValidator const&) = delete;
    FilepathValidator& operator=(FilepathValidator&&) = delete;
    virtual ~FilepathValidator();

    /*** Functions inherited from wxValidator virtuals ***/

    /**
     * @param parent should be passed the TextCtrl in
     * a FilepathPage,
     * that contains the filename entered by the user.
     * Then validator then validates this by ensuring that:\n
     * (a) it can be a valid filename; and\n
     * (b) the filepath consisting of text in \e parent, appended
     * to the directory also entered into the FilepathPage to which
     * the \e parent belongs, does not already exist.
     */
    bool Validate(wxWindow* parent) override;

    bool TransferFromWindow() override;

    bool TransferToWindow() override;

    wxObject* Clone() const override;

private:
    boost::filesystem::path* m_filepath;
};


/**
 * This is the first page the user sees when opening the application
 * without a file selected. This welcomes the user and prompts them to
 * select a file.
 */
class SetupWizard::FilepathPage: public wxWizardPageSimple
{
public:
    friend class FilepathValidator;

    FilepathPage
    (   SetupWizard* parent,
        DcmDatabaseConnection& p_database_connection
    );
    FilepathPage(FilepathPage const&) = delete;
    FilepathPage(FilepathPage&&) = delete;
    FilepathPage& operator=(FilepathPage const&) = delete;
    FilepathPage& operator=(FilepathPage&&) = delete;
    virtual ~FilepathPage();

    boost::optional<boost::filesystem::path> selected_filepath() const;
    sqloxx::Handle<Commodity> selected_currency() const;
    boost::gregorian::date selected_start_date() const;
private:

    void on_directory_button_click(wxCommandEvent& event);
    void on_wizard_page_changing(wxWizardEvent& event);
    void on_wizard_page_changed(wxWizardEvent& event);
    void on_currency_selection_change(wxCommandEvent& event);
    void on_precision_selection_change(wxCommandEvent& event);

    std::vector<sqloxx::Handle<Commodity> > const m_currencies;

    wxBoxSizer* m_top_sizer;
    wxBoxSizer* m_filename_row_sizer;
    wxBoxSizer* m_directory_row_sizer;
    TextCtrl* m_directory_ctrl;
    Button* m_directory_button;
    TextCtrl* m_filename_ctrl;
    ComboBox* m_currency_box;
    ComboBox* m_precision_box;
    DateCtrl* m_start_date_ctrl;

    // Things are simpler in the implementation if this is a raw pointer.
    // Don't be tempted to change this to a smart pointer or a reference.
    // Been there, tried that...
    boost::filesystem::path* m_selected_filepath;

    static int const s_directory_button_id = wxID_HIGHEST + 1;
    static int const s_currency_box_id = s_directory_button_id + 1;
    static int const s_precision_box_id = s_currency_box_id + 1;
    static int const s_start_date_ctrl_id = s_precision_box_id + 1;

    DECLARE_EVENT_TABLE()

};  // SetupWizard::FilepathPage


/**
 * Represents a page for setting up Accounts in a SetupWizard.
 * Abstract base class - more specific Account page types derive
 * from this.
 */
class SetupWizard::AccountPage: public wxWizardPageSimple
{
public:
    AccountPage
    (   SetupWizard* p_parent,
        AccountSuperType p_account_super_type,
        DcmDatabaseConnection& p_database_connection
    );

    AccountPage(AccountPage const&) = delete;
    AccountPage(AccountPage&&) = delete;
    AccountPage& operator=(AccountPage const&) = delete;
    AccountPage& operator=(AccountPage&&) = delete;

    virtual ~AccountPage();

    void render();

    std::vector<AugmentedAccount> augmented_accounts() const;

    void set_commodity(sqloxx::Handle<Commodity> const& p_commodity);

    jewel::Decimal total_amount() const;

    /**
     * @returns the total_amount() of the <em>balance sheet</em>
     * AccountPage that is in the same SetupWizard as this AccountPage.
     * This may or may not be the same AccountPage as this one. (Whether
     * this is a "balance sheet AccountPage" is determined by
     * whether or not AccountSuperType::balance_sheet was passed
     * to the constructor.)
     */
    jewel::Decimal total_balance_sheet_amount() const;

    /**
     * Tell the AccountPage that the strings in the given set have already
     * been used for Account names in a previous AccountPage. Should only
     * be called after render() has been called.
     */
    void set_account_names_already_taken
    (   std::set<wxString> const& p_account_names_already_taken
    );

protected:
    DcmDatabaseConnection& database_connection() const;
    SetupWizard const& parent() const;
    wxGridBagSizer& top_sizer();
    int current_row() const;
    void increment_row();

    bool account_names_valid(wxString& p_error_message) const;
    bool account_types_valid(wxString& p_error_message) const;

private:

    void on_remove_checked_button_click(wxCommandEvent& event);
    void on_push_row_button_click(wxCommandEvent& event);
    void on_wizard_page_changing(wxWizardEvent& event);
    void on_wizard_page_changed(wxWizardEvent& event);
    void on_check_box_clicked(wxCommandEvent& event);

    void render_main_text();
    void render_buttons();
    void render_account_view();
    void refresh_remove_checked_button_state();

    wxString main_text() const;

    static unsigned int const s_remove_checked_button_id = wxID_HIGHEST + 1;
    static unsigned int const s_push_row_button_id = s_remove_checked_button_id + 1;

    DcmDatabaseConnection& m_database_connection;
    AccountSuperType m_account_super_type;
    size_t const m_min_num_accounts;
    int m_current_row; 
    Button* m_remove_checked_button;
    Button* m_push_row_button;
    wxGridBagSizer* m_top_sizer;
    MultiAccountPanel* m_multi_account_panel;
    SetupWizard const& m_parent;

    DECLARE_EVENT_TABLE()
};

}  // namespace gui
}  // namespace dcm

#endif  // GUARD_setup_wizard_hpp_8623281646810137
