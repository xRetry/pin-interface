# Pin Interface

At its core, this repository contains a pin interface, which provides a generic wrapper for handling pin/gpio interactions for a microcontroller.
The library contains a simple and flexible API to manage the state of microcontrollers and dynamically change the active pin configuration at runtime.

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

## HTTP Server

TODO(marco): Describe setup steps

The HTTP server provides the following endpoints:
- `/config` serves a configuration page, allowing users to change the pin configuration
- `/api/config` takes a POST request to changes the pin configuration
- `/api/operations` returns all available pin operations as JSON
- `/api/active` returns the currently activated pin operations
- `/ws/pins` opens a websocket, which is used to read and write value to specific pins

TODO(marco): Show json formats

## MQTT Client

TODO(marco): Continue
