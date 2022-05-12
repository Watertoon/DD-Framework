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

namespace dd::hid {

    struct KeyboardState{};

    struct MouseState {
        s32 absolute_x;
        s32 absolute_y;
        s32 absolute_wheel;
        s32 delta_x;
        s32 delta_y;
        s32 delta_wheel;
    };

    void SetLastRawInput(HRAWINPUT input_handle);
    
    void BeginFrame();

    const KeyboardState *GetKeyboardState();

    const MouseState *GetMouseState();
}
