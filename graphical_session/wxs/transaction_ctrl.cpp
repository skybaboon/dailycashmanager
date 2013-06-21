// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "transaction_ctrl.hpp"
#include "account.hpp"
#include "account_ctrl.hpp"
#include "account_reader.hpp"
#include "account_type.hpp"
#include "b_string.hpp"
#include "date.hpp"
#include "date_ctrl.hpp"
#include "decimal_text_ctrl.hpp"
#include "decimal_validator.hpp"
#include "entry.hpp"
#include "entry_ctrl.hpp"
#include "finformat.hpp"
#include "frame.hpp"
#include "ordinary_journal.hpp"
#include "locale.hpp"
#include "phatbooks_database_connection.hpp"
#include "top_panel.hpp"
#include "transaction_type_ctrl.hpp"
#include "transaction_type.hpp"
#include <boost/optional.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/scoped_ptr.hpp>
#include <jewel/debug_log.hpp>
#include <jewel/decimal.hpp>
#include <jewel/on_windows.hpp>
#include <jewel/optional.hpp>
#include <wx/arrstr.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/panel.h>
#include <wx/event.h>
#include <wx/msgdlg.h>
#include <wx/gbsizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

using boost::optional;
using boost::scoped_ptr;
using jewel::Decimal;
using jewel::value;
using std::endl;
using std::vector;

namespace gregorian = boost::gregorian;

