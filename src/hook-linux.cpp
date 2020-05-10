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

#include "device-linux.hpp"
#include <gamepad/hook-linux.hpp>
#include <gamepad/log.hpp>
#include <filesystem>
#include <algorithm>
#include <tuple>
#include <chrono>
#include <vector>

namespace fs = std::filesystem;

using namespace std;

namespace gamepad {

    static void thread_method(hook_linux *h)
    {
        gdebug("Linux hook thread started");
        uint16_t sleep_time = h->get_sleep_time();

        while (h->running()) {
            h->get_mutex()->lock();
            for (auto &dev : h->get_devices()) {
                dev->update();
            }
            sleep_time = h->get_sleep_time();
            h->get_mutex()->unlock();
            this_thread::sleep_for(chrono::milliseconds(sleep_time));
        }
    }

    void hook_linux::query_devices()
    {
        close_devices();
        m_mutex.lock();
        for (const auto &entry : fs::directory_iterator("/dev/input/by-id")) {
            if (!entry.is_directory()) {
                auto path = entry.path().string();
                auto path_cpy = path;
                transform(path_cpy.begin(), path_cpy.end(), path_cpy.begin(),
                               [](unsigned char c){ return tolower(c); });

                if ((path.find("gamepad") != string::npos ||
                    path.find("joystick") != string::npos) &&
                    path.find("event") == string::npos)
                {
                    gdebug("Found potential gamepad at '%s'", path.c_str());
                    device_linux *dev = new device_linux(path);
                    if (dev->is_valid()) {
                        m_devices.emplace_back(dev);
                        auto b = get_binding_for_device(dev->get_id());

                        if (b) {
                            dev->set_binding(move(b));
                        } else {
                            auto b = make_shared<cfg::binding_linux>(cfg::linux_default_binding);
                            m_bindings[dev->get_id()] = b;
                            dev->set_binding(dynamic_pointer_cast<cfg::binding>(b));
                        }
                    } else {
                        delete dev;
                    }
                }
            }
        }
        m_mutex.unlock();
    }

    bool hook_linux::start()
    {
        bool result = false;
        query_devices();

        if (m_devices.size() > 0) {
            m_hook_thread = thread(thread_method, this);
            result = true;
        }
        m_running = result;
        return result;
    }

    void hook_linux::make_xbox_config(const shared_ptr<gamepad::device>
                                       &dv, json &out)
    {
        if (!m_running)
            return;
        vector<tuple<string, uint16_t>> button_prompts = {
            { "A", 		button::A },
            { "B", 		button::B },
            { "X", 		button::X },
            { "Y",		button::Y },
            { "Back", 	button::BACK },
            { "Start", 	button::START },
            { "Guide (Big circle in the middle)", button::GUIDE },
            { "left analog stick", button::L_THUMB },
            { "right analog stick", button::R_THUMB },
            { "dpad left", button::DPAD_LEFT },
            { "dpad right", button::DPAD_RIGHT },
            { "dpad up", button::DPAD_UP },
            { "dpad down", button::DPAD_DOWN },
            { "LB", button::LB },
            { "RB", button::RB }
        };

        vector<tuple<string, uint16_t>> axis_prompts = {
            { "left analog stick horizontally", axis::LEFT_STICK_X },
            { "left analog stick vertically", axis::LEFT_STICK_Y },
            { "left trigger", axis::LEFT_TRIGGER },
            { "right analog stick horizontally", axis::RIGHT_STICK_X },
            {"right analog vertically", axis::RIGHT_STICK_Y },
            { "right trigger", axis::RIGHT_TRIGGER }
        };

        ginfo("Starting config creation wizard");
        uint16_t sleep_time = get_sleep_time();

        auto binder = [&](const char *prompt, bool axis,
                      const input_event *e, uint16_t *last,
                      const vector<tuple<string, uint16_t>> &prompts) {
            for (const auto &p : prompts) {
                ginfo("Please %s %s on your gamepad.", prompt, get<0>(p).c_str());
                for (;;) {
                    m_mutex.lock();
                    if (e->id != *last) {
                        *last = e->id;
                        m_mutex.unlock();
                        break;
                    }
                    m_mutex.unlock();
                    this_thread::sleep_for(chrono::milliseconds(sleep_time));
                }

                ginfo("Received input with id %i", *last);
                json bind;
                bind["is_axis"] = axis;
                bind["from"] = *last;
                bind["to"] = get<1>(p);
                out.emplace_back(bind);
            }
        };

        uint16_t last_button = dv->last_button_event()->id,
                last_axis = dv->last_axis_event()->id;

        /* Button bindings */
        binder("press", false, dv->last_button_event(), &last_button,
               button_prompts);

        /* Axis bindings */
        binder("move", true, dv->last_axis_event(), &last_axis, axis_prompts);
    }
}
