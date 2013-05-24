// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_entry_list_ctrl_hpp
#define GUARD_entry_list_ctrl_hpp


#include "entry_reader.hpp"
#include <wx/listctrl.h>
#include <wx/wx.h>

namespace phatbooks
{

// Begin forward declarations

class Entry;
class OrdinaryJournal;
class PhatbooksDatabaseConnection;

// End forward declarations

namespace gui
{

class EntryListCtrl: public wxListCtrl
{
public:
		
	/**
	 * @returns a pointer to a heap-allocated EntryListCtrl, listing
	 * all and only the \e actual (non-budget) OrdinaryEntry's stored in \e
	 * dbc. The client does not need to take care of the memory - the memory
	 * is taken care of by the parent window.
	 */
	static EntryListCtrl* create_actual_ordinary_entry_list
	(	wxWindow* parent,
		PhatbooksDatabaseConnection& dbc
	);

	/**
	 * Update displayed entries to reflect that a \e journal has been
	 * posted.
	 */
	void update_for_posted_journal(OrdinaryJournal const& journal);

private:

	EntryListCtrl
	(	wxWindow* p_parent,
		EntryReader const& p_reader,
		PhatbooksDatabaseConnection& p_database_connection
	);
	
	void add_entry(Entry const& entry);

	PhatbooksDatabaseConnection& m_database_connection;
};


}  // namespace gui
}  // namespace phatbooks

#endif  // GUARD_entry_list_ctrl_hpp
