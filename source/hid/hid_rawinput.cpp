#include <dd.hpp>

namespace dd::hid {
    
    namespace {
        union MouseData {
            RAWMOUSE raw_mouse;
            char _buf[0x100];
        };
        MouseData raw_mouse_array[8] = {};
        RAWKEYBOARD raw_keyboard_array[8] = {};
        MouseState mouse_state = {};
        u32 last_mouse_index = 0;
        u32 last_keyboard_index = 0;
        
        void SetMouseState() {
            const RAWMOUSE *last_mouse = std::addressof(raw_mouse_array[last_mouse_index].raw_mouse);
        
            /* Get monitor dimensions */
            s32 width = 0, height = 0;
            if ((last_mouse->usFlags & MOUSE_VIRTUAL_DESKTOP) != 0) {
                width = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
                height = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
            } else {
                width = ::GetSystemMetrics(SM_CXSCREEN);
                height = ::GetSystemMetrics(SM_CYSCREEN);
            }
            
            /* Set Mouse State*/
            if ((last_mouse->usFlags & MOUSE_MOVE_ABSOLUTE) != 0) {
                mouse_state.absolute_x = static_cast<s32>(((last_mouse->lLastX / 65535.0f) * width));
                mouse_state.absolute_y = static_cast<s32>(((last_mouse->lLastY / 65535.0f) * height));

                /* This will be the same as absolute for the first frame */
                /* Assume we stay ABSOLUTE */
                const u32 last_last_index = (7 < (last_mouse_index - 1)) ? 0 : (last_mouse_index - 1);
                mouse_state.delta_x = last_mouse->lLastX - raw_mouse_array[last_last_index].raw_mouse.lLastX;
                mouse_state.delta_y = last_mouse->lLastY - raw_mouse_array[last_last_index].raw_mouse.lLastY;
            } else if ((last_mouse->usFlags & MOUSE_MOVE_RELATIVE) != 0) {
                mouse_state.delta_x = last_mouse->lLastX;
                mouse_state.delta_y = last_mouse->lLastY;
                
                /* Assume we stay RELATIVE */
                mouse_state.absolute_x += static_cast<s32>(((last_mouse->lLastX / 65535.0f) * width));
                mouse_state.absolute_y += static_cast<s32>(((last_mouse->lLastY / 65535.0f) * height));
            }
        }
    }
    
    
    void SetLastRawInput(HRAWINPUT input_handle) {
        RAWINPUTHEADER raw_header = {};
        u32 struct_size = sizeof(RAWINPUTHEADER);
        u32 result = ::GetRawInputData(input_handle, RID_HEADER, std::addressof(raw_header), std::addressof(struct_size), sizeof(RAWINPUTHEADER));
        DD_ASSERT(result == sizeof(RAWINPUTHEADER));
        
        if (raw_header.dwType == RIM_TYPEMOUSE) {
            last_mouse_index = (last_mouse_index + 1) % 8;
            struct_size = sizeof(MouseData);
            result = ::GetRawInputData(input_handle, RID_INPUT, std::addressof(raw_mouse_array[last_mouse_index].raw_mouse), std::addressof(struct_size), sizeof(RAWINPUTHEADER));
            DD_ASSERT(result <= sizeof(MouseData));
            std::cout << result << std::endl;
            SetMouseState();
        } else if (raw_header.dwType == RIM_TYPEKEYBOARD) {
            last_keyboard_index = (last_keyboard_index + 1) % 8;
            struct_size = sizeof(RAWKEYBOARD);
            result = ::GetRawInputData(input_handle, RID_INPUT, std::addressof(raw_keyboard_array[last_keyboard_index]), std::addressof(struct_size), sizeof(RAWINPUTHEADER));
            DD_ASSERT(result == sizeof(RAWKEYBOARD));
        }
    }

    const KeyboardState *GetKeyboardState() {
        return nullptr;
    }

    const MouseState *GetMouseState() {
        return std::addressof(mouse_state);
    }
}