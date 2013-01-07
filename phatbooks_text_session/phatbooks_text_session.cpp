#include "phatbooks_text_session.hpp"


/** \file phatbooks_text_session.cpp
 *
 * \brief Source file for text/console user interface code for Phatbooks.
 *
 * \author Matthew Harvey
 * \date 04 July 2012.
 *
 * Copyright (c) 2012, Matthew Harvey. All rights reserved.
 */


#include "account.hpp"
#include "account_reader.hpp"
#include "account_type.hpp"
#include "commodity.hpp"
#include "date.hpp"
#include "draft_journal.hpp"
#include "draft_journal_reader.hpp"
#include "entry.hpp"
#include "entry_reader.hpp"
#include "finformat.hpp"
#include "import_from_nap/import_from_nap.hpp"  // WARNING temp hack
#include "journal.hpp"
#include "ordinary_journal.hpp"
#include "ordinary_journal_reader.hpp"
#include "persistent_journal.hpp"
#include "proto_journal.hpp"
#include "phatbooks_database_connection.hpp"
#include "repeater.hpp"
#include "session.hpp"
#include "consolixx/get_input.hpp"
#include "consolixx/table.hpp"
#include "consolixx/text_session.hpp"
#include <sqloxx/database_connection.hpp>
#include <sqloxx/handle.hpp>
#include <sqloxx/sqloxx_exceptions.hpp>
#include <jewel/debug_log.hpp>
#include <jewel/decimal.hpp>
#include <jewel/decimal_exceptions.hpp>
#include <jewel/optional.hpp>
#include <boost/bimap.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/ref.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <sstream>  // for ostringstream
#include <string>
#include <vector>

// WARNING play code
#include "sqloxx/sql_statement.hpp"
#include <boost/unordered_map.hpp>
#include <jewel/stopwatch.hpp>
#include <vector>
using boost::unordered_map;
using jewel::Stopwatch;
using sqloxx::SQLStatement;
using std::vector;
// end play code

using consolixx::get_date_from_user;
using consolixx::get_user_input;
using consolixx::get_constrained_user_input;
using consolixx::get_decimal_from_user;
using consolixx::Table;
using consolixx::TextSession;
using jewel::Decimal;
using jewel::DecimalRangeException;
using jewel::value;
using sqloxx::DatabaseConnection;
using sqloxx::SQLiteException;
using boost::bad_lexical_cast;
using boost::bimap;
using boost::bind;
using boost::cref;
using boost::lexical_cast;
using boost::optional;
using boost::shared_ptr;
using boost::regex;
using boost::regex_match;
using std::cout;
using std::clog;
using std::endl;
using std::find;
using std::list;
using std::locale;
using std::map;
using std::ostringstream;
using std::string;
using std::vector;

namespace alignment = consolixx::alignment;
namespace gregorian = boost::gregorian;



namespace phatbooks
{




PhatbooksTextSession::PhatbooksTextSession():
	m_main_menu(new Menu)
{
	// Set up main menu.

	shared_ptr<MenuItem> elicit_commodity_item
	(	new MenuItem
		(	"New commodity",
			bind(&PhatbooksTextSession::elicit_commodity, this),
			true
		)
	);
	m_main_menu->add_item(elicit_commodity_item);

	// WARNING Until there is not yet at least one Commodity, this should
	// not appear in the main menu.
	shared_ptr<MenuItem> elicit_account_item
	(	new MenuItem
		(	"New account",
			bind(&PhatbooksTextSession::elicit_account, this),
			true
		)
	);
	m_main_menu->add_item(elicit_account_item);

	// WARNING Until there is at least one Account, this should not appear in
	// the main menu.
	shared_ptr<MenuItem> elicit_journal_item
	(	new MenuItem
		(	"New transaction",
			bind(&PhatbooksTextSession::elicit_journal, this),
			true
		)
	);
	m_main_menu->add_item(elicit_journal_item);

	shared_ptr<MenuItem> display_draft_journals_item
	(	new MenuItem
		(	"Display draft and recurring transactions",
			bind(&PhatbooksTextSession::display_draft_journals, this),
			true
		)
	);
	m_main_menu->add_item(display_draft_journals_item);

	shared_ptr<MenuItem> display_journal_from_id_item
	(	new MenuItem
		(	"Select a transaction by ID",
			bind(&PhatbooksTextSession::display_journal_from_id, this),
			true
		)
	);
	m_main_menu->add_item(display_journal_from_id_item);

	shared_ptr<MenuItem> display_ordinary_actual_entries_item
	(	new MenuItem
		(	"List actual transactions",
			bind
			(	&PhatbooksTextSession::display_ordinary_actual_entries,
				this
			),
			true
		)
	);
	m_main_menu->add_item(display_ordinary_actual_entries_item);

	// TODO Should this also display equity accounts? Do we even have
	// any equity accounts?
	shared_ptr<MenuItem> display_balance_sheet_selection
	(	new MenuItem
		(	"Display the balances of asset and liability accounts",
			bind(&PhatbooksTextSession::display_balance_sheet, this),
			true
		)
	);
	m_main_menu->add_item(display_balance_sheet_selection);

	shared_ptr<MenuItem> display_envelopes_selection
	(	new MenuItem
		(	"Display envelope balances",
			bind(&PhatbooksTextSession::display_envelopes, this),
			true
		)
	);
	m_main_menu->add_item(display_envelopes_selection);
	
	// WARNING This should be removed from any release version
	shared_ptr<MenuItem> import_from_nap_item
	(	new MenuItem
		(	"Import data from csv files",
			bind(&PhatbooksTextSession::import_from_nap, this),
			true
		)
	);
	m_main_menu->add_item(import_from_nap_item);

	shared_ptr<MenuItem> quit_item
	(	new MenuItem
		(	"Quit",
			bind(&PhatbooksTextSession::wrap_up, this),
			false,
			"x"
		)
	);
	m_main_menu->add_item(quit_item);
}


PhatbooksTextSession::~PhatbooksTextSession()
{
}

	

int PhatbooksTextSession::do_run(string const& filename)
{
	boost::filesystem::path filepath(filename);
	if (!boost::filesystem::exists(boost::filesystem::status(filepath)))
	{
		cout << "File does not exist. "
		     << "Create file \"" << filename << "\"? (y/n): ";
		string const response = get_constrained_user_input
		(	boost::lambda::_1 == "y" || boost::lambda::_1 == "n",
			"Try again, entering 'y' to create file, or 'n' to abort: ",
			false
		);
		if (response != "y")
		{
			assert (response == "n");
			cout << "Exiting program." << endl;
			return 0;
		}
		assert (response == "y");
	}
	try
	{
		database_connection().open(filepath);
	}
	catch (SQLiteException&)
	{
		cout << "Could not open file \"" << filename << "\"." << endl;
		return 1;
	}

	cout << "Welcome to " << application_name() << "!" << endl;

	database_connection().setup();
	gregorian::date const today = gregorian::day_clock::local_day();
	shared_ptr<list<OrdinaryJournal> > auto_posted_journals =
		update_repeaters_till(today);
	notify_autoposts(auto_posted_journals);
	m_main_menu->present_to_user();	
	return 0;
}



string
PhatbooksTextSession::elicit_existing_account_name(bool accept_empty)
{
	 while (true)
	 {
		string input = get_user_input();
	 	if
		(	database_connection().has_account_named(input) ||
			(accept_empty && input.empty())
		)
		{
			return input;
		}
		cout << "There is no account named " << input
		     << ". Please try again: ";
	}
}



void PhatbooksTextSession::display_draft_journals()
{
	typedef PhatbooksTextSession PTS;  // For brevity in the below

	// We need this loop here so the menu will update itself
	// if a journal is deleted.
	for (bool exiting = false; !exiting; )
	{
		cout << endl;	
		Menu menu
		(	"Select a transaction to view from the above menu, "
			"or 'x' to exit: "
		);
		DraftJournalReader const dj_reader(database_connection());
		for
		(	DraftJournalReader::const_iterator it = dj_reader.begin();
			it != dj_reader.end();
			++it
		)
		{
			shared_ptr<MenuItem const> const menu_item
			(	new MenuItem
				(	it->name(),
					bind(bind(&PTS::conduct_dj_editing, this, _1), *it)
				)
			);
			menu.add_item(menu_item);
		}
		shared_ptr<MenuItem> exit_item(MenuItem::provide_menu_exit());
		menu.add_item(exit_item);
		menu.present_to_user();
		if (menu.last_choice() == exit_item)
		{
			exiting = true;
		}
	}
	return;
}


void
PhatbooksTextSession::elicit_entry_insertion(PersistentJournal& journal)
{
	Entry entry(database_connection());
	cout << "Enter name of account or category for new transaction line: ";
	TransactionType const transaction_type =
		journal.is_actual()?
		generic_transaction:
		envelope_transaction;
	entry.set_account
	(	value(elicit_valid_account(transaction_type, primary_phase))
	);
	Commodity const commodity = entry.account().commodity();
	Decimal amount;
	// TODO This duplicates code also used in elicit_primary_entries etc.
	for (bool input_is_valid = false; !input_is_valid; )
	{
		cout << "Enter amount "
		     << dialogue_phrase(generic_transaction, amount_prompt)
			 << " (int units of "
			 << commodity.abbreviation()
			 << "): ";
		amount = value(get_decimal_from_user());
		Decimal::places_type const initial_precision = amount.places();
		try
		{
			amount = jewel::round(amount, commodity.precision());
			input_is_valid = true;
			if (amount.places() < initial_precision)
			{
				cout << "Amount rounded to " << amount << "." << endl;
			}
		}
		catch (DecimalRangeException&)
		{
			cout << "The number you entered cannot be safely "
				 << "rounded to the precision required for "
				 << commodity.abbreviation()
				 << ". Please try again."
				 << endl;
			assert (!input_is_valid);
		}
	}
	bool const sign_needs_changing = !journal.is_actual();
	entry.set_amount(sign_needs_changing? -amount: amount);
	cout << "Comment for this line (or Enter for no comment): ";
	entry.set_comment(get_user_input());
	entry.set_whether_reconciled(false);
	journal.add_entry(entry);
	return;
}




namespace
{
	bool
	has_entry_with_id_string
	(	PersistentJournal const& journal,
		string const& str
	)
	{
		try
		{
			Entry::Id const id = lexical_cast<Entry::Id>(str);
			return has_entry_with_id(journal, id);
		}
		catch (bad_lexical_cast&)
		{
			return false;
		}
	}


