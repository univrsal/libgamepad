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

#include <algorithm>
#include <chrono>
#include <fstream>
#include <gamepad/hook-dinput.hpp>
#include <gamepad/hook-linux.hpp>
#include <gamepad/hook-xinput.hpp>
#include <gamepad/hook.hpp>
#include <gamepad/log.hpp>
#include <iomanip>

using namespace std;
using namespace json11;

namespace gamepad {
vector<tuple<string, uint16_t>> hook::button_prompts = { { "A", button::A },
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
    { "RB", button::RB } };

vector<tuple<string, uint16_t>> hook::axis_prompts = { { "left analog stick horizontally", axis::LEFT_STICK_X },
    { "left analog stick vertically", axis::LEFT_STICK_Y },
    { "left trigger", axis::LEFT_TRIGGER },
    { "right analog stick horizontally", axis::RIGHT_STICK_X },
    { "right analog vertically", axis::RIGHT_STICK_Y },
    { "right trigger", axis::RIGHT_TRIGGER } };

void default_hook_thread(hook* h)
{
    auto sleep_time = h->m_thread_sleep;

    h->get_mutex()->lock();
    ginfo("Hook thread started");
    h->get_mutex()->unlock();

    auto plug_n_play_wait = ns(0);
    while (h->running()) {
        if (!h->get_devices().empty()) {
            h->get_mutex()->lock();
            for (const auto& dev : h->get_devices()) {
                const auto result = dev->update();
                if (result & update_result::AXIS && h->m_axis_handler)
                    h->m_axis_handler(dev);
                if (result & update_result::BUTTON && h->m_button_handler)
                    h->m_button_handler(dev);
            }
            sleep_time = h->m_thread_sleep;

            h->get_mutex()->unlock();
        }

        if (h->m_plug_and_play) {
            if (plug_n_play_wait >= h->m_plug_and_play_interval) {
                plug_n_play_wait = ns(0);
                gdebug("Updating device list");
                h->query_devices();
            }
            plug_n_play_wait += sleep_time;
        }
        this_thread::sleep_for(sleep_time);
    }
    ginfo("Hook thread ended");
}

void hook::on_bind(Json::object&, uint16_t, uint16_t, int16_t, bool)
{
    /* NO-OP */
}

hook::hook()
{
    m_running = false;
}

void hook::set_button_event_handler(std::function<void(std::shared_ptr<device>)> handler)
{
    m_button_handler = handler;
}

void hook::set_axis_event_handler(std::function<void(std::shared_ptr<device>)> handler)
{
    m_axis_handler = handler;
}

void hook::set_connect_event_handler(std::function<void(std::shared_ptr<device>)> handler)
{
    m_connect_handler = handler;
}

void hook::set_disconnect_event_handler(std::function<void(std::shared_ptr<device>)> handler)
{
    m_disconnect_handler = handler;
}

void hook::set_reconnect_event_handler(std::function<void(std::shared_ptr<device>)> handler)
{
    m_reconnect_handler = handler;
}

std::shared_ptr<cfg::binding> hook::make_native_binding(const std::string& json)
{
    if (json.empty()) {
        return make_native_binding(get_default_binding());
    } else {
        std::string err;
        auto j = Json::parse(json, err);
        if (err.empty())
            return make_native_binding(j);
        gerr("Failed to make gamepad binding from json: %s", err.c_str());
    }
    return nullptr;
}

uint64_t hook::ms_ticks()
{
    auto now = chrono::system_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
}

std::shared_ptr<cfg::binding> hook::get_binding_for_device(const std::string& id)
{
    // Update the binding map, in case a device changed bindings
    for (auto& dev : m_devices) {
        // check if the device has a custom binding
        if (!dev->has_binding())
            continue;
        auto bind_name = dev->get_binding()->get_name();
        if (get_binding_by_name(bind_name)) {
            auto bind = m_binding_map.find(dev->get_id());
            if (bind != m_binding_map.end()) {
                if (bind->second != bind_name)
                    bind->second = bind_name;
            }
        }
    }

    auto bind = m_binding_map.find(id);
    if (bind != m_binding_map.end()) {
        auto result = find_if(m_bindings.begin(), m_bindings.end(),
            [bind](shared_ptr<cfg::binding>& b) { return b->get_name() == bind->second; });

        if (result != m_bindings.end())
            return *result;
    }
    return nullptr;
}

std::shared_ptr<hook> hook::make(uint16_t flags)
{
#if LGP_WINDOWS
    if (flags & hook_type::XINPUT || flags & hook_type::NATIVE_DEFAULT)
        return std::make_shared<hook_xinput>();
    return std::make_shared<hook_dinput>();
#elif LGP_LINUX
    return std::make_shared<hook_linux>(flags);
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
            gwarn("Gamepad device '%s' is still in use! (Ref count %li)", m_devices[i]->get_id().c_str(),
                m_devices[i].use_count());
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
            gerr("Gamepad binding '%s' is still in use! (Ref count %li)", bind->get_name().c_str(), bind.use_count());
        }
    }

    m_bindings.clear();
    m_mutex.unlock();
}

