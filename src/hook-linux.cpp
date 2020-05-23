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
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <gamepad/hook-linux.hpp>
#include <gamepad/log.hpp>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

using namespace std;

namespace gamepad {

void hook_linux::query_devices()
{
    close_devices();
    m_mutex.lock();
    for (const auto& entry : fs::directory_iterator("/dev/input/by-id")) {
        if (!entry.is_directory()) {
            auto path = entry.path().string();
            auto path_cpy = path;
            transform(path_cpy.begin(), path_cpy.end(), path_cpy.begin(),
                [](unsigned char c) { return tolower(c); });

            if ((path.find("gamepad") != string::npos || path.find("joystick") != string::npos) && path.find("event") == string::npos) {
                gdebug("Found potential gamepad at '%s'", path.c_str());
                auto dev = make_shared<device_linux>(path);
                if (dev->is_valid()) {
                    m_devices.emplace_back(dev);
                    auto b = get_binding_for_device(dev->get_id());

                    if (b) {
                        dev->set_binding(move(b));
                    } else {
                        auto b = make_shared<cfg::binding_linux>(cfg::linux_default_binding);
                        dev->set_binding(dynamic_pointer_cast<cfg::binding>(b));
                    }
                }
            }
        }
    }
    m_mutex.unlock();
}

shared_ptr<cfg::binding> hook_linux::make_native_binding(const json& j)
{
    return make_shared<cfg::binding_linux>(j);
}

}
