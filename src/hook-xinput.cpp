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

#include <gamepad/hook-xinput.hpp>

namespace gamepad {
void hook_xinput::query_devices()
{
}

bool hook_xinput::load_bindings(const json& j)
{
    return false;
}

bool hook_xinput::start()
{
    return false;
}

void hook_xinput::make_xbox_config(const std::shared_ptr<gamepad::device>& dv, json& out)
{
}

}
