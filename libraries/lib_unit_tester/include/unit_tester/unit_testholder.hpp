#pragma once

namespace unit {
    
    class TestHolder {
        private:
            TestBase *m_head_test;
        public:
            constexpr TestHolder() : m_head_test(nullptr) {/*...*/}
            
            constexpr void AddTest(TestBase *test) {
                /* Append test to front of test list */
                test->m_next_test = m_head_test;
                m_head_test = test;
            }
            
            size_t CountTests() {
                TestBase *current_test = m_head_test;
                size_t test_count = 0;
                while (current_test != nullptr) {
                    ++test_count;
                    current_test = current_test->m_next_test;
                }
                return test_count;
            }
            
            void RunAllTests() {
                const size_t test_count  = this->CountTests();
                size_t       test_number = 1;
                TestBase *current_test   = m_head_test;

                while (current_test != nullptr) {
                    
                    /* Run the test and output file and test name on failure */
                    const int result = current_test->Run();
                    if (result != -1) {
                        char failure_output[0x200] = {};
                        ::snprintf(failure_output, sizeof(failure_output), "Test failed:    %s (%s:%d)(%lld/%lld)", current_test->GetTestName(), current_test->GetTestFileName(), result, test_number, test_count);
                        ::puts(failure_output);
                    } else {
                        char success_output[0x200] = {};
                        ::snprintf(success_output, sizeof(success_output), "Test succeeded: %s (%s)(%lld/%lld)", current_test->GetTestName(), current_test->GetTestFileName(), test_number, test_count);
                        ::puts(success_output);
                    }
                    
                    /* Iterate to next test */
                    current_test = current_test->m_next_test;
                    ++test_number;
                }
                
            }
            
            void ProfileAllTests() {
                
            }
    };
}