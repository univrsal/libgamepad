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

#include "hook.hpp"

#ifdef LGP_WINDOWS
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <string>

namespace gamepad {
namespace util {
    inline std::string wchar_to_utf8(const std::wstring& wstr)
    {
        if (wstr.empty())
            return "";
        std::string result;
        int char_count = 0;
        if ((char_count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, 0, 0) - 1) > 0) {
            char* buf = new char[int(char_count + 1)];
            if (buf) {
                WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buf, char_count, 0, 0);
                buf[char_count] = '\0';
                result = buf;
            }
        }
        return result;
    }
}

class hook_dinput : public hook {
    IDirectInput8* m_dinput = nullptr;
    HWND m_hook_window = nullptr; /* Invisible window needed to initialize devices */
    std::thread m_window_message_thread;

    int m_dev_counter = 0;
#ifdef LGP_ENABLE_JSON
    void on_bind(json11::Json::object& j, uint16_t native_code, uint16_t vc, int16_t val, bool is_axis) override;
#endif
public:
    void query_devices() override;
    bool start() override;

#ifdef LGP_ENABLE_JSON
    virtual std::shared_ptr<cfg::binding> make_native_binding(const json11::Json& j) override;
    virtual const json11::Json& get_default_binding() override;
#endif
    friend BOOL CALLBACK enum_callback(LPCDIDEVICEINSTANCE dev, LPVOID data);
};
}
#endif
