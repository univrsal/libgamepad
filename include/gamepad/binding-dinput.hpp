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

namespace gamepad
{
    class device_dinput;
}

namespace gamepad::cfg
{
    extern json dinput_default_binding;

    class binding_dinput : public binding
    {
        std::map<uint8_t, uint16_t> m_buttons_mappings;
        std::map<uint8_t, uint16_t> m_axis_mappings;
        int m_left_trigger_polarity = 0, m_right_trigger_polarity = 0;
        friend class gamepad::device_dinput;
    public:
        binding_dinput(const json &j);
        void load(const json &j) override;
        void save(json &j) override;
    };
}