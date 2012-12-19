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
#include "commodity.hpp"
#include "date.hpp"
#include "draft_journal.hpp"
#include "entry.hpp"
#include "entry_reader.hpp"
#include "finformat.hpp"
#include "import_from_nap/import_from_nap.hpp"  // WARNING temp hack
#include "journal.hpp"
#include "ordinary_journal.hpp"
#include "ordinary_journal_reader.hpp"
#include "phatbooks_database_connection.hpp"
#include "repeater.hpp"
#include "session.hpp"
#include "consolixx/get_input.hpp"
#include "consolixx/table.hpp"
#include "consolixx/text_session.hpp"
#include "sqloxx/database_connection.hpp"
#include "sqloxx/handle.hpp"
#include "sqloxx/sqloxx_exceptions.hpp"
#include <jewel/debug_log.hpp>
#include <jewel/decimal.hpp>
#include <jewel/decimal_exceptions.hpp>
#include <boost/bimap.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
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
using sqloxx::DatabaseConnection;
using sqloxx::SQLiteException;
using boost::bimap;
using boost::bind;
using boost::lexical_cast;
using boost::shared_ptr;
using boost::regex;
using boost::regex_match;
using std::cout;
using std::endl;
using std::list;
using std::locale;
using std::map;
using std::ostringstream;
using std::string;
using std::vector;

namespace alignment = consolixx::alignment;



