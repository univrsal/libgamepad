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

#include <json/json11.hpp>

namespace gamepad {
namespace cfg {

    class binding {
    protected:
        std::string m_binding_name;
        std::map<uint16_t, uint16_t> m_buttons_mappings;
        std::map<uint16_t, uint16_t> m_axis_mappings;

    public:
        binding() = default;

#ifdef LGP_ENABLE_JSON
        binding(const json11::Json& j);

        virtual bool load(const json11::Json& j);
        virtual void save(json11::Json& j) const;
#endif

        virtual bool load(const std::string& json);
        virtual void save(std::string& json);

        const std::string& get_name() const { return m_binding_name; }
    };
}
}
