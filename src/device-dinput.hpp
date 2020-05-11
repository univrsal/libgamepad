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
#include <gamepad/device.hpp>
#include <gamepad/binding-dinput.hpp>
#include <dinput.h>

namespace gamepad
{
	class device_dinput : public device
	{
        std::string m_product_name;
        std::string m_instance_name;
        std::string m_id;
        IDirectInput8 *m_dinput = nullptr;
        LPCDIDEVICEINSTANCE m_device_instance = nullptr;
        IDirectInputDevice8 *m_device = nullptr;
        GUID m_device_id;
        HWND m_hook_window;
        DIDEVCAPS m_capabilities;
        DIJOYSTATE2 m_new_state{}, m_old_state{};
        std::array<LONG*, 8> m_axis_new, m_axis_old;
        cfg::binding_dinput* m_native_binding = nullptr;
    public:
        device_dinput(LPCDIDEVICEINSTANCE dev, IDirectInput8* dinput, HWND hook_window);
        virtual ~device_dinput();

        const std::string& get_id() const override;
        void init() override;
        void deinit() override;
        void update() override;
        void set_binding(std::shared_ptr<cfg::binding>&& b) override;

        friend BOOL CALLBACK enum_device_objects_callback(
            LPCDIDEVICEOBJECTINSTANCE obj,
            LPVOID data);
	};
}