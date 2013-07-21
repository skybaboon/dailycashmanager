// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "entry_list_ctrl.hpp"
#include "account.hpp"
#include "app.hpp"
#include "b_string.hpp"
#include "date.hpp"
#include "entry.hpp"
#include "entry_reader.hpp"
#include "locale.hpp"
#include "ordinary_journal.hpp"
#include "phatbooks_database_connection.hpp"
#include "unfiltered_entry_list_ctrl.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <jewel/on_windows.hpp>
#include <jewel/optional.hpp>
#include <wx/gdicmn.h>
#include <wx/progdlg.h>
#include <wx/scrolwin.h>
#include <vector>
#include <string>

using boost::lexical_cast;
using boost::optional;
using jewel::value;
using std::string;
using std::vector;

// For debugging
	#include <jewel/debug_log.hpp>
	#include <iostream>
	using std::endl;

namespace gregorian = boost::gregorian;

namespace phatbooks
{
namespace gui
{

EntryListCtrl::EntryListCtrl
(	wxWindow* p_parent,
	wxSize const& p_size,
	PhatbooksDatabaseConnection
):
	wxListCtrl
	(	p_parent,
		wxID_ANY,
		wxDefaultPosition,
		p_size,
		wxLC_REPORT | wxFULL_REPAINT_ON_RESIZE
	),
	m_database_connection(p_database_connection)
{
}

EntryListCtrl*
EntryListCtrl::create_actual_ordinary_entry_list
(	wxWindow* p_parent,
	wxSize const& p_size,
	PhatbooksDatabaseConnection& p_database_connection
)
{
	EntryListCtrl* ret = new UnfilteredEntryListCtrl
	(	p_parent,
		p_size,
		p_database_connection
	);
	initialize(ret);
	return ret;
}

EntryListCtrl*
EntryListCtrl::create_actual_ordinary_entry_list
(	wxWindow* p_parent,
	wxSize const& p_size,
	Account const& p_account,
	optional<gregorian::date> const& p_maybe_min_date,
	optional<gregorian::date> const& p_maybe_max_date
)
{
	EntryListCtrl* ret = new EntryListCtrl
	(	p_parent,
		p_size,
		p_account,
		p_maybe_min_date,
		p_maybe_max_date
	);
	initialize(ret);
	return ret;
}

void
EntryListCtrl::initialize(EntryListCtrl* p_entry_list_ctrl)
{
	p_entry_list_ctrl->insert_columns();
	p_entry_list_ctrl->populate();
	p_entry_list_ctrl->set_column_widths();
	p_entry_list_ctrl->Fit();
	p_entry_list_ctrl->Layout();
	return;
}

void
EntryListCtrl::insert_columns()
{
	do_insert_columns();
	return;
}

void
EntryListCtrl::populate()
{
	boost::scoped_ptr<EntryReader> const reader(make_entry_reader());
	EntryReader::const_iterator it = reader->begin();
	EntryReader::const_iterator const end = reader->end();
	if (do_require_progress_log())
	{
		EntryReader::size_type i = 0;
		EntryReader::size_type progress = 0;
		EntryReader::size_type const progress_scaling_factor = 32;
		EntryReader::size_type const progress_max =
			reader.size() / progress_scaling_factor;
		wxProgressDialog progress_dialog
		(	wxEmptyString,
			"Loading transactions...",
			progress_max,
			this,
			wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxRESIZE_BORDER
		);
		for ( ; it != end; ++it, ++i)
		{
			process_candidate_entry(*it);
			if (i % progress_scaling_factor == 0)
			{
				assert (progress <= progress_max);
				progress_dialog.Update(progress);'
				++progress;
			}
		}
		progress_dialog.Destroy();
	}
	else
	{
		for ( ; it != end; ++it)
		{
			process_candidate_entry(*it);
		}
	}
	return;
}

void
EntryListCtrl::set_column_widths()
{
	do_set_column_widths();
	return;
}

void
EntryListCtrl::process_candidate_entry(Entry const& p_entry)
{
	assert (entry.has_id());
	if (do_approve_entry(p_entry))
	{
		do_push_entry(p_entry);
		m_id_set.insert(entry.id());
	}
	return;
}


EntryListCtrl::~EntryListCtrl()
{
}

// WARNING OLD STUFF BELOW

void
EntryListCtrl::insert_columns()
{
	InsertColumn(date_col_num(), "Date", wxLIST_FORMAT_RIGHT);
	if (!filtering_for_account())
	{
		InsertColumn(account_col_num(), "Account", wxLIST_FORMAT_LEFT);
	}
	InsertColumn(comment_col_num(), "Memo", wxLIST_FORMAT_LEFT);
	InsertColumn(amount_col_num(), "Amount", wxLIST_FORMAT_RIGHT);
	if (showing_reconciled_column())
	{
		InsertColumn(reconciled_col_num(), "R", wxLIST_FORMAT_LEFT);
	}
	return;
}

void
EntryListCtrl::set_column_widths()
{
	// We arrange the widths so that
	// the Account column (if present) takes up just enough size for the
	// Account name - up to a reasonable maximum - the other columns take up
	// just enough room for their contents, and then the comment column
	// is sized such that the total width of all columns occupies exactly
	// the full width of the available area.
	int const num_cols = num_columns();
	for (int j = 0; j != num_cols; ++j)
	{
		SetColumnWidth(j, wxLIST_AUTOSIZE);
	}
	int const max_account_col_width = 200;
	if (!filtering_for_account())
	{
		if (GetColumnWidth(account_col_num()) > max_account_col_width)
		{
			SetColumnWidth(account_col_num(), max_account_col_width);
		}
	}
	int total_widths = 0;
	for (int j = 0; j != num_cols; ++j)
	{
		total_widths += GetColumnWidth(j);
	}

	// TODO Make this more precise
	int const scrollbar_width_allowance = 50;

	int const shortfall =
		GetSize().GetWidth() - total_widths - scrollbar_width_allowance;
	int const current_comment_width = GetColumnWidth(comment_col_num());
	SetColumnWidth(comment_col_num(), current_comment_width + shortfall);
	return;
}

bool
EntryListCtrl::would_accept_entry(Entry const& p_entry) const
{
	if (filtering_for_account())
	{
		assert (m_maybe_account);
		if (p_entry.account() != value(m_maybe_account))
		{
			return false;
		}
	}
	return lies_within(p_entry.date(), m_min_date, m_maybe_max_date);
}

void
EntryListCtrl::add_entry(Entry const& entry)
{
	assert (entry.has_id());  // assert precondition
	assert (would_accept_entry(entry));

	OrdinaryJournal const journal(entry.journal<OrdinaryJournal>());
	wxString const wx_date_string = date_format_wx(journal.date());
	wxString const comment_string = bstring_to_wx(entry.comment());
	wxString const amount_string =
		finformat_wx(entry.amount(), locale(), false);

	// TODO Should have a tick icon here rather than a "Y".
	wxString const reconciled_string = (entry.is_reconciled()? "Y": "N");

	long const i = GetItemCount();

	// Populate 0th column
	assert (date_col_num() == 0);
	InsertItem(i, wx_date_string);

	// The item may change position due to e.g. sorting, so store the
	// Entry ID in the item's data
	// TODO Do a static assert to ensure second param will fit the id.
	assert (entry.has_id());
	SetItemData(i, entry.id());
	m_id_set.insert(entry.id());

	// Populate the other columns
	if (!filtering_for_account())
	{
		wxString const account_string = bstring_to_wx(entry.account().name());
		SetItem(i, account_col_num(), account_string);
	}
	SetItem(i, comment_col_num(), comment_string);
	SetItem(i, amount_col_num(), amount_string);
	if (showing_reconciled_column())
	{
		SetItem(i, reconciled_col_num(), reconciled_string);
	}
	return;
}

bool
EntryListCtrl::filtering_for_account() const
{
	return static_cast<bool>(m_maybe_account);
}

bool
EntryListCtrl::showing_reconciled_column() const
{
	if (!filtering_for_account())
	{
		return true;
	}
	assert (m_maybe_account);
	return
		super_type(value(m_maybe_account).account_type()) ==
		account_super_type::balance_sheet;
}

int
EntryListCtrl::date_col_num() const
{
	return 0;
}

int
EntryListCtrl::account_col_num() const
{
	assert (!filtering_for_account());
	return 1;
}

int
EntryListCtrl::comment_col_num() const
{
	return filtering_for_account()? 1: 2;
}

int
EntryListCtrl::amount_col_num() const
{
	return filtering_for_account()? 2: 3;
}

int
EntryListCtrl::reconciled_col_num() const
{
	assert (showing_reconciled_column());
	return filtering_for_account()? 3: 4;
}

int
EntryListCtrl::num_columns() const
{
	int ret = 4;
	if (filtering_for_account()) --ret;
	if (showing_reconciled_column()) ++ret;
	return ret;
}

void
EntryListCtrl::update_for_new(OrdinaryJournal const& p_journal)
{
	if (p_journal.is_actual())
	{
		vector<Entry>::const_iterator it = p_journal.entries().begin();
		vector<Entry>::const_iterator const end = p_journal.entries().end();
		for ( ; it != end; ++it)
		{
			if (would_accept_entry(*it)) add_entry(*it);
		}
	}
	set_column_widths();
	return;
}

void
EntryListCtrl::update_for_amended(OrdinaryJournal const& p_journal)
{
	if (!p_journal.is_actual())
	{
		return;
	}
	assert (p_journal.is_actual());
	vector<Entry>::const_iterator it = p_journal.entries().begin();
	vector<Entry>::const_iterator const end = p_journal.entries().end();
	wxString const wx_date_string = date_format_wx(p_journal.date());
	for ( ; it != end; ++it)
	{
		Entry::Id const id = it->id();
		IdSet::const_iterator const jt = m_id_set.find(id);
		if (jt == m_id_set.end())
		{
			// Entry not yet displayed.
			if (would_accept_entry(*it))
			{
				add_entry(*it);
			}
		}
		else
		{
			// Entry is displayed
			long const pos = FindItem(-1, id);
			assert (GetItemData(pos) == static_cast<unsigned long>(it->id()));
			if (!would_accept_entry(*it))
			{
				// Things have changed such that the Entry should no longer be
				// included in the display.
				DeleteItem(pos);
				assert (jt != m_id_set.end());
				m_id_set.erase(jt);
			}
			else
			{
				// Update the row for this Entry to match the current
				// state of the Entry.
				SetItemText(pos, wx_date_string);
				if (!filtering_for_account())
				{
					SetItem
					(	pos,
						account_col_num(),
						bstring_to_wx(it->account().name())
					);
				}
				SetItem
				(	pos,
					comment_col_num(),
					bstring_to_wx(it->comment())
				);
				SetItem
				(	pos,
					amount_col_num(),
					finformat_wx(it->amount(), locale(), false)
				);
				if (showing_reconciled_column())
				{
					SetItem
					(	pos,
						reconciled_col_num(),
						(it->is_reconciled()? "Y": "N")
					);
				}
			}
		}
	}
	set_column_widths();
	return;
}

void
EntryListCtrl::update_for_new(Account const& p_account)
{
	(void)p_account;  // Silence compiler re. unused parameter.
	// Nothing to do.
	return;
}

void
EntryListCtrl::update_for_amended(Account const& p_account)
{
	if (filtering_for_account())
	{
		// Then we are not showing Account name so nothing to do.
		return;
	}
	assert (!filtering_for_account());
	size_t i = 0;
	size_t const lim = GetItemCount();
	wxString const name = bstring_to_wx(p_account.name());
	for ( ; i != lim; ++i)
	{
		Entry const entry(m_database_connection, GetItemData(i));
		if (entry.account() == p_account)
		{
			SetItem(i, account_col_num(), name);
		}
	}
	return;
}

void
EntryListCtrl::update_for_deleted(vector<Entry::Id> const& p_doomed_ids)
{
	vector<Entry::Id>::const_iterator it = p_doomed_ids.begin();
	vector<Entry::Id>::const_iterator const end = p_doomed_ids.end();
	for ( ; it != end; ++it)
	{
		IdSet::iterator jt = m_id_set.find(*it);
		if (jt != m_id_set.end())
		{
			DeleteItem(FindItem(-1, *jt));
			m_id_set.erase(jt);
		}
	}
	return;
}

void
EntryListCtrl::selected_entries(vector<Entry>& out)
{
	size_t i = 0;
	size_t const lim = GetItemCount();
	for ( ; i != lim; ++i)
	{
		if (GetItemState(i, wxLIST_STATE_SELECTED))
		{
			Entry const entry(m_database_connection, GetItemData(i));
			out.push_back(entry);
		}
	}
	return;
}

void
EntryListCtrl::scroll_to_bottom()
{
	int const count = GetItemCount();
	if (count > 0)
	{
		EnsureVisible(count - 1);
	}
	/*
	int const max_range = GetScrollRange(wxVERTICAL);
	int const page_size = GetScrollPageSize(wxVERTICAL);
	Scroll(-1, max_range - page_size);
	*/
	return;
}


}  // namespace gui
}  // namespace phatbooks
