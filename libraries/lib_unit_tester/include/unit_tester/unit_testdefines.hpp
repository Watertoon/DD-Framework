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