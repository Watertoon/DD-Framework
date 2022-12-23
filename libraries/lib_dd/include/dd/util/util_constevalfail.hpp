#pragma once

namespace dd::util {

    /* This function acts as an unreachable in constant expression context */
    void _consteval_fail();
}
