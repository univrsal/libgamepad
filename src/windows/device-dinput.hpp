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

#define DIRECTINPUT_VERSION 0x0800
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <dinput.h>
#include <gamepad/binding-dinput.hpp>
#include <gamepad/device.hpp>

namespace gamepad {
class device_dinput : public device {
    struct dinput_axis {
        uint16_t id, offset;
    };

    std::string m_product_name;
    std::string m_instance_name;
    std::string m_id;
    std::string m_cache_id {};

    std::vector<dinput_axis> m_axis_inputs;
    std::array<LONG*, 32> m_axis_new, m_axis_old;
    cfg::binding_dinput* m_native_binding = nullptr;
    bool m_analog = false;
    int m_slider_count = 0;

    IDirectInput8* m_dinput = nullptr;
    LPCDIDEVICEINSTANCE m_device_instance = nullptr;
    IDirectInputDevice8* m_device = nullptr;
    GUID m_device_id;
    HWND m_hook_window;
    DIDEVCAPS m_capabilities;
    DIJOYSTATE2 m_new_state {}, m_old_state {};

    enum {
        DPAD_UP = 128, /* 0 - 127 is used for other gamepad buttons in DIJOYSTATE2 */
        DPAD_LEFT,
        DPAD_DOWN,
        DPAD_RIGHT
    };

public:
    device_dinput(LPCDIDEVICEINSTANCE dev, IDirectInput8* dinput, HWND hook_window);
    virtual ~device_dinput();

    const std::string& get_id() const override;
    void set_id(const std::string& id) override;
    void init() override;
    void deinit() override;
    int update() override;
    void set_binding(std::shared_ptr<cfg::binding> b) override;

    friend BOOL CALLBACK enum_device_objects_callback(LPCDIDEVICEOBJECTINSTANCE obj, LPVOID data);

    static std::string make_id(LPCDIDEVICEINSTANCE dev);
    const std::string& get_cache_id() override
    {
        if (m_cache_id.empty())
            m_cache_id = make_id(m_device_instance);
        return m_cache_id;
    }
};
}
