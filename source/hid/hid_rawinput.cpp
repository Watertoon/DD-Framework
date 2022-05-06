#include <dd.hpp>

namespace dd::hid {
    
    namespace {
        union MouseData {
            struct {
                RAWINPUT raw_input;
            };
            char _buf[0x100];
        };
        MouseData   raw_mouse = {};
        MouseData   last_raw_mouse = {};
        RAWKEYBOARD raw_keyboard = {};
        //RAWKEYBOARD last_raw_keyboard = {};
        MouseState  interim_mouse_state = {};
        MouseState  frame_mouse_state = {};
        SRWLOCK     state_lock = {};
        
        void SetMouseState() {
            const RAWMOUSE *last_mouse = std::addressof(last_raw_mouse.raw_input.data.mouse);
            const RAWMOUSE *mouse = std::addressof(raw_mouse.raw_input.data.mouse);
        
            /* Get monitor dimensions */
            s32 width = 0, height = 0;
            if ((last_mouse->usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP) {
                width = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
                height = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
            } else {
                width = ::GetSystemMetrics(SM_CXSCREEN);
                height = ::GetSystemMetrics(SM_CYSCREEN);
            }
            
            /* Set Mouse State*/
            ::AcquireSRWLockExclusive(std::addressof(state_lock));

            if ((last_mouse->usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE) {
                interim_mouse_state.absolute_x = static_cast<s32>(((mouse->lLastX / 65535.0f) * width));
                interim_mouse_state.absolute_y = static_cast<s32>(((mouse->lLastY / 65535.0f) * height));

                /* This will be the same as absolute for the first frame */
                interim_mouse_state.delta_x += mouse->lLastX - last_mouse->lLastX;
                interim_mouse_state.delta_y += mouse->lLastY - last_mouse->lLastY;
            } else if ((last_mouse->usFlags & MOUSE_MOVE_RELATIVE) == MOUSE_MOVE_RELATIVE) {
                interim_mouse_state.delta_x += mouse->lLastX;
                interim_mouse_state.delta_y += mouse->lLastY;
                
                /* Assume we stay RELATIVE */
                interim_mouse_state.absolute_x += static_cast<s32>(((mouse->lLastX / 65535.0f) * width));
                interim_mouse_state.absolute_y += static_cast<s32>(((mouse->lLastY / 65535.0f) * height));
                std::cout << "mouse_state: " << mouse->lLastX << " " << mouse->lLastY << std::endl;
                std::cout << "interimabs_state: " << interim_mouse_state.absolute_x << " " << interim_mouse_state.absolute_y << " " << std::endl;
                std::cout << "interimrel_state: " << interim_mouse_state.delta_x << " " << interim_mouse_state.delta_y << " " << std::endl;
            }

            ::ReleaseSRWLockExclusive(std::addressof(state_lock));
            
            ::memcpy(std::addressof(last_raw_mouse), std::addressof(raw_mouse), sizeof(MouseData));
        }
    }
    
    void BeginFrame() {
        /* Copy last mouse state */
        ::AcquireSRWLockExclusive(std::addressof(state_lock));

        ::memcpy(std::addressof(frame_mouse_state), std::addressof(interim_mouse_state), sizeof(MouseState));
        ::memset(std::addressof(interim_mouse_state), 0, sizeof(MouseState));

        ::ReleaseSRWLockExclusive(std::addressof(state_lock));
    }
    
    void SetLastRawInput(HRAWINPUT input_handle) {

        /* Get raw input header */
        RAWINPUTHEADER raw_header = {};
        u32 struct_size = sizeof(RAWINPUTHEADER);
        u32 result = ::GetRawInputData(input_handle, RID_HEADER, std::addressof(raw_header), std::addressof(struct_size), sizeof(RAWINPUTHEADER));
        DD_ASSERT(result == sizeof(RAWINPUTHEADER));
        
        std::cout << raw_header.dwType << std::endl;

        /* Get raw input data */
        if (raw_header.dwType == RIM_TYPEMOUSE) {
            struct_size = sizeof(MouseData);
            result = ::GetRawInputData(input_handle, RID_INPUT, std::addressof(raw_mouse.raw_input), std::addressof(struct_size), sizeof(RAWINPUTHEADER));
            DD_ASSERT(sizeof(RAWMOUSE) <= result && result <= sizeof(MouseData));

            SetMouseState();
        } else if (raw_header.dwType == RIM_TYPEKEYBOARD) {
            struct_size = sizeof(RAWKEYBOARD);
            result = ::GetRawInputData(input_handle, RID_INPUT, std::addressof(raw_keyboard), std::addressof(struct_size), sizeof(RAWINPUTHEADER));
            DD_ASSERT(result == sizeof(RAWKEYBOARD));
        }
    }

    const KeyboardState *GetKeyboardState() {
        return nullptr;
    }

    const MouseState *GetMouseState() {
        return std::addressof(frame_mouse_state);
    }
}