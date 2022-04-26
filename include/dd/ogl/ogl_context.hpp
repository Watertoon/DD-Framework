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
#pragma once

#include "ogl_procs.hpp"

namespace dd::ogl {

    class GLContextWindow {
        private:
            HWND m_hwnd;
            HDC m_hdc;
            HGLRC m_hglrc;
            char **m_extension_buffer;
            u32 m_extension_count;
        private:
            void ParseExtensions(const char *extensions) {
                /* Count spaces */
                u32 extension_count = 0, i = 0;
                while (extensions[i] != '\0') {
                    if (extensions[i] == ' ') {
                        ++extension_count;
                    }
                    ++i;
                }

                /* Allocate ptr_array */
                m_extension_buffer = new (std::nothrow) char*[extension_count + 1];
                m_extension_count = extension_count + 1;

                /* Parse extensions into the ptr array */
                i = 0;
                u32 current_extension = 0, extension_length = 0;
                while (extensions[i] != '\0') {
                    if (extensions[i] != ' ') {
                        ++extension_length;
                    } else {
                        m_extension_buffer[current_extension] = new (std::nothrow) char[extension_length + 1];
                        const char *name_offset = std::addressof(extensions[i-extension_length]);

                        ::memcpy(m_extension_buffer[current_extension], name_offset, extension_length);
                        m_extension_buffer[current_extension][extension_length] = '\0';

                        ++current_extension;
                        extension_length = 0;
                    }
                    ++i;
                }
                m_extension_buffer[current_extension] = new (std::nothrow) char[extension_length + 1];
                const char *name_offset = std::addressof(extensions[i-extension_length]);

                ::memcpy(m_extension_buffer[current_extension], name_offset, extension_length);
                m_extension_buffer[current_extension][extension_length] = '\0';
            }

            void CheckExtensions() {
                /*...*/
            }
        public:
            explicit GLContextWindow() {
                /* Create Dummy Window */
                const WNDCLASS wc {
                    .style = CS_OWNDC,
                    .lpfnWndProc = DummyWndProc,
                    .hInstance = ::GetModuleHandle(nullptr),
                    .hbrBackground = (HBRUSH)(COLOR_BACKGROUND),
                    .lpszClassName = "ForContextInitialization"
                };
                ATOM class_result = ::RegisterClass(std::addressof(wc));
                DD_ASSERT(class_result != 0);

                m_hwnd = ::CreateWindow("ForContextInitialization", "ForContextInitialization", WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, 0, 0, ::GetModuleHandle(nullptr), 0);
                DD_ASSERT(m_hwnd != nullptr);

                /* Create thread-specific hardware device context */
                m_hdc = ::GetDC(m_hwnd);
                DD_ASSERT(m_hdc != nullptr);

                /* Set Pixel Format descriptor */
                const PIXELFORMATDESCRIPTOR pfd = {
                    .nSize = sizeof(PIXELFORMATDESCRIPTOR),
                    .nVersion = 1,
                    .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                    .iPixelType = PFD_TYPE_RGBA,
                    .cColorBits = 32,
                    .cDepthBits = 24,
                    .cStencilBits = 8,
                    .iLayerType = PFD_MAIN_PLANE
                };
                s32 format = ::ChoosePixelFormat(m_hdc, std::addressof(pfd));
                bool last_result = ::SetPixelFormat(m_hdc, format, std::addressof(pfd));
                DD_ASSERT(last_result != false);

                /* Create dummy OpenGL context handle */
                m_hglrc = ::wglCreateContext(m_hdc);
                DD_ASSERT(m_hglrc != nullptr);

                /* Set current context */
                last_result = ::wglMakeCurrent(m_hdc, m_hglrc);
                DD_ASSERT(last_result != false);

                /* Load our static procs */
                LoadGLProcs();

                /* Get advanced pixel format */
                const int pixel_attribute_array[] = {
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_COLOR_BITS_ARB, 32,
                    WGL_RED_BITS_ARB, 8,
                    WGL_GREEN_BITS_ARB, 8,
                    WGL_BLUE_BITS_ARB, 8,
                    WGL_ALPHA_BITS_ARB, 8,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 8,
                    WGL_SAMPLES_ARB, 1,
                    WGL_STEREO_ARB, GL_FALSE,
                    WGL_COLORSPACE_EXT, WGL_COLORSPACE_SRGB_EXT,
                    0,0
                };
                s32 advanced_format = 0;
                u32 formats_found = 0;
                last_result = (pfn_wglChoosePixelFormatARB)(m_hdc, pixel_attribute_array, nullptr, 1, std::addressof(advanced_format), std::addressof(formats_found));
                if (last_result == false) {
                    /* no SRGB fallback */
                    advanced_format = format;
                }

                /* Delete dummy context */
                ::wglMakeCurrent(m_hdc, nullptr);
                ::wglDeleteContext(m_hglrc);
                ::ReleaseDC(m_hwnd, m_hdc);
                ::DestroyWindow(m_hwnd);
                ::UnregisterClass("ForContextInitialization", ::GetModuleHandle(nullptr));

                /* Create real window */
                const WNDCLASS wc_real {
                    .style = CS_OWNDC,
                    .lpfnWndProc = WndProc,
                    .hInstance = ::GetModuleHandle(nullptr),
                    .hbrBackground = (HBRUSH)(COLOR_BACKGROUND),
                    .lpszClassName = "OpenGLWindow"
                };
                class_result = ::RegisterClass(std::addressof(wc_real));
                DD_ASSERT(class_result != 0);

                m_hwnd = ::CreateWindow("OpenGLWindow", "Window", WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, 0, 0, ::GetModuleHandle(nullptr), 0);
                DD_ASSERT(m_hwnd != nullptr);

                /* Create real device context */
                m_hdc = ::GetDC(m_hwnd);
                DD_ASSERT(m_hdc != nullptr);

                /* Set advanced Pixel Format */
                last_result = ::SetPixelFormat(m_hdc, advanced_format, std::addressof(pfd));
                DD_ASSERT(last_result != false);

                /* Create new OpenGl rendering context */
                GLint context_attributes[] = {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 6,
                    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    0
                };
                m_hglrc = (pfn_wglCreateContextAttribsARB)(m_hdc, 0, context_attributes);
                DD_ASSERT(m_hglrc != nullptr);

                /* Set current context */
                last_result = ::wglMakeCurrent(m_hdc, m_hglrc);
                DD_ASSERT(last_result != false);

                /* Set global context */
                *GetOpenGLContext() = m_hglrc;

                /* Get extensions */
                const char *extensions = (pfn_wglGetExtensionsStringARB)(m_hdc);
                this->ParseExtensions(extensions);
                this->CheckExtensions();
                
                ::ShowWindow(m_hwnd, SW_SHOW);

                /* Enable VSync */
                (pfn_wglSwapIntervalEXT)(1);

                /* Set frame frequency */
                dd::util::SetFrameFrequency(60);

                /* Set up raw input */
                const RAWINPUTDEVICE raw_input_devices[2] = {
                    {
                        .usUsagePage = 1,
                        .usUsage = 2,
                        .dwFlags = RIDEV_INPUTSINK,
                        .hwndTarget = m_hwnd
                    },
                    {
                        .usUsagePage = 1,
                        .usUsage = 6,
                        .dwFlags = RIDEV_INPUTSINK,
                        .hwndTarget = m_hwnd
                    }
                };
                bool result = ::RegisterRawInputDevices(raw_input_devices, sizeof(raw_input_devices) / sizeof(RAWINPUTDEVICE), sizeof(RAWINPUTDEVICE));
                DD_ASSERT(result != false);
            }

