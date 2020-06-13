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

#include <array>
#include <gamepad/binding.hpp>
#include <map>
#include <memory>
#include <string>

namespace gamepad {

namespace button {
    enum {
        A = 0xEC00,
        B,
        X,
        Y,
        LB,
        RB,
        BACK,
        START,
        GUIDE,
        L_THUMB,
        R_THUMB,
        DPAD_LEFT,
        DPAD_RIGHT,
        DPAD_UP,
        DPAD_DOWN,
        COUNT
    };
}

namespace axis {
    enum {
        LEFT_STICK_X = button::COUNT,
        LEFT_STICK_Y,
        LEFT_TRIGGER,
        RIGHT_STICK_X,
        RIGHT_STICK_Y,
        RIGHT_TRIGGER,
        COUNT
    };
}

struct input_event {
    uint16_t native_id;
    uint16_t vc;
    int32_t value;
    uint64_t time;
};

class device {
protected:
    /**
     * @brief Maps a button code to its state
     * On Windows this will only contain X-Box gamepad::buttons
     * when using XInput. With DirectInput or on Linux it will
     * contain all button inputs.
     */
    std::map<uint16_t, bool> m_buttons;

    /**
     * @brief Maps an axis to it's state.
     * The state will range from -1 to 1.
     * For most gamepads the axis mapping in gamepad::axis is used
     */
    std::map<uint16_t, float> m_axis;

    /* Misc */

    /* These contain the last native input event received for this device*/
    input_event m_last_button_event = { 0xffff, 0, 0 };
    input_event m_last_axis_event = { 0xffff, 0, 0 };

    /* Device name, will be used for identifying if no other
     * IDs where found. Bindings will be mapped using this or
     * the ID returned from gamepad::device::get_id()
     */
    std::string m_name;

    /* Instance of the bindings used for this device
         * This uses bindings matched for the hook that is
         * used for this device
         */
    std::shared_ptr<cfg::binding> m_binding;
    bool m_valid = false;

    void button_event(uint16_t native_id, uint16_t vc, int32_t value);
    void axis_event(uint16_t native_id, uint16_t vc, int32_t value);

public:
    device() = default;

    ~device()
    {
    }

    const std::string& get_name() const
    {
        return m_name;
    }

    /* Can be overriden to make
         * saving of bindings more accurate since
         * they then will be mapped to the device id
         * instead of the device name which can occur
         * multiple times if the same controller type is
         * connected multiple times */
    virtual const std::string& get_id() const { return m_name; }

    bool is_button_pressed(uint16_t code)
    {
        return m_buttons[code];
    }

    float get_axis(uint16_t axis)
    {
        return m_axis[axis];
    }

    bool is_valid() const
    {
        return m_valid;
    }

    void invalidate()
    {
        m_valid = false;
    }

    bool has_binding() const
    {
        return m_binding != nullptr;
    }

    const input_event* last_button_event() const
    {
        return &m_last_button_event;
    }

    const input_event* last_axis_event() const
    {
        return &m_last_axis_event;
    }

    std::shared_ptr<cfg::binding> get_binding()
    {
        return m_binding;
    }

    virtual void set_binding(std::shared_ptr<cfg::binding>&& b)
    {
        m_binding.reset();
        m_binding = b;
    }

    virtual void deinit()
    { /* NO-OP */
    }
    virtual void init()
    { /* NO-OP */
    }
    virtual void update()
    { /* NO-OP */
    }
};
}
