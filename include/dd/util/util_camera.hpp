#pragma once

namespace dd::util {
    
    class LookAtCamera {
        private:
            math::Matrix34f m_camera_mtx;
            Vector3f        m_pos;
            Vector3f        m_at;
            Vector3f        m_up;
        public:
            
            void UpdateCameraMatrix(math::Matrix34f *out_view_mtx) {
                
            }
    };
}