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
#include "device-xinput.hpp"
#include <gamepad/binding-xinput.hpp>
#include <gamepad/device.hpp>
#include <gamepad/hook-xinput.hpp>

#define XINPUT_DEVICE_NAME_BASE "Generic Xinput gamepad "

namespace gamepad {
enum xinput_code {
    XINPUT_DPAD_UP = 0x0001,
    XINPUT_DPAD_DOWN = 0x0002,
    XINPUT_DPAD_LEFT = 0x0004,
    XINPUT_DPAD_RIGHT = 0x0008,
    XINPUT_START = 0x0010,
    XINPUT_BACK = 0x0020,
    XINPUT_LEFT_THUMB = 0x0040,
    XINPUT_RIGHT_THUMB = 0x0080,
    XINPUT_LEFT_SHOULDER = 0x0100,
    XINPUT_RIGHT_SHOULDER = 0x0200,
    XINPUT_GUIDE = 0x0400,
    XINPUT_A = 0x1000,
    XINPUT_B = 0x2000,
    XINPUT_X = 0x4000,
    XINPUT_Y = 0x8000
};

class device_xinput : public device {
    cfg::binding_xinput* m_native_binding = nullptr;
    xinput_pad m_current_state, m_old_state;
    xinput_refresh_t m_xinput_refresh;
    uint8_t m_id;
    std::string m_cache_id {};

public:
    device_xinput(uint8_t id, const xinput_refresh_t& refresh);

    int update() override;
    void set_binding(std::shared_ptr<cfg::binding> b) override;
    const std::string& get_cache_id() override
    {
        if (m_cache_id.empty())
            m_cache_id = XINPUT_DEVICE_NAME_BASE + std::to_string(m_id);
        return m_cache_id;
    }
};
}
