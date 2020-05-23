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

#include "device-dinput.hpp"
#include <gamepad/binding-dinput.hpp>
#include <gamepad/hook-dinput.hpp>
#include <gamepad/log.hpp>

using namespace std;

namespace gamepad {

static BOOL CALLBACK enum_callback(LPCDIDEVICEINSTANCE dev, LPVOID data)
{
    auto h = static_cast<hook_dinput*>(data);

    auto new_device = make_shared<device_dinput>(dev, h->m_dinput, h->m_hook_window);

    if (new_device->is_valid()) {
        h->m_devices.emplace_back(dynamic_pointer_cast<device>(new_device));
        auto b = h->get_binding_for_device(new_device->get_id());

        if (b) {
            new_device->set_binding(move(b));
        } else {
            auto b = make_shared<cfg::binding_dinput>(cfg::dinput_default_binding);
            new_device->set_binding(dynamic_pointer_cast<cfg::binding>(b));
        }
    }

    return DIENUM_CONTINUE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void hook_dinput::query_devices()
{
    m_mutex.lock();
    m_devices.clear();
    auto result = m_dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, enum_callback,
        this, DIEDFL_ATTACHEDONLY);

    if (FAILED(result)) {
        gerr("Enumeration of Direct Input devices failed");
    }

    m_mutex.unlock();
}

bool hook_dinput::start()
{
    auto result = DirectInput8Create(GetModuleHandle(nullptr),
        DIRECTINPUT_VERSION, IID_IDirectInput8W, reinterpret_cast<void**>(&m_dinput),
        nullptr);
    if (FAILED(result)) {
        gerr("Couldn't create DirectInput interface");
        return false;
    }

    /* Create window */
    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"libgamepad_window_class";

    if (RegisterClass(&wc)) {
        m_hook_window = CreateWindowEx(WS_EX_CLIENTEDGE, L"libgamepad_window_class",
            L"libgampead_window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            240, 120, nullptr, nullptr, wc.hInstance, nullptr);
    }

    if (m_hook_window == nullptr) {
        gerr("Couldn't create hook window");
        return false;
    }

    return hook::start();
}

shared_ptr<cfg::binding> hook_dinput::make_native_binding(const json& j)
{
    return make_shared<cfg::binding_dinput>(j);
}

void hook_dinput::make_xbox_config(const shared_ptr<gamepad::device>& dv,
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
            /* Special check since Direct Input puts left and right trigger on the same
                 * axis (one in positive and one in negative direction
                 */
            if (get<1>(p) == axis::LEFT_TRIGGER || get<1>(p) == axis::RIGHT_TRIGGER) {
                bind["trigger_polarity"] = e->value > 0 ? 1 : -1;
            }
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
