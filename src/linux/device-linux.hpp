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

#include <gamepad/binding-linux.hpp>
#include <gamepad/device.hpp>
#include <linux/joystick.h>
#include <string>

namespace gamepad {

class device_linux : public device {
    std::string m_device_path;
    std::string m_device_id;
    int m_fd;
    struct js_event m_event;
    cfg::binding_linux* m_native_binding = nullptr;

public:
    device_linux(std::string path);
    virtual ~device_linux();

    const std::string& get_id() const override;
    void set_id(const std::string& id) override;
    const std::string& get_path() const { return m_device_path; }
    const std::string& get_cache_id() override { return get_path(); }
    void init() override;
    void deinit() override;
    int update() override;
    void set_binding(std::shared_ptr<cfg::binding> b) override;
};
}
