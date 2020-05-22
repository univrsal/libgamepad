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

#include <json.hpp>

using json = nlohmann::json;

namespace gamepad::cfg {

class binding {
    std::string m_binding_name;

public:
    binding(const json& j)
    {
        load(j);
    }

    virtual void load(const json& j)
    {
        m_binding_name = j["name"];
    };

    virtual void save(json& j)
    {
        j["name"] = m_binding_name;
    }

    const std::string& get_name() const { return m_binding_name; }
};
}
