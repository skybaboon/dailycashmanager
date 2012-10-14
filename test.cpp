/** \file test.cpp
 *
 * \brief Executes tests.
 *
 * This executable compiled from this file should not be directly executed.
 * Rather, it is designed to be executed from the Tcl script in test.tcl.
 * The C++ executable needs to be caused to crash in order to test the
 * SQL transaction handling via the DatabaseConnection class. This is
 * facilitated by having an "external" script control the execution and
 * recovery process.
 *
 * \author Matthew Harvey
 * \date 29 Sep 2012.
 *
 * Copyright (c) 2012, Matthew Harvey. All rights reserved.
 */


#include "sqloxx/tests/atomicity_test.hpp"
#include <unittest++/UnitTest++.h>
#include <iostream>

// Should pull in a better way.
#include "sqloxx/tests/sqloxx_tests_common.hpp"

using sqloxx::DatabaseConnection;
using sqloxx::SharedSQLStatement;
using sqloxx::tests::do_atomicity_test;
using sqloxx::tests::do_speed_test;
using std::cout;
using std::endl;
using std::string;


int main()
{
	#ifndef NDEBUG
		cout << "Compiled in debugging mode (NDEBUG not defined)." << endl;
	#else
		cout << "Compiled in release mode (NDEBUG is defined)." << endl;
	#endif

	do_speed_test();

	/*
	int atomicity_test_result = do_atomicity_test();

	

	cout << "\nNow running various unit tests using UnitTest++..."
		 << endl;

	return atomicity_test_result + UnitTest::RunAllTests();
	*/
}
