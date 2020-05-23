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
#include <chrono>
#include <gamepad/binding-dinput.hpp>
#include <gamepad/hook-dinput.hpp>
#include <gamepad/log.hpp>

/* Nabbed from ppsspp dinput code */
#define DIFF (JOY_POVRIGHT - JOY_POVFORWARD) / 2
#define JOY_POVFORWARD_RIGHT JOY_POVFORWARD + DIFF
#define JOY_POVRIGHT_BACKWARD JOY_POVRIGHT + DIFF
#define JOY_POVBACKWARD_LEFT JOY_POVBACKWARD + DIFF
#define JOY_POVLEFT_FORWARD JOY_POVLEFT + DIFF

using namespace std;

#define DINPUT_AXIS_MAX 1000
#define DINPUT_AXIS_MIN -1000
#ifndef DIDFT_OPTIONAL
#define DIDFT_OPTIONAL 0x80000000
#endif

namespace gamepad {
/* Taken from Wine - Thanks! */
static DIOBJECTDATAFORMAT dfDIJoystick2[] = {
    { &GUID_XAxis, DIJOFS_X, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_YAxis, DIJOFS_Y, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_ZAxis, DIJOFS_Z, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RxAxis, DIJOFS_RX, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RyAxis, DIJOFS_RY, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RzAxis, DIJOFS_RZ, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Slider, DIJOFS_SLIDER(0), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Slider, DIJOFS_SLIDER(1), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_POV, DIJOFS_POV(0), DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
    { &GUID_POV, DIJOFS_POV(1), DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
    { &GUID_POV, DIJOFS_POV(2), DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
    { &GUID_POV, DIJOFS_POV(3), DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(0), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(1), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(2), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(3), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(4), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(5), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(6), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(7), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(8), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(9), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(10), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(11), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(12), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(13), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(14), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(15), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(16), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(17), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(18), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(19), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(20), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(21), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(22), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(23), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(24), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(25), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(26), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(27), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(28), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(29), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(30), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(31), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(32), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(33), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(34), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(35), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(36), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(37), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(38), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(39), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(40), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(41), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(42), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(43), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(44), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(45), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(46), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(47), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(48), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(49), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(50), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(51), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(52), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(53), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(54), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(55), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(56), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(57), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(58), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(59), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(60), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(61), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(62), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(63), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(64), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(65), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(66), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(67), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(68), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(69), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(70), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(71), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(72), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(73), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(74), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(75), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(76), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(77), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(78), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(79), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(80), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(81), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(82), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(83), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(84), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(85), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(86), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(87), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(88), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(89), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(90), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(91), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(92), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(93), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(94), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(95), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(96), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(97), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(98), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(99), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(100), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(101), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(102), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(103), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(104), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(105), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(106), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(107), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(108), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(109), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(110), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(111), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(112), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(113), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(114), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(115), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(116), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(117), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(118), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(119), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(120), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(121), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(122), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(123), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(124), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(125), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(126), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { NULL, DIJOFS_BUTTON(127), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
    { &GUID_XAxis, FIELD_OFFSET(DIJOYSTATE2, lVX), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_YAxis, FIELD_OFFSET(DIJOYSTATE2, lVY), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_ZAxis, FIELD_OFFSET(DIJOYSTATE2, lVZ), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lVRx), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lVRy), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lVRz), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglVSlider[0]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglVSlider[1]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_XAxis, FIELD_OFFSET(DIJOYSTATE2, lAX), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_YAxis, FIELD_OFFSET(DIJOYSTATE2, lAY), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_ZAxis, FIELD_OFFSET(DIJOYSTATE2, lAZ), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lARx), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lARy), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lARz), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglASlider[0]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglASlider[1]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_XAxis, FIELD_OFFSET(DIJOYSTATE2, lFX), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_YAxis, FIELD_OFFSET(DIJOYSTATE2, lFY), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_ZAxis, FIELD_OFFSET(DIJOYSTATE2, lFZ), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lFRx), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lFRy), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lFRz), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglFSlider[0]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
    { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglFSlider[1]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
};

