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

#include <gamepad/config.h>
#include <gamepad/log.hpp>
#include <stdarg.h>
#include <stdio.h>

#ifdef DEBUG
static int log_output_level = gamepad::LOG_DEBUG;
#else
static int log_output_level = gamepad::LOG_INFO;
#endif

namespace gamepad {

static void def_log_handler(int log_level, const char* format, va_list args, void*)
{
    char out[4096];
    vsnprintf(out, sizeof(out), format, args);

    if (log_level < log_output_level)
        return;
    switch (log_level) {
    case gamepad::LOG_DEBUG:
        fprintf(stdout, "debug: %s\n", out);
        fflush(stdout);
        break;

    case gamepad::LOG_INFO:
        fprintf(stdout, "info: %s\n", out);
        fflush(stdout);
        break;

    case gamepad::LOG_WARNING:
        fprintf(stdout, "warning: %s\n", out);
        fflush(stdout);
        break;

    case gamepad::LOG_ERROR:
        fprintf(stderr, "error: %s\n", out);
        fflush(stderr);
    }

    LGP_UNUSED(args);
}

static log_handler logger = def_log_handler;
static void* log_param = NULL;

void set_logger(log_handler handler, void* param)
{
    logger = handler;
    log_param = param;
}

void get_logger(log_handler* handler, void** param)
{
    *handler = logger;
    *param = log_param;
}

void logva(int log_level, const char* format, va_list args)
{
    logger(log_level, format, args, log_param);
}

void log(int log_level, const char* format, ...)
{
    va_list args;

    va_start(args, format);
    logva(log_level, format, args);
    va_end(args);
}
}
