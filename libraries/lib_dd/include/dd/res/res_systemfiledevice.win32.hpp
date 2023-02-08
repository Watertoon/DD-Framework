#pragma once

namespace dd::res {

    ALWAYS_INLINE Result ConvertWin32ErrorToResult() {
        const u32 last_error = ::GetLastError();
        switch(last_error) {
            case ERROR_FILE_NOT_FOUND:
                return ResultFileNotFound;
            case ERROR_PATH_NOT_FOUND:
                return ResultPathNotFound;
            case ERROR_SHARING_VIOLATION:
                return ResultFileSharingViolation;
            case ERROR_LOCK_VIOLATION:
                return ResultFileLockViolation;
            case ERROR_TOO_MANY_OPEN_FILES:
                return ResultOpenFileExhaustion;
            default:
                break;
        }
        return ResultUnknownWin32Error;
    }

    class SystemFileDevice : public FileDeviceBase {
        protected:
            virtual Result LoadFileImpl(FileLoadContext *file_load_context) {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_load_context->out_file != nullptr && file_load_context->file_size == 0, ResultInvalidFileBufferSize);
                RESULT_RETURN_UNLESS((file_load_context->read_div & 0x1f) != 0, ResultInvalidReadDivAlignment);

                /* Open file handle */
                FileHandle handle = {};
                const Result open_result = this->TryOpenFile(std::addressof(handle), file_load_context->file_path, OpenMode_Read);
                RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);

                /* Get file size */
                size_t file_size = 0;
                const Result size_result = this->GetFileSize(std::addressof(file_size), std::addressof(handle));
                RESULT_RETURN_UNLESS(size_result == ResultSuccess, size_result);

                /* Handle file buffer */
                if (file_load_context->out_file == nullptr) {
                    file_load_context->out_file  = ::operator new(file_size, file_load_context->heap, file_load_context->alignment);
                    file_load_context->file_size = file_size;
                    file_load_context->is_heap_allocated = true;
                } else {
                    RESULT_RETURN_UNLESS(file_size <= file_load_context->file_size, ResultInvalidFileBufferSize);
                }

