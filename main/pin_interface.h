#ifndef PIN_INTERFACE_H_
#define PIN_INTERFACE_H_


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

// Helper macro to initialize a pin_mod_t struct
#define PI_REGISTER_MODE(n, d, s, rw, a) { .name=n, .direction=d, .fn_init=s, .fn_rw=rw, .pins_allowed=a }

// The allowed length for the name of a mode
#define STRLEN_MODE_NAME 50

#define NUM_PINS 30

#define pin_dir_t uint8_t

typedef int pi_err_t;
typedef int pi_pin_nr_t;
typedef int pi_pin_mode_nr_t;

enum pi_pin_dir_t {
    PI_DISABLED,
    PI_READ,
    PI_WRITE
};

struct pi_pin_mode_t {
    char name[STRLEN_MODE_NAME];
    enum pi_pin_dir_t direction;
    pi_err_t (*fn_init)(pi_pin_mode_nr_t);
    pi_err_t (*fn_rw)(pi_pin_nr_t, double*);
    int pins_allowed[NUM_PINS];
    int pins_allowed_size;
};

void pi_init(void);

pi_err_t pi_exec_pin_op(pi_pin_nr_t pin_nr, double *val);

pi_err_t pi_init_pin_mode(pi_pin_nr_t pin_nr, pi_pin_mode_nr_t pin_mode_nr);

extern const struct pi_pin_mode_t PI_PIN_MODES[];

extern const int PI_NUM_MODES;

#endif

