/////////////////////////////////////////////////////////////////////////////
//
// Win32 ���ڼ�ͼ�λ���
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WD_H
#define WD_H

#define NOMINMAX
#include <windows.h>
#include <tchar.h>

// Check if MS Visual C compiler
#ifdef _MSC_VER          
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

typedef unsigned IUINT32;

int screen_init(int w, int h, const TCHAR *title, WNDPROC screen_events);	// ��Ļ��ʼ��
int screen_close();								// �ر���Ļ
void screen_dispatch();							// ������Ϣ
void screen_update();							// ��ʾ FrameBuffer

void setPixel(int x, int y, IUINT32 color);

#endif // !WD_H
