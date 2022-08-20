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
