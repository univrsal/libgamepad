# libgamepad
![nightly build](https://github.com/univrsal/libgamepad/workflows/nightly%20build/badge.svg)

C++11 library for crossplatform gamepad hooking. Supports DirectInput, Xinput and the Linux joystick API

## Compiling
```
$ git clone https://github.com/univrsal/libgamepad
$ cd libgamepad
$ mkdir build && cd build
$ cmake ..
$ make
```
## Usage
The library can either be integrated into existing projects as a subfolder with CMake or with any other build tool using a static or shared build. Binaries are build for both linux and windows i386, x86_64 and arm and available under the [github actions](https://github.com/univrsal/libgamepad/actions)

To use the library simply include the ``libgamepad.hpp`` header which will include all other necessary headers.
A simple demo program is located under [tests/test.cpp](./tests/test.cpp), or could look something like this:
```c++
#include <cstdio>
#include <libgamepad.hpp>

int main()
{
    /* Create a hook instance, this is a shared pointer so you can
     * reference it wherever you want.
     * You can also specify whether to use DirectInput or Xinput,
     * by default it will use XInput.
     */
    auto hook = hook::make();
    
    /* Make the hook check for connected and disconnected devices
     * automatically ever second
     */
    h->set_plug_and_play(true, gamepad::ms(1000));
    
    std::atomic<bool> run_flag = true;
    /* Lambdas for event callbacks */
    auto button_handler = [run_flag](std::shared_ptr<gamepad::device> dev) {
        ginfo("Received button event: Native id: %i, Virtual id: %i val: %i",
            dev->last_button_event()->native_id, dev->last_button_event()->vc,
            dev->last_button_event()->value);

        if (dev->is_button_pressed(gamepad::button::Y)) {
            printf("Y is pressed, exiting\n");
            run_flag = false;
        }
    };

    auto axis_handler = [](std::shared_ptr<gamepad::device> dev) {
        ginfo("Received axis event: Native id: %i, Virtual id: %i val: %i", dev->last_axis_event()->native_id,
            dev->last_axis_event()->vc, dev->last_axis_event()->value);
    };
    
    h->set_axis_event_handler(axis_handler);
    h->set_button_event_handler(button_handler);
    
    /* Create and start the hook thread. Also checks for any connected devices
     * The hook thread will wait if there are none connected, but will not block
     * this thread. The hook will only look for devices if plug and play is on.
     */
    if (!hook->start()) {
        printf("Failed to start hook\n");
        return -1;
    }

    printf("Press Y to exit\n");
    while (run_flag)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return 0;
}
```