            ~GLContextWindow() {
                if (m_extension_buffer != nullptr) {
                    for (u32 i = 0; i < m_extension_count; ++i) {
                        if (m_extension_buffer[i] != nullptr) {
                            delete[] m_extension_buffer[i];
                        }
                    }
                    delete[] m_extension_buffer;
                }
                ::wglMakeCurrent(m_hdc, nullptr);
                ::wglDeleteContext(m_hglrc);
                ::wglMakeCurrent(nullptr, nullptr);
                ::ReleaseDC(m_hwnd, m_hdc);
                ::DestroyWindow(m_hwnd);
                ::UnregisterClass("OpenGLWindow", ::GetModuleHandle(nullptr));
            }

            void SwapBuffers() {
                ::wglSwapLayerBuffers(m_hdc, WGL_SWAP_MAIN_PLANE);
            }

            constexpr HWND GetWindowHandle() const { return m_hwnd; }

            static HGLRC *GetOpenGLContext() {
                static HGLRC context;
                return std::addressof(context);
            }

            static LRESULT CALLBACK DummyWndProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
                if (message == WM_DESTROY) {
                    return 0;
                }
                return ::DefWindowProc(window_handle, message, w_param, l_param);
            }

            static LRESULT CALLBACK WndProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
                if (message == WM_DESTROY) {
                    ::PostQuitMessage(0);
                    return 0;
                } else if (message == WM_PAINT) {
                    return 0;
                } else if (message == WM_SIZE) {
                    dd::util::SetFrameFrequency(60);
                    (pfn_glViewport)(0, 0, LOWORD(l_param), HIWORD(l_param));
                } else if (message == WM_DISPLAYCHANGE || message == WM_MOVE) {
                    dd::util::SetFrameFrequency(60);
                } else if (message == WM_INPUT) {
                    dd::hid::SetLastRawInput(reinterpret_cast<HRAWINPUT>(l_param));
                }

                return ::DefWindowProc(window_handle, message, w_param, l_param);
            }
    };
}