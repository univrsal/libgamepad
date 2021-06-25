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
#include <dirent.h>
#include <gamepad/hook-linux.hpp>
#include <gamepad/log.hpp>
#include <tuple>
#include <vector>

using namespace std;
using namespace json11;

namespace gamepad {

hook_linux::hook_linux(uint16_t flags)
    : m_flags(flags)
{
}

std::shared_ptr<device> hook_linux::get_device_by_path(const std::string& path)
{
    for (auto& dev : m_devices) {
        if (dynamic_pointer_cast<device_linux>(dev)->get_path() == path)
            return dev;
    }
    return nullptr;
}

void hook_linux::check_dev_by_id()
{
    static const char* DEV_FOLDER = "/dev/input/by-id";
    DIR* dir;
    struct dirent* ent;
    int device_counter = 0;

    if ((dir = opendir(DEV_FOLDER)) == NULL) {
        gerr("Couldn't open %s", DEV_FOLDER);
        return;
    }

    while ((ent = readdir(dir)) != NULL) {
        DIR* dir2 = opendir(ent->d_name); /* NULL if not a directory */
        if (!dir2) {
            auto path = DEV_FOLDER + std::string("/") + std::string(ent->d_name);
            auto path_cpy = path;
            transform(path_cpy.begin(), path_cpy.end(), path_cpy.begin(), [](unsigned char c) { return tolower(c); });

            if ((path.find("gamepad") != string::npos || path.find("joystick") != string::npos) && path.find("event") == string::npos) {
                gdebug("Found potential gamepad at '%s'", path.c_str());
                auto existing_dev = get_device_by_path(path);
                auto cached_dev = m_device_cache[path];

                if (existing_dev) {
                    existing_dev->set_valid();
                    existing_dev->init(); /* Refresh file descriptor if needed */
                } else if (cached_dev) {
                    cached_dev->set_valid();
                    cached_dev->deinit();
                    cached_dev->init();
                    m_devices.emplace_back(cached_dev);
                    if (m_reconnect_handler)
                        m_reconnect_handler(cached_dev);
                } else {
                    auto dev = make_shared<device_linux>(path);
                    if (dev->is_valid()) {
                        dev->set_index(device_counter++);
                        m_devices.emplace_back(dev);
                        auto b = get_binding_for_device(dev->get_id());

                        if (b) {
                            dev->set_binding(move(b));
                        } else {
                            auto b = make_shared<cfg::binding_linux>(cfg::linux_default_binding);
                            dev->set_binding(dynamic_pointer_cast<cfg::binding>(b));
                        }
                        if (m_connect_handler)
                            m_connect_handler(dev);
                        m_device_cache[path] = dev;
                    }
                }
            }
        } else {
            closedir(dir2);
        }
    }
    closedir(dir);
}

void hook_linux::check_js()
{
    static const char* DEV_FOLDER = "/dev/input";
    DIR* dir;
    struct dirent* ent;
    int device_counter = 0;

    if ((dir = opendir(DEV_FOLDER)) == NULL) {
        gerr("Couldn't open %s", DEV_FOLDER);
        return;
    }

    while ((ent = readdir(dir)) != NULL) {
        DIR* dir2 = opendir(ent->d_name); /* NULL if not a directory */
        if (!dir2) {
            auto path = DEV_FOLDER + std::string("/") + std::string(ent->d_name);
            auto path_cpy = path;
            transform(path_cpy.begin(), path_cpy.end(), path_cpy.begin(), [](unsigned char c) { return tolower(c); });

            if (path.find("js") != std::string::npos) {
                gdebug("Found potential gamepad at '%s'", path.c_str());
                auto existing_dev = get_device_by_path(path);
                auto cached_dev = m_device_cache[path];

                if (existing_dev) {
                    existing_dev->set_valid();
                    existing_dev->init(); /* Refresh file descriptor if needed */
                } else if (cached_dev) {
                    gdebug("Using cached device instance");
                    cached_dev->set_valid();
                    cached_dev->deinit();
                    cached_dev->init();
                    m_devices.emplace_back(cached_dev);
                    if (m_reconnect_handler)
                        m_reconnect_handler(cached_dev);
                } else {
                    auto dev = make_shared<device_linux>(path);
                    if (dev->is_valid()) {
                        dev->set_index(device_counter++);
                        m_devices.emplace_back(dev);
                        auto b = get_binding_for_device(dev->get_id());

                        if (b) {
                            dev->set_binding(move(b));
                        } else {
                            auto b = make_shared<cfg::binding_linux>(cfg::linux_default_binding);
                            dev->set_binding(dynamic_pointer_cast<cfg::binding>(b));
                        }
                        if (m_connect_handler)
                            m_connect_handler(dev);
                        m_device_cache[path] = dev;
                    } else {
                        gdebug("'%s' is not a valid gamepad", path.c_str());
                    }
                }
            }
        } else {
            closedir(dir2);
        }
    }
    closedir(dir);
}

void hook_linux::query_devices()
{
    m_mutex.lock();
    /* Invalidate all devices so we can check later, which one
     * are still connected/newly connected and which ones were
     * disconnected
     */
    for (auto& dev : m_devices)
        dev->invalidate();

    if (m_flags & hook_type::JS)
        check_js();
    else
        check_dev_by_id();

    remove_invalid_devices();
    m_mutex.unlock();
}

shared_ptr<cfg::binding> hook_linux::make_native_binding(const Json& j)
{
    return make_shared<cfg::binding_linux>(j);
}

const Json& hook_linux::get_default_binding()
{
    return cfg::linux_default_binding;
}

}
