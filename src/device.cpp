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

#include <gamepad/device.hpp>
#include <gamepad/hook.hpp>

namespace gamepad {
void device::button_event(uint16_t native_id, uint16_t vc, int32_t value, float vv)
{
    m_last_button_event.native_id = native_id;
    m_last_button_event.value = value;
    m_last_button_event.vc = vc;
    m_last_button_event.virtual_value = vv;
    m_last_button_event.time = gamepad::hook::ms_ticks();
}

void device::axis_event(uint16_t native_id, uint16_t vc, int32_t value, float vv)
{
    m_last_axis_event.native_id = native_id;
    m_last_axis_event.value = value;
    m_last_axis_event.vc = vc;
    m_last_axis_event.virtual_value = vv;
    m_last_axis_event.time = hook::ms_ticks();
}
};
