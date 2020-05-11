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

#include <gamepad/log.hpp>
#include <gamepad/hook.hpp>
#include <gamepad/hook-dinput.hpp>
#include <gamepad/hook-xinput.hpp>
#include <gamepad/hook-linux.hpp>
#include <fstream>
#include <iomanip>
#include <chrono>

using namespace std;

namespace gamepad {
    void default_hook_thread(class hook *h)
    {
        auto sleep_time = h->get_sleep_time();

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

    std::shared_ptr<cfg::binding> hook::get_binding_for_device(const std::string &id)
    {
        return m_bindings[id];
    }

    std::shared_ptr<hook> hook::make(hook_type type)
    {
#if LGP_WINDOWS
        if (type == hook_type::XINPUT) {
            return std::make_shared<hook_xinput>();
        } else {
            return std::make_shared<hook_dinput>();
        }
#elif LGP_LINUX
        LGP_UNUSED(type);
        return std::make_shared<hook_linux>();
#endif
    }

    void hook::close_devices()
    {
        m_mutex.lock();
        for (size_t i = 0; i < m_devices.size(); i++) {
            if (m_devices[i].use_count() > 1) {
                gerr("Gamepad device '%s' is still in use! (Ref count %li)",
                     m_devices[i]->get_id().c_str(), m_devices[i].use_count());
            }
        }
        m_devices.clear();
        m_mutex.unlock();
    }

    void hook::close_bindings()
    {
        m_mutex.lock();
        for (const auto &bind : m_bindings) {
            if (bind.second.use_count() > 2) { /* The for loop also takes a reference */
                gerr("Gamepad binding for '%s' is still in use! (Ref count %li)",
                     bind.first.c_str(), bind.second.use_count());
            }
        }
        m_bindings.clear();
        m_mutex.unlock();
    }

    void hook::stop()
    {
        close_devices();
        close_bindings();
        if (m_running) {
            m_running = false;
            m_hook_thread.join();
        }
        gdebug("Hook stopped");
    }

    bool hook::save_bindings(const std::string &path)
    {
        json j;
        if (save_bindings(j)) {
            std::ofstream out(path);
            if (out.good()) {
                out << std::setw(4) << j << std::endl;
                out.close();
                return true;
            }
            gerr("Can't write gamepad bindings to '%s'", path.c_str());
        }
        return false;
    }

    bool hook::save_bindings(json &j)
    {
        for (const auto &bind : m_bindings) {
            json obj, binds;
            bind.second->save(binds);
            obj["device"] = bind.first;
            obj["binds"] = binds;
            j.emplace_back(obj);
        }
        /* TODO: error checking? */
        return true;
    }

    bool hook::load_bindings(const std::string &path)
    {
        std::ifstream in(path);
        json j;
        if (in.good()) {
            in >> j;
            if (load_bindings(j)) {
                return true;
            }
            gerr("Couldn't parse json when loading bindings from '%s'",
                 path.c_str());
        }
        gerr("Couldn't read bindings from '%s'", path.c_str());
        return false;
    }

    void hook::make_xbox_config(const std::shared_ptr<gamepad::device>& dv,
        json& out)
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