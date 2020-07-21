#ifndef _MAIN_H_
#define _MAIN_H_

#define VERSION				2
#define SUBVERSION			0
#define SUBSUBVERSION		2

#define HOOKS_NUM         17 // Hooked functions num
#define PHYS_BUTTONS_NUM  16 // Supported physical buttons num
#define TARGET_REMAPS     42 // Supported target remaps num

#define BUFFERS_NUM      			64

#define POSITIVE 0
#define NEGATIVE 1

#define PEEK 0
#define REED 1

extern char titleid[16];
extern uint8_t used_funcs[HOOKS_NUM];
extern uint16_t TOUCH_SIZE[4];
extern const uint32_t btns[PHYS_BUTTONS_NUM];

extern int model;
extern uint8_t internal_touch_call;
extern uint8_t internal_ext_call;

extern void delayedStart();

#endif