	optional<Entry> elicit_existing_entry(PersistentJournal const& journal)
	{
		string id_string;
		for
		(	vector<Entry>::const_iterator it = journal.entries().begin(),	
				end = journal.entries().end();
			it != end;
			++it
		)
		{
			Entry::Id const current_id = it->id();
			id_string += lexical_cast<string>(current_id);
			id_string += " ";
		}
		string input = get_constrained_user_input
		(	bind(has_entry_with_id_string, cref(journal), _1),
			"Transaction does not contain a line with this entry id. "
				"Try again, entering one of the following ids:\n" +
				id_string +
				"\nor hit Enter to abort: "
		);
		if (input.empty())
		{
			return optional<Entry>();
		}
		return Entry
		(	journal.database_connection(),
			lexical_cast<Entry::Id>(input)
		);
	}

}  // End anonymous namespace


void
PhatbooksTextSession::elicit_entry_deletion(PersistentJournal& journal)	
{
	cout << "Enter the entry id of the transaction line you wish to delete, "
	     << "or just hit Enter to abort: ";
	optional<Entry> maybe_entry = elicit_existing_entry(journal);
	if (!maybe_entry)
	{
		cout << "Entry deletion aborted" << endl;
		return;
	}
	assert (maybe_entry);
	Entry entry(value(maybe_entry));
	journal.remove_entry(entry);
	cout << "Entry deleted.\n" << endl;
	return;
}



void
PhatbooksTextSession::elicit_entry_amendment(PersistentJournal& journal)
{
	cout << "Enter the entry id of the transaction line you wish to amend, "
	     << "or just hit Enter to abort: ";
	optional<Entry> maybe_entry = elicit_existing_entry(journal);
	if (!maybe_entry)
	{
		cout << "Entry amendment aborted.\n" << endl;
		return;
	}
	assert (maybe_entry);
	Entry entry = value(maybe_entry);

	// Edit account
	cout << "Enter name of new account (or Enter to leave unchanged): ";
	TransactionType const transaction_type =
		journal.is_actual()?
		generic_transaction:
		envelope_transaction;
	optional<Account> maybe_new_account = elicit_valid_account
	(	transaction_type,
		primary_phase,
		true
	);
	if (maybe_new_account)
	{
		entry.set_account(value(maybe_new_account));
	}

	// Edit comment
 	cout << "Enter new comment for this line (or Enter to leave unchanged): ";
	string const new_comment = get_user_input();
	if (!new_comment.empty()) entry.set_comment(new_comment);

	// Edit amount
	for (bool input_is_valid = false; !input_is_valid; )
	{
		cout << "Enter new amount for this line "
		     << "(or Enter to leave unchanged): ";
		optional<Decimal> maybe_new_amount = get_decimal_from_user(true);
		if (!maybe_new_amount)
		{
			input_is_valid = true;
		}
		else
		{
			assert (maybe_new_amount);
			// TODO The below is virtually identical to code used elsewhere
			// in this file.
			// Factor out repeated code to separate function.
			Decimal new_amount = value(maybe_new_amount);
			Decimal::places_type const initial_precision =
				new_amount.places();
			Commodity const commodity = entry.account().commodity();
			try
			{
				new_amount = jewel::round(new_amount, commodity.precision());
				bool const sign_needs_changing = !journal.is_actual();
				entry.set_amount(sign_needs_changing? -new_amount: new_amount);
				input_is_valid = true;
				if (new_amount.places() < initial_precision)
				{
					cout << "Amount rounded to " << new_amount << "." << endl;
				}
			}
			catch (DecimalRangeException&)
			{
				cout << "The number you entered cannot be safely "
					 << "rounded to the precision required for "
					 << commodity.abbreviation()
					 << ". Please try again."
					 << endl;
				assert (!input_is_valid);
			}
		}
	}
	return;
}


void
PhatbooksTextSession::elicit_journal_deletion(PersistentJournal& journal)
{
	cout << "Are you sure you want to delete this entire transaction? (y/n) ";
	string const confirmation = get_constrained_user_input
	(	boost::lambda::_1 == "y" || boost::lambda::_1 == "n",
		"Try again, entering \"y\" to delete or \"n\" to keep: ",
		false
	);
	if (confirmation == "y")
	{
		journal.remove();
		cout << "\nTransaction deleted." << endl << endl;
	}
	else
	{
		assert (confirmation == "n");
		cout << "\nTransaction has not been deleted." << endl << endl;
	}
	return;
}



	
void
PhatbooksTextSession::elicit_comment_amendment
(	PersistentJournal& journal
)
{
	cout << "Enter new comment for this transaction: ";
	journal.set_comment(get_user_input());
	return;
}


void
PhatbooksTextSession::elicit_repeater_insertion(DraftJournal& journal)
{
	// TODO Implement this
	clog << endl << "We're now inside elicit_repeater_insertion." << endl;
	return;
}

void
PhatbooksTextSession::elicit_repeater_deletion(DraftJournal& journal)
{
	// TODO Implement this
	clog << endl << "We're now inside elicit_repeater_deletion." << endl;
	return;
}

void
PhatbooksTextSession::exit_journal_edit_without_saving
(	PersistentJournal& journal
)
{
	journal.ghostify();
	cout << "Changes have been cancelled. Transaction remains as follows: "
	     << endl << endl << journal << endl << endl;
	return;
}

void
PhatbooksTextSession::exit_journal_edit_saving_changes
(	PersistentJournal& journal
)
{
	journal.save();
	cout << "Changes have been saved. Saved transaction is as follows: "
	     << endl << endl << journal << endl << endl;
	return;
}

void
PhatbooksTextSession::elicit_date_amendment
(	OrdinaryJournal& journal
)
{
	// TODO Implement this	
	clog << endl << "We're now inside elicit_date_amendment." << endl;
	return;
}

void
PhatbooksTextSession::conduct_dj_editing(DraftJournal& journal)
{
	// The below is a bit ugly. There are two alternatives to this approach.
	// (1) Make the elicit... functions free-standing functions in an
	// anonymous namespace, that take as parameters
	// a PhatbooksDatabaseConnection& and a Journal&. This would
	// make the binding syntax a bit neater, at the cost of having
	// tonnes of functions in an anonymous namespace, which seems a bit
	// crappy somehow. I would prefer that functions of such seeming
	// importance be "catalogued" in the header, as (private) member
	// functions of PhatbooksTextSession. I'm not sure if this is a very
	// good reason, but it just feels nicer and more maintainable.
	// (2) Have a bunch of crude procedural code by comparing
	// menu.last_choice() to each of the menu items in turn and executing one
	// of the elicit... functions accordingly. This feels clunky and verbose,
	// and sidesteps the Menu class's facility that already implements
	// the general procedure where the user to selects a menu item and the
	// appropriate function is called.
	// (3) Have a comment explaining what's going on.
	// I like (3) best, so here goes.
	
	// What's going on is that where you see this...
	//
	// bind(bind(&PTS::elicit_yadda_yadda, this, _1), journal)
	//
	// ... it just means that if the user selects this menu item,
	// then we will call PhatbooksTextSession::elicit_yadda_yadda(journal).

	typedef shared_ptr<MenuItem const> ItemPtr;
	typedef PhatbooksTextSession PTS;  // For brevity below.

	for (bool exiting = false; !exiting; )
	{
		cout << journal << endl;
		Menu menu("Select an action from the above menu: ");

		ItemPtr add_entry_item
		(	new MenuItem
			(	"Add a line",
				bind(bind(&PTS::elicit_entry_insertion, this, _1), journal)
			)
		);
		menu.add_item(add_entry_item);

		ItemPtr delete_entry_item
		(	new MenuItem
			(	"Delete a line",
				bind(bind(&PTS::elicit_entry_deletion, this, _1), journal)
			)
		);
		menu.add_item(delete_entry_item);

		ItemPtr amend_entry_item
		(	new MenuItem
			(	"Amend a line",
				bind(bind(&PTS::elicit_entry_amendment, this, _1), journal)
			)
		);
		menu.add_item(amend_entry_item);

		ItemPtr amend_comment_item
		(	new MenuItem
			(	"Amend transaction comment",
				bind(bind(&PTS::elicit_comment_amendment, this, _1), journal)
			)
		);
		menu.add_item(amend_comment_item);

		ItemPtr delete_journal_item
		(	new MenuItem
			(	"Delete transaction",
				bind
				(	bind(&PTS::elicit_journal_deletion, this, _1),
					journal
				)
			)
		);
		menu.add_item(delete_journal_item);

		ItemPtr add_repeater_item
		(	new MenuItem
			(	"Add automatic recording cycle",
				bind(bind(&PTS::elicit_repeater_insertion, this, _1), journal)
			)
		);
		menu.add_item(add_repeater_item);

		ItemPtr delete_repeaters_item
		(	new MenuItem
			(	"Disable automatic recording",
				bind(bind(&PTS::elicit_repeater_deletion, this, _1), journal)
			)
		);
		menu.add_item(delete_repeaters_item);

		ItemPtr exit_without_saving_item
		(	new MenuItem
			(	"Exit without saving changes",
				bind
				(	bind(&PTS::exit_journal_edit_without_saving, this, _1),
					journal
				)
			)
		);
		menu.add_item(exit_without_saving_item);

		ItemPtr exit_with_saving_item
		(	new MenuItem
			(	"Save changes and exit",
				bind
				(	bind(&PTS::exit_journal_edit_saving_changes, this, _1),
					journal
				)
			)
		);
		if (journal.is_balanced())
		{
			menu.add_item(exit_with_saving_item);
		}
		else
		{
			assert (!journal.is_balanced());
			cout << "Note transaction is unbalanced. Entries must sum to nil "
				 << "before changes can be saved." << endl << endl;
		}
		menu.present_to_user();	
		if 
		(	(menu.last_choice() == exit_without_saving_item) ||
			(menu.last_choice() == exit_with_saving_item) ||
			(menu.last_choice() == delete_journal_item)
		)
		{
			exiting = true;
		}
		else assert (!exiting);
	}
	return;
}

void
PhatbooksTextSession::conduct_oj_editing(OrdinaryJournal& journal)
{
	// TODO
	// There is much code here that is duplicated with
	// PhatbooksTextSession::conduct_dj_editing(DraftJournal& journal).
	// Factor it out somehow.
	typedef shared_ptr<MenuItem const> ItemPtr;
	typedef PhatbooksTextSession PTS;
	for (bool exiting = false; !exiting; )
	{
		cout << journal << endl;
		Menu menu("Select an action from the above menu: ");

		ItemPtr add_entry_item
		(	new MenuItem
			(	"Add a line",
				bind(bind(&PTS::elicit_entry_insertion, this, _1), journal)
			)
		);
		menu.add_item(add_entry_item);

		ItemPtr delete_entry_item
		(	new MenuItem
			(	"Delete a line",
				bind(bind(&PTS::elicit_entry_deletion, this, _1), journal)
			)
		);
		menu.add_item(delete_entry_item);

		ItemPtr amend_entry_item
		(	new MenuItem
			(	"Amend a line",
				bind(bind(&PTS::elicit_entry_amendment, this, _1), journal)
			)
		);
		menu.add_item(amend_entry_item);

		ItemPtr amend_comment_item
		(	new MenuItem
			(	"Amend transaction comment",
				bind(bind(&PTS::elicit_comment_amendment, this, _1), journal)
			)
		);
		menu.add_item(amend_comment_item);

		ItemPtr amend_date_item
		(	new MenuItem
			(	"Amend journal date",
				bind(bind(&PTS::elicit_date_amendment, this, _1), journal)
			)
		);
		menu.add_item(amend_date_item);

		ItemPtr delete_journal_item
		(	new MenuItem
			(	"Delete transaction",
				bind
				(	bind(&PTS::elicit_journal_deletion, this, _1),
					journal
				)
			)
		);
		menu.add_item(delete_journal_item);

		ItemPtr exit_without_saving_item
		(	new MenuItem
			(	"Exit without saving changes",
				bind
				(	bind(&PTS::exit_journal_edit_without_saving, this, _1),
					journal
				)
			)
		);
		menu.add_item(exit_without_saving_item);

		ItemPtr exit_with_saving_item
		(	new MenuItem
			(	"Save changes and exit",
				bind
				(	bind(&PTS::exit_journal_edit_saving_changes, this, _1),
					journal
				)
			)
		);
		if (journal.is_balanced())
		{
			menu.add_item(exit_with_saving_item);
		}
		else
		{
			assert (!journal.is_balanced());
			cout << "Note transaction is unbalanced. Entries must sum to nil "
				 << "before changes can be saved." << endl << endl;
		}
		menu.present_to_user();	
		if 
		(	(menu.last_choice() == exit_without_saving_item) ||
			(menu.last_choice() == exit_with_saving_item) ||
			(menu.last_choice() == delete_journal_item)
		)
		{
			exiting = true;
		}
		else assert (!exiting);
	}
	return;
}


namespace
{
	// TODO This function is weird and misleading in what it does, and
	// it doesn't really belong here.
	bool identifies_existent_journal
	(	PhatbooksDatabaseConnection* dbc,
		string const& s
	)
	{
		try
		{
			ProtoJournal::Id const id = lexical_cast<ProtoJournal::Id>(s);
			return journal_id_exists(*dbc, id);
		}
		catch (bad_lexical_cast&)
		{
			return false;
		}
	}
}  // End anonymous namespace


void
PhatbooksTextSession::display_journal_from_id()
{
	// The lexical casts are to prevent the insertion of thousands
	// separators in the id numbers
	cout << "Enter the ID of the transaction you want to view ("
	     << lexical_cast<string>(min_journal_id(database_connection()))
		 << "-"
		 << lexical_cast<string>(max_journal_id(database_connection()))
		 << "): ";
	std::string const input = get_constrained_user_input
	(	bind(identifies_existent_journal, &database_connection(), _1),
		"There is no journal with this id. Try again: "
	);
	if (input.empty())
	{
		return;
	}
	// We know this will work
	ProtoJournal::Id const id = lexical_cast<ProtoJournal::Id>(input);
	if (journal_id_is_draft(database_connection(), id))
	{
		DraftJournal dj(database_connection(), id);
		conduct_dj_editing(dj);
	}
	else
	{
		OrdinaryJournal oj(database_connection(), id);
		conduct_oj_editing(oj);
	}
	// TODO In both conduct_..._editing functions,
	// the "save changes" and "exit without saving changes" options are
	// displayed even if the user has not made any changes to the journal.
	// This is potentially confusing for the user.
	return;
}


namespace
{
	void summarise_balance_movement
	(	Account const& account,
		Decimal const& opening_balance,
		Decimal const& closing_balance
	)
	{
		locale const orig_loc = cout.getloc();
		// TODO Are these expressed in terms of the "friendly balances"?
		// Should they be?
		switch (account.account_type())
		{
		case account_type::asset:
		case account_type::liability:
		case account_type::equity:
		case account_type::pure_envelope:
			cout << "Opening balance for "
			     << account.name() << ": "
				 << finformat(opening_balance) << endl;
			cout << "Movement in balance during date range: "
				 << finformat(closing_balance - opening_balance) << endl;
			cout << "Closing balance: " << finformat(closing_balance) << endl;
			break;
		case account_type::expense:
			cout << "Amount spent in period on " << account.name()
			     << ": " << finformat(closing_balance - opening_balance)
				 << endl;
			break;
		case account_type::revenue:
			cout << "Amount earned in period in " << account.name()
			     << ": " << finformat(closing_balance - opening_balance)
				 << endl;
			break;
		default:
			assert (false);
		}
		return;
	}	
}  // End anonymous namespace


void
PhatbooksTextSession::display_ordinary_actual_entries()
{
	// TODO There is probably factor-out-able code between this and the
	// Draft/Ordinary/Journal printing methods.
	
	// TODO I don't think this will work well with
	// account_type::pure_envelope. The user would be surprised by the result.

	// TODO Could the following procedure result in overflow as it may
	// add all and only the ACTUAL entries for a P&L account? How should we
	// deal with this possibility?
	
	// TODO The implementation here is a bit clumsy and slow.
	// Some possible ways of improving it are as follows.
	//
	// (1) Go and rescue the abandoned "filtering Reader" from svn, and
	// use that. That would probably take care of the speed issue; however,
	// I would then need another mechanism for calculating the opening
	// balance of account - which depends on inspecting every entry up
	// to the start date. (Would another Reader-like thing work here?
	// Yeah but I want to minimize my use of SQL-based solutions.)
	//
	// (2) Make date a field of Entry as well as of
	// Journal. This would solve a lot of problems but
	// at the cost of horrible extra complexity inside the EntryImpl and
	// ProtoJournal classes - to make sure the Entry attributes are
	// synched with the corresponding Journal attributes (date and
	// is_actual).
	//
	// (3) A related issue: is there a way to make a non-template
	// version of Entry<...>::journal()? This feels like it might also
	// solve some problems -- it might make the code to retrieve the date
	// and is_actual attributes of an Entry cleanly enough that we don't
	// need to worry about the other stuff... and see below.
	//
	// (4) Could we do something to bulk-load journals fast at the
	// start of the session. The loading method for journals is
	// really inefficient... NOTE But this seems to have been largely
	// solved by the creation of an index over entry(journal_id).

	optional<Account> maybe_account;
	cout << "Enter account name (or leave blank to show all accounts): ";
	string const account_name = elicit_existing_account_name(true);
	if (!account_name.empty())
	{
		maybe_account = Account(database_connection(), account_name);
	}
	else
	{
		assert (!maybe_account);
	}

	// TODO Factor out process of "getting date range from user" into a
	// separate function.
	cout << "Enter start date as an 8-digit number of the form YYYYMMDD, "
	        "or leave blank for no start date: ";
	optional<gregorian::date> const maybe_earliest_date =
		get_date_from_user(true);

	optional<gregorian::date> maybe_latest_date;
	for (bool input_is_valid = false; !input_is_valid; )
	{
		cout << "Enter end date as an 8-digit number of the form YYYYMMDD, "
				"or leave blank for no end date: ";
		maybe_latest_date = get_date_from_user(true);
		if 
		(	!maybe_earliest_date ||
			!maybe_latest_date || 
			(value(maybe_latest_date) >= value(maybe_earliest_date))
		)
		{
			input_is_valid = true;
		}
		else
		{
			assert (maybe_earliest_date);
			assert (maybe_latest_date);
			assert (value(maybe_latest_date) < value(maybe_latest_date));
			cout << "End date cannot be earlier than start date. "
			        "Please try again: ";
			assert (!input_is_valid);
		}
	}

	Decimal opening_balance(0, 0);
	bool const filtering_for_account = (maybe_account? true: false);
	if (account_name.empty()) assert (!filtering_for_account);
	Account::Id const account_id = (maybe_account? maybe_account->id(): 0);
	bool const accumulating_pre_start_date_entries =
	(	filtering_for_account &&
		(maybe_account->account_type() != account_type::revenue) &&
		(maybe_account->account_type() != account_type::expense)
	);

	ActualOrdinaryEntryReader reader(database_connection());
	ActualOrdinaryEntryReader::const_iterator it = reader.begin();
	ActualOrdinaryEntryReader::const_iterator const end = reader.end();

	// Examine pre-start-date entries
	if (maybe_earliest_date)
	{
		for
		(	gregorian::date const earliest = *maybe_earliest_date;
			(it != end) && (it->date() < earliest);
			++it
		)
		{
			if
			(	accumulating_pre_start_date_entries &&
				(it->account().id() == account_id)
			)
			{
				opening_balance += it->amount();
			}
		}
	}

	vector<Entry> table_vec;

	// Examine entries later than or equal to the start date
	Decimal closing_balance = opening_balance;
	for ( ; it != end; ++it)
	{
		if 
		(	maybe_latest_date &&
			(it->date() > *maybe_latest_date)
		)
		{
			break;
		}
		if (filtering_for_account)
		{
			if (it->account().id() == account_id)
			{
				table_vec.push_back(*it);
				closing_balance += it->amount();
			}
		}
		else
		{
			assert (!filtering_for_account);
			table_vec.push_back(*it);
		}
	}

	string const headings[] =
	{ 	"Date",
		"Journal id",
		"Entry id",
		"Account",
		"Comment",
		"Commodity",
		"Amount",
		"Reconciled"
	};
	using alignment::left;
	using alignment::right;
	alignment::Flag const alignments[] =
		{ left, right, right, left, left, left, right, left };
	Table<Entry> const table
	(	table_vec.begin(),
		table_vec.end(),
		make_augmented_ordinary_entry_row,
		vector<string>
		(	headings,
			headings + sizeof(headings) / sizeof(headings[0])
		),
		vector<alignment::Flag>
		(	alignments,
			alignments + sizeof(alignments) / sizeof(alignments[0])
		),
		2
	);
	cout << table << endl;

	if (filtering_for_account)
	{
		assert (maybe_account);
		summarise_balance_movement
		(	*maybe_account,
			opening_balance,
			closing_balance
		);
	}

	return;
}


void
PhatbooksTextSession::elicit_commodity()
{
	Commodity commodity(database_connection());

	// Get abbreviation
	cout << "Enter abbreviation for new commodity: ";
	for (bool input_is_valid = false; !input_is_valid; )
	{
		string input = get_user_input();
		if (input.empty())
		{
			cout << "Abbreviation cannot be blank. Please try again: ";
		}
		else if
		(	database_connection().has_commodity_with_abbreviation(input)
		)
		{
			cout << "A commodity with this abbreviation already exists. "
			     << "Please try again: ";
		}
		else
		{
			input_is_valid = true;
			commodity.set_abbreviation(input);
		}
	}

	// Get commodity name
	cout << "Enter name for new commodity (cannot be blank): ";
	for (bool input_is_valid = false; !input_is_valid; )
	{
		string input = get_user_input();
		if (input.empty())
		{
			cout << "Name cannot be blank. Please try again: ";
		}
		else if (database_connection().has_commodity_named(input))
		{
			cout << "A commodity with this name already exists. "
			     << "Please try a different name: ";
		}
		else
		{
			input_is_valid = true;
			commodity.set_name(input);
		}
	}
		
	// Get description 
	cout << "Enter description for new commodity (or hit enter for no "
	        "description): ";
	commodity.set_description(get_user_input());

	// Get precision 
	cout << "Enter precision required for this commodity "
	     << "(a number from 0 to 6, representing the number of decimal "
		 << "places of precision to the right of the decimal point): ";
	for (bool input_is_valid = false; !input_is_valid; )
	{
		string input = get_user_input();
		if (!regex_match(input, regex("^[0123456]$")))
		{
			cout << "Please try again, entering a number from 0 to 6: ";
		}
		else
		{
			commodity.set_precision(lexical_cast<int>(input));
			input_is_valid = true;
		}
	}
	assert (commodity.precision() >= 0 && commodity.precision() <= 6);

	// Get multiplier to base
	cout << "Enter rate by which this commodity should be multiplied in order"
	     << " to convert it to the base commodity for this entity: ";
	commodity.set_multiplier_to_base(value(get_decimal_from_user()));

	// Confirm with user before creating commodity
	cout << endl << "You have proposed to create the following commodity: "
	     << endl << endl
	     << "Abbreviation: " << commodity.abbreviation() << endl
		 << "Name: " << commodity.name() << endl
		 << "Description: " << commodity.description() << endl
		 << "Precision: " << commodity.precision() << endl
		 << "Conversion rate to base: " << commodity.multiplier_to_base()
		 << endl << endl;
	cout << "Proceed with creating this commodity? (y/n) ";
	string const confirmation = get_constrained_user_input
	(	boost::lambda::_1 == "y" || boost::lambda::_1 == "n",
		"Try again, entering \"y\" to create commodity "
		"or \"n\" to abort: ",
		false
	);
	if (confirmation == "n")
	{
		cout << "Commodity not created." << endl;
	}
	else
	{
		assert (confirmation == "y");
		commodity.save();
		cout << "Commodity created." << endl;
	}
	return;
}



void PhatbooksTextSession::elicit_account()
{

	Account account(database_connection());

	// Get account name
	cout << "Enter a name for the account: ";
	for (bool input_is_valid = false; !input_is_valid; )
	{
		string input = get_user_input();
		if (input.empty())
		{
			cout << "Name cannot be blank. Please try again: ";
		}
		else if (database_connection().has_account_named(input))
		{
			cout << "An account with this name already exists. "
			     << "Please try again: ";
		}
		else
		{
			input_is_valid = true;
			account.set_name(input);
		}
	}

	// Get commodity abbreviation
	cout << "Enter the abbreviation of the commodity that will be the "
	     << "native commodity of this account: ";
	for (bool input_is_valid = false; !input_is_valid; )
	{
		string input = get_user_input();
		if (!database_connection().has_commodity_with_abbreviation(input))
		{
			cout << "There is no commodity with this abbreviation. Please "
			     << "try again: ";
		}
		else
		{
			input_is_valid = true;
			account.set_commodity(Commodity(database_connection(), input));
		}
	}

	// Get account type
	Menu account_type_menu;
	typedef bimap<Account::AccountType, string> bimap_type;
	bimap_type account_type_info = database_connection().account_types();
	for
	(	bimap_type::iterator it = account_type_info.begin();
		it != account_type_info.end();
		++it
	)
	{
		shared_ptr<MenuItem> item(new MenuItem(it->right));
		account_type_menu.add_item(item);
	};
	cout << "What kind of account do you wish to create?" << endl;
	account_type_menu.present_to_user();
	string const account_type_name =
		account_type_menu.last_choice()->banner();
	account.set_account_type(account_type_info.right.at(account_type_name));


	// Get description 
	cout << "Enter description for new account (or hit enter for no "
	        "description): ";
	account.set_description(get_user_input());
	
	// Confirm with user before creating account
	cout << endl << "You have proposed to create the following account: "
	     << endl << endl
	     << "Name: " << account.name() << endl
		 << "Commodity: " << account.commodity().abbreviation() << endl
		 << "Type: " << account_type_name << endl
		 << "Description: " << account.description() << endl
		 << endl;
	cout << "Proceed with creating this account? (y/n) ";
	string const confirmation = get_constrained_user_input
	(	boost::lambda::_1 == "y" || boost::lambda::_1 == "n",
		"Try again, entering \"y\" to create account "
		"or \"n\" to abort: ",
		false
	);
	if (confirmation == "n")
	{
		cout << "Account not created." << endl;
	}
	else
	{
		assert (confirmation == "y");
		account.save();
		cout << "Account created." << endl;
	}
	return;
}



Repeater
PhatbooksTextSession::elicit_repeater()
{
	Repeater repeater(database_connection());
	cout << "\nHow often do you want this transaction to be recorded?"
		 << endl;
	Menu frequency_menu;
	shared_ptr<MenuItem> monthly_day_x
	(	new MenuItem
		(	"Every month, on a given day of the month (except the "
			"29th, 30th or 31st)"
		)
	);
	frequency_menu.add_item(monthly_day_x);
	shared_ptr<MenuItem> monthly_day_last
	(	new MenuItem("Every month, on the last day of the month")
	);
	frequency_menu.add_item(monthly_day_last);
	shared_ptr<MenuItem> N_monthly_day_x
	(	new MenuItem
		(	"Every N months, on a given day of the month (except the "
			"29th, 30th or 31st)"
		)
	);
	frequency_menu.add_item(N_monthly_day_x);
	shared_ptr<MenuItem> N_monthly_day_last
	(	new MenuItem("Every N months, on the last day of the month")
	);
	frequency_menu.add_item(N_monthly_day_last);
	shared_ptr<MenuItem> weekly(new MenuItem("Every week"));
	frequency_menu.add_item(weekly);
	shared_ptr<MenuItem> N_weekly(new MenuItem("Every N weeks"));
	frequency_menu.add_item(N_weekly);
	shared_ptr<MenuItem> daily(new MenuItem("Every day"));
	frequency_menu.add_item(daily);
	shared_ptr<MenuItem> N_daily(new MenuItem("Every N days"));
	frequency_menu.add_item(N_daily);
	frequency_menu.present_to_user();
	shared_ptr<MenuItem const> const choice =
		frequency_menu.last_choice();

	// Determine interval type
	if (choice == monthly_day_x || choice == N_monthly_day_x)
	{
		repeater.set_interval_type(interval_type::months);
	}
	else if
	(	choice == monthly_day_last ||
		choice == N_monthly_day_last
	)
	{
		repeater.set_interval_type(interval_type::month_ends);
	}
	else if (choice == weekly || choice == N_weekly)
	{
		repeater.set_interval_type(interval_type::weeks);
	}
	else if (choice == daily || choice == N_daily)
	{
		repeater.set_interval_type(interval_type::days);
	}

	// Determine interval units
	if
	(	choice == monthly_day_x ||
		choice == monthly_day_last ||
		choice == weekly ||
		choice == daily
	)
	{
		repeater.set_interval_units(1);
	}
	else
	{
		string unit_description;
		if (choice == N_weekly) unit_description = "week";
		else if (choice == N_daily) unit_description = "day";
		else unit_description = "month";
		cout << "Enter the number of "
			 << unit_description
			 << "s between each occurrence of the transaction: ";
		for (bool is_valid = false; !is_valid; )
		{
			string const input = get_user_input();
			if (!regex_match(input, regex("^[0-9]+$")))
			{
			}
			else if (input == "0")
			{
			}
			else
			{
				try
				{
					int const units = lexical_cast<int>(input);
					repeater.set_interval_units(units);
					is_valid = true;
				}
				catch (bad_lexical_cast&)
				{
				}
			}
			if (!is_valid)
			{
				cout << "Try again, entering a number greater "
						"than 0: ";
			}
		}
	}	
	// Determine next posting date
	cout << "Enter the first date on which the transaction will occur"
		 << ", as an eight-digit number of the form YYYYMMDD (or just"
		 << " hit enter for today's date): ";
	optional<gregorian::date> const d = get_date_from_user(true);
	if (d) repeater.set_next_date(*d);
	else repeater.set_next_date(gregorian::day_clock::local_day());
	return repeater;
}


string
PhatbooksTextSession::dialogue_phrase
(	TransactionType transaction_type,
	PhraseType phrase_type
)
{
	typedef string Vocab[num_phrase_types];
	typedef Vocab VocabMap[num_transaction_types];
	static VocabMap const dictionary =
	{	{	"account from which money was spent",
			"spent",
			"expenditure category",
			"expenditure category",
			"categories"
		},
		{	"account into which funds were deposited",
			"earned",
			"revenue category",
			"revenue category",
			"categories"
		},
		{	"destination account",
			"transferred",
			"source account",
			"source account",
			"source accounts"
		},
		{	"envelope you wish to top up",
			"to transfer",
			"envelope from which to source funds",
			"source envelope",
			"source envelopes"
		},
		{	"account or category",
			"amount",
			"account or category",
			"account or category",
			"accounts / categories"
		}
	};
	int const transaction_index = static_cast<int>(transaction_type);
	int const phrase_index = static_cast<int>(phrase_type);
	return dictionary[transaction_index][phrase_index];
}


namespace
{
	typedef bool (*AccountValidator)(Account const& account);

