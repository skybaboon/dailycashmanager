/*
 * This file is part of the Phatbooks project and is distributed under the
 * terms of the license contained in the file LICENSE.txt distributed
 * with this package.
 * 
 * Author: Matthew Harvey <matthew@matthewharvey.net>
 *
 * Copyright (c) 2012-2013, Matthew Harvey.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef GUARD_entry_list_panel_hpp_3556466034407013
#define GUARD_entry_list_panel_hpp_3556466034407013

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>
#include <sqloxx/handle_fwd.hpp>
#include <sqloxx/id.hpp>
#include <wx/button.h>
#include <wx/event.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/window.h>
#include <vector>

namespace phatbooks
{

// Begin forward declarations

class Account;
class Entry;
class OrdinaryJournal;
class PhatbooksDatabaseConnection;

namespace gui
{

class AccountCtrl;
class DateCtrl;
class EntryListCtrl;

// End forward declarations

/**
 * A panel consisting of an EntryListCtrl at the bottom, and widgets
 * at the top to enable the user to filter the displayed Entries for a
 * particular Account and/or date range.
 *
 * @todo MEDIUM PRIORITY Do we want the user to be able to view non-actual
 * transactions? If we do, then, among other things, the AccountCtrl will need
 * to allow the user to select Accounts of AccountType::pure_envelope.
 */
class EntryListPanel: public wxPanel
{
public:
	EntryListPanel
	(	wxWindow* p_parent,
		PhatbooksDatabaseConnection& p_database_connection,
		bool p_support_reconciliations = false
	);

	EntryListPanel(EntryListPanel const&) = delete;
	EntryListPanel(EntryListPanel&&) = delete;
	EntryListPanel& operator=(EntryListPanel const&) = delete;
	EntryListPanel& operator=(EntryListPanel&&) = delete;
	virtual ~EntryListPanel(); 

	void update_for_new(sqloxx::Handle<OrdinaryJournal> const& p_journal);
	void update_for_amended(sqloxx::Handle<OrdinaryJournal> const& p_journal);
	void update_for_new(sqloxx::Handle<Account> const& p_account);
	void update_for_amended(sqloxx::Handle<Account> const& p_account);
	void update_for_deleted(std::vector<sqloxx::Id> const& p_doomed_ids);
	void selected_entries(std::vector<sqloxx::Handle<Entry> >& out);

	// TODO LOW PRIORITY This should really be private, but we need to call it
	// from TopPanel to ensure EntryListCtrl is properly sized, AFTER the
	// EntryListPanel has been constructed. Make this nicer.
	void configure_entry_list_ctrl();

	void postconfigure_summary();

private:
	void preconfigure_summary();

	void on_refresh_button_click(wxCommandEvent& event);
	sqloxx::Handle<Account> selected_account() const;
	boost::optional<boost::gregorian::date> selected_min_date() const;
	boost::optional<boost::gregorian::date> selected_max_date() const;
	
	static int const s_account_ctrl_id = wxID_HIGHEST + 1;
	static int const s_min_date_ctrl_id = s_account_ctrl_id + 1;
	static int const s_max_date_ctrl_id = s_min_date_ctrl_id + 1;
	static int const s_refresh_button_id = s_max_date_ctrl_id + 1;
	static int const s_entry_list_ctrl_id = s_refresh_button_id + 1;

	bool m_support_reconciliations;
	int m_next_row;
	int m_client_size_aux;
	int m_text_ctrl_height;

	wxGridBagSizer* m_top_sizer;
	AccountCtrl* m_account_ctrl;
	DateCtrl* m_min_date_ctrl;
	DateCtrl* m_max_date_ctrl;
	wxButton* m_refresh_button;
	wxStaticText* m_reconciliation_hint;
	EntryListCtrl* m_entry_list_ctrl;
	std::vector<wxStaticText*> m_summary_label_text_items;
	std::vector<wxStaticText*> m_summary_data_text_items;
	PhatbooksDatabaseConnection& m_database_connection;

	DECLARE_EVENT_TABLE()

};  // class EntryListPanel

}  // namespace gui
}  // namespace phatbooks

#endif  // GUARD_entry_list_panel_hpp_3556466034407013
