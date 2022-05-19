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
