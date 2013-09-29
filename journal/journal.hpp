// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_journal_hpp_6157822681664407
#define GUARD_journal_hpp_6157822681664407

#include "entry_handle.hpp"
#include "phatbooks_persistent_object.hpp"
#include "transaction_type.hpp"
#include <jewel/decimal_fwd.hpp>
#include <sqloxx/general_typedefs.hpp>
#include <wx/string.h>
#include <ostream>
#include <vector>
#include <string>


namespace phatbooks
{

/**
 * Abstract base Journal class.
 *
 * Class to represent accounting journals.
 * An accounting journal will
 * typically comprise two or more accounting entries, plus some
 * "journal level" (as opposed to "entry level") data such as the date.
 *
 * A journal can be ProtoJournal, an OrdinaryJournal or a DraftJournal
 * A ordinary journal
 * has been reflected in the entity's financial state. A DraftJournal
 * has not, but has been saved for possible future reuse. Some
 * DraftJournal instances have got Repeater instances associated with them. A
 * DraftJournal with Repeater instances represents a recurring transaction.
 * A ProtoJournal is a journal in the process of being constructed by the
 * user. It might serve as a "seed" for either an OrdinaryJournal or a
 * DraftJournal.
 *
 * As well the ordinary/draft distinction, there is also a distinction between
 * \e actual and \e budget journals. An actual journal reflects an actual
 * change in the entity's wealth, whether the physical form of the wealth
 * (for example, by transferring between asset classes), or a dimimution
 * or augmentation in wealth (by spending or earning money). In contrast
 * a budget journal is a "conceptual" allocation or reallocation of wealth
 * in regards to the \e planned purpose to which the wealth will be put. For
 * example, allocating $100.00 of one's earnings to planned expenditure on
 * food represents a budget transaction.
 */
class Journal
{
public:

	Journal() = default;
	Journal(Journal const&) = default;
	Journal(Journal&&) = default;
	Journal& operator=(Journal const&) = default;
	Journal& operator=(Journal&&) = default;
	virtual ~Journal() = default;

	void set_transaction_type
	(	TransactionType p_transaction_type
	);
	void set_comment(wxString const& p_comment);

	void push_entry(EntryHandle const& entry);
	void remove_entry(EntryHandle const& entry);
	void clear_entries();

	std::vector<EntryHandle> const& entries() const;
	wxString comment() const;

	bool is_actual() const;

	TransactionType transaction_type() const;

	jewel::Decimal balance() const;

	/**
	 * @returns true if and only if the journal balances, i.e. the total
	 * of the entries is equal to zero.
	 *
	 * @todo Note, thinking a little about this function shows
	 * that all entries in a journal must be expressed in a common currency.
	 * It doesn't make sense to think of entries in a single journal as being
	 * in different currencies. An entry must have its value frozen in time.
	 */
	bool is_balanced() const;

	/**
	 * @returns a Decimal being the sum of the amounts of all the
	 * "destination" Entries in the Journal.
	 */
	jewel::Decimal primary_amount() const;

private:
	virtual std::vector<EntryHandle> const& do_get_entries() const = 0;
	virtual void do_set_transaction_type
	(	TransactionType p_transaction_type
	) = 0;
	virtual void do_set_comment(wxString const& p_comment) = 0;
	virtual void do_push_entry(EntryHandle const& entry) = 0;
	virtual void do_remove_entry(EntryHandle const& entry) = 0;
	virtual void do_clear_entries() = 0;
	virtual wxString do_get_comment() const = 0;
	virtual TransactionType
		do_get_transaction_type() const = 0;
};
	
}  // namespace phatbooks


#endif  // GUARD_journal_hpp_6157822681664407
