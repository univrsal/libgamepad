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

#include <gamepad/log.hpp>
#include <gamepad/hook.hpp>
#include <gamepad/hook-dinput.hpp>
#include <gamepad/hook-xinput.hpp>
#include <gamepad/hook-linux.hpp>

namespace gamepad {

    std::shared_ptr<cfg::binding> hook::get_binding_for_device(const std::string &id)
    {
        return m_bindings[id];
    }

    std::shared_ptr<hook> hook::make(hook_type type)
    {
#if LGP_WINDOWS
        if (type == hook_type::XINPUT) {
            return std::make_shared<hook_xinput>();
        } else {
            return std::make_shared<hook_dinput>();
        }
#elif LGP_LINUX
        LGP_UNUSED(type);
        return std::make_shared<hook_linux>();
#endif
    }

    void hook::stop()
    {
        close_devices();
        if (m_running) {
            m_running = false;
            m_hook_thread.join();
        }
        gdebug("Hook stopped");
    }
}
