#include "geometry.h"
//todo 为什么模板可以分离？？？？？？？
template <> template <> vec<3, int>  ::vec(const vec<3, float> &v) : x(int(v.x + .5f)), y(int(v.y + .5f)), z(int(v.z + .5f)) {}
template <> template <> vec<3, float>::vec(const vec<3, int> &v) : x(v.x), y(v.y), z(v.z) {}
template <> template <> vec<2, int>  ::vec(const vec<2, float> &v) : x(int(v.x + .5f)), y(int(v.y + .5f)) {}
template <> template <> vec<2, float>::vec(const vec<2, int> &v) : x(v.x), y(v.y) {}

//template <> template <> vec<3, int>  ::operator=(const vec<3, float> &v){
//	x = int(v.x + .5f), y = int(v.y + .5f), z= int(v.z + .5f);
//	return *this; }
//template <> template <> vec<3, float>::operator=(const vec<3, int> &v) : x(v.x), y(v.y), z(v.z) {}
//template <> template <> vec<2, int>  ::operator=(const vec<2, float> &v) : x(int(v.x + .5f)), y(int(v.y + .5f)) {}
//template <> template <> vec<2, float>::operator=(const vec<2, int> &v) : x(v.x), y(v.y) {}