namespace phatbooks
{




PhatbooksTextSession::PhatbooksTextSession():
	m_main_menu(new Menu)
{
	


	// Set up all the Menu objects.

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

	// WARNING This should be removed from any release version
	shared_ptr<MenuItem> import_from_nap_item
	(	new MenuItem
		(	"Import data from csv files",
			bind(&PhatbooksTextSession::import_from_nap, this),
			true
		)
	);
	m_main_menu->add_item(import_from_nap_item);

	// WARNING play code

	shared_ptr<MenuItem> display_journal_summaries_selection
	(	new MenuItem
		(	"Display a summary of each journal",
			bind(&PhatbooksTextSession::display_journal_summaries, this),
			true
		)
	);
	m_main_menu->add_item(display_journal_summaries_selection);
	shared_ptr<MenuItem> display_balances_selection
	(	new MenuItem
		(	"Display the balance of each envelope and balance sheet account",
			bind(&PhatbooksTextSession::display_balances, this),
			true
		)
	);
	m_main_menu->add_item(display_balances_selection);
	// WARNING end play code

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


string
PhatbooksTextSession::elicit_existing_account_name()
{
	 while (true)
	 {
		string input = get_user_input();
	 	if (database_connection().has_account_named(input))
		{
			return input;
		}
		cout << "There is no account named " << input
		     << ". Please try again: ";
	}
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

	/*
	// WARNING Hack
	// This causes entries to be loaded.
	OrdinaryEntryReader oe_reader(database_connection());
	ostringstream dummy_stream;
	for
	(	OrdinaryEntryReader::const_iterator it = oe_reader.begin();
		it != oe_reader.end();
		++it
	)
	{
		dummy_stream << it->amount();
	}
	*/
	
	m_main_menu->present_to_user();	
	return 0;
}


void PhatbooksTextSession::elicit_commodity()
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
	commodity.set_multiplier_to_base(get_decimal_from_user());

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


void PhatbooksTextSession::elicit_journal()
{
	Journal journal;

	// Find out what kind of journal this is going to be
	Menu transaction_menu;
	shared_ptr<MenuItem> expenditure_selection
	(	new MenuItem("Expenditure transaction")
	);
	transaction_menu.add_item(expenditure_selection);
	shared_ptr<MenuItem> revenue_selection
	(	new MenuItem("Revenue transaction")
	);
	transaction_menu.add_item(revenue_selection);
	shared_ptr<MenuItem> balance_sheet_transfer_selection
	(	new MenuItem("Transfer between assets or liabilities")
	);
	transaction_menu.add_item(balance_sheet_transfer_selection);
	shared_ptr<MenuItem> envelope_transaction_selection
	(	new MenuItem("Transfer between budgeting envelopes")
	);
	transaction_menu.add_item(envelope_transaction_selection);
	
	
	transaction_menu.present_to_user();
	shared_ptr<MenuItem const> const transaction_type =
		transaction_menu.last_choice();

	// Determine whether journal is actual
	journal.set_whether_actual
	(	transaction_type != envelope_transaction_selection
	);

	// Get journal comment
	cout << "Enter a comment describing the transaction (or Enter to "
	        "leave blank): ";
	journal.set_comment(get_user_input());

	// Set certain "prompt words"
	string account_prompt;
	string amount_prompt;
	string secondary_account_prompt;
	bool primary_sign_needs_changing;
	if (transaction_type == expenditure_selection)
	{
		account_prompt = "account from which money was spent";
		amount_prompt = "spent";
		secondary_account_prompt = "expenditure category";
		primary_sign_needs_changing = true;
	}
	else if (transaction_type == revenue_selection)
	{
		account_prompt = "account into which funds were deposited";
		amount_prompt = "earned";
		secondary_account_prompt = "revenue category";
		primary_sign_needs_changing = false;
	}
	else if (transaction_type == balance_sheet_transfer_selection)
	{
		account_prompt = "destination account";
		amount_prompt = "transferred";
		secondary_account_prompt = "source account";
		primary_sign_needs_changing = false;
	}
	else
	{
		assert (transaction_type == envelope_transaction_selection);
		account_prompt = "envelope you wish to top up";
		amount_prompt = "to transfer";
		secondary_account_prompt = "envelope from which to source funds";
		primary_sign_needs_changing = true;
	}
	
	// Primary entry
	Entry primary_entry(database_connection());

	// Get primary entry account
	cout << "Enter name of " << account_prompt << ": ";
	primary_entry.set_account
	(	Account(database_connection(), elicit_existing_account_name())
	);

	// Get primary entry amount
	Commodity const primary_commodity = primary_entry.account().commodity();
	Decimal primary_entry_amount;
	for (bool input_is_valid = false; !input_is_valid; )
	{
		cout << "Enter amount " << amount_prompt << " (in units of "
			 << primary_commodity.abbreviation() << "): ";
		primary_entry_amount = get_decimal_from_user();
		Decimal::places_type const initial_precision =
			primary_entry_amount.places();
		try
		{
			primary_entry_amount = jewel::round
			(	primary_entry_amount, primary_commodity.precision()
			);
			input_is_valid = true;
			if (primary_entry_amount.places() < initial_precision)
			{
				cout << "Amount rounded to " << primary_entry_amount
				     << "." << endl;
			}
		}
		catch (DecimalRangeException&)
		{
			cout << "The number you entered cannot be safely"
				 << " rounded to the precision required for "
				 << primary_commodity.abbreviation() << ". Please try again."
				 << endl;
			assert (!input_is_valid);
		}
	}
	// Primary entry amount must be changed to the appropriate sign
	// WARNING In theory this might throw.
	primary_entry.set_amount
	(	primary_sign_needs_changing?
		-primary_entry_amount:
		primary_entry_amount
	);

	// Get primary entry comment
	cout << "Line specific comment (or Enter for no comment): ";
	primary_entry.set_comment((get_user_input()));

	// Mark entry as unreconciled
	primary_entry.set_whether_reconciled(false);

	// Add primary entry to journal
	journal.add_entry(primary_entry);

	// Secondary entry
	Entry secondary_entry(database_connection());

	// Get other account and comment
	cout << "Enter name of " << secondary_account_prompt << ": ";
	secondary_entry.set_account
	(	Account(database_connection(), elicit_existing_account_name())
	);
	// WARNING if secondary account is in a different currency then we need to
	// deal with this here somehow.
 
	Commodity secondary_commodity = secondary_entry.account().commodity();
	if
	(	secondary_commodity.id() != primary_commodity.id()
	)
	{
		JEWEL_DEBUG_LOG << "Here's where we're supposed to respond to "
		                << "diverse commodities..." << endl;
	}

	cout << "Line specific comment (or Enter for no comment): ";
	secondary_entry.set_comment((get_user_input()));
	secondary_entry.set_amount(-(primary_entry.amount()));
	secondary_entry.set_whether_reconciled(false);
	journal.add_entry(secondary_entry);

	// WARNING We need to implement split transactions.
	// Find out whether the user wants to post the journal, abandon it,
	// or save it as a draft.
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
		OrdinaryJournal ordinary_journal(journal, database_connection());

		boost::gregorian::date const d =
			boost::gregorian::day_clock::local_day();
		cout << "Enter transaction date as an eight-digit number of the "
		     << "form YYYYMMDD, or just hit enter for today's date ("
			 << boost::gregorian::to_iso_string(d)
			 << "): ";
		boost::gregorian::date const e = get_date_from_user();
		ordinary_journal.set_date(e);
		ordinary_journal.save();
		cout << "\nJournal posted." << endl;
	}
	else if (journal_action == save_draft || journal_action == save_recurring)
	{
		DraftJournal draft_journal(journal, database_connection());

		// Ask for a name for the draft journal
		string prompt =
		(	journal_action == save_draft?
			"Enter a name for the draft transaction: ":
			"Enter a name for the recurring transaction: "
		);
		cout << prompt;
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
				draft_journal.set_name(name);
				is_valid = true;
			}
		}
		// Ask for any repeaters.
		if (journal_action == save_recurring)
		{
			Repeater repeater(database_connection());
			cout << "\nHow often do you want this transaction to be posted? "
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
						catch (boost::bad_lexical_cast&)
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
			repeater.set_next_date(get_date_from_user());

			// Add repeater to draft_journal
			draft_journal.add_repeater(repeater);	
		}
		draft_journal.save();
		cout << "Draft journal has been saved" << endl;
	}
	else if (journal_action == abandon)
	{
		cout << "\nJournal has not been posted or saved." << endl;
	}
	else
	{
		// Execution should not reach here.
		assert (false);
	}

	// WARNING
	// We also need to ensure the journal either
	// balances or is to be a draft journal. If it's draft, we need to create
	// Repeater objects if required. Before posting, we need to ensure
	// the entries will not overflow the account balances.
	// If the journal is a draft, it should also be given a unique name by the
	// user.
	// Note there are complications when a single Journal involves multiple
	// commodities.

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

