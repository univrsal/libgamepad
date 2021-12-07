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

#include <chrono>
#include <libgamepad.hpp>
#include <thread>
#ifdef LGP_UNIX
#include <csignal>
#else
#include <windows.h>
#endif
static volatile bool run_flag = true;

#ifdef LGP_UNIX
void handler(int s)
#else
BOOL WINAPI handler(DWORD s)
#endif
{
    LGP_UNUSED(s);
    run_flag = false;
#ifdef LGP_WINDOWS
    return TRUE;
#endif
}

int main()
{
#ifdef LGP_UNIX
    signal(SIGINT, handler);
#else
    SetConsoleCtrlHandler(handler, TRUE);
#endif

    auto h = gamepad::hook::make();
    h->set_plug_and_play(true, gamepad::ms(1000));
    h->set_sleep_time(gamepad::ms(5)); // just std::chrono::milliseconds

    auto button_handler = [](std::shared_ptr<gamepad::device> dev) {
        ginfo("Received button event: Native id: %i, Virtual id: 0x%X (%i) val: %f",
            dev->last_button_event()->native_id, dev->last_button_event()->vc,
            dev->last_button_event()->vc, dev->last_button_event()->virtual_value);
    };

    auto axis_handler = [](std::shared_ptr<gamepad::device> dev) {
        ginfo("Received axis event: Native id: %i, Virtual id: 0x%X (%i) val: %f", dev->last_axis_event()->native_id,
            dev->last_axis_event()->vc, dev->last_axis_event()->vc, dev->last_axis_event()->virtual_value);
    };

    auto connect_handler = [h](std::shared_ptr<gamepad::device> dev) {
        ginfo("%s connected", dev->get_name().c_str());
        if (!dev->has_binding()) {
#ifdef LGP_ENABLE_JSON
            ginfo("Found device, running config wizard");
            json11::Json cfg;
            h->make_xbox_config(dev, cfg);
            ginfo("Result config: %s", cfg.dump().c_str());
#else
            ginfo("Json isn't enabled for libgamepad, so the config wizard can't be used");
#endif
        }
    };

    auto disconnect_handler = [](std::shared_ptr<gamepad::device> dev) {
        ginfo("%s disconnected", dev->get_name().c_str());
    };

    h->set_axis_event_handler(axis_handler);
    h->set_button_event_handler(button_handler);
    h->set_connect_event_handler(connect_handler);
    h->set_disconnect_event_handler(disconnect_handler);

    if (!h->start()) {
        gerr("Couldn't start hook");
        return 1;
    }

    while (run_flag)
        std::this_thread::sleep_for(gamepad::ms(50));
}
