#ifndef PIN_INTERFACE_H_
#define PIN_INTERFACE_H_

// All available architectures

//#define __linux__ 1

// Detect the target architecture
#if !defined(PI_ARCH)
    #if defined(__x86_64__) || defined(__M_X64__) || defined(__linux__)
        #define PI_ARCH_LINUX 1
    #elif defined(ESP_PLATFORM)
        #define PI_ARCH_ESP32 1
    #endif
#endif

// TODO(marco): 
// Change number of pins depending on chip
// The best solution would be to automatically determine the number of pins
// from the input of the `PI_ALLOWED_PINS` macro.
// This is not straight forward, because `PI_NUM_PINS` would not be compile time constant.
#define PI_NUM_PINS 21

// A series of helper macros to retrieve the number of arguments passed to a macro.
#define PI_NUM_ARGS(...) PI_NUM_ARGS_(__VA_ARGS__,PI_PP_RSEQ_N())
#define PI_NUM_ARGS_(...) PI_PP_128TH_ARG(__VA_ARGS__)
#define PI_PP_128TH_ARG( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,_64,_65,_66,_67,_68,_69,_70, \
         _71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
         _81,_82,_83,_84,_85,_86,_87,_88,_89,_90, \
         _91,_92,_93,_94,_95,_96,_97,_98,_99,_100, \
         _101,_102,_103,_104,_105,_106,_107,_108,_109,_110, \
         _111,_112,_113,_114,_115,_116,_117,_118,_119,_120, \
         _121,_122,_123,_124,_125,_126,_127,N,...) N
#define PI_PP_RSEQ_N() \
         127,126,125,124,123,122,121,120, \
         119,118,117,116,115,114,113,112,111,110, \
         109,108,107,106,105,104,103,102,101,100, \
         99,98,97,96,95,94,93,92,91,90, \
         89,88,87,86,85,84,83,82,81,80, \
         79,78,77,76,75,74,73,72,71,70, \
         69,68,67,66,65,64,63,62,61,60, \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

// Helper macro to create an array and count its length at compile time
#define PI_ALLOWED_PINS(...) {__VA_ARGS__}, .pins_allowed_size=PI_NUM_ARGS(__VA_ARGS__)

/// This macro initializes a `pin_mod_t` struct.
///
/// @param n    The name of the pin operation (The maximal length is defined by `PI_STRLEN_OP_NAME`).
/// @param d    The direction of the operation, as defined by `pi_pin_dir_t`.
/// @param s    The callback function to initialize the pin operation.
/// @param rw   The callback function to execute as read/write operation.
/// @param a    An array pin numbers, defining for which pins the operation is allowed.
#define PI_ADD_OP(n, d, s, rw, a) { .name=n, .direction=d, .fn_init=s, .fn_rw=rw, .pins_allowed=a }

/// This macro initializes the `PI_PIN_OPS` and `PI_NUM_OPS` from a list of `pin_mod_t` structs.
#define PI_REGISTER_OPS(...) const struct pi_pin_op_t PI_PIN_OPS[] = {__VA_ARGS__}; const int PI_NUM_OPS = sizeof(PI_PIN_OPS)/sizeof(struct pi_pin_op_t)

// Helper definitions for error handling
#define PI_OK 0
#define PI_IS_OK(x) x == PI_OK
#define PI_IS_ERR(x) !(PI_IS_OK(x))
#define PI_OK_OR_RETURN(fn) {\
    pi_err_t err = fn; \
    if (err != 0) { return err; } \
}

/// The allowed length for the name of a pin operation.
#define PI_STRLEN_OP_NAME 50

typedef int pi_err_t;
typedef int pi_pin_nr_t;
typedef int pi_pin_op_nr_t;

/// Defines the possible directions of a pin operation.
enum pi_pin_dir_t {
    PI_DISABLED,    // The pin is disabled
    PI_READ,        // The operation reads values from a peripheral device
    PI_WRITE        // The operation writes values to a peripheral device
};

/// The complete definition of a pin operation.
/// This struct gets initialized by the `PI_ADD_OP` macro.
struct pi_pin_op_t {
    char name[PI_STRLEN_OP_NAME];
    enum pi_pin_dir_t direction;
    pi_err_t (*fn_init)(pi_pin_op_nr_t);
    pi_err_t (*fn_rw)(pi_pin_nr_t, double*);
    int pins_allowed[PI_NUM_PINS];
    int pins_allowed_size;
};

/// This struct contains the entire state of the pin interface library.
/// It gets initialized by the `pi_init` function.
extern struct pi_state_t {
    pi_pin_op_nr_t active_op_nrs[PI_NUM_PINS];
    pi_err_t (*rw_functions[PI_NUM_PINS])(pi_pin_nr_t, double*);
    enum pi_pin_dir_t directions[PI_NUM_PINS];
    pi_pin_op_nr_t allowed_op_nrs[PI_NUM_PINS];
} pi_state;

/// This function initializes the pin interface library.
/// The function needs to be called before any other pin interface function.
void pi_init(void);

/// Executes the pin operation previously defined by `pi_init_pin_op`.
///
/// @param pin_nr   The pin number, for which the operation should be executed
/// @param val      The value passed to the pin operation, or the storage for the current value. If the currently active operation for this pin is defined as `PI_WRITE`, the value is written as to the pin a output. If it is defined as `PI_READ`, the current value is stored in this parameter, after which it can be used from out the function.
/// @return         `PI_OK` if the operation was sucessful, the error code otherwise.
pi_err_t pi_exec_pin_op(pi_pin_nr_t pin_nr, double *val);

/// Initializes a pin operation for the specified pin.
///
/// @param pin_nr       The pin number, for which the operation should be enabled.
/// @param new_op_nr    The pin operation number of the new operation. The number is defined by the index inside the `PI_REGISTER_OPS` macro.
/// @return             `PI_OK` if the operation was sucessful, the error code otherwise.
pi_err_t pi_init_pin_op(const pi_pin_nr_t pin_nr, const pi_pin_op_nr_t new_op_nr);

/// This array contains all available pin operations.
/// The array is initialized by the `PI_REGISTER_OPS` macro.
extern const struct pi_pin_op_t PI_PIN_OPS[];

/// This constant shows how many pin operations are available.
extern const int PI_NUM_OPS;

#endif