bool hook::start()
{
    if (m_running)
        return true;

    query_devices();

    if (m_devices.empty())
        ginfo("No Devices detected. Waiting for connection...");
    m_running = true;
    m_hook_thread = thread(default_hook_thread, this);
    return true;
}

void hook::stop()
{
    if (m_running) {
        m_running = false;
        m_hook_thread.join();
    }
    close_devices();
    close_bindings();
    gdebug("Hook stopped");
}

bool hook::save_bindings(const std::string& path)
{
    Json j;
    if (save_bindings(j)) {
        std::ofstream out(path);
        if (out.good()) {
            out << std::setw(4) << j.dump() << std::endl;
            out.close();
            return true;
        }
        gerr("Can't write gamepad bindings to '%s'", path.c_str());
    }
    return false;
}

bool hook::save_bindings(Json& j)
{
    std::vector<Json> binding_array, binding_map;

    for (const auto& bind : m_bindings) {
        Json b;
        bind->save(b);
        binding_array.emplace_back(b);
    }

    for (const auto& device : m_devices) {
        if (device->has_binding()) {
            Json m = Json::object { { "device_id", device->get_id() }, { "binding_id", device->get_binding()->get_name() } };
            binding_map.emplace_back(m);
        }
    }

    j = Json::object { { "bindings", binding_array }, { "bindings_map", binding_map } };

    return true;
}

bool hook::load_bindings(const std::string& path)
{
    std::ifstream in(path);

    if (in.good()) {
        std::string content = std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        std::string err;
        auto j = Json::parse(content, err);

        if (load_bindings(j)) {
            return true;
        }
        gerr("Couldn't parse json when loading bindings from '%s': %s", path.c_str(), err.c_str());
    }
    gerr("Couldn't read bindings from '%s'", path.c_str());
    return false;
}

bool hook::load_bindings(const Json& j)
{
    Json bindings_map = j["bindings_map"], binding_array = j["bindings"];

    for (const auto& bind : binding_array.array_items())
        m_bindings.emplace_back(make_native_binding(bind));

    for (const auto& entry : bindings_map.array_items()) {
        auto device_id = entry["device_id"];
        auto bind_id = entry["binding_id"];
        m_binding_map[device_id.string_value()] = bind_id.string_value();
        if (!set_device_binding(device_id.string_value(), bind_id.string_value()))
            gwarn("Couldn't set binding.");
    }
    return true;
}

bool hook::set_device_binding(const std::string& device_id, const std::string& binding_id)
{
    auto dev = get_device_by_id(device_id);
    auto result = false;

    if (dev) {
        auto bind = get_binding_by_name(binding_id);
        if (bind) {
            dev->set_binding(move(bind));
            result = true;
        } else {
            gwarn("No binding with name '%s'", binding_id.c_str());
        }
    } else {
        gwarn("No device with id '%s'", device_id.c_str());
    }

    return result;
}

