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

namespace dd::util {

    class LogicalFramebuffer {
        private:
            math::Vector2f m_virtual_size;
            math::Vector2f m_dimensions;
        public:
            constexpr LogicalFramebuffer() : m_virtual_size(), m_dimensions() {/*...*/}

            void SetVirtualCanvasSize(float x, float y) {
                math::Vector2f temp = {x, y};
                m_virtual_size = temp;
            }

            void SetDimensions(float x, float y) {
                math::Vector2f temp = {x, y};
                m_dimensions = temp;
            }

            constexpr void GetVirtualCanvasSize(float *out_x, float *out_y) const {
                *out_x = m_virtual_size[0];
                *out_y = m_virtual_size[1];
            }

            constexpr void GetDimensions(float *out_x, float *out_y) const {
                *out_x = m_dimensions[0];
                *out_y = m_dimensions[1];
            }
    };
}
