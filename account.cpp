#include "account.hpp"
#include <cassert>
#include <string>
#include <vector>

/** \file account.cpp
 *
 * \brief Source file for code pertaining to Account class.
 *
 * \author Matthew Harvey
 * \date 04 July 2012.
 *
 * Copyright (c) 2012, Matthew Harvey. All rights reserved.
 */


using std::string;
using std::vector;

namespace phatbooks
{

Account::Account
(	string p_name,
	string p_commodity_abbreviation,
	AccountType p_account_type,
	string p_description
):
	m_name(p_name),
	m_commodity_abbreviation(p_commodity_abbreviation),
	m_account_type(p_account_type),
	m_description(p_description)
{
}

Account::AccountType
Account::account_type() const
{
	return m_account_type;
}

string Account::name() const
{
	return m_name;
}

string Account::commodity_abbreviation() const
{
	return m_commodity_abbreviation;
}

string Account::description() const
{
	return m_description;
}



}  // namespace phatbooks
