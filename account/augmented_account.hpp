#ifndef GUARD_augmented_account_hpp_15782473057407176
#define GUARD_augmented_account_hpp_15782473057407176

#include "account_handle.hpp"
#include "commodity_handle_fwd.hpp"
#include <jewel/decimal.hpp>

namespace phatbooks
{

class PhatbooksDatabaseConnection;

/**
 * Represents an Account (via an AccountHandle), together with an amount
 * representing its opening balance. This is simply a convenient way of
 * grouping these two pieces of information together - hence a
 * struct.
 */
struct AugmentedAccount
{
	AugmentedAccount
	(	PhatbooksDatabaseConnection& p_database_connection,
		CommodityHandle const& p_commodity
	);
	AugmentedAccount
	(	AccountHandle const& p_account,
		jewel::Decimal const& p_technical_opening_balance
	);
	AccountHandle account;
	jewel::Decimal technical_opening_balance;

};  // struct AugmentedAccount

}  // namespace phatbooks

#endif  // GUARD_augmented_account_hpp_15782473057407176