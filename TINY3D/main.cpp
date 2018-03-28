/////////////////////////////////////////////////////////////////////////////
//
// a tiny 3d renderer
//
// author : ch
//
// version : 1.0
//
/////////////////////////////////////////////////////////////////////////////

#define NOMINMAX
#include <iostream>
#include <algorithm>
#include <string>
#include <memory>
#include <ctime>
#include "model.h"
#include "gl_shader.h"

int screen_keys[512];	// 当前键盘按下状态
int SCREEN_EXIT = 0;
const int screen_width = 800, screen_height = 800;
int main(int argc, char ** argv);
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);
const IUINT32 white = 0xFFFFFF, black = 0, red = 0xFF0000, green = 0x00FF00, blue =0x0000FF;
unsigned char *screen_fb = NULL;

Vec3f light_dir = Vec3f(1.f, 1.f, 1.f).normalize();
Vec3f eye(1.f, 1.f, 3.f);
Vec3f center(0, 0, 0);
Vec3f up(0, 1.f, 0);

// Bresenham's line algorithm
// https://zh.wikipedia.org/wiki/%E5%B8%83%E9%9B%B7%E6%A3%AE%E6%BC%A2%E5%A7%86%E7%9B%B4%E7%B7%9A%E6%BC%94%E7%AE%97%E6%B3%95
void line(Device& device,float x1, float y1, float x2, float y2, const IUINT32 color) {
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
			device.device_pixel(y, x, color);
		}
		else{
			device.device_pixel(x, y, color);
		}
		error -= dy;
		if (error < 0){
			y += ystep;
			error += dx;
		}
	}
}

//struct Shader : public IShader {
//	mat<4, 4, float> uniform_M;   //  Projection*ModelView
//	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()
//	mat<4, 4, float> uniform_Mshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates
//	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
//	mat<3, 3, float> varying_tri; // triangle coordinates before Viewport transform, written by VS, read by FS
//
//	Shader(std::shared_ptr<Model>& m,Matrix M, Matrix MIT, Matrix MS) :model(m), uniform_M(M), uniform_MIT(MIT), uniform_Mshadow(MS), varying_uv(), varying_tri() {}
//
//	virtual Vec4f vertex(int iface, int nthvert) {
//		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
//		Vec4f gl_Vertex = Viewport*Projection*ModelView*embed<4>(model->vert(iface, nthvert));
//		varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
//		return gl_Vertex;
//	}
//
//	virtual bool fragment(Vec3f bar, TGAColor &color) {
//		Vec4f sb_p = uniform_Mshadow*embed<4>(varying_tri*bar); // corresponding point in the shadow buffer
//		sb_p = sb_p / sb_p[3];
//		int idx = int(sb_p[0]) + int(sb_p[1])*width; // index in the shadowbuffer array
//		float shadow = .3 + .7*(shadowbuffer[idx]<sb_p[2]); // magic coeff to avoid z-fighting
//		Vec2f uv = varying_uv*bar;                 // interpolate uv for the current pixel
//		Vec3f n = proj<3>(uniform_MIT*embed<4>(model->normal(uv))).normalize(); // normal
//		Vec3f l = proj<3>(uniform_M  *embed<4>(light_dir)).normalize(); // light vector
//		Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
//		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
//		float diff = std::max(0.f, n*l);
//		TGAColor c = model->diffuse(uv);
//		for (int i = 0; i<3; i++) color[i] = std::min<float>(20 + c[i] * shadow*(1.2*diff + .6*spec), 255);
//		return false;
//	}
//private:
//	std::shared_ptr<Model> model;
//};

struct GouraudShader : public IShader {
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<4, 3, float> varying_tri; // triangle coordinates (clip coordinates), written by VS, read by FS
	mat<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
	mat<3, 3, float> ndc_tri;     // triangle in normalized device coordinates

	GouraudShader(std::shared_ptr<Model>& m):model(m),varying_uv(),varying_tri(),varying_nrm(),ndc_tri() {};

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_nrm.set_col(nthvert, proj<3>((Projection*ModelView).invert_transpose()*embed<4>(model->norm(iface, nthvert), 0.f)));

