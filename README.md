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
     * You can also specify wether to use DirectInput or Xinput,
     * by default it will use XInput.
     */
    auto hook = hook::make();
    
    /* Create and start the hook thread. Also checks for any connected devices
     * The hook thread will wait if there are none connected, but will not block
     * this thread.
     */
    if (!hook->start()) {
        printf("Failed to start hook\n");
        return -1;
    }
    
    /* Get a vector populated with shared pointers of all currently 
     * connected devices */
    auto devices = hook->get_devices();
    auto dev = devices[0];
    
    /* Get the last event to compare it to the next event
     * These structs contain the native event id,
     * the virtual code, which is platform independent,
     * and the event value reported by the device
     * (0 or 1 for buttons, or the value of the axis)
     */
    h->get_mutex()->lock();
    auto last_axis = dev->last_axis_event()->time;
    auto last_button = dev->last_button_event()->time;
    h->get_mutex()->unlock();
    
    /* Output any events for the first device */
    bool run_flag = true;
    printf("Press Y to exit\n");
    
    while (run_flag) {
        /* The hook mutex should be locked whenever you need access
         * to the hook or device data */
        h->get_mutex()->lock();
        
        if (!dev->is_valid()) {
            printf("Lost first device, closing\n");
            h->get_mutex()->unlock();
            break;
        }
        
        if (dev->last_axis_event()->time != last_axis) {
            printf("Received axis event: Native id: %i, Virtual id: %i val: %i", dev->last_axis_event()->native_id,
                dev->last_axis_event()->vc, dev->last_axis_event()->value);
            last_axis = dev->last_axis_event()->time;
        }

        if (dev->last_button_event()->time != last_button) {
            printf("Received button event: Native id: %i, Virtual id: %i val: %i",
                dev->last_button_event()->native_id, dev->last_button_event()->vc,
                dev->last_button_event()->value);
            last_button = dev->last_button_event()->time;
        }
        
        /* The device state can also be queried directly: */
        if (dev->is_button_pressed(gamepad::button::Y)) {
            printf("Y is pressed, exiting\n");
            run_flag = false;
        }
        
        if (dev->get_axis(gamepad::axis::LEFT_TRIGGER) > 0.1f)
            printf("Left trigger value: %.2f\n", dev->get_axis(gamepad::axis::LEFT_TRIGGER));
        h->get_mutex()->unlock();
        
        /* Make sure to sleep long enough otherwise the hook thread
         * will not get a chance to refresh the devices */
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}
```
