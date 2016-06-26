#ifndef __SHELL_COMARG_H__
#define __SHELL_COMARG_H__

#define CHAR_W 8
#define CHAR_H 8
#define D_CHAR_W 16
#define D_CHAR_H 16
#define TIMES (D_CHAR_W / CHAR_W)
#define SCR_W 640
#define SCR_H 480

#define CHAR_R (SCR_W / D_CHAR_W)
#define CHAR_L (SCR_H / D_CHAR_H)

#define KEY_UP 0xe048
#define KEY_DOWN 0xe050
#define KEY_LEFT 0xe04b
#define KEY_RIGHT 0xe04d

#endif
