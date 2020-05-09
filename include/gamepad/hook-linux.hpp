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
#include <gamepad/hook.hpp>
#include <gamepad/config.h>

#ifdef LGP_LINUX
namespace gamepad {
    class hook_linux : public hook {
    public:
        void close_devices() override;
        void query_devices() override;
        bool start() override;
        void make_xbox_config(const std::shared_ptr<gamepad::device> &dv,
                              json &out) override;

    };
}
#endif
