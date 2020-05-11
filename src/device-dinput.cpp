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
    device_dinput::device_dinput(LPCDIDEVICEINSTANCE dev, IDirectInput8* dinput)
		: m_device_instance(dev), m_dinput(dinput)
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
            gdebug("Initalized gamepad %s", m_product_name.c_str());
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
