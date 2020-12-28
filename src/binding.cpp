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

using namespace json11;

namespace gamepad {
namespace cfg {

    binding::binding(const std::string& json)
    {
        binding::load(json);
    }

    binding::binding(const Json& j)
    {
        binding::load(j);
    }

    void binding::copy(const std::shared_ptr<binding> other)
    {
        m_axis_mappings = other->m_axis_mappings;
        m_buttons_mappings = other->m_buttons_mappings;
        m_binding_name = other->m_binding_name;
    }

    bool binding::load(const Json& j)
    {
        bool result = false;
        if (j.is_object()) {
            m_binding_name = j["name"].string_value();
            auto arr = j["binds"];

            if (arr.is_array()) {
                m_axis_mappings.clear();
                m_buttons_mappings.clear();

                for (const auto& val : arr.array_items()) {
                    if (val["is_axis"].bool_value()) {
                        m_axis_mappings[val["from"].int_value()] = val["to"].int_value();
                    } else {
                        m_buttons_mappings[val["from"].int_value()] = val["to"].int_value();
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

    void binding::save(Json& j) const
    {
        Json name = Json::object { { "name", m_binding_name } };
        Json arr;
        std::vector<Json> binds;

        for (const auto& val : m_axis_mappings) {
            Json obj = Json::object { { "is_axis", true }, { "from", val.first }, { "to", val.second } };
            binds.emplace_back(obj);
        }

        for (const auto& val : m_buttons_mappings) {
            Json obj = Json::object { { "is_axis", false }, { "from", val.first }, { "to", val.second } };
            binds.emplace_back(obj);
        }
        j = Json::object { { "name", m_binding_name }, { "binds", binds } };
    }

    bool binding::load(const std::string& json)
    {
        std::string err;
        const auto j = Json::parse(json.c_str(), err);
        if (err.empty()) {
            return load(j);
        }
        gerr("Couldn't load JSON: %s", err.c_str());
        return false;
    }

    void binding::save(std::string& json)
    {
        Json j;
        save(j);
        j.dump(json);
    }

}
}