		//varying_intensity[nthvert] = std::max(0.f, model->norm(iface, nthvert)*light_dir); // get diffuse lighting intensity
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        gl_Vertex = Projection*ModelView*gl_Vertex;    
		varying_tri.set_col(nthvert, gl_Vertex);
		ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
        return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, IUINT32 &color) {
		Vec3f bn = (varying_nrm*bar).normalize();
		Vec2f uv = varying_uv*bar;
		
		mat<3, 3, float> A;
		A[0] = ndc_tri.col(1) - ndc_tri.col(0);
		A[1] = ndc_tri.col(2) - ndc_tri.col(0);
		A[2] = bn;
		mat<3, 3, float> AI = A.invert();

		Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

		mat<3, 3, float> B;                      //B为TBN矩阵的逆矩阵！
		B.set_col(0, i.normalize());
		B.set_col(1, j.normalize());
		B.set_col(2, bn);

		Vec3f n = (B*model->normal(uv)).normalize();  //换算到世界坐标下进行计算
		//todo 把世界坐标换算到TBN矩阵空间 这样可以优化，减少fragment内矩阵计算次数！

		float diff = std::max(0.f, n*light_dir);
		color = (model->diffuse(uv)*diff).toColor32();
		return false;
	}
private:
	std::shared_ptr<Model> model;
};

struct DepthShader : public IShader {
	mat<3, 3, float> varying_tri;

	DepthShader(std::shared_ptr<Model>& m) : varying_tri(),model(m) {}

	virtual Vec4f vertex(int iface, int nthvert) {
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;          // transform it to screen coordinates
		varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, IUINT32 &color) {
		Vec3f p = varying_tri*bar;
		//color = TGAColor(255, 255, 255)*(p.z / depth);
		return false;
	}
private:
	std::shared_ptr<Model> model;
};

void drawModelWithShader(std::shared_ptr<Model> model,Device& device) {
	lookat(eye, center, up);
	//projection(45.f, (float)screen_width / (float)screen_height, 0.1f, 50.f);
	projection(-1.f / (eye - center).norm());
	viewport(screen_width / 8, screen_height / 8, screen_width * 3 / 4, screen_height * 3 / 4);
	light_dir = proj<3>((Projection*ModelView*embed<4>(light_dir, 0.f))).normalize();

	std::shared_ptr<GouraudShader> shader = make_shared<GouraudShader>(model);
	for (int i = 0; i< model->nfaces(); i++) {
		for (int j = 0; j<3; j++) {
			shader->vertex(i, j);
		}
		triangle(device, shader->varying_tri, shader);
	}
}

clock_t current_ticks, delta_ticks;
clock_t fps = 0;

int main(int argc, char **argv) {
	//init windows
	TCHAR *title = _T("TINY3D_BY_CH_VERSION_1.0 ");
	memset(screen_keys, 0, sizeof(screen_keys));

	//利用shared_ptr管理资源
	std::shared_ptr<Model> model = std::make_shared<Model>("obj/african_head/african_head.obj");
	//std::shared_ptr<Model> model(new Model("obj/african_head.obj"));

	if (screen_init(screen_width, screen_height, title, (WNDPROC)screen_events))
		return -1;
	Device device(screen_width, screen_height, screen_fb);
	int x = 0, y = 0;
	//render function
	while (SCREEN_EXIT == 0 && screen_keys[VK_ESCAPE] == 0) {
		current_ticks = clock();
		screen_dispatch();
		//清空buffer
		//device.device_clear(1);
		//keyboard function
		if (screen_keys[VK_RIGHT]) eye.x += 0.1f;
		if (screen_keys[VK_LEFT]) eye.x -= 0.1f;
		if (screen_keys[VK_UP]) eye.y += 0.1f;
		if (screen_keys[VK_DOWN])eye.y -= 0.1f;

		if (screen_keys[VK_SPACE]) {
		}
		//line(device, 0.f, 0.f, 500.f, 500.f, red);
		drawModelWithShader(model, device);
		screen_update();
		delta_ticks = clock() - current_ticks; //ms
		if (delta_ticks > 0)
			fps = CLOCKS_PER_SEC / delta_ticks;
		cout << fps << endl;
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


