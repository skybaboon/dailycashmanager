#include "persistent_object_event.hpp"
#include "phatbooks_persistent_object.hpp"
#include <boost/optional.hpp>
#include <wx/event.h>

using boost::optional;

namespace phatbooks
{
namespace gui
{

DEFINE_EVENT_TYPE(PHATBOOKS_ACCOUNT_CREATING_EVENT)
DEFINE_EVENT_TYPE(PHATBOOKS_ACCOUNT_EDITING_EVENT)
DEFINE_EVENT_TYPE(PHATBOOKS_ACCOUNT_CREATED_EVENT)
DEFINE_EVENT_TYPE(PHATBOOKS_ACCOUNT_EDITED_EVENT)
DEFINE_EVENT_TYPE(PHATBOOKS_ACCOUNT_DELETED_EVENT)
DEFINE_EVENT_TYPE(PHATBOOKS_JOURNAL_CREATING_EVENT)
DEFINE_EVENT_TYPE(PHATBOOKS_JOURNAL_EDITING_EVENT)
DEFINE_EVENT_TYPE(PHATBOOKS_JOURNAL_CREATED_EVENT)
DEFINE_EVENT_TYPE(PHATBOOKS_JOURNAL_EDITED_EVENT)
DEFINE_EVENT_TYPE(PHATBOOKS_JOURNAL_DELETED_EVENT)

IMPLEMENT_DYNAMIC_CLASS(PersistentObjectEvent, wxCommandEvent)

PersistentObjectEvent::PersistentObjectEvent
(	wxEventType p_event_type,
	int p_event_id,
	boost::optional<Id> p_maybe_po_id
):
	wxCommandEvent(p_event_type, p_event_id),
	m_maybe_po_id(p_maybe_po_id)
{
}

PersistentObjectEvent::PersistentObjectEvent
(	PersistentObjectEvent const& rhs
):
	wxCommandEvent(rhs.GetEventType(), rhs.GetId()),
	m_maybe_po_id(rhs.m_maybe_po_id)
{
}

optional<PersistentObjectEvent::Id>
PersistentObjectEvent::maybe_po_id() const
{
	return m_maybe_po_id;
}

wxEvent*
PersistentObjectEvent::Clone()
{
	return new PersistentObjectEvent(*this);
}

}  // namespace gui
}  // namespace phatbooks