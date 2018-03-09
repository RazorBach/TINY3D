#include "my_gl.h"

Matrix ModelView;
Matrix Viewport;
Matrix Projection;


void viewport(int x, int y, int w, int h) {
	//x y 偏移量 w宽度 h高度
	Viewport = Matrix::identity();
	Viewport[0][3] = x + w / 2.f;
	Viewport[1][3] = y + h / 2.f;
	Viewport[2][3] = 255.f / 2.f;

	Viewport[0][0] = w / 2.f;
	Viewport[1][1] = h / 2.f;
	Viewport[2][2] = 255.f / 2.f;
}

void projection(float coeff) {
	Projection = Matrix::identity();
	Projection[3][2] = coeff;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up) {
	//camera face -z in the right hand coordinate system
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	ModelView = Matrix::identity();
	for (int i = 0; i<3; i++) {
		ModelView[0][i] = x[i];
		ModelView[1][i] = y[i];
		ModelView[2][i] = z[i];
		ModelView[i][3] = -center[i];
	}

}


IShader::~IShader()
{
}

Vec3f barycentric(Vec3f *pts, Vec2i P) {
	//p的重心坐标
	Vec3f u = cross(Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]), Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]));
	if (std::abs(u[2])> 1e-2) return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1);// simply return a negative coordinate
}

//Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2i P) {
//	Vec3f s[2];
//	for (int i = 2; i--; ) {
//		s[i][0] = C[i] - A[i];
//		s[i][1] = B[i] - A[i];
//		s[i][2] = A[i] - P[i];
//	}
//	Vec3f u = cross(s[0], s[1]);
//	if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
//		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
//	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
//}

//void triangle(Vec3f *pts, Vec2i *uv, float *zbuffer, float* intensity) {
//	//重心坐标光栅化
//	//draw triangle with bounding box
//	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
//	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
//	Vec2f clamp(screen_width - 1, screen_height - 1);
//	for (int i = 0; i < 3; ++i) {
//		for (int j = 0; j < 2; ++j) {
//			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
//			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
//		}
//	}
//	Vec2i bboxmini(bboxmin);
//	Vec2i bboxmaxi(bboxmax);
//	Vec3i P;
//	for (P.x = bboxmini.x; P.x <= bboxmaxi.x; ++P.x) {
//		for (P.y = bboxmini.y; P.y <= bboxmaxi.y; ++P.y) {
//			Vec3f bc = barycentric(pts, P);
//			if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
//			P.z = 0;
//			//use barycentric coordinates to calculate zbuffer and texture coordinates and light intensity
//			Vec2i uvP;
//			float intens(0.f);
//			for (int i = 0; i < 3; ++i) {
//				/*	if(intensity[i] < 0)
//				std::cout << intensity[i] << " ";*/
//				P.z += pts[i][2] * bc[i];
//				uvP += uv[i] * bc[i];
//				intens += intensity[i] * bc[i];
//			}
//			//intens = intens > 0 ? 1 : 0;
//			if (intens <= 0) continue;
//			int idx = P.y * screen_width + P.x;
//			if (zbuffer[idx] < P.z) {
//				zbuffer[idx] = P.z;
//				//TGAColor color = model->diffuse(uvP);
//				//use diffuse texture color
//				//setPixel(P.x, P.y, _color(color.r*intensity, color.g*intensity, color.b*intensity));
//				setPixel(P.x, P.y, _color(255 * intens, 255 * intens, 255 * intens));
//			}
//		}
//	}
//}

void triangle(Device& device, Vec3f *pts, IShader &shader) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(device.width - 1, device.width - 1);
	for (int i = 0; i<3; i++) {
		for (int j = 0; j<2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec2i bboxmini(bboxmin);
	Vec2i bboxmaxi(bboxmax);
	Vec2i P;
	IUINT32 color;
	for (P.x = bboxmini.x; P.x <= bboxmaxi.x; P.x++) {
		for (P.y = bboxmini.y; P.y <= bboxmaxi.y; P.y++) {
			Vec3f bc = barycentric(pts, P);
			if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
			int frag_depth = pts[0][2] * bc.x + pts[0][1] * bc.y + pts[0][2] * bc.z ;
			if (device.zbuffer[P.y][P.x] < frag_depth) {
				device.zbuffer[P.y][P.x] = frag_depth;
				bool discard = shader.fragment(bc, color);
				if (!discard) {
					//printf("%d,%d,%d \n", P.x, P.y, color);
					device.device_pixel(P.x, P.y, color);
				}
			}
		}
	}
}


//void _tri(Vec2i t0, Vec2i t1, Vec2i t2, IUINT32 color) {
//	float invleft = (float)(t1.x - t0.x) / (t1.y - t0.y);
//	float invright = (float)(t2.x - t0.x) / (t2.y - t0.y);
//	float xs = (float)t0.x, xe = (float)t0.x;
//	int flag = t0.y > t2.y ? -1 : 1;
//	for (int y = t0.y; y != t1.y + flag; y += flag) {
//		line(xs, y, xe, y, color);
//		xs += invleft * flag;
//		xe += invright * flag;
//	}
//}
//
//void triangle(Vec2i t0, Vec2i t1, Vec2i t2, IUINT32 color) {
//	//扫描线光栅化方法(暂时弃用
//
//	if (t0.y == t1.y && t1.y == t2.y) return;
//	// sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
//	if (t0.y > t1.y) std::swap(t0, t1);
//	if (t0.y > t2.y) std::swap(t0, t2);
//	if (t1.y > t2.y) std::swap(t1, t2);
//	if (t0.y == t1.y) _tri(t2, t0, t1, color);
//	else if (t1.y == t2.y) _tri(t0, t1, t2, color);
//	else {
//		int t3x = (t2.x - t0.x) * (t1.y - t0.y) / (t2.y - t0.y) + t0.x;
//		Vec2i t3(t3x, t1.y);
//		_tri(t0, t1, t3, color);
//		_tri(t2, t1, t3, color);
//	}
//}
//Matrix perspective(float theta, float ar, float zNear, float zFar) {
//	//m[3][2] = -1 to make the z value opposite
//	//opengl写法 最后z值在 [-w, +w] （除以w以后在[-1, +1])
//	Matrix m = Matrix::identity();
//	const float tanHalfFOV = tanf(theta / 2.f);
//	m[0][0] = 1 / (tanHalfFOV * ar);
//	m[1][1] = 1 / tanHalfFOV;
//	m[2][2] = (-zNear - zFar) / (zFar - zNear);
//	m[2][3] = -2.f * zFar * zNear / (zFar - zNear);
//	m[3][2] = -1;
//	return m;
//}

//Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
//	//camera face -z in the right hand coordinate system
//	Vec3f z = (eye - center).normalize();
//	Vec3f x = cross(up, z).normalize();
//	Vec3f y = cross(z, x).normalize();
//	Matrix res = Matrix::identity();
//	for (int i = 0; i<3; i++) {
//		res[0][i] = x[i];
//		res[1][i] = y[i];
//		res[2][i] = z[i];
//	}
//	res[0][3] = -(eye * x);
//	res[1][3] = -(eye * y);
//	res[2][3] = -(eye * z);
//
//	return res;
//}
