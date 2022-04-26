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

bool ProcessWindowUntilNextFrame() {
    const s64 process_target_time = dd::util::GetSystemTick() + dd::util::GetTickUntilNextFrame();
    MSG msg = {};
    while (::GetMessage(std::addressof(msg), nullptr, 0, 0) != 0) {
        /* Perform our window functions */
        ::TranslateMessage(std::addressof(msg));
        ::DispatchMessage(std::addressof(msg));
        
        /* Stop processing to process frame */
        if (process_target_time < dd::util::GetSystemTick()) {
            return false;
        }
    }
    return true;
}

long unsigned int WindowThreadMain(void *arg) {
    /* Initialize Window and OpenGL context */
    dd::ogl::GLContextWindow gl_window;
    ::SetEvent(reinterpret_cast<Handle>(arg));

    /* Setup OpenGL */
    dd::learn::SetupTriangle();

    /* Perform message loop */
    bool should_exit = false;
    while (should_exit == false) {
        /* Swap our window buffer */
        gl_window.SwapBuffers();
        
        /* Next frame */
        dd::util::BeginFrame();

        /* Draw */
        dd::learn::DrawTriangle();

        /* Wait until next frame */
        should_exit = ProcessWindowUntilNextFrame();
    }

    /* Clean up */
    dd::learn::CleanTriangle();

    return 0;
}

int main() {
    /* Initialize System Time */
    dd::util::InitializeTime();
    
    /* Set flush denormals to 0 */
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    /* Create OGL Window Thread and wait for initialization */
    Handle ogl_event = ::CreateEvent(nullptr, true, false, nullptr);
    DD_ASSERT(ogl_event != nullptr);
    Handle window_thread = ::CreateThread(nullptr, 0x3000, WindowThreadMain, ogl_event, 0, nullptr);
    DD_ASSERT(window_thread != nullptr);
    ::WaitForSingleObject(ogl_event, INFINITE);
    ::CloseHandle(ogl_event);

    /* Wait For Window thread to close */
    std::cout << "waiting for window exit" << std::endl;
    ::WaitForSingleObject(window_thread, INFINITE);
    return 0;
}