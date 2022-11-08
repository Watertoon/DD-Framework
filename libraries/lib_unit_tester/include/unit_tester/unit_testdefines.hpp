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
#pragma once

namespace unit {

    #ifndef TOSTRING
        #define TOSTRING(name) #name
    #endif

    #define TEST(test_name) \
        class _Test##test_name final : public unit::TestBase { \
            public: \
                constexpr virtual const char *GetTestName()     const override { return TOSTRING(test_name); } \
                constexpr virtual const char *GetTestFileName() const override { return __FILE__; } \
                virtual int TestMain() const override; \
        }; \
        _Test##test_name test_name##Instance; \
        int _Test##test_name::TestMain() const 

    #define TEST_ASSERT(condition) \
        { \
            if ((condition) == false) { \
                return __LINE__; \
            } \
        }

    #define TEST_SUCCESS return -1;

    #define DECLARE_UNIT_TESTER_INSTANCE \
        constinit unit::TestHolder TestHolderInstance; \
        void unit::AddTestToTester(TestBase *test) { \
            TestHolderInstance.AddTest(test); \
        } \
        int main() { \
            TestHolderInstance.RunAllTests(); \
        }
}