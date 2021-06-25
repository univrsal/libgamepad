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
    m_fd = -1;
    device_linux::init();
}

device_linux::~device_linux()
{
    device_linux::deinit();
}

void device_linux::init()
{
    /* If this descriptor is still valid do nothing */
    if (fcntl(m_fd, F_GETFD) != -1 || errno != EBADF) {
        gdebug("Descriptor is still valid");
        if (errno == EBADF)
            gerr("errno is EBADF");
        return;
    }

    device_linux::deinit();
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
        gdebug("Initialized gamepad from '%s' with id '%s'", m_device_path.c_str(), m_device_id.c_str());
    } else {
        gerr("Invalid file descriptor from '%s'", m_device_path.c_str());
    }
}

void device_linux::deinit()
{
    if (m_fd < 0)
        return;

    if (close(m_fd) == -1)
        gerr("Couldn't close file descriptor for device '%s'", device_linux::get_id().c_str());

    m_fd = -1;
}

const std::string& device_linux::get_id() const
{
    return m_device_id;
}

int device_linux::update()
{
    /* Process only the next event, this can result in a delay if there are many
     * events queued up, but it'll prevent buttons from getting stuck, which happens
     * if the button released event is skipped */
    read(m_fd, &m_event, sizeof(m_event));
    uint16_t vc = 0;
    float vv = 0.0f;
    int result = update_result::NONE;

    if (m_event.type == JS_EVENT_AXIS) {
        if (m_native_binding) {
            vc = m_native_binding->m_axis_mappings[m_event.number];
            auto val = float(m_event.value);

            if (fabs(val - m_last_axis_event.value) > m_axis_deadzones[vc]) {
                vv = clamp(val / 0xffff + 0.5f, -1.f, 1.f);
                m_axis[vc] = vv;
                result = update_result::AXIS;
            }
        }
        axis_event(m_event.number, vc, m_event.value, vv);
    } else if (m_event.type == JS_EVENT_BUTTON) {
        if (m_native_binding) {
            vc = m_native_binding->m_buttons_mappings[m_event.number];
            vv = m_event.value;
            auto old_value = m_buttons[vc];
            if (old_value != vv) {
                m_buttons[vc] = m_event.value;
                result = update_result::BUTTON;
            }
        }
        button_event(m_event.number, vc, m_event.value, vv);
    }

    return result;
}

void device_linux::set_binding(std::shared_ptr<cfg::binding> b)
{
    device::set_binding(b);
    m_native_binding = dynamic_cast<cfg::binding_linux*>(b.get());
}

void device_linux::set_id(const std::string& id)
{
    m_device_id = id;
}
}
