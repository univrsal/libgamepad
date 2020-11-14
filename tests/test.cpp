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
    h->set_sleep_time(5);
    if (!h->start()) {
        gerr("Couldn't start hook");
        return 0;
    }

    if (h->get_devices().size() < 1) {
        bool flag = true;
        while (flag && run_flag) {
            h->get_mutex()->lock();
            if (h->get_devices().size() > 0)
                flag = false;
            h->get_mutex()->unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (!run_flag)
            return 0;
    }

    auto devs = h->get_devices();

    const auto& dev = devs[0];

    if (dev->has_binding()) {
        ginfo("First device already has default bindings");

        auto button_handler = [](std::shared_ptr<gamepad::device> dev) {
            ginfo("Received button event: Native id: %i, Virtual id: %i val: %i",
                dev->last_button_event()->native_id, dev->last_button_event()->vc,
                dev->last_button_event()->value);
        };

        auto axis_handler = [](std::shared_ptr<gamepad::device> dev) {
            ginfo("Received axis event: Native id: %i, Virtual id: %i val: %i", dev->last_axis_event()->native_id,
                dev->last_axis_event()->vc, dev->last_axis_event()->value);
        };

        h->set_axis_event_handler(axis_handler);
        h->set_button_event_handler(button_handler);

        while (run_flag)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    } else {
#ifdef LGP_ENABLE_JSON
        ginfo("Found device, running config wizard");
        json11::Json cfg;
        h->make_xbox_config(dev, cfg);
        ginfo("Result config: %s", cfg.dump().c_str());
#else
        ginfo("Json isn't enabled for libgamepad, so the config wizard can't be used");
#endif
    }
}
