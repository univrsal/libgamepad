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

#include "device-xinput.hpp"

using namespace std;

namespace gamepad {

static xinput_code XINPUT_BUTTONS[] = { XINPUT_DPAD_UP,
    XINPUT_DPAD_DOWN,
    XINPUT_DPAD_LEFT,
    XINPUT_DPAD_RIGHT,
    XINPUT_START,
    XINPUT_BACK,
    XINPUT_LEFT_THUMB,
    XINPUT_RIGHT_THUMB,
    XINPUT_LEFT_SHOULDER,
    XINPUT_RIGHT_SHOULDER,
    XINPUT_GUIDE,
    XINPUT_A,
    XINPUT_B,
    XINPUT_X,
    XINPUT_Y };

device_xinput::device_xinput(uint8_t id, const xinput_refresh_t& refresh)
    : m_xinput_refresh(refresh)
    , m_id(id)
{
    m_name = "Generic Xinput gamepad " + to_string(id);
    device_xinput::update();
}

void device_xinput::update()
{
    if (m_xinput_refresh(m_id, &m_current_state) == ERROR_SUCCESS) {
        for (const auto& btn : XINPUT_BUTTONS) {
            bool state = m_current_state.wButtons & btn;
            bool old_state = m_old_state.wButtons & btn;
            uint16_t vc = 0;

            if (m_native_binding) {
                vc = m_native_binding->m_buttons_mappings[btn];
                m_buttons[vc] = state;
            }

            if (state != old_state) {
                button_event(btn, vc, state);
            }
        }

        /* Check axis
		 * Since Xinput doesn't assign ids to axis we just use our ids
		 * and map our ids to our ids, which by default means no changes,
		 * but maybe someone really wants to bind his right trigger to his
		 * left trigger
		 */
#define CHECK(var, m, id)                                                      \
    if (m_current_state.var != m_old_state.var) {                              \
        uint16_t vc = 0;                                                       \
        if (m_native_binding) {                                                \
            vc = m_native_binding->m_axis_mappings[id];                        \
            m_axis[vc] = m_current_state.bLeftTrigger / (float(m));            \
        }                                                                      \
        if (abs(m_current_state.var - m_old_state.var) > m_axis_deadzones[vc]) \
            axis_event(id, vc, m_current_state.var);                           \
    }

        CHECK(bRightTrigger, 0xff, axis::RIGHT_TRIGGER);
        CHECK(bLeftTrigger, 0xff, axis::LEFT_TRIGGER);
        CHECK(sThumbLX, 0xffff, axis::LEFT_STICK_X);
        CHECK(sThumbLY, 0xffff, axis::LEFT_STICK_Y);
        CHECK(sThumbRX, 0xffff, axis::RIGHT_STICK_X);
        CHECK(sThumbRY, 0xffff, axis::RIGHT_STICK_Y);

#undef CHECK
        m_old_state = m_current_state;
    } else {
        m_valid = false;
    }
}

void device_xinput::set_binding(shared_ptr<cfg::binding>&& b)
{
    device::set_binding(move(b));
    m_native_binding = dynamic_cast<cfg::binding_xinput*>(b.get());
}
}
