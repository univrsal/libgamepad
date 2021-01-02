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

#include "device-xinput.hpp"
#include <gamepad/binding-xinput.hpp>
#include <gamepad/hook-dinput.hpp> /* for utf8 conversion */
#include <gamepad/hook-xinput.hpp>
#include <gamepad/log.hpp>

using namespace json11;

namespace gamepad {

void hook_xinput::query_devices()
{
    m_mutex.lock();

    /* Invalidate all devices so we can check later, which one
     * are still connected/newly connected and which ones were
     * disconnected
     */
    for (auto& dev : m_devices)
        dev->invalidate();

    xinput_pad tmp {};

    for (int i = 0; i < LGP_XINPUT_DEVICES; i++) {
        if (m_xinput_refresh(i, &tmp) == ERROR_SUCCESS) {
            gdebug("Xinput device %i present", i);
            std::string id = XINPUT_DEVICE_NAME_BASE + std::to_string(i);

            auto existing_device = get_device_by_id(id);
            auto cached_device = m_device_cache[id];

            if (existing_device) {
                existing_device->set_valid();
            } else if (cached_device) {
                cached_device->set_valid();
                m_devices.emplace_back(cached_device);
                if (m_reconnect_handler)
                    m_reconnect_handler(cached_device);
            } else {
                auto new_device = std::make_shared<device_xinput>(i, m_xinput_refresh);
                new_device->set_index(i);
                m_devices.emplace_back(new_device);
                auto b = get_binding_for_device(new_device->get_id());

                if (b) {
                    new_device->set_binding(std::move(b));
                } else {
                    auto b = std::make_shared<cfg::binding_xinput>(cfg::xinput_default_binding);
                    new_device->set_binding(std::dynamic_pointer_cast<cfg::binding>(b));
                }
                new_device->set_valid();
                m_device_cache[id] = new_device;
                if (m_connect_handler)
                    m_connect_handler(new_device);
            }
        }
    }

    remove_invalid_devices();
    m_mutex.unlock();
}

const json11::Json& hook_xinput::get_default_binding()
{
    return cfg::xinput_default_binding;
}

bool hook_xinput::start()
{
    if (m_running)
        return true;

    TCHAR sys_dir[MAX_PATH];
    GetSystemDirectory(sys_dir, sizeof(sys_dir));
    std::wstring dll = L"\\xinput1_3.dll";
    dll = sys_dir + dll;

    ginfo("Loading Xinput from %s", util::wchar_to_utf8(dll).c_str());

    m_xinput = LoadLibrary(dll.c_str());

    if (m_xinput) {
        m_xinput_refresh = reinterpret_cast<xinput_refresh_t>(GetProcAddress(m_xinput, LPCSTR(100)));

        if (m_xinput_refresh) {
            ginfo("Xinput 1.3 loaded successfuly");
        } else {
            gerr("Loading Xinput failed: %lu", GetLastError());
        }
    } else {
        auto code = GetLastError();
        gerr("Loading Xinput failed: %lu", GetLastError());
        return false;
    }
    return hook::start();
}

std::shared_ptr<cfg::binding> hook_xinput::make_native_binding(const Json& j)
{
    return std::make_shared<cfg::binding_xinput>(j);
}
}
