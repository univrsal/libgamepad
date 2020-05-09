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
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <gamepad/device.hpp>
#include <gamepad/binding.hpp>
#include <gamepad/config.h>

using device_list = std::vector<std::shared_ptr<gamepad::device>>;
using bindings = std::map<std::string, std::shared_ptr<gamepad::cfg::binding>>;

namespace gamepad {
    enum class hook_type {
        XINPUT,
        DIRECT_INPUT,
    };

    class hook {
    protected:
        device_list m_devices;
        bindings m_bindings;
        std::thread m_hook_thread;
        std::mutex m_mutex;
        volatile bool m_running = false;
        uint16_t m_thread_sleep = 50;
    public:

        std::mutex *get_mutex() { return &m_mutex; }
        uint16_t get_sleep_time() const { return m_thread_sleep; }
        bool running() const { return m_running; }
        void set_sleep_time(uint16_t ms)
        {
            m_mutex.lock();
            m_thread_sleep = ms;
            m_mutex.unlock();
        }

        virtual void close_devices() = 0;
        virtual void query_devices() = 0;
        virtual bool start() = 0;
        virtual void stop();

        virtual void make_xbox_config(const std::shared_ptr<gamepad::device>
                                       &dv, json &out) = 0;

        const device_list &get_devices() const { return m_devices; }

        std::shared_ptr<cfg::binding> get_binding_for_device(const std::string &id);

        static std::shared_ptr<hook> make(hook_type type = hook_type::XINPUT);

    };

}