shared_ptr<device> hook::get_device_by_id(const std::string& id)
{
    auto result = find_if(m_device_cache.begin(), m_device_cache.end(),
        [&id](pair<const string, shared_ptr<device>>& d) { return d.second->get_id() == id; });

    if (result != m_device_cache.end())
        return result->second;

    /* Device isn't in cache, if it's in the normal device list, cache it and
     * return it */
    auto result2 = find_if(m_devices.begin(), m_devices.end(), [&id](shared_ptr<device>& d) { return d->get_id() == id; });

    if (result2 != m_devices.end()) {
        m_device_cache[(*result2)->get_cache_id()] = *result2;
        return *result2;
    }
    return nullptr;
}

std::shared_ptr<cfg::binding> hook::get_binding_by_name(const std::string& name)
{
    auto result = find_if(m_bindings.begin(), m_bindings.end(),
        [&name](shared_ptr<cfg::binding>& b) { return b->get_name() == name; });

    if (result == m_bindings.end())
        return nullptr;
    return *result;
}

void hook::make_xbox_config(const std::shared_ptr<gamepad::device>& dv, Json& out)
{
    if (!m_running)
        return;

    /* Up the deadzones to make sure we don't pick up the wrong input accidentally */
    dv->set_axis_deadzone(axis::RIGHT_TRIGGER, 500);
    dv->set_axis_deadzone(axis::LEFT_TRIGGER, 500);
    dv->set_axis_deadzone(axis::LEFT_STICK_X, 500);
    dv->set_axis_deadzone(axis::LEFT_STICK_Y, 500);
    dv->set_axis_deadzone(axis::RIGHT_STICK_X, 500);
    dv->set_axis_deadzone(axis::RIGHT_STICK_Y, 500);

    ginfo("Starting config creation wizard");
    auto sleep_time = get_sleep_time();
    uint64_t last_key_input = 0;
    bool running = true;
    mutex key_thread_mutex;
    std::vector<Json> bind_array;

    auto key_thread_method = [&]() {
        while (running) {
            getchar();
            key_thread_mutex.lock();
            last_key_input = ms_ticks();
            key_thread_mutex.unlock();
        }
    };

    thread key_thread(key_thread_method);

    auto binder = [&](const char* prompt, bool axis, const input_event* e, uint16_t* last,
                      const vector<tuple<string, uint16_t>>& prompts) {
        for (const auto& p : prompts) {
            ginfo("Please %s %s on your gamepad or press enter on your keyboard "
                  "to skip this bind.",
                prompt, get<0>(p).c_str());
            bool success = false;
            for (;;) {
                m_mutex.lock();
                if (e->native_id != *last && e->value != 0) {
                    *last = e->native_id;
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
                this_thread::sleep_for(sleep_time);
            }

            if (!success)
                continue;

            ginfo("Received input with id %i", *last);

            Json::object bind = Json::object { { "is_axis", axis }, { "from", *last }, { "to", get<1>(p) } };
            on_bind(bind, *last, get<1>(p), e->value, axis);

            bind_array.emplace_back(bind);
        }
    };

    uint16_t last_button = dv->last_button_event()->native_id, last_axis = dv->last_axis_event()->native_id;

    /* Button bindings */
    binder("press", false, dv->last_button_event(), &last_button, button_prompts);

    /* Axis bindings */
    binder("move", true, dv->last_axis_event(), &last_axis, axis_prompts);

    out = Json::array(bind_array);

    key_thread_mutex.lock();
    running = false;
    key_thread_mutex.unlock();
    ginfo("Done. Press Enter to print config json.");
    key_thread.join();
}

void hook::remove_invalid_devices()
{
    auto it = std::remove_if(m_devices.begin(), m_devices.end(), [this](std::shared_ptr<device>& d) {
        auto result = !d->is_valid();
        if (result && this->m_disconnect_handler)
            m_disconnect_handler(d);
        return result;
    });

    m_devices.erase(it, m_devices.end());

    /* Invalidate cached devices that aren't referenced anywhere except in the cache */
    for (auto it = m_device_cache.cbegin(); it != m_device_cache.cend();) {
        if ((*it).second.use_count() < 2) {
            it = m_device_cache.erase(it);
        } else {
            ++it;
        }
    }
}

}
