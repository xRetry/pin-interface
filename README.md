# Pin Interface

At its core, this repository contains a pin interface, which provides a generic wrapper for handling pin/gpio interactions for a microcontroller.
The library contains a simple and flexible API to manage the state of microcontrollers and dynamically change the active pin configuration at runtime.

TODO(marco): Add image

# Usage

The entire interface is contained in the files `include/pin_interface.h` and `main/pin_interface.c`.
Those two files are standalone and can be freely included in other projects.

The rest of the code serves as an example how the interface can be used in combination with a HTTP-server and a MQTT-client.

## Hardware Bindings

Each pin operations (e.g. digital read, analog write) need to be defined with two callback functions.

The initalization function is called whenever a the specific pin configuration is requested.
It prepares the hardware to be able to read/write values later on.

```c
pi_err_t init_my_read_operation(pi_pin_nr_t pin_nr) {
    ...
}
```

The execution function performs the actual read/write operation for a specific pin.

```c
pi_err_t exec_my_read_operation(pi_pin_nr_t pin_nr, double *val) {
    ...
}
```

The specific implentation for both functions depends on the target hardware.

Then, the new pin operation can be registered in the interface.
To do so, the macros `PI_REGISTER_OPS` and `PI_ADD_OP` are provided.

```c
PI_REGISTER_OPS(
    PI_ADD_OP("My Read Operation", PI_READ, init_my_read_operations, exec_my_read_operation, PI_ALLOWED_PINS(0,1,7,8,9,10,18,19)),
    PI_ADD_OP("My Write Operation", PI_WRITE, init_my_write_operations, exec_my_write_operation, PI_ALLOWED_PINS(6, 17))
);
```

The `PI_ADD_OP` macro requires an operation name, the direction (read, write or disabled), both callback functions and the GPIO numbers for the new operation.
For examples of hardware bindings, see `main/pin_operations_linux.c` and `main/pin_operations_esp32c3.c`.

**NOTE**: The pin interface needs to be initialized before adding pin operations.
This can be done by calling `pi_init()`.

## Configuration and Operation Execution

After a custom pin operation has been registered, it can be used by the interface.

The function `pi_init_pin_op` is used to initialize a pin operation and `pi_exec_pin_op` performs the actual read/write operation.

## State Management

The internal state of the pin interface is stored in the publically available struct `pi_state`.
It is initialized by the function `pi_init` and can be used by the user to retrieve information about the configuration.
It is not recommended to mutate any of its fields directly.

To make the state persistent after restarts, it is stored in a file after a change and reloaded at initialization.

## Additional Exports

The pin interface header file exposes additional parameters to the user:
- `PI_PIN_OPS` is an array containing all registered pin operations
- `PI_NUM_OPS` contains the amount of elements in `PI_PIN_OPS`
- `PI_STRLEN_OP_NAME` defines the maximum amount of characters for a pin operation name, as defined with `PI_ADD_OP`

# Examples

Even though the pin interface is the core part of this this repository, it also showcases implementations for different hardware and communication methods.

## Compilation

Depending on the compiliation target, choose one of the following comilation methods:
- Linux: Zig
- ESP32c3: Espressif IDF

### Zig

Make sure you have zig installed on your system.
The file `build.zig` in the root of the repository allows choosing between HTTP server and MQTT client.
To do so, either use

```zig
exe.defineCMacro("CONFIG_PI_USE_HTTPSERVER", "1");
```
**or**

```zig
exe.defineCMacro("CONFIG_PI_USE_MQTT", "1");
```

Then, compile and run the program with the following terminal command:

```sh
zig build run
```

### Espressif IDF

The build process for the ESP32c3 microcontroller uses the Espressif IDF build tools.
To set them up correctly, follow the [official documentation](https://docs.espressif.com/projects/esp-idf/en/v5.1.2/esp32/get-started/index.html#installation).

Once installed, set the correct target:

```sh
idf.py set-target esp32c3
```

And select the desired configuration using the configuration utility:

```sh
idf.py menuconfig
```

All relevant options are under the menu entry `Pin Interface Config`.

Finally compile the program and flash it to the microcontroller:

```sh
idf.py build flash
```

When using the HTTP server, the IP address for connecting to the microcontroller is printed to the serial console.
To see the outputs, add the `monitor` option to the `idf.py` commmand.

```sh
idf.py build flash monitor
```

## HTTP Server

The HTTP server provides the following endpoints:
- `/config` serves a configuration page, allowing users to change the pin configuration
- `/api/config` takes a POST request to change the pin configuration
- `/api/operations` returns all available pin operations as JSON
- `/api/active` returns the currently activated pin operations as JSON
- `/ws/pins` opens a websocket, which is used to read and write value to specific pins

For the websocket, use the following message format:

```
{
    "write": {"<pin number>": <value>, "<pin number>": <value>, ...},
    "read": [<pin number>, <pin number>, ...]
}
```

Example:

```json
{
    "write": {"0": 10.4, "4": 1},
    "read": [3, 5, 10]
}
```

The `read` or `write` key can be omitted if not used.

## MQTT Client

The MQTT client currently only allows reading and writing values.
A way of changing the configuration has not been implemented.

The following options can be set for MQTT client:
- `CONFIG_PI_MQTT_SERVER_URL`: The URL of the MQTT server to connect to
- `CONFIG_PI_MQTT_WRITE_TOPIC`: The MQTT topic for write operations
- `CONFIG_PI_MQTT_READ_TOPIC`: The MQTT topic for read operations

Both, read and write messages use a binary format.
For sending, the `double pin_values[PI_NUM_PINS]` array is simply cast to a `char *`.
The program assumes, that incoming messages follow the same method.
