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

#pragma once
#include <gamepad/binding.hpp>
#include <gamepad/config.h>
#include <gamepad/device.hpp>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

using device_list = std::vector<std::shared_ptr<gamepad::device>>;
using bindings_list = std::vector<std::shared_ptr<gamepad::cfg::binding>>;

namespace gamepad {
enum class hook_type {
    NATIVE_DEFAULT,
    XINPUT,
    DIRECT_INPUT,
};

extern void default_hook_thread(class hook* h);

class hook {
protected:
    static std::vector<std::tuple<std::string, uint16_t>> button_prompts;
    static std::vector<std::tuple<std::string, uint16_t>> axis_prompts;
    /* List of all devices found after querying */
    device_list m_devices;
    /* List of all bindnigs */
    bindings_list m_bindings;

    std::thread m_hook_thread;
    std::mutex m_mutex;
    volatile bool m_running = false;
    uint16_t m_thread_sleep = 50;

    /* Can be used for platform specific bind options
	 * Only used for DirectInput currently, which needs a sepcial hack
	 * for separating the left and right trigger
	 */
    virtual void on_bind(json& j, uint16_t native_code, uint16_t vc, int16_t val, bool is_axis);

public:
    ~hook() { stop(); }

    /**
     * @brief get the hook thread mutex, use this to safely access
     * input data
     * @return The hook mutex
     */
    std::mutex* get_mutex() { return &m_mutex; }

    /**
     * @return Thread sleep time in miliseconds
     */
    uint16_t get_sleep_time() const { return m_thread_sleep; }

    /**
     * @return true if the hook thread is running
     */
    bool running() const { return m_running; }

    /**
     * @brief Save bindings to a file
     * @param path The target path
     * @return true on success
     */
    bool save_bindings(const std::string& path);

    /**
     * @brief Saves bindings and the mapping to json
     * @param j Output json
     * @return true on success
     */
    virtual bool save_bindings(json& j);

    /**
     * @brief load bindings from file
     * @param path
     * @return true on sucess
     */
    bool load_bindings(const std::string& path);

    virtual bool load_bindings(const json& j);

    void set_sleep_time(uint16_t ms)
    {
        m_mutex.lock();
        m_thread_sleep = ms;
        m_mutex.unlock();
    }

    virtual void close_devices();
    virtual void close_bindings();
    virtual void query_devices() = 0;
    virtual bool start();
    virtual void stop();
    virtual std::shared_ptr<cfg::binding> make_native_binding(const json& j) = 0;
    virtual void make_xbox_config(const std::shared_ptr<gamepad::device>& dv, json& out);

    std::shared_ptr<cfg::binding> get_binding_for_device(const std::string& id);
    std::shared_ptr<device> get_device_by_id(const std::string& id);
    std::shared_ptr<cfg::binding> get_binding_by_name(const std::string& name);
    const device_list& get_devices() const { return m_devices; }
    bool set_device_binding(const std::string& device_id, const std::string& binding_id);

    static std::shared_ptr<hook> make(hook_type type = hook_type::NATIVE_DEFAULT);
    static uint64_t ms_ticks();
};
}
