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

DECLARE_UNIT_TESTER_INSTANCE;

TEST(SchedulerInit) {

    /* Query win32 core mask */
    dd::ukern::UKernCoreMask core_mask = 3;

    /* Initialize scheduler */
    dd::ukern::InitializeUKern(core_mask);

    TEST_SUCCESS;
}
