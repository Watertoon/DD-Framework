#pragma once



namespace dd::trace {
    
    ALWAYS_INLINE bool IsDebuggerPresent() {
        return ::IsDebuggerPresent();
    }
    
    ALWAYS_INLINE void Break() {
        if (IsDebuggerPresent() == true) {
            ::DebugBreak();
        }
    }

    namespace impl {

        NO_RETURN void AbortImpl(const char *expected_result, const char *function_name, const char *full_file_path, u32 line_number, const Result result, const char *format, ...);

        NO_RETURN void Abort(const Result result);
    }
}
