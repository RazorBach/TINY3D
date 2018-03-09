/////////////////////////////////////////////////////////////////////////////
//
// Win32 窗口及图形绘制
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WD_H
#define WD_H

#define NOMINMAX
#include <windows.h>
#include <tchar.h>
#include "device.h"

// Check if MS Visual C compiler
#ifdef _MSC_VER          
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif


int screen_init(int w, int h, const TCHAR *title, WNDPROC screen_events);	// 屏幕初始化
int screen_close();								// 关闭屏幕
void screen_dispatch();							// 处理消息
void screen_update();							// 显示 FrameBuffer

void device_clear(int mwidth, int mheight, float* zbuffer);

extern unsigned char *screen_fb;		// frame buffer

#endif // !WD_H
