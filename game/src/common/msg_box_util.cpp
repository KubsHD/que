//
// Created by Jakub Szczukiewicz on 14/03/2025.
//

#include "msg_box_util.h"

#if defined(_WIN32)
#include <Windows.h>
void MsgBoxUtil::Show(String title, String msg) {
}

#endif

#if defined(__APPLE__)
void MsgBoxUtil::Show(String title, String msg) {
}

#endif

