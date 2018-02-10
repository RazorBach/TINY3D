/////////////////////////////////////////////////////////////////////////////
//
// a tiny 3d engine 
//
// author : ch
//
// version : 1.0
//
/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <algorithm>
#include "MathUtil.h"
#include "wd.h"
#include "geometry.h"
#include "model.h"


int screen_keys[512];	// 当前键盘按下状态
int SCREEN_EXIT = 0;
const int screen_width = 800, screen_height = 800;
int main(int argc, char ** argv);
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);
const IUINT32 white = 0xFFFFFF, black = 0, red = 0xFF0000, green = 0x00FF00, blue =0x0000FF;
const IUINT32 _color(int r, int g, int b);

//
Model *model = NULL;
float *zbuffer;


Vec3f light_dir = Vec3f(1, -1, 1).normalize();
const int depth = 255;
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);

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

void rasterize(Vec2i p0, Vec2i p1, IUINT32 color, int ybuffer[]) {
	//easy rasterize function with "ybuffer"
	if (p0.x > p1.x) std::swap(p0, p1);
	for (int x = p0.x; x <= p1.x; ++x) {
		float t = (x - p0.x) / float(p1.x - p0.x);
		int y = interp(p0.y, p1.y, t);
		if (ybuffer[x] < y) {
			ybuffer[x] = y;
			for (int j = 0; j < 16; ++j) {
				setPixel(x, j, color);
			}
		}
	}
}

void _tri(Vec2i t0, Vec2i t1, Vec2i t2, IUINT32 color) {
	float invleft = (float)(t1.x - t0.x) / (t1.y - t0.y);
	float invright = (float)(t2.x - t0.x) / (t2.y - t0.y);
	float xs = (float) t0.x, xe = (float) t0.x;
	int flag = t0.y > t2.y ? -1 : 1;
	for (int y = t0.y; y != t1.y + flag; y += flag) {
		line(xs, y, xe, y, color);
		xs += invleft * flag;
		xe += invright * flag;
	}
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, IUINT32 color) {
	//draw triangle with scanline
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

Vec3f barycentric(Vec3f *pts, Vec3i P) {
	//barycentric coordinates of p
	Vec3f u = cross(Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]), Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]));
	if (std::abs(u[2])> 1e-2) return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z); 
	return Vec3f(-1, 1, 1);// simply return a negative coordinate
}

void triangle2(Vec3f *pts,Vec2i *uv, float *zbuffer,float* intensity) {
	//draw triangle with bounding box
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(screen_width - 1 , screen_height - 1);
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 2; ++j) {
			bboxmin[j] = std::max(0.f , std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	//todo
	//why vec3i can remove error?
	Vec3i P;
	for(P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
			Vec3f bc = barycentric(pts, P);
			if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
			P.z = 0;
			//use barycentric coordinates to calculate zbuffer and texture coordinates and light intensity
			Vec2i uvP;
			float intens(0.f);
			for (int i = 0; i < 3; ++i) {
				P.z += pts[i][2] * bc[i];
				uvP += uv[i] * bc[i];
				intens += intensity[i] * bc[i];
			}
			//intens = intens > 0 ? 1 : 0;
			if (intens <= 0 ) continue;
			if (zbuffer[int(P.y * screen_width + P.x)] < P.z) {
				zbuffer[int(P.y * screen_width + P.x)] = P.z;
				TGAColor color = model->diffuse(uvP);
				//use diffuse texture color
				//setPixel(P.x, P.y, _color(color.r*intensity, color.g*intensity, color.b*intensity));
				setPixel(P.x, P.y, _color(255 * intens, 255 * intens, 255 * intens));
			}
			
		}
	}
}


Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity();
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}

