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
#include <gamepad/binding-dinput.hpp>
#include <gamepad/hook.hpp>
#include <gamepad/log.hpp>

using namespace json11;

namespace gamepad {
namespace cfg {
    static std::string default_error;
    Json dinput_default_binding = Json::parse(defaults::dinput_bind_json, default_error);

    binding_dinput::binding_dinput(const std::string& json)
    {
        load(json);
    }

    binding_dinput::binding_dinput(const Json& j)
    {
        binding_dinput::load(j);
    }

    bool binding_dinput::load(const Json& j)
    {
        /* Do not call super method here since
         * we reimplement the entire save process */
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
                        if (val["to"] == axis::LEFT_TRIGGER) {
                            m_left_trigger_polarity = val["trigger_polarity"].int_value();
                        } else if (val["to"] == axis::RIGHT_TRIGGER) {
                            m_right_trigger_polarity = val["trigger_polarity"].int_value();
                        }
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

    void binding_dinput::save(Json& j) const
    {
        /* Do not call super method here since
	     * we reimplement the entire save process */

        std::vector<Json> binds;

        for (const auto& val : m_axis_mappings) {
            Json obj;
            if (val.second == axis::LEFT_TRIGGER) {
                obj = Json::object { { "is_axis", true },
                    { "from", val.first },
                    { "to", val.second },
                    { "trigger_polarity", m_left_trigger_polarity } };
            } else if (val.second == axis::RIGHT_TRIGGER) {
                obj = Json::object { { "is_axis", true }, { "from", val.first }, { "to", val.second }, { "trigger_polarity", m_right_trigger_polarity } };
            } else {
                obj = Json::object { { "is_axis", true }, { "from", val.first }, { "to", val.second } };
            }
            binds.emplace_back(obj);
        }

        for (const auto& val : m_buttons_mappings) {
            Json obj = Json::object { { "is_axis", false }, { "from", val.first }, { "to", val.second } };
            binds.emplace_back(obj);
        }
        j = Json::object { { "name", m_binding_name }, { "binds", binds } };
    }

}
}
