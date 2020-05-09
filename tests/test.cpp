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

#include <libgamepad.hpp>

int main()
{
    auto h = gamepad::hook::make();

    if (h->start()) {
        ginfo("Started hook");
    } else {
        gerr("Couldn't start hook");
    }

    auto devs = h->get_devices();

    if (devs.size() > 0 && !devs[0]->has_binding()) {
        ginfo("Found device, running config wizard");
        auto dev = devs[0];
        json cfg;
        h->make_xbox_config(dev, cfg);
        ginfo("Result config: %s", cfg.dump(4).c_str());
    } else {
        ginfo("First device already has bindings");
    }
}
