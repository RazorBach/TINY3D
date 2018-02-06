/////////////////////////////////////////////////////////////////////////////
//
// Win32 窗口及图形绘制
//
/////////////////////////////////////////////////////////////////////////////
#include "wd.h"

int SCREEN_WIDTH, SCREEN_HEIGHT;
static HWND screen_handle = NULL;		// 主窗口 HWND
static HDC screen_dc = NULL;			// 配套的 HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// 老的 BITMAP
unsigned char *screen_fb = NULL;		// frame buffer
IUINT32 **framebuffer;      // 像素缓存：framebuffer[y] 代表第 y行
long screen_pitch = 0;


int screen_init(int w, int h, const TCHAR *title, WNDPROC screen_events) {
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	screen_close();

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	hDC = GetDC(screen_handle);
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	SCREEN_WIDTH = w;
	SCREEN_HEIGHT = h;
	screen_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	screen_dispatch();

	
	memset(screen_fb, 0, w * h * 4);
	//

	int need = sizeof(void*) * (SCREEN_HEIGHT * 2 + 1024) + SCREEN_WIDTH * SCREEN_HEIGHT * 8;
	char *c_ptr = (char*)malloc(need + 64);

	//char *c_ptr = new char(sizeof(void*) * (SCREEN_HEIGHT * 2 + 1024) + SCREEN_WIDTH * SCREEN_HEIGHT * 8 + 64);
	framebuffer = (IUINT32 **)c_ptr;
	for (int j = 0; j < SCREEN_HEIGHT; j++) {
		//flip the image to set the y axis up
		framebuffer[j] = (IUINT32*)(screen_fb + SCREEN_WIDTH * 4 *(SCREEN_HEIGHT - j -1));
	}

	return 0;
}

int screen_close() {
	if (screen_dc) {
		if (screen_ob) {
			SelectObject(screen_dc, screen_ob);
			screen_ob = NULL;
		}
		DeleteDC(screen_dc);
		screen_dc = NULL;
	}
	if (screen_hb) {
		DeleteObject(screen_hb);
		screen_hb = NULL;
	}
	if (screen_handle) {
		CloseWindow(screen_handle);
		screen_handle = NULL;
	}
	return 0;
}

void screen_dispatch() {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void screen_update() {
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(screen_handle, hDC);
	screen_dispatch();
}

void setPixel(int x, int y, IUINT32 color) {
	if (((IUINT32)x) < (IUINT32) SCREEN_WIDTH && ((IUINT32)y) < (IUINT32)SCREEN_HEIGHT) {
		framebuffer[y][x] = color;
	}
}
