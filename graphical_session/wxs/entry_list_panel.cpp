// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "entry_list_panel.hpp"
#include "account.hpp"
#include "account_ctrl.hpp"
#include "date.hpp"
#include "date_ctrl.hpp"
#include "entry.hpp"
#include "entry_list_ctrl.hpp"
#include "locale.hpp"
#include "ordinary_journal.hpp"
#include "phatbooks_database_connection.hpp"
#include "string_flags.hpp"
#include "reconciliation_entry_list_ctrl.hpp"
#include "sizing.hpp"
#include "summary_datum.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/optional.hpp>
#include <jewel/assert.hpp>
#include <jewel/on_windows.hpp>
#include <jewel/optional.hpp>
#include <wx/button.h>
#include <wx/event.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/wupdlock.h>
#include <wx/window.h>
#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

using boost::optional;
using jewel::value;
using std::back_inserter;
using std::remove_copy;
using std::vector;

namespace gregorian = boost::gregorian;


// For debugging
	#include <jewel/log.hpp>
	#include <iostream>
	using std::endl;


namespace phatbooks
{
namespace gui
{

BEGIN_EVENT_TABLE(EntryListPanel, wxPanel)
	EVT_BUTTON(s_refresh_button_id, EntryListPanel::on_refresh_button_click)
END_EVENT_TABLE()


EntryListPanel::EntryListPanel
(	wxWindow* p_parent,
	PhatbooksDatabaseConnection& p_database_connection,
	bool p_support_reconciliations
):
	wxPanel(p_parent, wxID_ANY),
	m_support_reconciliations(p_support_reconciliations),
	m_next_row(0),
	m_client_size_aux(0),
	m_text_ctrl_height(0),
	m_top_sizer(0),
	m_account_ctrl(0),
	m_min_date_ctrl(0),
	m_max_date_ctrl(0),
	m_refresh_button(0),
	m_entry_list_ctrl(0),
	m_database_connection(p_database_connection)
{
	m_top_sizer = new wxGridBagSizer(standard_gap(), standard_gap());
	SetSizer(m_top_sizer);

	bool const include_pl_accounts = !m_support_reconciliations;
	bool const allow_blank_dates = !m_support_reconciliations;

	++m_next_row;  // To leave some space at top.

	wxString account_label_text(" ");
	account_label_text += account_concept_name
	(	account_super_type::balance_sheet,
		AccountPhraseFlags().set(string_flags::capitalize)
	);
	if (include_pl_accounts)
	{
		account_label_text += wxString(" or ");
		account_label_text += account_concept_name(account_super_type::pl);
	}
	account_label_text += wxString(":");
	wxStaticText* account_label =
		new wxStaticText(this, wxID_ANY, account_label_text);
	m_top_sizer->Add(account_label, wxGBPosition(m_next_row, 1));
	wxStaticText* min_date_label =
		new wxStaticText(this, wxID_ANY, wxString(" From:"));
	m_text_ctrl_height = min_date_label->GetSize().GetY();
	m_top_sizer->Add(min_date_label, wxGBPosition(m_next_row, 2));
	wxStaticText* max_date_label =
		new wxStaticText(this, wxID_ANY, wxString(" To:"));
	m_top_sizer->Add(max_date_label, wxGBPosition(m_next_row, 3));

	++m_next_row;

	if (include_pl_accounts)
	{
		using account_type::AccountType;
		vector<AccountType> const& all_account_types = account_types();
		vector<AccountType> impure_account_types;
		remove_copy
		(	all_account_types.begin(),
			all_account_types.end(),
			back_inserter(impure_account_types),
			account_type::pure_envelope
		);
		m_account_ctrl = new AccountCtrl
		(	this,
			s_account_ctrl_id,
			wxSize(large_width(), wxDefaultSize.y),
			impure_account_types,
			m_database_connection
		);
	}
	else
	{
		m_account_ctrl = new AccountCtrl
		(	this,
			s_account_ctrl_id,
			wxSize(large_width(), wxDefaultSize.y),
			account_types(account_super_type::balance_sheet),
			m_database_connection
		);
	}
	int const std_height = m_account_ctrl->GetSize().GetHeight();
	m_top_sizer->Add(m_account_ctrl, wxGBPosition(m_next_row, 1));
	m_min_date_ctrl = new DateCtrl
	(	this,
		s_min_date_ctrl_id,
		wxSize(medium_width(), std_height),
		today(),
		allow_blank_dates
	);
	m_top_sizer->Add(m_min_date_ctrl, wxGBPosition(m_next_row, 2));
	m_max_date_ctrl = new DateCtrl
	(	this,
		s_max_date_ctrl_id,
		wxSize(medium_width(), std_height),
		today(),
		allow_blank_dates
	);
	m_top_sizer->Add(m_max_date_ctrl, wxGBPosition(m_next_row, 3));
	m_refresh_button = new wxButton
	(	this,
		s_refresh_button_id,
		wxString("&Refresh"),
		wxDefaultPosition,
		m_max_date_ctrl->GetSize()
	);
	m_refresh_button->SetDefault();
	m_top_sizer->Add(m_refresh_button, wxGBPosition(m_next_row, 4));

	++m_next_row;

	preconfigure_summary();
	configure_entry_list_ctrl();
	postconfigure_summary();

	// "Admin"
	m_top_sizer->Fit(this);
	m_top_sizer->SetSizeHints(this);
	Fit();
	Layout();
}

void
EntryListPanel::on_refresh_button_click(wxCommandEvent& event)
{
	(void)event;  // Silence compiler re. unused parameter.
	configure_entry_list_ctrl();
	postconfigure_summary();
	return;
}

void
EntryListPanel::update_for_new(OrdinaryJournal const& p_journal)
{
	JEWEL_ASSERT (m_entry_list_ctrl);
	m_entry_list_ctrl->update_for_new(p_journal);
	postconfigure_summary();
	return;
}

void
EntryListPanel::update_for_amended(OrdinaryJournal const& p_journal)
{
	JEWEL_ASSERT (m_entry_list_ctrl);
	m_entry_list_ctrl->update_for_amended(p_journal);
	postconfigure_summary();
	return;
}

void
EntryListPanel::update_for_new(Account const& p_account)
{
	JEWEL_ASSERT (m_entry_list_ctrl);
	m_entry_list_ctrl->update_for_new(p_account);
	m_account_ctrl->update_for_new(p_account);
	postconfigure_summary();
	return;
}

void
EntryListPanel::update_for_amended(Account const& p_account)
{
	JEWEL_ASSERT (m_entry_list_ctrl);
	m_entry_list_ctrl->update_for_amended(p_account);
	m_account_ctrl->update_for_amended(p_account);
	postconfigure_summary();
	return;
}

void
EntryListPanel::update_for_deleted(vector<Entry::Id> const& p_doomed_ids)
{
	JEWEL_ASSERT (m_entry_list_ctrl);
	m_entry_list_ctrl->update_for_deleted(p_doomed_ids);
	postconfigure_summary();
	return;
}

void
EntryListPanel::selected_entries(vector<Entry>& out)
{
	JEWEL_ASSERT (m_entry_list_ctrl);
	m_entry_list_ctrl->selected_entries(out);
	return;
}

void
EntryListPanel::configure_entry_list_ctrl()
{
	wxWindowUpdateLocker const update_locker(this);

	if (m_client_size_aux < 100)  // WARNING Ugly hack
	{
		m_client_size_aux = GetClientSize().GetY();
	}
	int const num_extra_rows = 2;
	int height_aux =
		m_client_size_aux -
		m_account_ctrl->GetSize().GetY() * num_extra_rows -
		standard_gap() * (num_extra_rows + 1) -
		standard_border() * 2;

#	ifdef JEWEL_ON_WINDOWS
		height_aux -= standard_gap() * (num_extra_rows + 1);
#	endif

	EntryListCtrl* temp = 0;
	if (m_support_reconciliations)
	{
		JEWEL_ASSERT (selected_min_date());
		JEWEL_ASSERT (selected_max_date());
		temp = EntryListCtrl::create_reconciliation_entry_list
		(	this,
			wxSize
			(	large_width() + medium_width() * 3 + standard_gap() * 3,
				height_aux - 
					// WARNING - ugly - "tightly coupled"
					2 * m_text_ctrl_height - standard_gap() * 2
			),
			selected_account(),
			value(selected_min_date()),
			value(selected_max_date())
		);
	}
	else
	{
		temp = EntryListCtrl::create_actual_ordinary_entry_list
		(	this,
			wxSize
			(	large_width() + medium_width() * 3 + standard_gap() * 3,
				height_aux
			),
			selected_account(),
			selected_min_date(),
			selected_max_date()
		);
	}

	using std::swap;
	swap(temp, m_entry_list_ctrl);
	if (temp)
	{
		m_top_sizer->Detach(temp);
		temp->Destroy();
		temp = 0;
		--m_next_row;
	}
	m_top_sizer->Add
	(	m_entry_list_ctrl,
		wxGBPosition(m_next_row, 1),
		wxGBSpan(1, 4),
		wxEXPAND
	);
	++m_next_row;


	// m_top_sizer->Fit(this);
	// m_top_sizer->SetSizeHints(this);
	Fit();
	// Layout();
	
	m_entry_list_ctrl->scroll_to_bottom();

	return;
}

void
EntryListPanel::preconfigure_summary()
{
	if (m_support_reconciliations)
	{
		JEWEL_ASSERT (m_summary_label_text_items.empty());
		JEWEL_ASSERT (m_summary_data_text_items.empty());

		wxSize const size(wxDefaultSize.x, m_text_ctrl_height);

		wxStaticText* closing_balance_label = new wxStaticText
		(	this,
			wxID_ANY,
			wxEmptyString,
			wxDefaultPosition,
			size
		);
		m_top_sizer->Add(closing_balance_label, wxGBPosition(m_next_row, 2));
		m_summary_label_text_items.push_back(closing_balance_label);

		wxStaticText* reconciled_balance_label = new wxStaticText
		(	this,
			wxID_ANY,
			wxEmptyString,
			wxDefaultPosition,
			size
		);
		m_top_sizer->
			Add(reconciled_balance_label, wxGBPosition(m_next_row, 3));
		m_summary_label_text_items.push_back(reconciled_balance_label);

		++m_next_row;

		wxStaticText* hint = new wxStaticText
		(	this,
			wxID_ANY,
			wxString("(Right-click item to toggle whether reconciled.)"),
			wxDefaultPosition,
			size
		);
		m_top_sizer->Add(hint, wxGBPosition(m_next_row, 1));

		wxStaticText* closing_balance_amount = new wxStaticText
		(	this,
			wxID_ANY,
			wxEmptyString,
			wxDefaultPosition,
			size,
			wxALIGN_RIGHT
		);
		m_top_sizer->Add
		(	closing_balance_amount,
			wxGBPosition(m_next_row, 2),
			wxDefaultSpan,
			wxALIGN_RIGHT
		);
		m_summary_data_text_items.push_back(closing_balance_amount);
		wxStaticText* reconciled_balance_amount = new wxStaticText
		(	this,
			wxID_ANY,
			wxEmptyString,
			wxDefaultPosition,
			size,
			wxALIGN_RIGHT
		);
		m_top_sizer->Add
		(	reconciled_balance_amount,
			wxGBPosition(m_next_row, 3),
			wxDefaultSpan,
			wxALIGN_RIGHT
		);
		m_summary_data_text_items.push_back(reconciled_balance_amount);
		
		++m_next_row;
	}
	return;
}

void
EntryListPanel::postconfigure_summary()
{
	JEWEL_ASSERT
	(	m_summary_label_text_items.size() ==
		m_summary_data_text_items.size()
	);
	if (m_support_reconciliations)
	{
		JEWEL_ASSERT (m_entry_list_ctrl);
		vector<SummaryDatum> const& summary_data =
			m_entry_list_ctrl->summary_data();
		vector<SummaryDatum>::size_type i = 0;
		vector<SummaryDatum>::size_type const sz = summary_data.size();
		JEWEL_ASSERT (sz == m_summary_label_text_items.size());
		for ( ; i != sz; ++i)
		{
			m_summary_label_text_items[i]->SetLabel
			(	summary_data[i].label() + wxString(":")
			);
			m_summary_data_text_items[i]->SetLabel
			(	finformat_wx
				(	summary_data[i].amount(),
					locale(),
					DecimalFormatFlags().clear(string_flags::dash_for_zero)
				)
			);
		}
	}
	Layout();
	return;
}

Account
EntryListPanel::selected_account() const
{
	JEWEL_ASSERT (m_account_ctrl);
	return m_account_ctrl->account();
}

optional<gregorian::date>
EntryListPanel::selected_min_date() const
{
	return m_min_date_ctrl->date();
}

optional<gregorian::date>
EntryListPanel::selected_max_date() const
{
	return m_max_date_ctrl->date();
}
	


}  // namespace gui
}  // namespace phatbooks
