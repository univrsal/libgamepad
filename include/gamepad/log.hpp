/**
 ** This file is part of the libgamepad project.
 ** Copyright 2020 univrsal <universailp@web.de>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as
 ** published by the Free Software Foundation, either version 3 of the
 ** License, or (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#pragma once
#include <stdarg.h>

#ifndef LGP_LOG_PREFIX
#define LGP_LOG_PREFIX ""
#endif

#if DEBUG
#define gdebug(format, ...) gamepad::log(gamepad::LOG_DEBUG, LGP_LOG_PREFIX format, ##__VA_ARGS__)
#else
#define gdebug(format, ...) LGP_UNUSED(format)
#endif

#define ginfo(format, ...) gamepad::log(gamepad::LOG_INFO, LGP_LOG_PREFIX format, ##__VA_ARGS__)
#define gwarn(format, ...) gamepad::log(gamepad::LOG_WARNING, LGP_LOG_PREFIX format, ##__VA_ARGS__)
#define gerr(format, ...) gamepad::log(gamepad::LOG_ERROR, LGP_LOG_PREFIX format, ##__VA_ARGS__)
namespace gamepad {
enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
};

typedef void (*log_handler)(int lvl, const char* msg, va_list args, void* param);

void set_logger(log_handler handler, void* param);
void get_logger(log_handler* handler, void** param);

#if !defined(_MSC_VER) && !defined(SWIG)
#define PRINTFATTR(f, a) __attribute__((__format__(__printf__, f, a)))
#else
#define PRINTFATTR(f, a)
#endif

PRINTFATTR(2, 3)
void log(int log_level, const char* format, ...);

#undef PRINTFATTR
}
