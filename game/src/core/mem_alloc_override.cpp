#include "pch.h"

#if _DEBUG && XR_OS_WINDOWS
#include <tracy/Tracy.hpp>

#include <cstdlib>

void *
operator new(std::size_t n)
{
    void *ptr = malloc(n);
    TracyAlloc(ptr, n);
    return ptr;
}

void operator delete(void *ptr) noexcept
{
    TracyFree(ptr);
    free(ptr);
}
#endif