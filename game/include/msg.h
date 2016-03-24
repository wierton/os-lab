#ifndef __MSG_H__
#define __MSG_H__

typedef uint32_t OSKEY;

#define OS_KEYUP 1
#define OS_KEYDOWN 0

#define KEY_RETURN 0x1c
#define KEY_z 0x2c
#define KEY_x 0x2d
#define KEY_r 0x13
#define KEY_l 0x26
#define KEY_w 0x11
#define KEY_s 0x1f
#define KEY_a 0x1e
#define KEY_d 0x20
#define KEY_UP 0xe048
#define KEY_DOWN 0xe050
#define KEY_LEFT 0xe04b
#define KEY_RIGHT 0xe04d

typedef enum tagPALDIRECTION {
    kdirUp = 0,
    kdirDown,
    kdirLeft,
    kdirRight,
    kdirUnknown
} PALDIRECTION, *LPPALDIRECTION;

typedef struct tagINPUTSTATE {
    PALDIRECTION prevdir, dir;
    OSKEY dwKey;
} INPUTSTATE;

#define KEY_UNKNOWN 0

extern INPUTSTATE g_InputState;

void msgloop();

#endif
