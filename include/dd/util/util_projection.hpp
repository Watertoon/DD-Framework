#pragma once

namespace dd::util {
    
    class Projection {
        private:
            util::Matrix44 m_projection_matrix;
        public:
            constexpr Projection() {}
    };
    
    class PerspectiveProjection {
        
    }
}