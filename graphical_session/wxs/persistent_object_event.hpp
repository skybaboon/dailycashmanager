#ifndef GUARD_persistent_object_event_hpp
#define GUARD_persistent_object_event_hpp

#include "phatbooks_persistent_object.hpp"
#include <boost/optional.hpp>
#include <wx/event.h>
#include <wx/window.h>

namespace phatbooks
{
namespace gui
{

class PersistentObjectEvent: public wxCommandEvent
{
public:
	typedef PhatbooksPersistentObjectBase::Id Id;

	/**
	 * @param p_event_id is the wxWidgets event id.
	 *
	 * @param p_maybe_po_id may contain the id() of the
	 * PhatbooksPersistentObject to which this event pertains.
	 */
	PersistentObjectEvent
	(	wxEventType p_event_type = wxEVT_NULL,
		int p_event_id = 0,
		boost::optional<Id> p_maybe_po_id = boost::optional<Id>()
	);

	PersistentObjectEvent(PersistentObjectEvent const& rhs);

	boost::optional<Id> maybe_po_id() const;

    /**
	 * Required for sending with wxPostEvent().
	 */
    wxEvent* Clone();

	/**
	 * Convenience function to fire a PersistentObjectEvent with which
	 * no existing PhatbooksPersistentObject is associated.
	 */
	static void fire
	(	wxWindow* p_originator,
		wxEventType p_event_type
	);

	/**
	 * Convenience function to fire a PersistentObjectEvent with which
	 * an existing PhatbooksPersistentObject is associated.
	 *
	 * Precondition: \e p_object must have an id.
	 */
	static void fire
	(	wxWindow* p_originator,
		wxEventType p_event_type,
		PhatbooksPersistentObjectBase& p_object
	);

	/**
	 * Convenience function to fire one or more PersistentObjectEvents
	 * notifying that Entries that had ids \e p_doomed_ids, are now deleted.
	 * Use for Entries that were associated with a a DraftJournal.
	 */
	static void notify_doomed_draft_entries
	(	wxWindow* p_originator,
		std::vector<Id> const& p_doomed_ids
	);
	
	/**
	 * Convenience function to fire on or more PersistentObjectEvents
	 * notifying that Entries that has ids \e p_doomed_ids, are now
	 * deleted. Use for Entries that were associated with an OrdinaryJournal.
	 */
	static void notify_doomed_ordinary_entries
	(	wxWindow* p_originator,
		std::vector<Id> const& p_doomed_ids
	);

private:

	static void notify_many
	(	wxWindow* p_originator,
		wxEventType p_event_type,
		std::vector<Id> const& p_po_ids
	);

	boost::optional<Id> m_maybe_po_id;	

	DECLARE_DYNAMIC_CLASS(PersistentObjectEvent)
};


/**
 * Event types associated with PersistentObjectEvent.
 */
BEGIN_DECLARE_EVENT_TYPES()

	/**
	 * Fire to signify that we want the user to be given the opportunity
	 * to create an Account.
	 */
	DECLARE_EVENT_TYPE(PHATBOOKS_ACCOUNT_CREATING_EVENT, -1)

	/**
	 * Fire to signify that we want the user to be given the opportunity
	 * to edit an Account.
	 */
	DECLARE_EVENT_TYPE(PHATBOOKS_ACCOUNT_EDITING_EVENT, -1)

	/**
	 * Fire to signifiy that the user has just created (and saved) an
	 * Account.
	 */
	DECLARE_EVENT_TYPE(PHATBOOKS_ACCOUNT_CREATED_EVENT, -1)

	/**
	 * Fire to signify that the user has just edited an Account
	 * (and saved the changes).
	 */
	DECLARE_EVENT_TYPE(PHATBOOKS_ACCOUNT_EDITED_EVENT, -1)

	/**
	 * Fire to indicated that the user has just deleted an Account.
	 */
	DECLARE_EVENT_TYPE(PHATBOOKS_ACCOUNT_DELETED_EVENT, -1)

	/**
	 * The following are analogous to "_ACCOUNT" event types, but for
	 * PersistentJournals.
	 */
	DECLARE_EVENT_TYPE(PHATBOOKS_JOURNAL_CREATING_EVENT, -1)
	DECLARE_EVENT_TYPE(PHATBOOKS_JOURNAL_EDITING_EVENT, -1)
	DECLARE_EVENT_TYPE(PHATBOOKS_JOURNAL_CREATED_EVENT, -1)
	DECLARE_EVENT_TYPE(PHATBOOKS_JOURNAL_EDITED_EVENT, -1)
	DECLARE_EVENT_TYPE(PHATBOOKS_JOURNAL_DELETED_EVENT, -1)

	/**
	 * And for Entries.
	 */
	DECLARE_EVENT_TYPE(PHATBOOKS_DRAFT_ENTRY_DELETED_EVENT, -1)
	DECLARE_EVENT_TYPE(PHATBOOKS_ORDINARY_ENTRY_DELETED_EVENT, -1)

END_DECLARE_EVENT_TYPES()



// Various boilerplate called for by the wxWidgets event system.

typedef
	void (wxEvtHandler::*PersistentObjectEventFunction)
	(PersistentObjectEvent&);

#define PHATBOOKS_EVT_ACCOUNT_CREATING(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_ACCOUNT_CREATING_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_ACCOUNT_EDITING(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_ACCOUNT_EDITING_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_ACCOUNT_CREATED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_ACCOUNT_CREATED_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_ACCOUNT_EDITED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_ACCOUNT_EDITED_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_ACCOUNT_DELETED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_ACCOUNT_DELETED_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_JOURNAL_CREATING(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_JOURNAL_CREATING_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_JOURNAL_EDITING(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_JOURNAL_EDITING_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_JOURNAL_CREATED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_JOURNAL_CREATED_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_JOURNAL_EDITED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_JOURNAL_EDITED_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_JOURNAL_DELETED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_JOURNAL_DELETED_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_DRAFT_ENTRY_DELETED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_DRAFT_ENTRY_DELETED_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),

#define PHATBOOKS_EVT_ORDINARY_ENTRY_DELETED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( PHATBOOKS_ORDINARY_ENTRY_DELETED_EVENT, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) \
	(wxNotifyEventFunction) \
	wxStaticCastEvent(PersistentObjectEventFunction, &fn), (wxObject*) NULL),





}  // namespace gui
}  // namespace phatbooks

#endif  // GUARD_persistent_object_event_hpp
