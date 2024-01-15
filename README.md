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
    PI_ADD_OP("My Read Operation", PI_READ, init_my_read_operations, exec_my_read_operation, PI_ALLOWED_PINS(0,1,7,8,9,10,18,19))
);
```

The `PI_ADD_OP` macro requires an operation name, the direction (read, write or disabled), both callback functions and the GPIO numbers for the new operation.
For examples of hardware bindings, see `main/pin_operations_linux.c` and `main/pin_operations_esp32c3.c`.

## Configuration and Operation Execution

After a custom pin operation has been registered, it can be used by the interface.

The function `pi_init_pin_op` is used to initialize a pin operation and `pi_exec_pin_op` performs the previously initialized operation.

TODO(marco): Continue


