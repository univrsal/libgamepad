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

#include <gamepad/binding.hpp>
#include <gamepad/config.h>

#if LGP_LINUX

namespace gamepad {
    class device_linux;
}

namespace gamepad::cfg {

    extern json linux_default_binding;
    class binding_linux : public binding {
        std::map<uint8_t, uint16_t> m_buttons_mappings;
        std::map<uint8_t, uint16_t> m_axis_mappings;
        friend class gamepad::device_linux;
    public:
        binding_linux() = default;
        binding_linux(const json &json);

        void load(const json &j) override;
        void save(json &j) override;
    };
}
#endif
