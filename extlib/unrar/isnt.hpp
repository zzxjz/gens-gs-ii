#ifndef _RAR_ISNT_
#define _RAR_ISNT_

enum WINNT_VERSION {
  WNT_NONE=0,WNT_NT351=0x0333,WNT_NT4=0x0400,WNT_W2000=0x0500,
  WNT_WXP=0x0501,WNT_W2003=0x0502,WNT_VISTA=0x0600,WNT_W7=0x0601,
  WNT_W8=0x0602,WNT_W81=0x0603,WNT_W10=0x0a00
};

DWORD WinNT();

#endif
