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

namespace dd::hid {

    namespace {

        RAWINPUTDEVICELIST *keyboard_handle;
        RAWINPUTDEVICELIST *mouse_handle;
        HWND                hid_hwnd;
        HANDLE              hid_thread;

        bool CheckRealKeyboard(HANDLE keyboard) {
            RID_DEVICE_INFO info = {};
            u32 size = sizeof(RID_DEVICE_INFO);
            s32 result = ::GetRawInputDeviceInfo(keyboard, RIDI_DEVICEINFO, std::addressof(info), std::addressof(size));
            
            if (result != -1 || static_cast<u32>(result) < sizeof(RID_DEVICE_INFO) || info.dwType != RIM_TYPEKEYBOARD) {
                return false;
            }
            return (info.keyboard.dwNumberOfKeysTotal >= 30);
        }

        LRESULT CALLBACK HidWndProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
            switch (message) {
                case WM_DESTROY:
                    ::PostQuitMessage(0);
                    return 0;
                case WM_INPUT:
                    SetLastRawInput(reinterpret_cast<HRAWINPUT>(l_param));
                    break;
                case WM_INPUT_DEVICE_CHANGE:
                    /* Check for current device migration */
                    
                default:
                    break;
            }
            return ::DefWindowProc(window_handle, message, w_param, l_param);
        }

        long unsigned int HidThreadMain(void *arg) {
            DD_ASSERT(arg == nullptr);

            /* Create input window */
            const HINSTANCE process_handle = ::GetModuleHandle(nullptr);
            const WNDCLASS wc = {
                .style = 0,
                .lpfnWndProc = HidWndProc,
                .hInstance = process_handle,
                .hbrBackground = (HBRUSH)(COLOR_BACKGROUND),
                .lpszClassName = "HidDDWindow"
            };
            const u32 result0 = ::RegisterClass(std::addressof(wc));
            DD_ASSERT(result0 != 0);

            hid_hwnd = ::CreateWindowEx(0 ,"HidDDWindow", "InputWindow", 0, 0, 0, 1280, 720, HWND_MESSAGE, 0, ::GetModuleHandle(nullptr), 0);
            DD_ASSERT(hid_hwnd != nullptr);
            
            //::EnableWindow(parent_hwnd, true);
            
            /* Query input devices */
            u32 device_count = 0;
            s32 result = ::GetRawInputDeviceList(nullptr, std::addressof(device_count), sizeof(RAWINPUTDEVICELIST));
            DD_ASSERT(result != -1);

            /* Get raw input device list */
            RAWINPUTDEVICELIST *device_list = new RAWINPUTDEVICELIST[device_count];
            DD_ASSERT(device_list != nullptr);
            u32 second_count = 0;
            while (device_count != second_count) {

                result = ::GetRawInputDeviceList(device_list, std::addressof(second_count), sizeof(RAWINPUTDEVICELIST));
                if (result == -1) {
                    delete[] device_list;
                    device_list = new RAWINPUTDEVICELIST[second_count];
                    DD_ASSERT(device_list != nullptr);
                } else {
                    device_count = result;
                }
            }

            /* Find and register mouse and keyboard */
            for (u32 i = 0; i < device_count; ++i) {
                if (device_list[i].dwType == RIM_TYPEKEYBOARD) {
                    if (CheckRealKeyboard(device_list[i].hDevice) == true) {
                        keyboard_handle = std::addressof(device_list[i]);
                    }
                }
                if (device_list[i].dwType == RIM_TYPEMOUSE) {
                    mouse_handle = std::addressof(device_list[i]);
                }
            }
            const RAWINPUTDEVICE raw_input[2] = {
                {
                    .usUsagePage = 1,
                    .usUsage = 2,
                    .dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY,
                    .hwndTarget = hid_hwnd
                }, 
                {
                    .usUsagePage = 1,
                    .usUsage = 6,
                    .dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY,
                    .hwndTarget = hid_hwnd
                }
            };
            ::RegisterRawInputDevices(raw_input, sizeof(raw_input) / sizeof(RAWINPUTDEVICE), sizeof(RAWINPUTDEVICE));

            /* Process input messages */
            MSG msg = {};
            while (::GetMessage(std::addressof(msg), nullptr, 0, 0) != 0) {
                ::TranslateMessage(std::addressof(msg));
                ::DispatchMessage(std::addressof(msg));
            }
            
            /* Cleanup */
            delete[] device_list;
            return 0;
        }
    }

    void InitializeRawInputThread() {
        hid_thread = ::CreateThread(nullptr, 0x1000, HidThreadMain, nullptr, 0, nullptr);
        DD_ASSERT(hid_thread != nullptr);
    }

    void FinalizeRawInputThread() {
        ::SendMessage(hid_hwnd, WM_DESTROY, 0, 0);
        ::WaitForSingleObject(hid_thread, INFINITE);
    }
}
