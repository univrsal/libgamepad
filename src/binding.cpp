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

#include <gamepad/binding.hpp>
#include <gamepad/log.hpp>

namespace gamepad {
namespace cfg {

    binding::binding(const json& j)
    {
        binding::load(j);
    }

    bool binding::load(const json& j)
    {
        bool result = false;
        if (j.is_object()) {
            m_binding_name = j["name"].get<std::string>();
            auto arr = j["binds"];

            if (arr.is_array()) {
                m_axis_mappings.clear();
                m_buttons_mappings.clear();

                for (const auto& val : arr) {
                    if (val["is_axis"]) {
                        m_axis_mappings[val["from"]] = val["to"];
                    } else {
                        m_buttons_mappings[val["from"]] = val["to"];
                    }
                }
                result = true;
            } else {
                gerr("Expected json array when loading gamepad bindings");
            }
        } else {
            gerr("Expected json object when loading gamepad binding");
        }
        return result;
    }

    void binding::save(json& j) const
    {
        j["name"] = m_binding_name;
        json arr = json::array();

        for (const auto& val : m_axis_mappings) {
            json obj;
            obj["is_axis"] = true;
            obj["from"] = val.first;
            obj["to"] = val.second;
            arr.push_back(obj);
        }

        for (const auto& val : m_buttons_mappings) {
            json obj;
            obj["is_axis"] = false;
            obj["from"] = val.first;
            obj["to"] = val.second;
            arr.push_back(obj);
        }
        j["binds"] = arr;
    }

}
}
