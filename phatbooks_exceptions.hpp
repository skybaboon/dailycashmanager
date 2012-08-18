#ifndef GUARD_phatbooks_exceptions_hpp
#define GUARD_phatbooks_exceptions_hpp

#include <jewel/exception.hpp>

/** \file phatbooks_exceptions.hpp
 *
 * \brief Header file containing exception classes defined in the phatbooks
 * namespace. These exceptions are for throwing by Phatbooks-specific
 * functions (although Phatbooks-specific functions need not throw \e only
 * these exceptions).
 *
 * These exceptions are defined using the JEWEL_DERIVED_EXCEPTION macro from
 * the Jewel library. They all have \c jewel::Exception as a (direct or
 * indirect) base class.
 *
 * \author Matthew Harvey
 * \date 04 July 2012.
 *
 * Copyright (c) 2012, Matthew Harvey. All rights reserved.
 */

namespace phatbooks
{

/**
 * Base class for all the exception classes defined in the \c phatbooks
 * namespace.
 */
JEWEL_DERIVED_EXCEPTION(PhatbooksException, jewel::Exception);

/**
 * Exception to be thrown when a table is not properly constituted to perform
 * a task, for example, is not suitable for storage of a particular type.
 */
JEWEL_DERIVED_EXCEPTION(BadTable, PhatbooksException);





}  // namespace phatbooks

#endif  // GUARD_phatbooks_exceptions_hpp

