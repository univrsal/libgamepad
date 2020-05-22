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

    auto h = gamepad::hook::make(gamepad::hook_type::DIRECT_INPUT);

    if (!h->start()) {
        gerr("Couldn't start hook");
        return 0;
    }

    auto devs = h->get_devices();

    /* Print out devices */
    for (const auto& dev : devs) {
        ginfo("Device ID: %s \t\t\tDevice Name: %s", dev->get_id().c_str(),
            dev->get_name().c_str());
    }

    if (devs.size() < 1) {
        ginfo("No devices found, shutting down");
        return 0;
    }

    auto dev = devs[0];

    if (!dev->has_binding()) {
        ginfo("Found device, running config wizard");
        json cfg;
        h->make_xbox_config(dev, cfg);
        ginfo("Result config: %s", cfg.dump(4).c_str());
    } else {
        ginfo("First device already has default bindings");
        h->get_mutex()->lock();
        auto last_axis = dev->last_axis_event()->time;
        auto last_button = dev->last_button_event()->time;
        h->get_mutex()->unlock();

        while (run_flag) {
            h->get_mutex()->lock();

            if (dev->last_axis_event()->time != last_axis) {
                ginfo("Received axis event: %i val: %i", dev->last_axis_event()->id,
                    dev->last_axis_event()->value);
                last_axis = dev->last_axis_event()->time;
            }

            if (dev->last_button_event()->time != last_button) {
                ginfo("Received button event: %i val: %i", dev->last_button_event()->id,
                    dev->last_button_event()->value);
                last_button = dev->last_button_event()->time;
            }

            h->get_mutex()->unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}
