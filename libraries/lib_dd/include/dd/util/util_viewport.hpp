#pragma once

namespace dd::util {

    template <typename T>
    struct BoundingBox2 {
        T min_x;
        T min_y;
        T max_x;
        T max_y;
    };

    class Viewport {
        private:
            BoundingBox2<float> m_bounding_box;
        public:
            constexpr Viewport() {/*...*/}
            constexpr Viewport(float min_x, float min_y, float max_x, float max_y) : m_bounding_box{min_x, min_y, max_x, max_y } {/*...*/}
            constexpr Viewport(const BoundingBox2<float>& bound_box) : m_bounding_box(bound_box) {/*...*/}

            constexpr Viewport(const LogicalFrameBuffer *framebuffer) : m_bounding_box() {
                float width  = 0.0f;
                float height = 0.0f;
                framebuffer->GetVirtualCanvasSize(std::addressof(width), std::addressof(height));

                m_bounding_box.min_x = math::Min(width, 0.0f);
                m_bounding_box.min_y = math::Min(height, 0.0f);
                m_bounding_box.max_x = math::Max(width, 0.0f);
                m_bounding_box.max_y = math::Max(height, 0.0f);
            }

    };
}
