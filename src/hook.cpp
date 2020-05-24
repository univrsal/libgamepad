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
#include <fstream>
#include <gamepad/hook-dinput.hpp>
#include <gamepad/hook-linux.hpp>
#include <gamepad/hook-xinput.hpp>
#include <gamepad/hook.hpp>
#include <gamepad/log.hpp>
#include <iomanip>

using namespace std;

namespace gamepad {
vector<tuple<string, uint16_t>> hook::button_prompts = {
    { "A", button::A },
    { "B", button::B },
    { "X", button::X },
    { "Y", button::Y },
    { "Back", button::BACK },
    { "Start", button::START },
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

vector<tuple<string, uint16_t>> hook::axis_prompts = {
    { "left analog stick horizontally", axis::LEFT_STICK_X },
    { "left analog stick vertically", axis::LEFT_STICK_Y },
    { "left trigger", axis::LEFT_TRIGGER },
    { "right analog stick horizontally", axis::RIGHT_STICK_X },
    { "right analog vertically", axis::RIGHT_STICK_Y },
    { "right trigger", axis::RIGHT_TRIGGER }
};

void default_hook_thread(class hook* h)
{
    auto sleep_time = h->get_sleep_time();
    ginfo("Hook thread started");
    while (h->running()) {

        if (h->get_devices().size() < 1) {
            h->query_devices();
        } else {
            h->get_mutex()->lock();
            for (auto& dev : h->get_devices())
                dev->update();
            sleep_time = h->get_sleep_time();
            h->get_mutex()->unlock();
        }
        this_thread::sleep_for(chrono::milliseconds(sleep_time));
    }
    ginfo("Hook thread ended");
}

uint64_t hook::ms_ticks()
{
    auto now = chrono::system_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
}

std::shared_ptr<cfg::binding> hook::get_binding_for_device(const std::string& id)
{
    auto result = find_if(m_devices.begin(), m_devices.end(),
        [&id](shared_ptr<device>& b) {
            return b->get_id() == id;
        });

    return (*result)->get_binding();
}

std::shared_ptr<hook> hook::make(hook_type type)
{
#if LGP_WINDOWS
    if (type == hook_type::XINPUT || type == hook_type::NATIVE_DEFAULT) {
        return std::make_shared<hook_xinput>();
    }
    return std::make_shared<hook_dinput>();
#elif LGP_LINUX
    LGP_UNUSED(type);
    return std::make_shared<hook_linux>();
#else
#error "No native hook found"
    return nullptr;
#endif
}

void hook::close_devices()
{
    m_mutex.lock();
    for (size_t i = 0; i < m_devices.size(); i++) {
        /* Tell any left over references that this instance isn't updated anymore */
        m_devices[i]->invalidate();
        if (m_devices[i].use_count() > 1) {
            gwarn("Gamepad device '%s' is still in use! (Ref count %li)",
                m_devices[i]->get_id().c_str(), m_devices[i].use_count());
        }
    }
    m_devices.clear();
    m_mutex.unlock();
}

void hook::close_bindings()
{
    m_mutex.lock();
    for (const auto& bind : m_bindings) {
        /* One reference in the bindings list
         * and one for this for loop */
        if (bind.use_count() > 2) {
            gerr("Gamepad binding '%s' is still in use! (Ref count %li)",
                bind->get_name().c_str(), bind.use_count());
        }
    }

    m_bindings.clear();
    m_mutex.unlock();
}

bool hook::start()
{
    query_devices();

    if (m_devices.size() < 1)
        ginfo("No Devices detected. Waiting for connection...");
    m_running = true;
    m_hook_thread = thread(default_hook_thread, this);
    return true;
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

bool hook::save_bindings(const std::string& path)
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

bool hook::save_bindings(json& j)
{
    json binding_array, binding_map;

    for (const auto& bind : m_bindings) {
        json b;
        bind->save(b);
        binding_array.emplace_back(b);
    }

    for (const auto& device : m_devices) {
        if (device->has_binding()) {
            json m;
            m["device_id"] = device->get_id();
            m["binding_id"] = device->get_binding()->get_name();
        }
    }

    j["bindings"] = binding_array;
    j["bindings_map"] = binding_map;

    return true;
}

bool hook::load_bindings(const std::string& path)
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

bool hook::load_bindings(const json& j)
{
    json device_array = j["devices"],
         binding_array = j["bindings"];

    for (const auto& bind : binding_array)
        m_bindings.emplace_back(make_native_binding(bind));

    for (const auto& entry : device_array) {
        auto device_id = entry["device_id"];
        auto bind_id = entry["binding_id"];

        if (!set_device_binding(device_id, bind_id))
            gwarn("Couldn't set binding.");
    }
    return true;
}

bool hook::set_device_binding(const std::string& device_id,
    const std::string& binding_id)
{
    auto dev = get_device_by_id(device_id);
    auto result = false;

    if (dev) {
        auto bind = get_binding_by_name(binding_id);
        if (bind)
            dev->set_binding(move(bind));
        else
            gwarn("No binding with name '%s'", binding_id.c_str());
    } else {
        gwarn("No device with id '%s'", device_id.c_str());
    }

    return result;
}

shared_ptr<device> hook::get_device_by_id(const std::string& id)
{
    auto result = find_if(m_devices.begin(), m_devices.end(),
        [&id](shared_ptr<device>& d) {
            return d->get_id() == id;
        });

    if (result == m_devices.end())
        return nullptr;
    return *result;
}

std::shared_ptr<cfg::binding> hook::get_binding_by_name(const std::string& name)
{
    auto result = find_if(m_bindings.begin(), m_bindings.end(),
        [&name](shared_ptr<cfg::binding>& b) {
            return b->get_name() == name;
        });

    if (result == m_bindings.end())
        return nullptr;
    return *result;
}

void hook::make_xbox_config(const std::shared_ptr<gamepad::device>& dv,
    json& out)
{
    if (!m_running)
        return;

    ginfo("Starting config creation wizard");
    uint16_t sleep_time = get_sleep_time();
    uint64_t last_key_input = 0;
    bool running = true;
    mutex key_thread_mutex;

    auto key_thread_method = [&]() {
        while (running) {
            getchar();
            key_thread_mutex.lock();
            last_key_input = ms_ticks();
            key_thread_mutex.unlock();
        }
    };

    thread key_thread(key_thread_method);

    auto binder = [&](const char* prompt, bool axis,
                      const input_event* e, uint16_t* last,
                      const vector<tuple<string, uint16_t>>& prompts) {
        for (const auto& p : prompts) {
            ginfo("Please %s %s on your gamepad or press enter on your keyboard "
                  "to skip this bind.",
                prompt, get<0>(p).c_str());
            bool success = false;
            for (;;) {
                m_mutex.lock();
                if (e->id != *last && e->value != 0) {
                    *last = e->id;
                    m_mutex.unlock();
                    success = true;
                    break;
                }
                m_mutex.unlock();

                key_thread_mutex.lock();
                if (ms_ticks() - last_key_input < 100) {
                    ginfo("Received key input, skipping bind...");
                    last_key_input = 0;
                    key_thread_mutex.unlock();
                    break;
                }
                key_thread_mutex.unlock();
                this_thread::sleep_for(chrono::milliseconds(sleep_time));
            }

            if (!success)
                continue;

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
    key_thread_mutex.lock();
    running = false;
    key_thread_mutex.unlock();
    ginfo("Done. Press Enter to print config json.");
    key_thread.join();
}

}
