#include <dd.hpp>

namespace dd::trace::impl {

    void AbortImpl(const char *expected_result, const char *function_name, const char *full_file_path, u32 line_number, const Result result, const char *format, ...) {

        /* Stop ukern */
        //dd::ukern::StopForAbort();

        /* Open an output file for out crash data */
        HANDLE crash_file = ::CreateFile("save/crash_report.txt", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (crash_file == INVALID_HANDLE_VALUE) {
            Abort(result);
        }

        /* Create format string */
        va_list var_args;
        ::va_start(var_args, format);
        const int abort_info_length = ::vsnprintf(nullptr, 0, format, var_args);
        char *abort_info = reinterpret_cast<char*>(::malloc(abort_info_length));
        if (abort_info == nullptr) { Abort(result); }
        ::vsnprintf(abort_info, abort_info_length, format, var_args);
        ::va_end(var_args);
        
        /* Create file output */
        const int output_length = ::snprintf(nullptr, 0, "DD ABORT\n Function: %s\n File: %s:%d\n Expected: %s\n%s", function_name, full_file_path, line_number, expected_result, abort_info);
        char *output = reinterpret_cast<char*>(::malloc(output_length));
        if (output == nullptr) { Abort(result); }
        ::snprintf(output, output_length, "DD ABORT\n Function: %s\n File: %s:%d\n Expected: %s\n%s", function_name, full_file_path, line_number, expected_result, abort_info);
        
        /* Output to console */
        ::puts(output);
        
        /* Write output to file */
        const bool write_result = ::WriteFile(crash_file, output, output_length, nullptr, nullptr);
        if (write_result == false) {
            Abort(result);
        }

        /* Append backtraces */
        //dd::ukern::OutputBacktraceToFileAll(crash_file);
        
        /* Abort */
        Abort(result);
    }

    void Abort(const Result result) {
        /* Signal debugger if able */
        Break();
        ::ExitProcess(result);
    }
}
