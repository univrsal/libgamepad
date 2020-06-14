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

#include <gamepad/binding-default.hpp>
#include <gamepad/binding-linux.hpp>

using namespace json11;

namespace gamepad {
namespace cfg {
    static std::string default_error;
    Json linux_default_binding = Json::parse(gamepad::defaults::linux_bind_json, default_error);

    binding_linux::binding_linux(const std::string& json)
        : binding(json)
    {
    }

    binding_linux::binding_linux(const Json& j)
    {
        load(j);
    }
}
}
