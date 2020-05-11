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

#include "device-dinput.hpp"
#include <gamepad/hook-dinput.hpp>
#include <gamepad/log.hpp>

namespace gamepad
{
    static BOOL CALLBACK enum_device_objects_callback(
        LPCDIDEVICEOBJECTINSTANCE obj,
        LPVOID data)
    {
        auto d = static_cast<device_dinput*>(data);

        if (obj->dwFlags & DIDFT_AXIS)
        {
            DIPROPRANGE axis_range;
            axis_range.lMax = 100;
            axis_range.lMin = 100;
            axis_range.diph.dwSize = sizeof(DIPROPRANGE);
            axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
            axis_range.diph.dwHow = DIPH_BYID;
            axis_range.diph.dwObj = obj->dwType;
            if (FAILED(d->m_device->SetProperty(DIPROP_RANGE, &axis_range.diph)))
            {
                gerr("Couldn't set axis ranges for %s", util::wchar_to_utf8(obj->tszName).c_str());
                return DIENUM_STOP;
            }
        }
        else if (obj->dwFlags & DIDFT_BUTTON)
        {
            
        }
        return DIENUM_CONTINUE;
    }

    device_dinput::device_dinput(LPCDIDEVICEINSTANCE dev, IDirectInput8* dinput, HWND hook_window)
		: m_device_instance(dev), m_dinput(dinput), m_hook_window(hook_window)
	{
		device_dinput::init();
	}

	device_dinput::~device_dinput()
	{
		device_dinput::deinit();
	}


    const std::string& device_dinput::get_id() const
    {
        return m_id;
    }

    void device_dinput::init()
    {
        deinit();
        m_product_name = util::wchar_to_utf8(m_device_instance->tszProductName);
        m_instance_name = util::wchar_to_utf8(m_device_instance->tszInstanceName);
        m_device_id = m_device_instance->guidInstance;
        m_valid = m_dinput->CreateDevice(m_device_id, &m_device, nullptr) == DI_OK;

		wchar_t buf[512];
        auto result = StringFromGUID2(m_device_id, buf, 512);
		if (result > 0 )
            m_id = util::wchar_to_utf8(buf) + " " + m_product_name;

		if (m_valid)
		{
            gdebug("Initalized gamepad %s", m_product_name.c_str());
            if (FAILED(m_device->SetCooperativeLevel(m_hook_window,
                DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
            {
                gerr("Failed to set device cooperative level");
                m_valid = false;
            } else
            {
                auto result = m_device->EnumObjects(enum_device_objects_callback, this, DIDFT_ALL);
                if (FAILED(result))
                {
                    gerr("Device object enumeration failed:");
                    if (result == DIERR_NOTINITIALIZED)
                        gerr("libgamepad lost access to device");
                    else if (result == DIERR_INVALIDPARAM)
                        gerr("Invalid parameter");
                    else
                        gerr("Unknown error");
                    m_valid = false;
                } else
                {
                    m_capabilities.dwSize = sizeof(DIDEVCAPS);
                    if (FAILED(m_device->GetCapabilities(&m_capabilities)))
                    {
                        gerr("Couldn't get device capabilities");
                        m_valid = false;
                    }
                }
            }
		}
    }
	
    void device_dinput::deinit()
    {
		/* TODO: do devices have to be closed individually? */
    }
	
    void device_dinput::update()
    {
    }
	
    void device_dinput::set_binding(std::shared_ptr<cfg::binding>&& b)
    {
    }
}
