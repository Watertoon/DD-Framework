#pragma once

namespace hid {

    namespace {
        
        bool CheckRealKeyboard(HANDLE keyboard) {
            RID_DEVICE_INFO info = {};
            u32 size = sizeof(RID_DEVICE_INFO);
            s32 result = ::GetRawInputDeviceInfo(keyboard, RIDI_DEVICEINFO, std::addressof(info), std::addressof(size));
            
            if (result != -1 || result < sizeof(RID_DEVICE_INFO) || info.dwType != RIM_TYPEKEYBOARD) {
                return false;
            }
            return (info.keyboard.dwNumberOfKeysTotal >= 30);
        }

        void HidThreadMain(void *arg) {
            /* Query input devices */
            u32 device_count = 0;
            s32 result = ::GetRawInputDeviceList(nullptr, std::addressof(device_count), sizeof(RAWINPUTDEVICELIST));
            DD_ASSERT(result != -1);

            /* Get raw input device list */
            RAWINPUTDEVICELIST *device_list = new RAWINPUTDEVICELIST[device_count];
            DD_ASSERT(device_list != nullptr);
            u32 second_count = 0;
            while (device_count != second_count) {
                
                result = ::GetRawInputDeviceList(std::addressof(device_list), std::addressof(second_count), sizeof(RAWINPUTDEVICELIST));
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
                
            }

            /* Process input messages */
            while () {
                
            }
            
            /* Cleanup */
            delete[] device_list;
        }
    }

    void InitializeRawInputThread(HANDLE hwnd_thread_handle) {
        ::CreateThread();
    }
}