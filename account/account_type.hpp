#ifndef GUARD_account_type_hpp
#define GUARD_account_type_hpp

#include "b_string.hpp"
#include <vector>

namespace phatbooks
{

namespace account_type
{
	enum AccountType
	{
		// enum order is significant, as the database contains
		// a table with primary keys in this order.
		// Other functions in this file also depend on
		// the exact size and order of this enumeration.
		asset = 1,
		liability,
		equity,
		revenue,
		expense,
		pure_envelope
	};
}  // namespace account_type



/**
 * @todo There should probably be a table in the database listing the
 * AccountSuperTypes. The account_types table should then contain the
 * information about which AccountType maps to which AccountSuperType.
 */
namespace account_super_type
{
	enum AccountSuperType
	{
		balance_sheet,
		pl  			// Profit and loss
	};
}  // namespace account_super_type


account_super_type::AccountSuperType
super_type(account_type::AccountType p_account_type);

std::vector<BString>
account_type_names();

BString
account_type_to_string(account_type::AccountType p_account_type);

/**
 * @throws InvalidAccountTypeStringException if there is no
 * AccountType that corresponds to this string.
 */
account_type::AccountType
string_to_account_type(BString const& p_string);



}  // namespace phatbooks

#endif  // GUARD_account_type_hpp


