// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_top_panel_hpp
#define GUARD_top_panel_hpp

#include "account.hpp"
#include <wx/panel.h>
#include <wx/sizer.h>
#include <vector>

namespace phatbooks
{

// Begin forward declarations

class OrdinaryJournal;
class PhatbooksDatabaseConnection;

namespace gui
{

class AccountListCtrl;
class Frame;
class TransactionCtrl;

// End forward declarations


/**
 * Top level panel intended as immediate child of Frame.
 */
class TopPanel: public wxPanel
{
public:

	TopPanel
	(	Frame* parent,
		PhatbooksDatabaseConnection& p_database_connection
	);

	/**
	 * Populates \e out with a vector of the balance sheet Accounts currently
	 * selected by the user in the main window.
	 */
	void selected_balance_sheet_accounts(std::vector<Account>& out) const;

	/**
	 * Populates \e out with a vector of the P&L Accounts currently selected
	 * by the user in the main window.
	 */
	void selected_pl_accounts(std::vector<Account>& out) const;

	/**
	 * Update the display to reflect that \e journal has been posted.
	 */
	void update_for_posted_journal(OrdinaryJournal const& journal);

	/**
	 * Redraw m_transaction_ctrl on the basis of Accounts in
	 * p_accounts
	 */
	void redraw_transaction_ctrl(std::vector<Account> const& p_accounts);

private:

	void configure_account_lists();
	void configure_transaction_ctrl();
	void configure_transaction_ctrl(std::vector<Account> const& p_accounts);

	PhatbooksDatabaseConnection& m_database_connection;
	wxBoxSizer* m_top_sizer;
	AccountListCtrl* m_bs_account_list;
	AccountListCtrl* m_pl_account_list;
	TransactionCtrl* m_transaction_ctrl;
};



}  // namespace gui
}  // namespace phatbooks


#endif  // GUARD_top_panel_hpp
