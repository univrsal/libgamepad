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
#define LGP_XINPUT_DEVICES 4
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace gamepad {

typedef struct {
    unsigned long eventCount;
    WORD wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
} xinput_pad;

typedef int(__stdcall* xinput_refresh_t)(int, xinput_pad*);

class hook_xinput : public hook {
    HINSTANCE m_xinput = nullptr;
    xinput_refresh_t m_xinput_refresh = nullptr;

public:
    void query_devices() override;
    bool start() override;

#ifdef LGP_ENABLE_JSON
    virtual std::shared_ptr<cfg::binding> make_native_binding(const json11::Json& j) override;
    virtual const json11::Json& get_default_binding() override;
#endif
};
}
#endif
