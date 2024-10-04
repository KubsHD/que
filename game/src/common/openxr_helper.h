#pragma once

#if defined(XR_OS_ANDROID)
#include "jni.h"
#endif

// For DEBUG_BREAK
#include "HelperFunctions.h"

inline void OpenXRDebugBreak() {
    DEBUG_BREAK;
}

inline const char* GetXRErrorString(XrInstance xrInstance, XrResult result) {
    static char string[XR_MAX_RESULT_STRING_SIZE];
    xrResultToString(xrInstance, result, string);
    return string;
}

#define OPENXR_CHECK(x, y)                                                                                                                                  \
    {                                                                                                                                                       \
        XrResult result = (x);                                                                                                                              \
        if (!XR_SUCCEEDED(result)) {                                                                                                                        \
            std::cerr << "ERROR: OPENXR: " << int(result) << "(" << (m_xrInstance ? GetXRErrorString(m_xrInstance, result) : "") << ") " << y << std::endl; \
            OpenXRDebugBreak();                                                                                                                             \
        }                                                                                                                                                   \
    }

#define OPENXR_CHECK_PORTABLE(instance, x, y)                                                                                                               \
    {                                                                                                                                                       \
        XrResult result = (x);                                                                                                                              \
        if (!XR_SUCCEEDED(result)) {                                                                                                                        \
            std::cerr << "ERROR: OPENXR: " << int(result) << "(" << (instance ? GetXRErrorString(instance, result) : "") << ") " << y << std::endl; \
            OpenXRDebugBreak();                                                                                                                             \
        }                                                                                                                                                   \
    }