const DIDATAFORMAT dinput_dataformat = {
    sizeof(DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDF_ABSAXIS,
    sizeof(DIJOYSTATE2),
    164,
    dfDIJoystick2
};

BOOL CALLBACK enum_device_objects_callback(
    LPCDIDEVICEOBJECTINSTANCE obj,
    LPVOID data)
{
    auto d = static_cast<device_dinput*>(data);

    DIPROPRANGE axis_range;
    axis_range.lMax = DINPUT_AXIS_MAX;
    axis_range.lMin = DINPUT_AXIS_MIN;
    axis_range.diph.dwSize = sizeof(DIPROPRANGE);
    axis_range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    axis_range.diph.dwHow = DIPH_DEVICE;
    axis_range.diph.dwObj = 0;

    if (FAILED(d->m_device->SetProperty(DIPROP_RANGE, &axis_range.diph))) {
        gerr("Couldn't set axis ranges for '%s'", util::wchar_to_utf8(obj->tszName).c_str());
        gerr("Probably an analog device");
    } else {
        gdebug("Registered '%s' at 0x%X", util::wchar_to_utf8(obj->tszName).c_str(), obj->dwOfs);
        d->m_analog = true;
    }

    /* Set Deadzone */
    DIPROPDWORD dipw;
    dipw.diph.dwSize = sizeof(DIPROPDWORD);
    dipw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipw.diph.dwHow = DIPH_DEVICE;
    dipw.diph.dwObj = 0;
    dipw.dwData = 0;

    if (d->m_analog && FAILED(d->m_device->SetProperty(DIPROP_DEADZONE, &dipw.diph))) {
        gdebug("Failed to set deadzone for '%s' at 0x%X", util::wchar_to_utf8(obj->tszName).c_str(), obj->dwOfs);
    } else {
        d->m_analog = true;
        device_dinput::dinput_axis a;
        a.id = d->m_axis.size();

        if (!memcmp(&obj->guidType, &GUID_XAxis, sizeof(obj->guidType)))
            a.offset = DIJOFS_X;
        else if (!memcmp(&obj->guidType, &GUID_YAxis, sizeof(obj->guidType)))
            a.offset = DIJOFS_Y;
        else if (!memcmp(&obj->guidType, &GUID_ZAxis, sizeof(obj->guidType)))
            a.offset = DIJOFS_Z;
        else if (!memcmp(&obj->guidType, &GUID_RxAxis, sizeof(obj->guidType)))
            a.offset = DIJOFS_RX;
        else if (!memcmp(&obj->guidType, &GUID_RyAxis, sizeof(obj->guidType)))
            a.offset = DIJOFS_RY;
        else if (!memcmp(&obj->guidType, &GUID_RzAxis, sizeof(obj->guidType)))
            a.offset = DIJOFS_RZ;
        else if (!memcmp(&obj->guidType, &GUID_Slider, sizeof(obj->guidType))) {
            a.offset = DIJOFS_SLIDER(d->m_slider_count++);
        } else {
            gdebug("Unkown axis");
        }
        d->m_axis_inputs.emplace_back(a);
    }

    return DIENUM_CONTINUE;
}

void device_dinput::button_event(uint16_t id, uint16_t value)
{
    m_last_button_event.id = id;
    m_last_button_event.value = value;
    m_last_button_event.time = hook::ms_ticks();
}

void device_dinput::axis_event(uint16_t id, uint16_t value)
{

    m_last_axis_event.id = id;
    m_last_axis_event.value = value;
    m_last_axis_event.time = hook::ms_ticks();
}

device_dinput::device_dinput(LPCDIDEVICEINSTANCE dev, IDirectInput8* dinput, HWND hook_window)
    : m_device_instance(dev)
    , m_dinput(dinput)
    , m_hook_window(hook_window)
{
    device_dinput::init();
}

device_dinput::~device_dinput()
{
    device_dinput::deinit();
}

const string& device_dinput::get_id() const
{
    return m_id;
}

void device_dinput::init()
{
    deinit();
    m_axis_new = { &m_new_state.lX,
        &m_new_state.lY,
        &m_new_state.lZ,
        &m_new_state.lRx,
        &m_new_state.lRy,
        &m_new_state.lRz,
        &m_new_state.lVX,
        &m_new_state.lVY,
        &m_new_state.lVZ,
        &m_new_state.lVRx,
        &m_new_state.lVRy,
        &m_new_state.lVRz,
        &m_new_state.lAX,
        &m_new_state.lAY,
        &m_new_state.lAZ,
        &m_new_state.lARx,
        &m_new_state.lARy,
        &m_new_state.lARz,
        &m_new_state.lFX,
        &m_new_state.lFY,
        &m_new_state.lFZ,
        &m_new_state.lFRx,
        &m_new_state.lFRy,
        &m_new_state.lFRz,
        &m_new_state.rglSlider[0],
        &m_new_state.rglSlider[1],
        &m_new_state.rglVSlider[0],
        &m_new_state.rglVSlider[1],
        &m_new_state.rglASlider[0],
        &m_new_state.rglASlider[1],
        &m_new_state.rglFSlider[0],
        &m_new_state.rglFSlider[1] };
    m_axis_old = { &m_old_state.lX,
        &m_old_state.lY,
        &m_old_state.lZ,
        &m_old_state.lRx,
        &m_old_state.lRy,
        &m_old_state.lRz,
        &m_old_state.lVX,
        &m_old_state.lVY,
        &m_old_state.lVZ,
        &m_old_state.lVRx,
        &m_old_state.lVRy,
        &m_old_state.lVRz,
        &m_old_state.lAX,
        &m_old_state.lAY,
        &m_old_state.lAZ,
        &m_old_state.lARx,
        &m_old_state.lARy,
        &m_old_state.lARz,
        &m_old_state.lFX,
        &m_old_state.lFY,
        &m_old_state.lFZ,
        &m_old_state.lFRx,
        &m_old_state.lFRy,
        &m_old_state.lFRz,
        &m_old_state.rglSlider[0],
        &m_old_state.rglSlider[1],
        &m_old_state.rglVSlider[0],
        &m_old_state.rglVSlider[1],
        &m_old_state.rglASlider[0],
        &m_old_state.rglASlider[1],
        &m_old_state.rglFSlider[0],
        &m_old_state.rglFSlider[1] };

    m_product_name = util::wchar_to_utf8(m_device_instance->tszProductName);
    m_instance_name = util::wchar_to_utf8(m_device_instance->tszInstanceName);

    if (m_product_name != m_instance_name) /* Sometimes reported as the same */
        m_name = "(" + m_product_name + ") " + m_instance_name;
    else
        m_name = m_instance_name;

    m_device_id = m_device_instance->guidInstance;
    m_valid = m_dinput->CreateDevice(m_device_id, &m_device, nullptr) == DI_OK;

    wchar_t buf[512];
    auto result = StringFromGUID2(m_device_id, buf, 512);
    if (result > 0)
        m_id = util::wchar_to_utf8(buf) + " " + m_product_name;

    if (m_valid) {
        gdebug("Initalized gamepad '%s'", m_product_name.c_str());
        gdebug("Axis count: %i", m_axis.size());
        if (FAILED(m_device->SetCooperativeLevel(m_hook_window,
                DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
            gerr("Failed to set device cooperative level");
            m_valid = false;
        } else {
            result = m_device->EnumObjects(enum_device_objects_callback, this, DIDFT_AXIS);
            if (FAILED(result)) {
                gerr("Device object enumeration failed:");
                if (result == DIERR_NOTINITIALIZED)
                    gerr("libgamepad lost access to device");
                else if (result == DIERR_INVALIDPARAM)
                    gerr("Invalid parameter");
                else
                    gerr("Unknown error");
                m_valid = false;
            } else {
                m_capabilities.dwSize = sizeof(DIDEVCAPS);
                if (FAILED(m_device->GetCapabilities(&m_capabilities))) {
                    gerr("Couldn't get device capabilities");
                    m_valid = false;
                }
                /* The second joystick format seems to be better*/
                else if (m_valid && FAILED(m_device->SetDataFormat(&dinput_dataformat))) {
                    gerr("Couldn't set data format for device '%s'", m_name.c_str());
                }
            }
        }
    }

    result = m_device->Acquire();
    if (FAILED(result)) {
        if (result == DIERR_NOTINITIALIZED)
            gerr("libgamepad lost access to device");
        else if (result == DIERR_INVALIDPARAM)
            gerr("Invalid parameter");
        else
            gerr("Unknown error");
        m_valid = false;
    }
}

void device_dinput::deinit()
{
    if (m_device) {
        if (m_valid)
            m_device->Unacquire();
        m_device->Release();
    }
    m_axis_inputs.clear();
    m_valid = false;
    m_device = nullptr;
}

void device_dinput::update()
{
    if (!m_valid)
        return;

    m_old_state = m_new_state;
    ZeroMemory(&m_new_state, sizeof(DIJOYSTATE));

    auto result = m_device->Poll();
    if (FAILED(result)) {
        gerr("Polling for device '%s' failed, trying to reacquire...", m_name.c_str());
        result = m_device->Acquire();
        while (result == DIERR_INPUTLOST)
            result = m_device->Acquire();

        if (FAILED(result))
            m_valid = false;

        if (result == DIERR_INVALIDPARAM) {
            gerr("Invalid parameter error for %s", m_name.c_str());
        } else if (result == DIERR_NOTINITIALIZED) {
            gerr("Device '%s' is exclusively used by another process", m_name.c_str());
        } else if (result == DIERR_OTHERAPPHASPRIO) {
            gdebug("Device '%s' is occupied by another process, waiting...", m_name.c_str());
            return;
        } else {
            gerr("Unknown error");
        }
    }

    if (FAILED(m_device->GetDeviceState(sizeof(DIJOYSTATE2), &m_new_state))) {
        gerr("Couldn't get device state for '%s'", m_name.c_str());
        m_valid = false;
        return;
    }

    /* Check all buttons */
    uint16_t last_button = 0x0;
    for (auto i = 0ul; i < 128; i++) {
        if (m_new_state.rgbButtons[i] == m_old_state.rgbButtons[i])
            continue;

        bool pressed = (m_new_state.rgbButtons[i] & 0x80) == 0x80;
        bool old_pressed = (m_old_state.rgbButtons[i] & 0x80) == 0x80;

        if (m_native_binding) {
            auto new_code = m_native_binding->m_buttons_mappings[i];
            m_buttons[new_code] = pressed;
        }

        /* This button changed over to pressed
             * Since button presses aren't sent in individually this means
             * that if multiple buttons have been pressed since the last update
             * only the one with the highest ID will be reported, but since
             * this is only used for creating binds it's not an issue.
             */
        if (pressed != old_pressed) {
            button_event(i, pressed);
        }
    }

    auto check_pov = [&](DWORD value, bool& up, bool& down, bool& left, bool& right) {
        if (LOWORD(value) == 0xFFFF)
            return;
        /* SDL2 Magic maps to eight directions */
        value += 4500 / 2;
        value %= 36000;
        value /= 4500;
        switch (value) {
        case 0:
            up = true;
            break;
        case 1:
            up = true;
            right = true;
            break;
        case 2:
            right = true;
            break;
        case 3:
            down = true;
            right = true;
            break;
        case 4:
            down = true;
            break;
        case 5:
            down = true;
            left = true;
            break;
        case 6:
            left = true;
            break;
        case 7:
            up = true;
            left = true;
        default:;
        }
    };

    /* Check POV aka DPad */
    bool left, old_left, right, old_right, up, old_up, down, old_down;
    check_pov(m_new_state.rgdwPOV[0], up, down, left, right);
    check_pov(m_old_state.rgdwPOV[0], old_up, old_down, old_left, old_right);

    if (up != old_up)
        button_event(DPAD_UP, up);
    if (left != old_left)
        button_event(DPAD_LEFT, left);
    if (down != old_down)
        button_event(DPAD_DOWN, down);
    if (right != old_right)
        button_event(DPAD_RIGHT, right);

    if (m_native_binding) {
        auto up_code = m_native_binding->m_buttons_mappings[DPAD_UP];
        auto left_code = m_native_binding->m_buttons_mappings[DPAD_LEFT];
        auto down_code = m_native_binding->m_buttons_mappings[DPAD_DOWN];
        auto right_code = m_native_binding->m_buttons_mappings[DPAD_RIGHT];

        m_buttons[up_code] = up;
        m_buttons[down_code] = down;
        m_buttons[left_code] = left;
        m_buttons[right_code] = right;
    }

    /* Check all axis */
    if (m_analog) {
        for (const auto& axis : m_axis_inputs) {
            int16_t val = 0, last_val = 0;
            switch (axis.offset) {
            case DIJOFS_X:
                val = m_new_state.lX;
                last_val = m_old_state.lX;
                break;
            case DIJOFS_Y:
                val = m_new_state.lY;
                last_val = m_old_state.lY;
                break;
            case DIJOFS_Z:
                val = m_new_state.lZ;
                last_val = m_old_state.lZ;
                break;
            case DIJOFS_RX:
                val = m_new_state.lRx;
                last_val = m_old_state.lRx;
                break;
            case DIJOFS_RY:
                val = m_new_state.lRy;
                last_val = m_old_state.lRy;
                break;
            case DIJOFS_RZ:
                val = m_new_state.lRz;
                last_val = m_old_state.lRz;
                break;
            case DIJOFS_SLIDER(0):
                val = m_new_state.rglSlider[0];
                last_val = m_old_state.rglSlider[0];
                break;
            case DIJOFS_SLIDER(1):
                val = m_new_state.rglSlider[1];
                last_val = m_old_state.rglSlider[1];
                break;
            default:;
            }

            if (m_native_binding) {

                auto vc = m_native_binding->m_axis_mappings[axis.offset];
                if (vc == axis::LEFT_TRIGGER || vc == axis::RIGHT_TRIGGER) {
                    if (val > 0) {
                        if (m_native_binding->m_right_trigger_polarity > 0) {
                            m_axis[axis::RIGHT_TRIGGER] = (val - (DINPUT_AXIS_MAX / 2)) / (DINPUT_AXIS_MAX / 2);
                        } else {
                            m_axis[axis::LEFT_TRIGGER] = (val - (DINPUT_AXIS_MAX / 2)) / (DINPUT_AXIS_MAX / 2);
                        }
                    } else {
                        if (m_native_binding->m_left_trigger_polarity < 0) {
                            m_axis[axis::LEFT_TRIGGER] = (val + (DINPUT_AXIS_MAX / 2)) / (DINPUT_AXIS_MAX / 2);
                        } else {
                            m_axis[axis::RIGHT_TRIGGER] = (val + (DINPUT_AXIS_MAX / 2)) / (DINPUT_AXIS_MAX / 2);
                        }
                    }
                } else {
                    m_axis[vc] = float(val) / DINPUT_AXIS_MAX;
                }
            }

            if (abs(last_val - val) > 50) {
                axis_event(axis.offset, val);
            }
        }
    }

    for (auto i = 0ul; i < m_axis_new.size(); i++) {
        /* Range is -1000 to 1000 so 10 should be a good threshold */
        bool moved = *(m_axis_new[i]) > 50 || *(m_axis_new[i]) < -50;

        if (m_native_binding) {
            auto new_code = m_native_binding->m_axis_mappings[i];
            m_axis[new_code] = *(m_axis_new[i]) / DINPUT_AXIS_MAX;
        }

        /* If the position changed */
        if (moved && abs((*(m_axis_old)[i]) - (*m_axis_new[i])) > 50) {
            axis_event(i, *m_axis_new[i]);
        }
    }
}

void device_dinput::set_binding(shared_ptr<cfg::binding>&& b)
{
    device::set_binding(move(b));
    m_native_binding = dynamic_cast<cfg::binding_dinput*>(b.get());
}
}