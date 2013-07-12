// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_frequency_ctrl_hpp
#define GUARD_frequency_ctrl_hpp

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <wx/combobox.h>
#include <wx/gdicmn.h>
#include <wx/window.h>
#include <wx/windowid.h>

namespace phatbooks
{

// Begin forward declarations

class Frequency;

// End forward declarations

namespace gui
{

/**
 * Widget for the the user to select a Frequency (for a DraftJournal), or
 * else to select no Frequency (for an OrdinaryJournal).
 *
 * @todo HIGH PRIORITY Reflected the selected Frequency in
 * TransactionCtrl::post_journal().
 *
 * @todo HIGH PRIORITY Ensure that the "every month" and "last day of month"
 * options work sensibly in relation to the date entered in the date control.
 */
class FrequencyCtrl: public wxComboBox, private boost::noncopyable
{
public:

	/**
	 * @precondition must pass true to either or both
	 * p_support_ordinary_journal or p_support_draft_journal.
	 */
	FrequencyCtrl
	(	wxWindow* p_parent,
		wxWindowID p_id,
		wxSize const& p_size,
		bool p_support_ordinary_journal = true,
		bool p_support_draft_journal = true
	);

	/**
	 * If no Frequency is selected (i.e. "Once off" or equivalent is selected)
	 * then this returns an uninitialized boost::optional<Frequency>.
	 * If a Frequency is selected, then this returns a
	 * boost::optional<Frequency> that has been initialized with that
	 * Frequency.
	 */
	boost::optional<Frequency> frequency() const;

private:
	bool m_support_ordinary_journal;
	bool m_support_draft_journal;

};  // class FrequencyCtrl




}  // namespace gui
}  // namespace phatbooks

#endif  // GUARD_frequency_ctrl_hpp
