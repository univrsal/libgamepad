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
#include <gamepad/binding-dinput.hpp>
#include <gamepad/hook.hpp>

namespace gamepad::cfg {
json dinput_default_binding = json::parse(gamepad::defaults::dinput_bind_json);

binding_dinput::binding_dinput(const json& j)
{
    load(j);
}

void binding_dinput::load(const json& j)
{
    m_axis_mappings.clear();
    m_buttons_mappings.clear();

    for (const auto& val : j) {
        if (val["is_axis"]) {
            m_axis_mappings[val["from"]] = val["to"];
            if (val["to"] == axis::LEFT_TRIGGER) {
                m_left_trigger_polarity = val["trigger_polarity"];
            } else if (val["to"] == axis::RIGHT_TRIGGER) {
                m_right_trigger_polarity = val["trigger_polarity"];
            }
        } else {
            m_buttons_mappings[val["from"]] = val["to"];
        }
    }
}

void binding_dinput::save(json& j)
{
    for (const auto& val : m_axis_mappings) {
        json obj;
        obj["is_axis"] = true;
        obj["from"] = val.first;
        obj["to"] = val.second;
        if (val.second == axis::LEFT_TRIGGER) {
            obj["trigger_polarity"] = m_left_trigger_polarity;
        } else if (val.second == axis::RIGHT_TRIGGER) {
            obj["trigger_polarity"] = m_right_trigger_polarity;
        }
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