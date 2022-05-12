 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <dd.hpp>

namespace dd::res {
    
    void LoadFile(const char *path, void **out_file, u32 *out_file_size) {
        /* Find file */
        Handle file = ::CreateFile(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        DD_ASSERT(file != INVALID_HANDLE_VALUE);

        /* Get file size */
        LARGE_INTEGER file_size = {};
        bool result = ::GetFileSizeEx(file, std::addressof(file_size));
        DD_ASSERT(result != false && file_size.HighPart == 0);

        if (out_file != nullptr) {
            /* Make a file buffer */
            void *buffer = new (std::nothrow) char[file_size.LowPart];
            DD_ASSERT(buffer != nullptr);

            /* Load the file */
            long unsigned int size_read = 0;
            result = ::ReadFile(file, buffer, file_size.LowPart, std::addressof(size_read), nullptr);
            DD_ASSERT(result != false);
            DD_ASSERT(file_size.LowPart == size_read);

            *out_file = buffer;
        }

        if (out_file_size != nullptr) {
            *out_file_size = file_size.LowPart;
        }

        ::CloseHandle(file);
    }

    void LoadTextFile(const char *path, char **out_file, u32 *out_file_size) {
        /* Find file */
        Handle file = ::CreateFile(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        DD_ASSERT(file != INVALID_HANDLE_VALUE);

        /* Get file size */
        LARGE_INTEGER file_size = {};
        bool result = ::GetFileSizeEx(file, std::addressof(file_size));
        file_size.LowPart += 1;
        DD_ASSERT(result != false && file_size.HighPart == 0 && file_size.LowPart != 0);

        if (out_file != nullptr) {
            /* Make a file buffer */
            char *buffer = new (std::nothrow) char[file_size.LowPart];
            DD_ASSERT(buffer != nullptr);

            /* Load the file */
            long unsigned int size_read = 0;
            result = ::ReadFile(file, buffer, file_size.LowPart - 1, std::addressof(size_read), nullptr);
            DD_ASSERT(result != false && file_size.LowPart - 1 == size_read);
            std::cout << size_read << std::endl;

            buffer[file_size.LowPart - 1] = '\0';
            *out_file = buffer;
        }

        if (out_file_size != nullptr) {
            *out_file_size = file_size.LowPart;
        }

        ::CloseHandle(file);
    }
}
#pragma GCC push_options
#pragma GCC optimize("-O2")

/* STB doesn't play nice with optimizations */
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace dd::res {

    void LoadStbImage(const char *path, s32 desired_channels, unsigned char **out_image_data, s32 *out_width, s32 *out_height, s32 *out_channels) {
        void *file_buffer = nullptr;
        u32 size = 0;
        LoadFile(path, std::addressof(file_buffer), std::addressof(size));

        ::stbi_set_flip_vertically_on_load(true);
        *out_image_data = ::stbi_load_from_memory(reinterpret_cast<unsigned char*>(file_buffer), size, out_width, out_height, out_channels, desired_channels);

        delete [] reinterpret_cast<char*>(file_buffer);
    }

    void FreeStbImage(unsigned char *image_data) {
        ::stbi_image_free(image_data);
    }
}

#pragma GCC pop_options
