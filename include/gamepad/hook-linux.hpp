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
#include "config.h"
#include "hook.hpp"

#ifdef LGP_LINUX
namespace gamepad {
class hook_linux : public hook {

    /* Check for devices in /dev/input/by-id, which gives us better device ids
     * to tell similiar gamepads apart, but will cause issues when using xboxdrv
     * or gamepads that don't show up in /dev/input/by-id
     */
    void check_dev_by_id();

    /* Check for devices in /dev/js*, sadly this doesn't give us much to identify the gamepad
     * so if you have multiple identical gamepads identification will come down to which
     * /dev/js* path they're connected as, which usually means the order in which they are
     * connected*/
    void check_js();
    const uint16_t m_flags = 0;

public:
    hook_linux(uint16_t flags);

#ifdef LGP_ENABLE_JSON
    virtual std::shared_ptr<cfg::binding> make_native_binding(const json11::Json& j) override;
    virtual const json11::Json& get_default_binding() override;
#endif

    void query_devices() override;
    std::shared_ptr<device> get_device_by_path(const std::string& path);
};
}
#endif
