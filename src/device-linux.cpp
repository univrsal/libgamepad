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

#include "device-linux.hpp"
#include <algorithm>
#include <fcntl.h>
#include <gamepad/binding-linux.hpp>
#include <gamepad/log.hpp>
#include <unistd.h>

namespace gamepad {
device_linux::device_linux(const std::string path)
    : m_device_path(path)
{
    init();
}

device_linux::~device_linux()
{
    deinit();
}

void device_linux::init()
{
    deinit();
    m_fd = open(m_device_path.c_str(), O_RDONLY | O_NONBLOCK);
    m_valid = m_fd != -1;

    if (m_valid) {
        char namebuffer[256];
        if (ioctl(m_fd, JSIOCGNAME(256), &namebuffer) != -1) {
            m_name = namebuffer;
        }

        if (m_name.empty()) {
            int begin_idx = m_device_path.rfind('/');
            m_name = m_device_path.substr(begin_idx + 1);
            m_device_id = m_name;
        } else {
            int begin_idx = m_device_path.rfind('/');
            std::string fd_name = m_device_path.substr(begin_idx + 1);
            m_device_id = "(" + fd_name + ") " + m_name;
        }
        gdebug("Initialized gamepad from '%s' with id '%s'",
            m_device_path.c_str(), m_device_id.c_str());
    }
}

void device_linux::deinit()
{
    close(m_fd);
    m_fd = 0;
}

const std::string& device_linux::get_id() const
{
    return m_device_id;
}

inline float clamp(float x, float upper, float lower)
{
    return fminf(upper, fmaxf(x, lower));
}

void device_linux::update()
{
    /* This will only process the last event, since any other
         * queued up events aren't useful anymore, I think */
    while (read(m_fd, &m_event, sizeof(m_event)) > 0)
        ;

    if (m_event.type == JS_EVENT_AXIS) {
        if (m_native_binding) {
            auto new_code = m_native_binding->m_axis_mappings[m_event.number];
            auto val = float(m_event.value);

            m_axis[new_code] = clamp(val / 0xffff, -1.f, 1.f);
        }

        m_last_axis_event.id = m_event.number;
        m_last_axis_event.value = m_event.value;
        m_last_axis_event.time = m_event.time;
    } else if (m_event.type == JS_EVENT_BUTTON) {
        if (m_native_binding) {
            auto new_code = m_native_binding->m_buttons_mappings[m_event.number];
            m_buttons[new_code] = m_event.value;
        }
        m_last_button_event.id = m_event.number;
        m_last_button_event.value = m_event.value;
        m_last_button_event.time = m_event.time;
    }
}

void device_linux::set_binding(std::shared_ptr<cfg::binding>&& b)
{
    device::set_binding(std::move(b));
    m_native_binding = dynamic_cast<cfg::binding_linux*>(b.get());
}
}
