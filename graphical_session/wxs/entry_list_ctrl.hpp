// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_entry_list_ctrl_hpp
#define GUARD_entry_list_ctrl_hpp


#include "entry_reader.hpp"
#include "entry.hpp"
#include <wx/gdicmn.h>
#include <wx/listctrl.h>
#include <boost/unordered_map.hpp>
#include <vector>

namespace phatbooks
{

// Begin forward declarations

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
	 * Update displayed entries to reflect that a \e p_journal has been newly
	 * posted (having not previously existed in the database).
	 */
	void update_for_new(OrdinaryJournal const& p_journal);

	/**
	 * Update displayed entries to reflect that an already-saved
	 * OrdinaryJournal p_journal has just been amended, and the amendments
	 * saved.
	 *
	 * @todo Implement this.
	 */
	void update_for_amended(OrdinaryJournal const& p_journal);

	/**
	 * Update displayed entries to reflect that the Entries with IDs
	 * (as in Entry::id()) in p_doomed_ids have been deleted from the
	 * database.
	 */
	void update_for_deleted(std::vector<Entry::Id> const& p_doomed_ids);

	/**
	 * Populates \e out with the currently selected Entries (if any).
	 */
	void selected_entries(std::vector<Entry>& out);

private:

	EntryListCtrl
	(	wxWindow* p_parent,
		EntryReader const& p_reader,
		PhatbooksDatabaseConnection& p_database_connection
	);
	
	/**
	 * @param entry must be an Entry with an id.
	 *
	 * @todo This doesn't take care of sorting by date.
	 */
	void add_entry(Entry const& entry);

	/**
	 * Map from Entry ids to wxWidgets-managed identifiers in the widget.
	 */
	typedef boost::unordered_map<Entry::Id, long> IndexMap;
	IndexMap m_index_map;

	PhatbooksDatabaseConnection& m_database_connection;
};


}  // namespace gui
}  // namespace phatbooks

#endif  // GUARD_entry_list_ctrl_hpp
