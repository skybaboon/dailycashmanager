// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "multi_account_panel.hpp"
#include "account.hpp"
#include "account_type.hpp"
#include "account_type_ctrl.hpp"
#include "b_string.hpp"
#include "commodity.hpp"
#include "decimal_text_ctrl.hpp"
#include "finformat.hpp"
#include "locale.hpp"
#include "gridded_scrolled_panel.hpp"
#include "make_default_accounts.hpp"
#include "phatbooks_database_connection.hpp"
#include "phrase_flags.hpp"
#include "setup_wizard.hpp"
#include "sizing.hpp"
#include "visibility.hpp"
#include <jewel/decimal.hpp>
#include <wx/button.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <cassert>
#include <numeric>
#include <set>
#include <vector>

using jewel::Decimal;
using jewel::round;
using std::accumulate;
using std::set;
using std::vector;

// for debugging
	#include <jewel/debug_log.hpp>
	#include <iostream>
	using std::endl;

namespace phatbooks
{
namespace gui
{

namespace
{
	vector<Account> suggested_accounts
	(	PhatbooksDatabaseConnection& p_database_connection,
		account_super_type::AccountSuperType p_account_super_type
	)
	{
		vector<Account> ret;
		assert (ret.empty());
		typedef vector<account_type::AccountType> ATypeVec;
		ATypeVec const& account_types =
			phatbooks::account_types(p_account_super_type);
		ATypeVec::const_iterator it = account_types.begin();
		ATypeVec::const_iterator const end = account_types.end();
		for ( ; it != end; ++it)
		{
			make_default_accounts(p_database_connection, ret, *it);
		}
		return ret;
	}
	Decimal total_amount_aux(Decimal const& dec, DecimalTextCtrl* ctrl)
	{
		return dec + ctrl->amount();
	}

}  // end anonymous namespace

MultiAccountPanel::MultiAccountPanel
(	SetupWizard::AccountPage* p_parent,
	wxSize const& p_size,
	PhatbooksDatabaseConnection& p_database_connection,
	account_super_type::AccountSuperType p_account_super_type,
	Commodity const& p_commodity,
	size_t p_minimum_num_rows
):
	GriddedScrolledPanel(p_parent, p_size, p_database_connection),
	m_account_super_type(p_account_super_type),
	m_commodity(p_commodity),
	m_summary_amount_text(0),
	m_minimum_num_rows(p_minimum_num_rows)
{
	// Row of total text etc.
	wxString summary_label("Total");
	if (m_account_super_type == account_super_type::pl)
	{
		summary_label += wxString(" to allocate");
	}
	summary_label += ":";
	display_text(summary_label, 3, wxALIGN_RIGHT);
	m_summary_amount_text = display_decimal
	(	summary_amount(),
		4,
		false
	);
	// Dummy column to right
	display_text(wxEmptyString, 5);

	increment_row();
	increment_row();

	// Row of column headings
	phrase_flags::PhraseFlags const flags = phrase_flags::capitalize;
	wxString const account_name_label =
		wxString(" ") +
		bstring_to_wx(account_concept_name(m_account_super_type, flags)) +
		wxString(" name:");
	display_text(account_name_label, 0);
	display_text(wxString(" Type:"), 1);
	display_text(wxString(" Description:"), 2);
	// Deliberately skipping column 3.
	display_text(wxString(" Opening balance:"), 4);

	increment_row();

	// Main body of MultiAccountPanel - a grid of fields where user
	// can edit Account attributes and opening balances.
	vector<Account> sugg_accounts =
		suggested_accounts(database_connection(), m_account_super_type);
	vector<Account>::size_type const sz = sugg_accounts.size();
	m_account_name_boxes.reserve(sz);
	m_account_type_boxes.reserve(sz);
	m_description_boxes.reserve(sz);
	m_opening_balance_boxes.reserve(sz);
	vector<Account>::iterator it = sugg_accounts.begin();
	vector<Account>::iterator const end = sugg_accounts.end();
	for ( ; it != end; ++it)
	{
		push_row(*it);
	}
	while (num_rows() < m_minimum_num_rows)
	{
		push_row();
	}
	assert (num_rows() >= p_minimum_num_rows);

	// "Admin"
	FitInside();
	// Layout();
}

MultiAccountPanel::~MultiAccountPanel()
{
}

int
MultiAccountPanel::required_width()
{
	return
		medium_width() * 3 +
		large_width() * 1 +
		standard_gap() * 3 +
		standard_border() * 2 +
		scrollbar_width_allowance();
}

bool
MultiAccountPanel::push_row()
{
	Account account = blank_account();
	return push_row(account);
}

bool
MultiAccountPanel::pop_row()
{
	if (m_account_name_boxes.size() <= m_minimum_num_rows)
	{
		return false;
	}
	assert (m_account_name_boxes.size() > 1);
#	ifndef NDEBUG
		vector<wxTextCtrl*>::size_type const sz = m_account_name_boxes.size();
		assert (sz > 0);
		assert (sz == m_account_type_boxes.size());
		assert (sz == m_description_boxes.size());
		assert (sz == m_opening_balance_boxes.size());
#	endif
	pop_widget_from(m_opening_balance_boxes);
	pop_widget_from(m_description_boxes);
	pop_widget_from(m_account_type_boxes);
	pop_widget_from(m_account_name_boxes);
	decrement_row();
	FitInside();
	return true;
}

Decimal
MultiAccountPanel::summary_amount() const
{
	if (m_account_super_type == account_super_type::balance_sheet)
	{
		return total_amount();
	}
	assert (m_account_super_type == account_super_type::pl);
	SetupWizard::AccountPage const* const parent =
		dynamic_cast<SetupWizard::AccountPage const*>(GetParent());
	assert (parent);
	return parent->total_balance_sheet_amount() - total_amount();
}

void
MultiAccountPanel::update_summary()
{
	m_summary_amount_text->
		SetLabel(finformat_wx(summary_amount(), locale(), false));
	Layout();  // This is essential.
	return;
}

size_t
MultiAccountPanel::num_rows() const
{
	size_t const sz = m_account_name_boxes.size();
	assert (sz == m_account_type_boxes.size());
	assert (sz == m_description_boxes.size());
	assert (sz == m_opening_balance_boxes.size());
	return m_account_name_boxes.size();
}

bool
MultiAccountPanel::account_type_is_selected
(	account_type::AccountType p_account_type
) const
{
	vector<AccountTypeCtrl*>::size_type i = 0;
	vector<AccountTypeCtrl*>::size_type const sz =
		m_account_type_boxes.size();
	for ( ; i != sz; ++i)
	{
		if (m_account_type_boxes[i]->account_type() == p_account_type)
		{
			return true;
		}
	}
	return false;
}

Account
MultiAccountPanel::blank_account()
{
	Account ret(database_connection());
	BString const empty_string;
	assert (empty_string.empty());
	ret.set_name(empty_string);
	ret.set_description(empty_string);
	ret.set_visibility(visibility::visible);
	vector<account_type::AccountType> const& atypes =
		account_types(m_account_super_type);
	assert (!atypes.empty());
	ret.set_account_type(atypes.at(0));
	return ret;
}

bool
MultiAccountPanel::push_row(Account& p_account)
{
	int const row = current_row();

	// Account name
	wxTextCtrl* account_name_box = new wxTextCtrl
	(	this,
		wxID_ANY,
		bstring_to_wx(p_account.name()),
		wxDefaultPosition,
		wxSize(medium_width(), wxDefaultSize.y),
		wxALIGN_LEFT
	);
	top_sizer().Add(account_name_box, wxGBPosition(row, 0));
	m_account_name_boxes.push_back(account_name_box);

	int const height = account_name_box->GetSize().GetY();

	// Account type
	AccountTypeCtrl* account_type_box = new AccountTypeCtrl
	(	this,
		wxID_ANY,
		wxSize(medium_width(), height),
		database_connection(),
		m_account_super_type
	);
	account_type_box->set_account_type(p_account.account_type());
	top_sizer().Add(account_type_box, wxGBPosition(row, 1));
	m_account_type_boxes.push_back(account_type_box);

	// Description
	wxTextCtrl* description_box = new wxTextCtrl
	(	this,
		wxID_ANY,
		p_account.description(),
		wxDefaultPosition,
		wxSize(large_width(), height),
		wxALIGN_LEFT
	);
	top_sizer().
		Add(description_box, wxGBPosition(row, 2), wxGBSpan(1, 2));
	m_description_boxes.push_back(description_box);

	p_account.set_commodity(m_commodity);

	// Opening balance
	SpecialDecimalTextCtrl* opening_balance_box = new SpecialDecimalTextCtrl
	(	this,
		wxSize(medium_width(), height)
	);
	top_sizer().Add(opening_balance_box, wxGBPosition(row, 4));
	m_opening_balance_boxes.push_back(opening_balance_box);

	increment_row();

	FitInside();

	return true;
}

void
MultiAccountPanel::set_commodity(Commodity const& p_commodity)
{
	Decimal::places_type const precision = p_commodity.precision();
	if (precision == m_commodity.precision())
	{
		return;
	}
	assert (precision != m_commodity.precision());
	vector<SpecialDecimalTextCtrl*>::size_type i = 0;
	vector<SpecialDecimalTextCtrl*>::size_type const sz =
		m_opening_balance_boxes.size();
	for ( ; i != sz; ++i)
	{
		// TODO Handle potential Decimal exception here on rounding.
		Decimal const old_amount = m_opening_balance_boxes[i]->amount();
		Decimal const new_amount = round(old_amount, precision);
		m_opening_balance_boxes[i]->set_amount(new_amount);
	}
	m_commodity = p_commodity;
	return;
}

void
MultiAccountPanel::selected_augmented_accounts
(	vector<AugmentedAccount>& out
)
{
#	ifndef NDEBUG
		vector<AugmentedAccount>::size_type const original_size =
			out.size();
#	endif
	vector<AugmentedAccount>::size_type const sz =
		m_account_name_boxes.size();
	assert (m_account_type_boxes.size() == sz);
	assert (m_description_boxes.size() == sz);
	assert (m_opening_balance_boxes.size() == sz);
	vector<AugmentedAccount>::size_type i = 0;
	for ( ; i != sz; ++i)
	{
		AugmentedAccount augmented_account
		(	database_connection(),
			m_commodity
		);
		Account& account = augmented_account.account;
		account.set_name
		(	wx_to_bstring(m_account_name_boxes[i]->GetValue().Trim())
		);
		account_type::AccountType const account_type =
			m_account_type_boxes[i]->account_type();
		assert (super_type(account_type) == m_account_super_type);
		account.set_account_type(account_type);
		account.set_description(m_description_boxes[i]->GetValue());
		account.set_visibility(visibility::visible);
		account.set_commodity(m_commodity);

		// TODO Make sure it is clear to the user which way round the
		// signs are supposed to go, especially for account_type::liability
		// (where the user should normally enter a negative number).
		augmented_account.technical_opening_balance =
		(	m_account_super_type == account_super_type::pl?
			-m_opening_balance_boxes[i]->amount():
			m_opening_balance_boxes[i]->amount()
		);

		assert (!account.has_id());
		out.push_back(augmented_account);
	}
#	ifndef NDEBUG
		assert (out.size() == original_size + sz);
#	endif
	return;
}

bool
MultiAccountPanel::account_names_valid(wxString& p_error_message) const
{
	set<wxString> account_names;
	vector<wxTextCtrl*>::size_type i = 0;
	vector<wxTextCtrl*>::size_type const sz = m_account_name_boxes.size();
	phrase_flags::PhraseFlags const flags = phrase_flags::capitalize;
	for ( ; i != sz; ++i)
	{
		wxString const name =
			m_account_name_boxes[i]->GetValue().Trim().Lower();
		if (name.IsEmpty())
		{
			p_error_message =
				bstring_to_wx
				(	account_concept_name(m_account_super_type, flags)
				) +
				wxString(" name is blank");
			return false;
		}
		if (account_names.find(name) != account_names.end())
		{
			p_error_message = wxString("Duplicate account name: ");
			p_error_message += name;
			return false;
		}
		account_names.insert(name);
	}
	return true;
}

Decimal
MultiAccountPanel::total_amount() const
{
	return accumulate
	(	m_opening_balance_boxes.begin(),
		m_opening_balance_boxes.end(),
		Decimal(0, m_commodity.precision()),
		total_amount_aux
	);
}

MultiAccountPanel::SpecialDecimalTextCtrl::SpecialDecimalTextCtrl
(	MultiAccountPanel* p_parent,
	wxSize const& p_size
):
	DecimalTextCtrl
	(	p_parent,
		wxID_ANY,
		p_size,
		p_parent->m_commodity.precision(),
		false
	)
{
}

MultiAccountPanel::SpecialDecimalTextCtrl::~SpecialDecimalTextCtrl()
{
}

void
MultiAccountPanel::SpecialDecimalTextCtrl::do_on_kill_focus
(	wxFocusEvent& event
)
{
	GetParent()->Validate();
	GetParent()->TransferDataToWindow();
	MultiAccountPanel* const parent =
		dynamic_cast<MultiAccountPanel*>(GetParent());	
	assert (parent);
	parent->update_summary();
	event.Skip();
	return;
}


}  // namespace gui
}  // namespace phatbooks
