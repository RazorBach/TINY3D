#ifndef MY_GL_H
#define MY_GL_H
#define NOMINMAX 
#include "device.h"
#include "image.h"
#include "math.h"
#include "wd.h"
// Include C++ headers 
#include <algorithm> 
#include <limits>
#include <memory>
#include <cstdlib>
using namespace std;


extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f); // coeff = -1/c
void projection(float fovy, float aspect, float near_z, float far_z);
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
	virtual ~IShader();
	virtual Vec4f vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vec3f bar, IUINT32 &color) = 0;
};

const IUINT32 _color(int r, int g, int b);
void triangle(Device& device, mat<4, 3, float> &clipc, std::shared_ptr<IShader> shader);
#endif // !MY_GL_H
