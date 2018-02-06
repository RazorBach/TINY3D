//todo: 加入齐次坐标
//3d vector class
#ifndef VECTOR3_H
#define VECTOR3_H
#include <math.h>

class Vector3{
public:
	float x, y, z;
	Vector3() {}
	
	Vector3(const Vector3 &v) : x(v.x), y(v.y), z(v.z) {}
	
	Vector3(float value_x, float value_y, float value_z) : x(value_x), y(value_y), z(value_z) {}

	void zero() {
		x = y = z = 0.0f;
	}

	void normal() {
		float lengSq = x*x + y*y + z*z;
		if (lengSq > 0) {
			float inv = 1.0f / sqrt(lengSq);
			x *= inv;
			y *= inv;
			z *= inv;
		}
	}

	//操作符重载
	Vector3 &operator =(const Vector3 &rhv) {
		x = rhv.x; y = rhv.y; z = rhv.z;
		return *this;
	}

	//向量运算
	Vector3 operator - () const{
		return Vector3(-x, -y, -z);
	}
	
	Vector3 &operator +=(const Vector3 &v){
		x += v.x; y += v.y; z += v.z;
		return *this;
	}

	Vector3 operator -=(const Vector3 &v){
		x -= v.x, y -= v.y; z -= v.z;
		return *this;
	}

	//标量运算
	Vector3 operator *=(float scalar) {
		x *= scalar; y *= scalar; z *= scalar;
		return *this;
	}

	Vector3 operator /=(float scalar) {
		float inv = 1.0f / scalar;
		x *= inv; y *= inv; z *= inv;
		return *this;
	}

};

//向量
inline bool operator ==(const Vector3 &lhv, const Vector3 &rhv){
	return lhv.x == rhv.x && lhv.y == rhv.y && lhv.z == rhv.z;
}

inline bool operator !=(const Vector3 &lhv, const Vector3 &rhv){
	return !(lhv == rhv);
}


Vector3 operator +(const Vector3 lhv, const Vector3 &rhv) {
	Vector3 ret(lhv);
	ret += rhv;
	return ret;
}

Vector3 operator -(const Vector3 lhv, const Vector3 &rhv) {
	Vector3 ret(lhv);
	ret -= rhv;
	return ret;
}

Vector3 operator *(const Vector3 lhv, const Vector3 &rhv) {
	return Vector3(lhv.x * rhv.x, lhv.y * rhv.y, lhv.z * rhv.z);
}

//标量
Vector3 operator * (const Vector3 &v, float scalar) {
	Vector3 ret(v);
	ret *= scalar;
	return ret;
}

//左乘
Vector3 operator * (float scalar, const Vector3 v) {
	Vector3 ret(v);
	ret *= scalar;
	return ret;
}


Vector3 operator / (const Vector3 &v, float scalar) {
	Vector3 ret(v);
	ret /= scalar;
	return ret;
}


//向量模
inline float _length(const Vector3 &v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

//叉乘
inline Vector3 cross(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.z);
}

//计算距离
inline float distance(const Vector3 &v1, const Vector3 &v2) {
	float dx = v1.x - v2.x; float dy = v1.y - v2.y; float dz = v1.z - v2.z;
	return sqrt(dx*dx + dy*dy + dz*dz);
}

//零向量
extern const Vector3 ZeroVector;

#endif

