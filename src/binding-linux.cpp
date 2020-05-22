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

#include "binding-default.hpp"
#include <gamepad/binding-linux.hpp>

namespace gamepad::cfg {
json linux_default_binding = json::parse(gamepad::defaults::linux_bind_json);

binding_linux::binding_linux(const json& j)
    : binding(j)
{
}

void binding_linux::load(const json& j)
{
    binding::load(j);
    m_axis_mappings.clear();
    m_buttons_mappings.clear();

    for (const auto& val : j) {
        if (val["is_axis"]) {
            m_axis_mappings[val["from"]] = val["to"];
        } else {
            m_buttons_mappings[val["from"]] = val["to"];
        }
    }
}

void binding_linux::save(json& j)
{
    binding::save(j);

    for (const auto& val : m_axis_mappings) {
        json obj;
        obj["is_axis"] = true;
        obj["from"] = val.first;
        obj["to"] = val.second;
        j.push_back(obj);
    }

    for (const auto& val : m_buttons_mappings) {
        json obj;
        obj["is_axis"] = false;
        obj["from"] = val.first;
        obj["to"] = val.second;
        j.push_back(obj);
    }
}
}
