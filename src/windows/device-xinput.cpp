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
    m_name = XINPUT_DEVICE_NAME_BASE + to_string(id);
    // Values range from 0 - 255, so we don't really need any deadzones
    m_axis_deadzones[axis::LEFT_TRIGGER] = 0;
    m_axis_deadzones[axis::RIGHT_TRIGGER] = 0;
    device_xinput::update();
}

int device_xinput::update()
{
    int result = 0;
    if (m_xinput_refresh(m_id, &m_current_state) == ERROR_SUCCESS) {
        for (const auto& btn : XINPUT_BUTTONS) {
            bool state = m_current_state.wButtons & btn;
            bool old_state = m_old_state.wButtons & btn;
            uint16_t vc = 0;
            float vv = 0.0f;

            if (m_native_binding) {
                vc = m_native_binding->m_buttons_mappings[btn];
                m_buttons[vc] = state;
                vv = state ? 1.0f : 0.0f;
            }

            if (state != old_state) {
                button_event(btn, vc, state, vv);
                result |= update_result::BUTTON;
            }
        }

        /* Check axis
         * Since Xinput doesn't assign ids to axis we just use our ids
         * and map our ids to our ids, which by default means no changes,
         * but maybe someone really wants to bind his right trigger to his
         * left trigger
         */
#define CHECK(var, m, id, mult)                                                  \
    if (m_current_state.var != m_old_state.var) {                                \
        uint16_t vc = 0;                                                         \
        float vv = 0.0f;                                                         \
        if (m_native_binding) {                                                  \
            vc = m_native_binding->m_axis_mappings[id];                          \
            vv = clamp(m_current_state.var / (float(m)) * mult, -1, 1);          \
            if (vc == axis::LEFT_STICK_Y || vc == axis::RIGHT_STICK_Y)           \
                vv *= -1; /* Xinput inverts them for some reason */              \
            m_axis[vc] = vv;                                                     \
        }                                                                        \
        if (abs(m_current_state.var - m_old_state.var) > m_axis_deadzones[vc]) { \
            axis_event(id, vc, m_current_state.var, vv);                         \
            result |= update_result::AXIS;                                       \
        }                                                                        \
    }

        CHECK(bRightTrigger, 0xff, axis::RIGHT_TRIGGER, 1);
        CHECK(bLeftTrigger, 0xff, axis::LEFT_TRIGGER, 1);
        CHECK(sThumbLX, 0xffff, axis::LEFT_STICK_X, 2);
        CHECK(sThumbLY, 0xffff, axis::LEFT_STICK_Y, 2);
        CHECK(sThumbRX, 0xffff, axis::RIGHT_STICK_X, 2);
        CHECK(sThumbRY, 0xffff, axis::RIGHT_STICK_Y, 2);

#undef CHECK
        m_old_state = m_current_state;
    } else {
        m_valid = false;
    }
    return result;
}

void device_xinput::set_binding(shared_ptr<cfg::binding> b)
{
    device::set_binding(b);
    m_native_binding = dynamic_cast<cfg::binding_xinput*>(b.get());
}
}
