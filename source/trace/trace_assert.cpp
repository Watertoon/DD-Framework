#include <dd.hpp>

namespace dd::trace {

    void OnAssertFailure(int line, const char *file) {
        char buffer0[0x200] = { '\0' };
        std::snprintf(buffer0, sizeof(buffer0), "%d, %s", line, file);
        ::puts(buffer0);
    }
}