                /* Read loop */
                const u32 div_size  = file_load_context->read_div;
                size_t    remaining = util::AlignDown(file_size, div_size);
                size_t    offset    = 0;
                while (offset != remaining) {
                    this->TryReadFile(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(file_load_context->out_file)) + offset), std::addressof(handle), div_size);
                    offset = offset + div_size;
                }
                this->TryReadFile(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(std::addressof(file_load_context->out_file)) + offset), std::addressof(handle), file_size - remaining);

                /* Close file handle */
                const Result close_result = this->TryCloseFile(std::addressof(handle));
                RESULT_RETURN_UNLESS(close_result == ResultSuccess, close_result);

                return ResultSuccess;
            }

            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(out_file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);
                RESULT_RETURN_UNLESS((open_mode & OpenMode_ReadWriteAppend) != 0, ResultInvalidOpenMode);

                /* Format path */
                util::FixedString<util::MaxPath> formatted_path;
                const Result format_result = this->FormatPath(std::addressof(formatted_path), path);
                RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

                /* Open file */
                out_file_handle->win32_handle = ::CreateFile(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (out_file_handle->win32_handle == INVALID_HANDLE_VALUE) {
                    return ConvertWin32ErrorToResult();
                }

                /* Get file size */
                RESULT_RETURN_IF(::GetFileSizeEx(out_file_handle->win32_handle, reinterpret_cast<LARGE_INTEGER*>(std::addressof(out_file_handle->file_size))), ResultFileSizeRetrievalFailed);

                return ResultSuccess;
            }

            virtual Result CloseFileImpl(FileHandle *file_handle) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->win32_handle != nullptr && file_handle->win32_handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);

                /* Close win32 handle */
                const bool close_result = ::CloseHandle(file_handle->win32_handle);
                if (close_result == false) {
                    return ConvertWin32ErrorToResult();
                }

                /* Null res::FileHandle */
                file_handle->win32_handle = nullptr;
                file_handle->file_offset  = 0;
                file_handle->file_size    = 0;

                return ResultSuccess;
            }

            virtual Result ReadFileImpl(void *out_read_buffer, FileHandle *file_handle, u32 read_size) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->win32_handle != nullptr && file_handle->win32_handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);

                /* Set file location */
                LARGE_INTEGER offset = {
                    .QuadPart = file_handle->file_offset
                };
                u32 set_result = ::SetFilePointerEx(file_handle->win32_handle, offset, nullptr, FILE_BEGIN);
                if (set_result != 0) {
                    return ConvertWin32ErrorToResult();
                }

                /* Read File */
                u32 out_read_size = 0;
                const bool read_result = ::ReadFile(file_handle->win32_handle, std::addressof(out_read_buffer), read_size, reinterpret_cast<long unsigned int*>(std::addressof(out_read_size)), nullptr);
                if (read_result == false) {
                    return ConvertWin32ErrorToResult();
                }

                return ResultSuccess;
            }
            
            virtual Result WriteFileImpl(FileHandle *file_handle, void *write_buffer, u32 write_size) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->win32_handle != nullptr && file_handle->win32_handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);
                RESULT_RETURN_UNLESS(write_buffer != nullptr, ResultNullOutBuffer);
                RESULT_RETURN_UNLESS(write_size != 0, ResultInvalidSize);

                /* Write file */
                u32 out_write_size = 0;
                const bool write_result = ::WriteFile(file_handle->win32_handle, write_buffer, write_size, reinterpret_cast<long unsigned int*>(std::addressof(out_write_size)), nullptr);
                //if () {
                    
                //}
                //RESULT_RETURN_IF()
            }

            virtual Result FlushFileImpl(FileHandle *file_handle) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->win32_handle != nullptr && file_handle->win32_handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);

                /* Flush file buffers */
                const bool flush_result = ::FlushFileBuffers(file_handle->win32_handle);
                if (flush_result == false) {
                    return ConvertWin32ErrorToResult();
                }

                return ResultSuccess;
            }

            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->win32_handle != nullptr && file_handle->win32_handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);

                *out_size = file_handle->file_size;

                return ResultSuccess;
            }
            
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);

                /* Format path */
                util::FixedString<util::MaxPath> formatted_path;
                const Result format_result = this->FormatPath(std::addressof(formatted_path), path);
                RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

                /* Open file */
                FileHandle handle = {};
                const Result open_result = this->TryOpenFile(std::addressof(handle), formatted_path.GetString(), OpenMode_Read);
                RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);

                /* Copy size */
                *out_size = handle.file_size;

                /* Close file */
                const Result close_result = this->TryCloseFile(std::addressof(handle));
                RESULT_RETURN_UNLESS(close_result == ResultSuccess, close_result);

                return ResultSuccess;
            }

            virtual Result CheckFileExistsImpl(const char *path) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);

                /* Format path */
                util::FixedString<util::MaxPath> formatted_path;
                const Result format_result = this->FormatPath(std::addressof(formatted_path), path);
                RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

                /* Check file path */
                const u32 check_result = ::GetFileAttributesA(formatted_path.GetString());
                if (check_result == INVALID_FILE_ATTRIBUTES) {
                    return ConvertWin32ErrorToResult();
                }

                return ResultSuccess;
            }
        public:
            explicit constexpr ALWAYS_INLINE SystemFileDevice(const char *device_name) : FileDeviceBase(device_name) {/*...*/}
    };

    class ContentFileDevice : public SystemFileDevice {
        protected:
            virtual Result FormatPath(util::FixedString<util::MaxPath> *out_formatted_path, const char *path) override {
                util::FixedString<util::MaxPath> path_no_drive;
                util::GetPathWithoutDrive(std::addressof(path_no_drive), path);
                RESULT_RETURN_UNLESS(out_formatted_path->Format("content/%s", path_no_drive) != util::MaxPath, ResultPathTooLong);
                return ResultSuccess;
            }
        public:
            constexpr ALWAYS_INLINE ContentFileDevice() : SystemFileDevice("resource") {/*...*/};
    };

    class SaveFileDevice : public SystemFileDevice {
        protected:
            virtual Result FormatPath(util::FixedString<util::MaxPath> *out_formatted_path, const char *path) override {
                util::FixedString<util::MaxPath> path_no_drive;
                util::GetPathWithoutDrive(std::addressof(path_no_drive), path);
                RESULT_RETURN_UNLESS(out_formatted_path->Format("save/%s", path_no_drive) != util::MaxPath, ResultPathTooLong);
                return ResultSuccess;
            }
        public:
            constexpr ALWAYS_INLINE SaveFileDevice() : SystemFileDevice("save") {/*...*/};
    };
}
