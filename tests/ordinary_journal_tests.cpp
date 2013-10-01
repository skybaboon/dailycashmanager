// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "phatbooks_tests_common.hpp"
#include "draft_journal_handle.hpp"
#include "entry_handle.hpp"
#include "ordinary_journal_handle.hpp"
#include "proto_journal.hpp"
#include "phatbooks_exceptions.hpp"
#include "transaction_side.hpp"
#include "transaction_type.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <jewel/log.hpp>
#include <jewel/decimal.hpp>
#include <UnitTest++/UnitTest++.h>
#include <vector>

using boost::gregorian::date;
using jewel::Decimal;
using std::vector;

#include <iostream>
#include <stdexcept>
using std::cerr;
using std::endl;

namespace phatbooks
{
namespace test
{


TEST_FIXTURE(TestFixture, test_ordinary_journal_mimic)
{
	PhatbooksDatabaseConnection& dbc = *pdbc;

	ProtoJournal journal1;
	journal1.set_transaction_type(TransactionType::generic);
	journal1.set_comment("igloo");

	EntryHandle entry1a(dbc);
	entry1a->set_account(AccountHandle(dbc, Account::id_for_name(dbc, "cash")));
	entry1a->set_comment("igloo entry a");
	entry1a->set_whether_reconciled(true);
	entry1a->set_amount(Decimal("0.99"));
	entry1a->set_transaction_side(TransactionSide::source);
	journal1.push_entry(entry1a);

	EntryHandle entry1b(dbc);
	entry1b->set_account(AccountHandle(dbc, Account::id_for_name(dbc, "food")));
	entry1b->set_comment("igloo entry b");
	entry1b->set_whether_reconciled(false);
	entry1b->set_amount(Decimal("-0.99"));
	entry1b->set_transaction_side(TransactionSide::destination);
	journal1.push_entry(entry1b);
	OrdinaryJournalHandle oj1(dbc);
	oj1->set_date(date(3000, 1, 5));
	oj1->mimic(journal1);
	CHECK_EQUAL(oj1->date(), date(3000, 1, 5));
	CHECK_EQUAL(oj1->is_actual(), true);
	CHECK_EQUAL(oj1->comment(), "igloo");
	CHECK_EQUAL(oj1->entries().size(), size_t(2));
	CHECK
	(	oj1->transaction_type() ==
		TransactionType::generic
	);
	oj1->save();
	CHECK(!oj1->entries().empty());

	for (EntryHandle const& entry: oj1->entries())
	{
		CHECK(entry->id() == 1 || entry->id() == 2);
		if (entry->account() == AccountHandle(dbc, Account::id_for_name(dbc, "cash")))
		{
			CHECK_EQUAL(entry->comment(), "igloo entry a");
			CHECK_EQUAL(entry->amount(), Decimal("0.99"));
			CHECK_EQUAL(entry->is_reconciled(), true);
			CHECK(entry->transaction_side() == TransactionSide::source);
		}
		else
		{
			CHECK(entry->account() == AccountHandle(dbc, Account::id_for_name(dbc, "food")));
			CHECK_EQUAL(entry->is_reconciled(), false);
			CHECK_EQUAL(entry->amount(), Decimal("-0.99"));
			CHECK_EQUAL(entry->comment(), "igloo entry b");
			CHECK
			(	entry->transaction_side() ==
				TransactionSide::destination
			);
		}
	}
	DraftJournalHandle dj2(dbc);
	dj2->set_transaction_type(TransactionType::envelope);
	dj2->set_comment("steam engine");
	dj2->set_name("some journal");
	
	EntryHandle entry2a(dbc);
	entry2a->set_account(AccountHandle(dbc, Account::id_for_name(dbc, "food")));
	entry2a->set_comment("steam");
	entry2a->set_amount(Decimal("0"));
	entry2a->set_whether_reconciled(false);
	entry2a->set_transaction_side(TransactionSide::source);
	dj2->push_entry(entry2a);
	
	oj1->mimic(*dj2);

	CHECK_EQUAL(oj1->is_actual(), false);
	CHECK_EQUAL(oj1->comment(), "steam engine");
	CHECK_EQUAL(oj1->entries().size(), size_t(1));
	CHECK_EQUAL(oj1->date(), date(3000, 1, 5));
	CHECK
	(	oj1->transaction_type() ==
		TransactionType::envelope
	);
	oj1->save();
	for (EntryHandle const& entry: oj1->entries())
	{
		CHECK_EQUAL(entry->account()->id(), AccountHandle(dbc, Account::id_for_name(dbc, "food"))->id());
		CHECK_EQUAL(entry->comment(), "steam");
		CHECK_EQUAL(entry->is_reconciled(), false);
		CHECK(entry->transaction_side() == TransactionSide::source);
	}
}


TEST_FIXTURE(TestFixture, test_ordinary_journal_is_balanced)
{
	PhatbooksDatabaseConnection& dbc = *pdbc;

	OrdinaryJournalHandle journal1(dbc);
	journal1->set_transaction_type(TransactionType::generic);
	journal1->set_comment("igloo");

	EntryHandle entry1a(dbc);
	entry1a->set_account(AccountHandle(dbc, Account::id_for_name(dbc, "cash")));
	entry1a->set_comment("igloo entry a");
	entry1a->set_whether_reconciled(true);
	entry1a->set_amount(Decimal("-10.99"));
	entry1a->set_transaction_side(TransactionSide::source);
	journal1->push_entry(entry1a);
	CHECK_THROW(journal1->save(), UnbalancedJournalException);

	EntryHandle entry1b(dbc);
	entry1b->set_account(AccountHandle(dbc, Account::id_for_name(dbc, "cash")));
	entry1b->set_comment("igloo entry b");
	entry1b->set_whether_reconciled(false);
	entry1b->set_amount(Decimal("50.09"));
	entry1b->set_transaction_side(TransactionSide::destination);
	journal1->push_entry(entry1b);
	journal1->set_date(date(3000, 1, 5));

	CHECK(!journal1->is_balanced());
	CHECK_THROW(journal1->save(), UnbalancedJournalException);
	entry1b->set_amount(Decimal("10.99"));
	CHECK(journal1->is_balanced());

	journal1->save();

	// We already have a system journal (the budget instrument) so
	// we expect journal1b to have an id of 2, not 1.
	OrdinaryJournalHandle journal1b(dbc, 2);

	CHECK(journal1b->is_balanced());
	EntryHandle entry1c(dbc);
	entry1c->set_account(AccountHandle(dbc, Account::id_for_name(dbc, "food")));
	entry1c->set_comment("Ummm");
	entry1c->set_whether_reconciled(true);
	entry1c->set_amount(Decimal(0, 0));
	entry1c->set_transaction_side(TransactionSide::destination);
	journal1b->push_entry(entry1c);
	CHECK(journal1b->is_balanced());
	CHECK(journal1->is_balanced());
	journal1b->save();
	
	entry1c->set_amount(Decimal("0.0000001"));
	CHECK_EQUAL(journal1->is_balanced(), false);
	CHECK(!journal1b->is_balanced());
}



}  // namespace test
}  // namespace phatbooks