namespace phatbooks
{
namespace gui
{

BEGIN_EVENT_TABLE(TransactionCtrl, wxPanel)
	EVT_BUTTON
	(	wxID_OK,
		TransactionCtrl::on_ok_button_click
	)
	EVT_BUTTON
	(	s_recurring_transaction_button_id,
		TransactionCtrl::on_recurring_transaction_button_click
	)
	EVT_BUTTON
	(	wxID_CANCEL,
		TransactionCtrl::on_cancel_button_click
	)
END_EVENT_TABLE()

// WARNING There are bugs in wxWidgets' wxDatePickerCtrl under wxGTK.
// Firstly, tab traversal gets stuck on that control.
// Secondly, if we type a different date and then press "Enter" for OK,
// the date that actually gets picked up as the transaction date always
// seems to be TODAY's date, not the date actually entered. This appears to
// be an unresolved bug in wxWidgets.
// Note adding wxTAB_TRAVERSAL to style does not seem to fix the problem.
// We have used a simple custom class, DateCtrl here instead, to avoid
// these problems. Might later add a button to pop up a wxCalendarCtrl
// if the user wants one.

TransactionCtrl::TransactionCtrl
(	TopPanel* p_parent,
	vector<Account> const& p_balance_sheet_accounts,
	vector<Account> const& p_pl_accounts,
	PhatbooksDatabaseConnection& p_database_connection
):
	wxPanel
	(	p_parent,
		wxID_ANY,
		wxDefaultPosition,
		wxDefaultSize
	),
	m_top_sizer(0),
	m_transaction_type_ctrl(0),
	m_source_entry_ctrl(0),
	m_destination_entry_ctrl(0),
	m_primary_amount_ctrl(0),
	m_date_ctrl(0),
	m_cancel_button(0),
	m_recurring_transaction_button(0),
	m_ok_button(0),
	m_database_connection(p_database_connection)
{
	assert (m_account_name_boxes.empty());
	assert (m_comment_boxes.empty());
	assert (m_split_buttons.empty());
	assert (!p_balance_sheet_accounts.empty() || !p_pl_accounts.empty());
	assert (p_balance_sheet_accounts.size() + p_pl_accounts.size() >= 2);
	
	// Figure out the natural TransactionType given the Accounts we have
	// been passed. We will use this to initialize the TransactionTypeCtrl.
	Account account_x(p_database_connection);
	Account account_y(p_database_connection);
	if (p_balance_sheet_accounts.empty())
	{
		assert (p_pl_accounts.size() >= 2);
		account_x = p_pl_accounts[0];
		account_y = p_pl_accounts[1];
	}
	else if (p_pl_accounts.empty())
	{
		assert (p_balance_sheet_accounts.size() >= 2);
		account_x = p_balance_sheet_accounts[0];
		account_y = p_balance_sheet_accounts[1];
	}
	else
	{
		assert (!p_balance_sheet_accounts.empty());
		assert (!p_pl_accounts.empty());
		account_x = p_balance_sheet_accounts[0];
		account_y = p_pl_accounts[0];
	}
	if (account_y.account_type() == account_type::revenue)
	{
		using std::swap;
		swap(account_x, account_y);
	}
	assert (account_x.has_id());
	assert (account_y.has_id());
	transaction_type::TransactionType const initial_transaction_type =
		natural_transaction_type(account_x, account_y);

	size_t row = 0;	

	// Top sizer
	m_top_sizer = new wxGridBagSizer();
	SetSizer(m_top_sizer);

	m_transaction_type_ctrl = new TransactionTypeCtrl
	(	this,
		s_transaction_type_ctrl_id,
		wxSize(160, wxDefaultSize.y)
	);

	m_transaction_type_ctrl->set_transaction_type(initial_transaction_type);
	m_top_sizer->Add(m_transaction_type_ctrl, wxGBPosition(row, 0));
	wxSize const text_box_size = m_transaction_type_ctrl->GetSize();

	m_primary_amount_ctrl = new DecimalTextCtrl
	(	this,
		s_primary_amount_ctrl_id,
		text_box_size,
		m_database_connection.default_commodity().precision(),
		false
	);
	m_top_sizer->Add
	(	m_primary_amount_ctrl,
		wxGBPosition(row, 2),
		wxDefaultSpan,
		wxALIGN_RIGHT
	);
	wxString currency_abbreviation = bstring_to_wx
	(	m_database_connection.default_commodity().abbreviation()
	);
	currency_abbreviation = wxString(" ") + currency_abbreviation;
	wxStaticText* const currency_text = new wxStaticText
	(	this,
		wxID_ANY,
		currency_abbreviation,
		wxDefaultPosition,
		text_box_size
	);
	m_top_sizer->Add(currency_text, wxGBPosition(row, 3));

	// We need the names of available Accounts, for the given
	// TransactionType, from which the user will choose
	// Accounts, for each side of the transaction.

	assert_transaction_type_validity(initial_transaction_type);

	scoped_ptr<AccountReaderBase> const account_reader_x
	(	create_source_account_reader
		(	m_database_connection,
			initial_transaction_type
		)
	);
	scoped_ptr<AccountReaderBase> const account_reader_y
	(	create_destination_account_reader
		(	m_database_connection,
			initial_transaction_type
		)
	);

	// Rows for entering Entry details
	typedef vector<Account>::size_type Size;
	vector<Account> accounts;
	accounts.push_back(account_x);
	accounts.push_back(account_y);

	row += 3;

	// WARNING Temp hack
	assert (accounts.size() >= 2);

	vector<Account> source_accounts;
	source_accounts.push_back(accounts[0]);
	vector<Account> destination_accounts;
	destination_accounts.push_back(accounts[1]);

	Decimal::places_type const precision =
		m_database_connection.default_commodity().precision();
	m_source_entry_ctrl = new EntryCtrl
	(	this,
		source_accounts,
		m_database_connection,
		initial_transaction_type,
		text_box_size,
		true,
		Decimal(0, precision)
	);
	m_destination_entry_ctrl = new EntryCtrl
	(	this,
		destination_accounts,
		m_database_connection,
		initial_transaction_type,
		text_box_size,
		false,
		Decimal(0, precision)	
	);
	m_top_sizer->Add
	(	m_source_entry_ctrl,
		wxGBPosition(row, 0),
		wxGBSpan(1, 4),
		wxEXPAND
	);
	
	row += 2;

	m_top_sizer->Add
	(	m_destination_entry_ctrl,
		wxGBPosition(row, 0),
		wxGBSpan(1, 4),
		wxEXPAND
	);
	
	row += 2;


	// Buttons and date control

	m_cancel_button = new wxButton
	(	this,
		wxID_CANCEL,
		wxString("&Clear"),
		wxDefaultPosition,
		wxSize(text_box_size.x, text_box_size.y)
	);
	m_top_sizer->Add(m_cancel_button, wxGBPosition(row, 0));
	m_date_ctrl = new DateCtrl
	(	this,
		wxID_ANY,
		wxSize(text_box_size.x, text_box_size.y)
	);
	m_top_sizer->Add(m_date_ctrl, wxGBPosition(row, 1));
	m_recurring_transaction_button = new wxButton
	(	this,
		s_recurring_transaction_button_id,
		wxString("&Recurring..."),
		wxDefaultPosition,
		wxSize(text_box_size.x, text_box_size.y)
	);
	m_top_sizer->Add(m_recurring_transaction_button, wxGBPosition(row, 2));
	m_ok_button = new wxButton
	(	this,
		wxID_OK,
		wxString("&OK"),
		wxDefaultPosition,
		wxSize(text_box_size.x, text_box_size.y)
	);

	m_top_sizer->Add(m_ok_button, wxGBPosition(row, 3));
	m_ok_button->SetDefault();  // Enter key will now trigger "OK" button

	++row;

	// "Admin"
	// SetSizer(m_top_sizer);
	m_top_sizer->Fit(this);
	m_top_sizer->SetSizeHints(this);
	Fit();
	Layout();
}

void
TransactionCtrl::refresh_for_transaction_type
(	transaction_type::TransactionType p_transaction_type
)
{
	m_source_entry_ctrl->refresh_for_transaction_type(p_transaction_type);
	m_destination_entry_ctrl->refresh_for_transaction_type(p_transaction_type);
	return;
}

void
TransactionCtrl::reset_entry_ctrl_amounts()
{
	Decimal const pa = primary_amount();
	m_source_entry_ctrl->set_primary_amount(pa);
	m_destination_entry_ctrl->set_primary_amount(pa);
	return;
}

void
TransactionCtrl::on_ok_button_click(wxCommandEvent& event)
{
	(void)event;  // Silence compiler re. unused parameter.
	if (Validate() && TransferDataFromWindow())
	{
		if (is_balanced())
		{
			post_journal();
			TopPanel* const panel = dynamic_cast<TopPanel*>(GetParent());
			assert (panel);
			panel->update();
		}
		else
		{
			wxMessageBox("Transaction does not balance.");
		}
	}
	return;
}

void
TransactionCtrl::on_recurring_transaction_button_click(wxCommandEvent& event)
{
	(void)event;  // Silence compiler re. unused parameter.
	return;
}

void
TransactionCtrl::on_cancel_button_click(wxCommandEvent& event)
{
	(void)event;  // Silence compiler re. unused parameter.
	TopPanel* const panel = dynamic_cast<TopPanel*>(GetParent());
	assert (panel);
	panel->update();
}

Decimal
TransactionCtrl::primary_amount() const
{
	return Decimal
	(	wx_to_decimal
		(	wxString(m_primary_amount_ctrl->GetValue()),
			locale()
		)
	);
}

void
TransactionCtrl::post_journal() const
{
	// TODO HIGH PRIORITY Fix this
	// to work now that we have EntryCtrl instead of storing
	// Entry info directly in the TransactionCtrl.
	OrdinaryJournal journal(m_database_connection);
	// TODO What if the dereferencing of optional fails?
	transaction_type::TransactionType const ttype =
		value(m_transaction_type_ctrl->transaction_type());
	journal.set_whether_actual(transaction_type_is_actual(ttype));
	size_t const sz = m_account_name_boxes.size();
	assert (sz == m_comment_boxes.size());
	// WARNING This can't yet handle Journals with a number of entries
	// other than 2.
	assert (sz == 2);
	for (size_t i = 0; i != sz; ++i)
	{
		Account const account
		(	m_database_connection,
			wx_to_bstring(wxString(m_account_name_boxes[i]->GetValue()))
		);
		Entry entry(m_database_connection);
		entry.set_account(account);
		entry.set_comment
		(	wx_to_bstring(m_comment_boxes[i]->GetValue())
		);
		Decimal amount = primary_amount();
		if (i == 0)
		{
			// This is the source account
			amount = -amount;
		}
		if (!journal.is_actual())
		{
			amount = -amount;
		}
		amount = round(amount, account.commodity().precision());
		entry.set_amount(amount);
		entry.set_whether_reconciled(false);
		journal.push_entry(entry);
	}
	assert (journal.is_balanced());
	journal.set_comment("");

	// Process date
	journal.set_date(m_date_ctrl->date());

	// Save journal
	journal.save();
}

bool
TransactionCtrl::is_balanced() const
{
	// WARNING For now this is trivial, as we have only the primary_amount
	// informing one each of only two sides of the transaction. But it
	// probably won't always be trivial.
	return true;
	/*
	Decimal balance(0, 0);
	vector<DecimalTextCtrl*>::size_type i = 0;
	vector<DecimalTextCtrl*>::size_type const sz = m_amount_boxes.size();
	for ( ; i != sz; ++i)
	{
		balance += wx_to_decimal(m_amount_boxes[i]->GetValue(), locale());
	}
	return balance == Decimal(0, 0);
	*/
}


}  // namespace gui
}  // namespace phatbooks
