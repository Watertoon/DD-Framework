 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#include <dd.hpp>
#include <unit_tester.hpp>
#include <iostream>

DECLARE_UNIT_TESTER_INSTANCE;

TEST(TimeSpanToTickConversion) {

    /* Init timestamp */
    dd::util::InitializeTimeStamp();

    /* Base case no conversion */
    dd::TimeSpan span = dd::TimeSpan::FromMilliSeconds(2);
    TEST_ASSERT(span.GetMilliSeconds() == 2);

    /* Convert case */
    dd::TimeSpan convert_span = dd::TimeSpan::FromTick(span.GetTick());
    TEST_ASSERT(convert_span.GetMilliSeconds() == 2);

    TEST_SUCCESS;
}
