/////////////////////////////////////////////////////////////////////////////
//
// a tiny 3d engine 
//
// author : ch
//
// version : 1.0
//
/////////////////////////////////////////////////////////////////////////////
#include "wd.h"
#include <iostream>
#include "geometry.h"


int screen_keys[512];	// 当前键盘按下状态
int SCREEN_EXIT = 0;
int screen_width = 1280, screen_height = 720;
int main(int argc, char ** argv);
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);
const IUINT32 white = 0xFFFFFF, black = 0, red = 0xFF0000, green = 0x00FF00;

// Bresenham's line algorithm
// https://zh.wikipedia.org/wiki/%E5%B8%83%E9%9B%B7%E6%A3%AE%E6%BC%A2%E5%A7%86%E7%9B%B4%E7%B7%9A%E6%BC%94%E7%AE%97%E6%B3%95
void line(float x1, float y1, float x2, float y2, const IUINT32 color) {
	const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
	if (steep){
		std::swap(x1, y1);
		std::swap(x2, y2);
	}
	if (x1 > x2){
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
	const float dx = x2 - x1;
	const float dy = fabs(y2 - y1);

	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = (int)y1;

	for (int x = (int)x1; x < (int)x2; x++) {
		if (steep){
			setPixel(y, x, color);
		}
		else{
			setPixel(x, y, color);
		}
		error -= dy;
		if (error < 0){
			y += ystep;
			error += dx;
		}
	}
}

template<class T1,class T2>
void line(T1 v1, T2 v2, const IUINT32 color) {
	line(v1.x, v1.y, v2.x, v2.y, color);
}


void _tri(Vec2i t0, Vec2i t1, Vec2i t2, IUINT32 color) {
	float invleft = (float)(t1.x - t0.x) / (t1.y - t0.y);
	float invright = (float)(t2.x - t0.x) / (t2.y - t0.y);
	float xs = t0.x, xe = t0.x;
	int flag = t0.y > t2.y ? -1 : 1;
	for (int y = t0.y; y != t1.y + flag; y += flag) {
		line(xs, y, xe, y, color);
		xs += invleft * flag;
		xe += invright * flag;
	}
}


void triangle(Vec2i t0, Vec2i t1, Vec2i t2, IUINT32 color) {
	if (t0.y == t1.y && t1.y == t2.y) return;
	// sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	if (t0.y == t1.y) _tri(t2, t0, t1, color);
	else if (t1.y == t2.y) _tri(t0, t1, t2, color);
	else {
		int t3x = (t2.x - t0.x) * (t1.y - t0.y) / (t2.y - t0.y) + t0.x;
		Vec2i t3(t3x, t1.y);
		_tri(t0, t1, t3, color);
		_tri(t2, t1, t3, color);
	}
	
	
}

void draw() {
	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
	triangle(t0[0], t0[1], t0[2], red);
	triangle(t1[0], t1[1], t1[2], white);
	triangle(t2[0], t2[1], t2[2], green);
}

int main(int argc, char **argv) {
	//init windows
	TCHAR *title = _T("TINY3D_BY_CH_VERSION_1.0 ")
		_T("Left/Right: rotation, Up/Down: forward/backward, Space: switch state");
	memset(screen_keys, 0, sizeof(int) * 512);
	if (screen_init(screen_width, screen_height, title, (WNDPROC)screen_events))
		return -1;
	int x = 0, y = 0;
	draw();
	while (SCREEN_EXIT == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch();
		//keyboard function
		/*if (screen_keys[VK_UP]) pos -= 0.01f;
		if (screen_keys[VK_DOWN]) pos += 0.01f;
		if (screen_keys[VK_LEFT]) alpha += 0.01f;
		if (screen_keys[VK_RIGHT]) alpha -= 0.01f;*/

		if (screen_keys[VK_SPACE]) {
		}

		screen_update();
		Sleep(1);
	}
	return 0;
}


static LRESULT screen_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE: SCREEN_EXIT = 1; break;
	case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}


