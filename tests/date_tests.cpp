// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "date.hpp"
#include "interval_type.hpp"
#include "phatbooks_tests_common.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <UnitTest++/UnitTest++.h>
#include <cassert>


namespace gregorian = boost::gregorian;

namespace phatbooks
{
namespace test
{


TEST(test_is_valid_date_for_interval_type)
{
	for (int y = 1990; y <= 2015; y += 3)
	{
		for (int m = 1; m != 13; ++m)
		{
			gregorian::date const first(y, m, 1);
			gregorian::date const last =
				first - gregorian::date_duration(1);
			assert (last.day() >= 28);
			CHECK
			(	is_valid_date_for_interval_type
				(	last,
					interval_type::month_ends
				)
			);
			for
			(	gregorian::date prev = last;
				prev.day() != 28;
				prev -= gregorian::date_duration(1)
			)
			{
				assert (prev.day() >= 28);
				assert (prev.day() <= 31);
				CHECK
				(	!is_valid_date_for_interval_type
					(	prev,
						interval_type::months
					)
				);
			}
			for (int d = 1; d != 29; ++d)
			{
				gregorian::date dt(y, m, d);
				CHECK
				(	is_valid_date_for_interval_type
					(	dt,
						interval_type::days
					)
				);
				CHECK
				(	is_valid_date_for_interval_type
					(	dt,
						interval_type::weeks
					)
				);
				CHECK
				(	is_valid_date_for_interval_type
					(	dt,
						interval_type::months
					)
				);
			}
		}
	}
}

TEST(test_month_end_for_date)
{
	using gregorian::date;

	date const d0(2013, 9, 14);
	CHECK_EQUAL(month_end_for_date(d0), date(2013, 9, 30));

	date const d1(1900, 12, 31);
	CHECK_EQUAL(month_end_for_date(d1), date(1900, 12, 31));

	date const d2(2980, 6, 1);
	CHECK_EQUAL(month_end_for_date(d2), date(2980, 6, 30));

	date const d3(1980, 1, 29);
	CHECK_EQUAL(month_end_for_date(d3), date(1980, 1, 31));

	date const d4(1642, 2, 28);
	CHECK_EQUAL(month_end_for_date(d4), date(1642, 2, 28));

	date const d5(2000, 2, 3);
	CHECK_EQUAL(month_end_for_date(d5), date(2000, 2, 29));

	date const d6(2100, 2, 26);
	CHECK_EQUAL(month_end_for_date(d6), date(2100, 2, 28));

	date const d7(1962, 5, 30);
	CHECK_EQUAL(month_end_for_date(d7), date(1962, 5, 31));

	date const d8(1500, 11, 30);
	CHECK_EQUAL(month_end_for_date(d8), date(1500, 11, 30));

	date const d9(2015, 12, 2);
	CHECK_EQUAL(month_end_for_date(d9), date(2015, 12, 31));
}

}  // namespace test
}  // namespace phatbooks