void PhatbooksTextSession::display_journal_summaries()
{
	cout << "For each ORDINARY journal, here's what's in it. "
	     << endl;
	OrdinaryJournalReader oj_reader(database_connection());
	for
	(	OrdinaryJournalReader::const_iterator it =
			oj_reader.begin(),
			end = oj_reader.end();
		it != end;
		++it
	)
	{
		OrdinaryJournal const journal = *it;
		cout << endl << journal.date() << endl;
		typedef vector<Entry> EntryVec;
		EntryVec::const_iterator jt = journal.entries().begin();
		EntryVec::const_iterator endpoint = journal.entries().end();
		for ( ; jt != endpoint; ++jt)
		{
			Decimal const amount = jt->amount();
			cout << jt->account().name() << "\t"
			     << jt->comment() << "\t"
			     << finformat(amount) << endl;
		}
		cout << endl;
	}
	cout << "Done!" << endl << endl;
	return;
}

namespace
{
	shared_ptr<vector<string> > make_account_row
	(	Account const& account
	)
	{
		shared_ptr<vector<string> > ret(new vector<string>);
		ret->push_back(account.name());
		ret->push_back(finformat(account.friendly_balance()));
		return ret;
	}

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


void PhatbooksTextSession::display_balances()
{
	BalanceSheetAccountReader bs_reader(database_connection());
	PLAccountReader pl_reader(database_connection());
	cout.imbue(locale(""));
	for (int i = 0; i != 2; ++i)
	{
		cout << endl << endl;;
		if (i == 0)
		{
			cout << "BALANCE SHEET: " << endl << endl;
			print_account_reader(bs_reader);
		}
		else
		{
			assert (i == 1);
			cout << "P&L: " << endl << endl;
			print_account_reader(pl_reader);
		}
	}
	return;
}



// WARNING end play code

void PhatbooksTextSession::wrap_up()
{
	return;
}


}  // namespace phatbooks