	string validator_description(AccountValidator validator)
	{
		string const ret =
		(	validator == is_asset_or_liability?
			"asset or liability account":
			validator == is_expense?
			"expense category":
			validator == is_revenue?
			"revenue category":
			validator == is_envelope?
			"envelope (revenue, expense or pure envelope)":  // TODO This description sucks
			validator == is_not_pure_envelope?
			"account or category for this transaction":  // TODO This description sucks
			"ERROR"
		);
		assert (ret != "ERROR");
		return ret;
	}

}  // End anonymous namespace


bool
PhatbooksTextSession::account_is_valid
(	TransactionType transaction_type,
	TransactionPhase transaction_phase,
	Account const& account,
	string& validity_description
)
{
	int const transaction_type_index = static_cast<int>(transaction_type);
	int const transaction_phase_index = static_cast<int>(transaction_phase);
	typedef AccountValidator ValidatorArray[num_transaction_phases];
	typedef ValidatorArray ValidatorMatrix[num_transaction_types];
	ValidatorMatrix const validator_matrix =
	{	{ is_asset_or_liability, is_expense },
		{ is_asset_or_liability, is_revenue },
		{ is_asset_or_liability, is_asset_or_liability },
		{ is_envelope, is_envelope },
		{ is_not_pure_envelope, is_not_pure_envelope }
	};
	AccountValidator const validate =
		validator_matrix[transaction_type_index][transaction_phase_index];
	validity_description = validator_description(validate);
	return validate(account);
}


optional<Account>
PhatbooksTextSession::elicit_valid_account
(	TransactionType transaction_type,
	TransactionPhase transaction_phase,
	bool allow_empty_to_escape
)
{
	while (true)
	{
		optional<Account> ret;
		string const account_name = elicit_existing_account_name(true);
		if (allow_empty_to_escape && account_name.empty())
		{
			return ret;
		}
		else if (account_name.empty())
		{
			assert (!allow_empty_to_escape);
			cout << "There is no account with this name. Please try again: ";
			continue;
		}
		assert (!account_name.empty());
		Account const account(database_connection(), account_name);
		string guide;
		if
		(	!account_is_valid
			(	transaction_type,
				transaction_phase,
				account,
				guide
			)
		)
		{
			cout << account.name() << " is not a valid " << guide << ". ";
			cout << "Please try again: ";
		}
		else
		{
			ret = account;
			return ret;
		}
	}
}

	
	
PhatbooksTextSession::TransactionType
PhatbooksTextSession::elicit_transaction_type()
{
	Menu menu;
	shared_ptr<MenuItem> expenditure_selection
	(	new MenuItem("Expenditure transaction")
	);
	menu.add_item(expenditure_selection);
	shared_ptr<MenuItem> revenue_selection
	(	new MenuItem("Revenue transaction")
	);
	menu.add_item(revenue_selection);
	shared_ptr<MenuItem> balance_sheet_selection
	(	new MenuItem("Transfer between assets or liabilities")
	);
	menu.add_item(balance_sheet_selection);
	shared_ptr<MenuItem> envelope_selection
	(	new MenuItem("Transfer between budgeting envelopes")
	);
	menu.add_item(envelope_selection);
	menu.present_to_user();
	shared_ptr<MenuItem const> const selection = menu.last_choice();
	TransactionType const ret =
	(	selection == expenditure_selection? expenditure_transaction:
		selection == revenue_selection? revenue_transaction:
		selection == balance_sheet_selection? balance_sheet_transaction:
		envelope_transaction
	);
	return ret;
}

void
PhatbooksTextSession::elicit_primary_entries
(	ProtoJournal& journal,
	TransactionType transaction_type
)
{
	// TODO Enable multiple (split) primary entries
	Entry entry(database_connection());
	cout << "Enter name of "
	     << dialogue_phrase(transaction_type, account_prompt)
		 << ": ";
	entry.set_account
	(	value(elicit_valid_account(transaction_type, primary_phase))
	);
	Commodity const commodity = entry.account().commodity();
	Decimal amount;
	for (bool input_is_valid = false; !input_is_valid; )
	{
		cout << "Enter amount "
		     << dialogue_phrase(transaction_type, amount_prompt)
			 << " (in units of "
			 << commodity.abbreviation()
			 << "): ";
		amount = value(get_decimal_from_user());
		Decimal::places_type const initial_precision = amount.places();
		try
		{
			amount = jewel::round(amount, commodity.precision());
			input_is_valid = true;
			if (amount.places() < initial_precision)
			{
				cout << "Amount rounded to " << amount << "." << endl;
			}
		}
		catch (DecimalRangeException&)
		{
			cout << "The number you entered cannot be safely "
			     << "rounded to the precision required for "
				 << commodity.abbreviation()
				 << ". Please try again."
				 << endl;
			assert (!input_is_valid);
		}
	}
	bool const sign_needs_changing =
	(	transaction_type == expenditure_transaction ||
		transaction_type == envelope_transaction
	);
	entry.set_amount(sign_needs_changing? -amount: amount);
	cout << "Comment for this line (or Enter for no comment): ";
	entry.set_comment(get_user_input());
	entry.set_whether_reconciled(false);
	journal.add_entry(entry);
	return;
}


void
PhatbooksTextSession::elicit_secondary_entries
(	ProtoJournal& journal,
	TransactionType transaction_type
)
{
	Entry secondary_entry(database_connection());
	bool const sign_needs_changing =
	(	transaction_type != expenditure_transaction &&
		transaction_type != envelope_transaction
	);
	Decimal const initial_friendly_balance =
	(	sign_needs_changing?
		journal.balance():
		-journal.balance()
	);
	cout << "Enter name of "
	     << dialogue_phrase(transaction_type, secondary_account_prompt)
		 << ", or leave blank to split between multiple "
		 << dialogue_phrase(transaction_type, secondary_account_prompt_plural)
		 << ": ";
	Commodity const primary_commodity =
		journal.entries().begin()->account().commodity();
	optional<Account> const account_opt =
		elicit_valid_account(transaction_type, secondary_phase, true);
	if (!account_opt)
	{
		// We have multiple secondary entries (split transaction)
		cout << endl;
		Decimal unmatched_amount = initial_friendly_balance;
		Decimal const zero(0, 0);
		for (int i = 1; unmatched_amount != zero; ++i)
		{
			Entry current_entry(database_connection());
			cout << "Enter name of "
			     << dialogue_phrase
				 	(	transaction_type,
						secondary_account_prompt_simple
					)
				<< " no. " << i << ": ";
			current_entry.set_account
			(	value(elicit_valid_account(transaction_type, secondary_phase))
			);
			Commodity const current_commodity
				= current_entry.account().commodity();
			if (current_commodity.id() != primary_commodity.id())
			{
				// TODO Deal with this!
				JEWEL_DEBUG_LOG << "Here's where we're supposed to deal with"
				                << " diverse commodities..." << endl;
			}
			// TODO Remove code duplication between here and
			// elicit_primary_entry
			Decimal current_entry_amount(0, 0);
			for (bool input_is_valid = false; !input_is_valid; )
			{
				cout << "Amount remaining to split: "
				     << current_commodity.abbreviation()
					 << " " << unmatched_amount << endl;
				cout << "Enter amount for this line: ";
				current_entry_amount = value(get_decimal_from_user());
				Decimal::places_type const entered_precision =
					current_entry_amount.places();
				try
				{
					current_entry_amount = jewel::round
					(	current_entry_amount, current_commodity.precision()
					);
					if (current_entry_amount.places() < entered_precision)
					{
						cout << "Amount rounded to " << current_entry_amount
						     << ". " << endl;
					}
					if (current_entry_amount > unmatched_amount)
					{
						cout << "You have entered an amount greater than "
						     << "what remains to be split. Please try again."
							 << endl;
					}
					else
					{
						unmatched_amount -= current_entry_amount;
						input_is_valid = true;
					}
				}
				catch (DecimalRangeException&)
				{
					cout << "The number you entered cannot be safely"
					     << " rounded to the precision required for "
						 << current_commodity.abbreviation()
						 << ". Please try again."
						 << endl;
					assert (!input_is_valid);
				}
			}
			current_entry.set_amount
			(	sign_needs_changing?
				-current_entry_amount:
				current_entry_amount
			);
			cout << "Comment for this line (or Enter for no comment): ";
			current_entry.set_comment((get_user_input()));
			current_entry.set_whether_reconciled(false);
			journal.add_entry(current_entry);
			cout << endl;
		}
	}
	else
	{
		assert (account_opt);
		secondary_entry.set_account(value(account_opt));
		// WARNING if secondary account is in a different currency then we
		// need to deal with this here somehow.
		Commodity const secondary_commodity =
			secondary_entry.account().commodity();
		if
		(	secondary_commodity.id() != primary_commodity.id()
		)
		{
			// TODO Deal with this.
			JEWEL_DEBUG_LOG << "Here's where we're supposed to respond to "
							<< "diverse commodities..." << endl;
		}
		cout << "Line specific comment (or Enter for no comment): ";
		secondary_entry.set_comment((get_user_input()));
		secondary_entry.set_amount(-journal.balance());
		secondary_entry.set_whether_reconciled(false);
		journal.add_entry(secondary_entry);
	}
	return;
}


void
PhatbooksTextSession::finalize_ordinary_journal(OrdinaryJournal& journal)
{
	gregorian::date d = gregorian::day_clock::local_day();
	cout << "Enter transaction date as an eight-digit number of the "
		 << "form YYYYMMDD, or just hit enter for today's date ("
		 << gregorian::to_iso_string(d)
		 << "): ";

	optional<gregorian::date> const date_input = get_date_from_user(true);
	if (date_input) d = *date_input;
	journal.set_date(d);
	journal.save();
	cout << "\nTransaction recorded:" << endl << endl
		 << journal << endl;
	return;
}


void
PhatbooksTextSession::finalize_draft_journal
(	DraftJournal& journal,
	bool autopost
)
{
	cout << "Enter a name for the "
	     << (autopost? "recurring": "draft")
		 << " transaction: ";
	for (bool is_valid = false; is_valid != true; )
	{
		string name = get_user_input();
		if (name.empty())
		{
			cout << "Name cannot be blank. Please try again: ";
		}
		else if (database_connection().has_draft_journal_named(name))
		{
			cout << "A draft or recurring transaction has already "
				 << "been saved under this name. Please enter a "
				 << "another name: ";
		}
		else
		{
			journal.set_name(name);
			is_valid = true;
		}
	}
	// TODO Allow for multiple repeaters
	if (autopost)
	{
		Repeater repeater = elicit_repeater();
		journal.add_repeater(repeater);
	}
	journal.save();
	cout << "Draft journal has been saved:" << endl << endl
		 << journal << endl;
	return;
}


void
PhatbooksTextSession::finalize_journal(ProtoJournal& journal)
{
	cout << endl << journal << endl << endl;
	shared_ptr<MenuItem> post(new MenuItem("Record transaction"));
	shared_ptr<MenuItem> save_draft
	(	new MenuItem("Save as a draft to return and complete later")
	);
	shared_ptr<MenuItem> save_recurring
	(	new MenuItem("Save as a recurring transaction")
	);
	shared_ptr<MenuItem> abandon
	(	new MenuItem("Abandon transaction without saving")
	);
	Menu journal_action_menu;
	journal_action_menu.add_item(post);
	journal_action_menu.add_item(save_draft);
	journal_action_menu.add_item(save_recurring);
	journal_action_menu.add_item(abandon);
	journal_action_menu.present_to_user();
	shared_ptr<MenuItem const> journal_action =
		journal_action_menu.last_choice();

	if (journal_action == post)
	{
		OrdinaryJournal oj(database_connection());
		oj.mimic(journal);
		finalize_ordinary_journal(oj);
	}
	else if (journal_action == save_draft)
	{
		DraftJournal dj(database_connection());
		dj.mimic(journal);
		finalize_draft_journal(dj, false);
	}
	else if (journal_action == save_recurring)
	{
		DraftJournal dj(database_connection());
		dj.mimic(journal);
		finalize_draft_journal(dj, true);
	}
	else if (journal_action == abandon)
	{
		cout << "\nTransaction has not been posted or saved." << endl;
	}
	else
	{
		// Execution should not reach here.
		assert (false);
	}
	return;

	// WARNING
	// We also need to ensure the journal either
	// balances or is to be a draft journal. If it's draft, we need to create
	// Repeater objects if required. Before posting, we need to ensure
	// the entries will not overflow the account balances.
	// If the journal is a draft, it should also be given a unique name by the
	// user.
	// Note there are complications when a single Journal involves multiple
	// commodities.
}



void
PhatbooksTextSession::elicit_journal()
{
	// TODO The user should be prevented from entering a non-balance-sheet
	// account where a balance sheet is expected; prevented from entering
	// a pure envelope account where a P&L account is expected; etc..
	// Make this so.
	ProtoJournal journal;
	TransactionType const transaction_type = elicit_transaction_type();
	journal.set_whether_actual(transaction_type != envelope_transaction);
	cout << "Enter a comment describing the transaction (or Enter to "
	        "leave blank): ";
	journal.set_comment(get_user_input());
	cout << endl;
	elicit_primary_entries(journal, transaction_type);
	cout << endl;
	elicit_secondary_entries(journal, transaction_type);
	cout << endl << "Completed transaction is as follows:" << endl;
	finalize_journal(journal);
	return;
}


void PhatbooksTextSession::import_from_nap()
{
	cout << "Directory containing csv files: ";
	boost::filesystem::path directory(get_user_input());
	if (!boost::filesystem::exists(boost::filesystem::status(directory)))
	{
		cout << "There is no directory with this filepath." << endl;
	}
	else
	{
		phatbooks::import_from_nap
		(	database_connection(),
			directory
		);
		cout << "Import complete." << endl;
	}
	return;
}



void PhatbooksTextSession::notify_autoposts
(	shared_ptr<list<OrdinaryJournal> > journals
) const
{
	if (!journals->empty())
	{
		cout << "The following journals have been posted automatically since "
			 << "the last session:"
			 << endl << endl;
		for
		(	list<OrdinaryJournal>::const_iterator it = journals->begin(),
				end = journals->end();
			it != end;
			++it
		)
		{
			cout << *it << endl << endl;
		}
	}
	return;
}


namespace
{
	template <typename AccountReaderT>
	void print_account_reader(AccountReaderT& p_reader)
	{
		vector<string> headings;
		headings.push_back("ACCOUNT");
		headings.push_back("BALANCE ");
		vector<alignment::Flag> alignments;
		alignments.push_back(alignment::left);
		alignments.push_back(alignment::right);
		Table<Account> const table
		(	p_reader.begin(),
			p_reader.end(),
			make_account_row,
			headings,
			alignments
		);
		cout << table;
		return;
	}
}  // End anonymous namespace


void PhatbooksTextSession::display_balance_sheet()
{
	// TODO Locale reversion is not exception-safe here.
	BalanceSheetAccountReader bs_reader(database_connection());
	locale const orig_loc = cout.getloc();
	cout.imbue(locale(""));
	cout << endl << endl;
	cout << "BALANCE SHEET: " << endl << endl;
	print_account_reader(bs_reader);
	cout.imbue(orig_loc);
	return;
}

void PhatbooksTextSession::display_envelopes()
{
	// TODO Locale reversion is not exception-safe here.
	PLAccountReader pl_reader(database_connection());
	locale const orig_loc = cout.getloc();
	cout.imbue(locale(""));
	cout << endl << endl;
	cout << "ENVELOPES: " << endl << endl;
	print_account_reader(pl_reader);
	cout.imbue(orig_loc);
	return;
}



void PhatbooksTextSession::wrap_up()
{
	return;
}


}  // namespace phatbooks
