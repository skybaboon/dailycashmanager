// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_draft_journal_naming_ctrl_hpp
#define GUARD_draft_journal_naming_ctrl_hpp

#include "b_string.hpp"
#include <boost/noncopyable.hpp>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

namespace phatbooks
{

// Begin forward declarations

class PhatbooksDatabaseConnection;

namespace gui
{

class TransactionCtrl;

// End forward declarations


/**
 * Widget for extracting a name from the user, for a DraftJournal created
 * via the GUI.
 */
class DraftJournalNamingCtrl: public wxDialog, private boost::noncopyable
{
public:
	
	DraftJournalNamingCtrl
	(	TransactionCtrl* p_parent,
		PhatbooksDatabaseConnection& p_database_connection
	);

	BString draft_journal_name() const;

private:

	void set_draft_journal_name(BString const& p_name);

	void on_ok_button_click(wxCommandEvent& event);

	wxFlexGridSizer* m_top_sizer;
	wxTextCtrl* m_name_ctrl;
	wxButton* m_ok_button;
	PhatbooksDatabaseConnection& m_database_connection;
	BString m_draft_journal_name;

	DECLARE_EVENT_TABLE()

};  // class DraftJournalNamingCtrl

}  // namespace gui
}  // namespace phatbooks

#endif  // GUARD_draft_journal_naming_ctrl_hpp