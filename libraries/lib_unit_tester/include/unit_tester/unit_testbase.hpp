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
    
    class TestBase;
    
    void AddTestToTester(TestBase *test);
    
    class TestBase {
        private:
            friend class TestHolder;
        private:
            TestBase *m_next_test;
        public:
            TestBase() : m_next_test(nullptr) {
                AddTestToTester(this);
            }

            int Run() { return this->TestMain(); }
            virtual int TestMain() const { return true; };

            constexpr virtual const char *GetTestName()     const { return "Default"; }
            constexpr virtual const char *GetTestFileName() const { return __FILE__; }
    };
}
