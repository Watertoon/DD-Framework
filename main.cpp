#include <dd.h>

long unsigned int WindowThreadMain(void *arg) {
    /* Initialize Window and OpenGL context */
    dd::ogl::GLContextWindow gl_window;
    ::SetEvent(reinterpret_cast<Handle>(arg));

    /* Setup OpenGL */
    dd::learn::SetupTriangle();

    /* Perform message loop */
    MSG msg = {};
    while(::GetMessage(std::addressof(msg), nullptr, 0, 0) != 0) {
        /* Swap our window buffer */
        gl_window.SwapBuffers();
        
        /* Next frame */
        dd::util::BeginFrame();

        /* Draw */
        dd::learn::DrawTriangle();

        /* Perform our window functions */
        ::TranslateMessage(std::addressof(msg));
        ::DispatchMessage(std::addressof(msg));

        /* Wait until next frame */
        dd::util::WaitUntilNextFrame();
    }

    /* Clean up */
    dd::learn::CleanTriangle();

    return 0;
}

int main() {
    /* Initialize System Time */
    dd::util::InitializeTime();

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