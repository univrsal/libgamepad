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
#include <gamepad/binding-dinput.hpp>
#include <gamepad/log.hpp>
#include <chrono>

using namespace std;
using namespace std::chrono;

namespace gamepad
{
    static map<uint32_t, uint16_t> dinput_to_vc =
    {
        { JOY_BUTTON1, button::A },
        { JOY_BUTTON2, button::B },
        { JOY_BUTTON3, button::X },
        { JOY_BUTTON4, button::Y },
        { JOY_BUTTON5, button::A },
    };

    static BOOL CALLBACK enum_device_objects_callback(
        LPCDIDEVICEOBJECTINSTANCE obj,
        LPVOID data)
    {
        auto d = static_cast<device_dinput*>(data);
        bool result = false;
        

        DIPROPRANGE axis_range;
        axis_range.lMax = 100;
        axis_range.lMin = 100;
        axis_range.diph.dwSize = sizeof(DIPROPRANGE);
        axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        axis_range.diph.dwHow = DIPH_BYID;
        axis_range.diph.dwObj = obj->dwType;
        if (FAILED(d->m_device->SetProperty(DIPROP_RANGE, &axis_range.diph)))
        {
            gerr("Couldn't set axis ranges for '%s'", util::wchar_to_utf8(obj->tszName).c_str());
            return DIENUM_STOP;
        }
        else
        {
            result = true;
        }
        

        if (result)
            gdebug("Registered '%s' at 0x%X", util::wchar_to_utf8(obj->tszName).c_str(), obj->dwOfs);

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


    const string& device_dinput::get_id() const
    {
        return m_id;
    }

    void device_dinput::init()
    {
        deinit();
        m_product_name = util::wchar_to_utf8(m_device_instance->tszProductName);
        m_instance_name = util::wchar_to_utf8(m_device_instance->tszInstanceName);

        if (m_product_name != m_instance_name) /* Sometimes reported as the same */
            m_name = "(" + m_product_name + ") " + m_instance_name;
        else
            m_name = m_instance_name;

        m_device_id = m_device_instance->guidInstance;
        m_valid = m_dinput->CreateDevice(m_device_id, &m_device, nullptr) == DI_OK;

		wchar_t buf[512];
        auto result = StringFromGUID2(m_device_id, buf, 512);
		if (result > 0 )
            m_id = util::wchar_to_utf8(buf) + " " + m_product_name;

		if (m_valid)
		{
            gdebug("Initalized gamepad '%s'", m_product_name.c_str());
            if (FAILED(m_device->SetCooperativeLevel(m_hook_window,
                DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
            {
                gerr("Failed to set device cooperative level");
                m_valid = false;
            } else
            {
                result = m_device->EnumObjects(enum_device_objects_callback, this, DIDFT_AXIS);
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
                    else if (m_valid && FAILED(m_device->SetDataFormat(&c_dfDIJoystick)))
                    {
                        gerr("Couldn't set data format for device '%s'", m_name.c_str());
                    }
                }
            }
		}

        result = m_device->Acquire();
        if (FAILED(result))
        {
            if (result == DIERR_NOTINITIALIZED)
                gerr("libgamepad lost access to device");
            else if (result == DIERR_INVALIDPARAM)
                gerr("Invalid parameter");
            else
                gerr("Unknown error");
            m_valid = false;
        }
    }
	
    void device_dinput::deinit()
    {
		if (m_device)
		{
            if (m_valid)
                m_device->Unacquire();
            m_device->Release();
		}
        m_valid = false;
        m_device = nullptr;
    }
	
    void device_dinput::update()
    {
        if (m_valid)
        {
            m_previous_state = m_new_state;
            ZeroMemory(&m_new_state, sizeof(DIJOYSTATE));

            auto result = m_device->Poll();
            if (FAILED(result))
            {
                gerr("Polling for device '%s' failed, trying to reacquire...", m_name.c_str());
                result = m_device->Acquire();
                while (result == DIERR_INPUTLOST)
                    result = m_device->Acquire();

                if (FAILED(result))
                    m_valid = false;

                if (result == DIERR_INVALIDPARAM)
                {
                    gerr("Invalid parameter error for %s", m_name.c_str());
                }
                else if (result == DIERR_NOTINITIALIZED)
                {
                    gerr("Device '%s' is exclusively used by another process", m_name.c_str());
                }
                else if (result == DIERR_OTHERAPPHASPRIO)
                {
                    gdebug("Device '%s' is occupied by another process, waiting...", m_name.c_str());
                    return;
                }
                else
                {
                    gerr("Unknown error");
                }
            }

            if (FAILED(m_device->GetDeviceState(sizeof(DIJOYSTATE), &m_new_state)))
            {
                gerr("Couldn't get device state for '%s'", m_name.c_str());
                m_valid = false;
            }
            else
            {
                
                /* Check all buttons */
                uint16_t last_button = 0x0;
                for (int i = 0; i < m_capabilities.dwButtons; i++)
                {
                    bool pressed = m_new_state.rgbButtons[i] & 0x80;
                    if (m_native_binding)
                    {
                        auto new_code = m_native_binding->m_buttons_mappings[i];
                        m_buttons[new_code] = pressed;
                    }

                    /* This button changed over to pressed
                     * Since button presses aren't sent in individually this means
                     * that if multiple buttons have been pressed since the last update
                     * only the one with the highest ID will be reported, but since
                     * this is only used for creating binds it's not an issue.
                     */
                    if (pressed && !(m_previous_state.rgbButtons[i] & 0x80))
                    {
                        auto n = system_clock::now();
                        m_last_button_event.value = 1;
                        m_last_button_event.time = duration_cast<milliseconds>(n.time_since_epoch()).count();
                        m_last_button_event.id = i;
                    }
                }

                /* Check all axis */

            }
        }
    }
	
    void device_dinput::set_binding(shared_ptr<cfg::binding>&& b)
    {
        device::set_binding(move(b));
        m_native_binding = dynamic_cast<cfg::binding_dinput *>(b.get());
    }
}
