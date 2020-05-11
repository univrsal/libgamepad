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
                    /* TODO: use make_shared and just let the pointer free
                     * by leaving the scope */
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

    bool hook_linux::load_bindings(const json &j)
    {
        for (const auto &bind : j) {
            json obj = bind["binds"];
            std::string device = bind["device"];
            auto loaded_binding = make_shared<cfg::binding_linux>(obj);
            if (m_bindings[device]) {
                m_bindings[device].reset();
            }
            m_bindings[device] = loaded_binding;
        }
        return true;
    }
}
