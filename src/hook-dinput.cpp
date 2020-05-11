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
#include <gamepad/hook-dinput.hpp>
#include <gamepad/log.hpp>
#include <gamepad/binding-dinput.hpp>

using namespace std;

namespace gamepad {

    static void thread_method(hook_dinput* h)
    {
        ginfo("Started DInput hook");
    }
	
    static BOOL CALLBACK enum_callback(LPCDIDEVICEINSTANCE dev, LPVOID data)
	{
        auto h = static_cast<hook_dinput*>(data);

        auto new_device = make_shared<device_dinput>(dev, h->m_dinput, h->m_hook_window);

		if (new_device->is_valid()) {
            h->m_devices.emplace_back(dynamic_pointer_cast<device>(new_device));
            auto b = h->get_binding_for_device(new_device->get_id());
            
            if (b) {
                new_device->set_binding(move(b));
            }
            else {
              /*  auto b = make_shared<cfg::binding_dinput>(cfg::linux_default_binding);
                m_bindings[dev->get_id()] = b;
                dev->set_binding(dynamic_pointer_cast<cfg::binding>(b));*/
            }
		}
		
        return DIENUM_CONTINUE;
	}

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
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

    static void window_message_thread(hook_dinput *h)
    {
        MSG msg;
        while (h->running() && GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
	
    void hook_dinput::query_devices()
    {
        m_mutex.lock();
        m_devices.clear();
        auto result = m_dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, enum_callback,
            this, DIEDFL_ATTACHEDONLY);

		if (FAILED(result))
		{
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
        
        if (RegisterClass(&wc))
        {
            m_hook_window = CreateWindowEx(WS_EX_CLIENTEDGE, L"libgamepad_window_class", 
                L"libgampead_window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                240, 120, nullptr, nullptr, wc.hInstance, nullptr);
        }
        
        if (m_hook_window == nullptr)
        {
            gerr("Couldn't create hook window");
            return false;
        }

        query_devices();

		if (m_devices.size() > 0)
		{
            m_hook_thread = thread(thread_method, this);
            result = true;
		}
        m_running = result;
        
        return result;
    }
	
    bool hook_dinput::load_bindings(const json& j)
    {
        return false;
    }
	
    void hook_dinput::make_xbox_config(const std::shared_ptr<gamepad::device>& dv,
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

        ///* Axis bindings */
        //binder("move", true, dv->last_axis_event(), &last_axis, axis_prompts);
    }
}