Matrix perspective(float theta, float ar, float zNear, float zFar) {
	//m[3][2] = -1 to make the z value opposite
	Matrix m = Matrix::identity();
	const float tanHalfFOV = tanf(theta / 2.f);
	m[0][0] = 1 / (tanHalfFOV * ar);
	m[1][1] = 1 / tanHalfFOV;
	m[2][2] = (-zNear - zFar) / (zFar - zNear);
	m[2][3] = - 2.f * zFar * zNear / (zFar - zNear);
	m[3][2] = -1;
	return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
	//camera face -z in the right hand coordinate system
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	Matrix res = Matrix::identity();
	for (int i = 0; i<3; i++) {
		res[0][i] = x[i];
		res[1][i] = y[i];
		res[2][i] = z[i];
		//res[i][3] = -center[i];
	}
	res[0][3] = - (eye * x);
	res[1][3] = - (eye * y);
	res[2][3] = - (eye * z);

	return res;
}


void drawModel() {

	Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
	Matrix Projection = perspective(kPi / 6, screen_width / screen_height, 1.f, 100.f);
	Matrix ViewPort = viewport(screen_width / 8, screen_height / 8, screen_width * 3 / 4, screen_height * 3 / 4);
	Matrix z = (ViewPort*Projection*ModelView);
	for (int i = 0; i<model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		//todo
		//Use int to represent screencoords
		Vec3f screen_coords[3];
		Vec3f world_coords[3];
		float intensity[3];
		for (int j = 0; j < 3; ++j) {
			world_coords[j] = model->vert(face[j]);
			Vec4f v(world_coords[j].x, world_coords[j].y, world_coords[j].z, 1);
			screen_coords[j] = (z * v).tovec3();
			intensity[j] = model->norm(i,j) * light_dir;
		} 
		/*Vec3f n = cross((world_coords[2] - world_coords[0]) , (world_coords[1] - world_coords[0]));
		n.normalize();*/
		//Gouraud shading
		//if (intensity > 0) {
			//texture coordinates
			Vec2i uv[3];
			for (int k = 0; k<3; k++) {
				uv[k] = model->uv(i, k);
			}
			triangle2(screen_coords, uv, zbuffer, intensity);
		//}
	}
	//delete model;
	//delete[] zbuffer;
}

void draw2D() {
	//// scene "2d mesh"
	//line(Vec2i(20, 34), Vec2i(744, 400), red);
	//line(Vec2i(120, 434), Vec2i(444, 400), green);
	//line(Vec2i(330, 463), Vec2i(594, 200), blue);

	//// screen line
	//line(Vec2i(10, 10), Vec2i(790, 10), white);
	int ybuffer[screen_width];
	for (auto &i : ybuffer) {
		i = std::numeric_limits<int>::min();
	}

	rasterize(Vec2i(20, 34), Vec2i(744, 400), red, ybuffer);
	rasterize(Vec2i(120, 434), Vec2i(444, 400), green, ybuffer);
	rasterize(Vec2i(330, 463), Vec2i(594, 200), blue, ybuffer);
}

int main(int argc, char **argv) {
	//init windows
	TCHAR *title = _T("TINY3D_BY_CH_VERSION_1.0 ");
	memset(screen_keys, 0, sizeof(int) * 512);

	model = new Model("obj/african_head.obj");
	zbuffer = new float[screen_width * screen_height];
	for (int i = 0; i < screen_width * screen_height; ++i) {
		zbuffer[i] = std::numeric_limits<float>::min();
	}
	

	if (screen_init(screen_width, screen_height, title, (WNDPROC)screen_events))
		return -1;
	int x = 0, y = 0;
	//render function
	while (SCREEN_EXIT == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch();

		
		//keyboard function
		if (screen_keys[VK_RIGHT]) eye.x += 0.01f;
		if (screen_keys[VK_LEFT]) eye.x -= 0.01f;
		/*if (screen_keys[VK_UP]) pos -= 0.01f;
		if (screen_keys[VK_DOWN]) pos += 0.01f;
		if (screen_keys[VK_LEFT]) alpha += 0.01f;
		if (screen_keys[VK_RIGHT]) alpha -= 0.01f;*/

		if (screen_keys[VK_SPACE]) {
		}

		drawModel();
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

const IUINT32 _color(int r, int g, int b) {
	//build color with rgb
	r = r <= 255 ? r >= 0 ? r : 0 : 255;
	g = g <= 255 ? g >= 0 ? g : 0 : 255;
	b = b <= 255 ? b >= 0 ? b : 0 : 255;
	return (r << 16) + (g << 8) + b;
}


