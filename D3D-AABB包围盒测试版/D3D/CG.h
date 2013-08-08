#ifndef __D3DCOMPUTINGGEOMETRYH__
#define __D3DCOMPUTINGGEOMETRYH__
#include"d3dUtility.h"
#define EPSILON 1e-5
inline float Dot(const D3DXVECTOR3 &p, const D3DXVECTOR3 &q){
	return p.x * q.x + p.y * q.y + p.z * q.z;
}
inline D3DXVECTOR3 Cross(const D3DXVECTOR3 &p, const D3DXVECTOR3 &q){
	return D3DXVECTOR3(p.y*q.z - p.z*q.y,p.z*q.x-p.x*q.z,p.x*q.y - p.y*q.x);
}
inline float Dot2D(const D3DXVECTOR2 &p, const D3DXVECTOR2 &q,const D3DXVECTOR2 &r){
	return (q.x-p.x)*(r.x-p.x) + (q.y-p.y)*(r.y-p.y);
}
inline float Cross2D(const D3DXVECTOR2 &p1,const D3DXVECTOR2 &p2,const D3DXVECTOR2 &p3){
	//>0表示p1p3在p1p2逆时针方向
	//=0表示共线
	//<0表示p1p3在p1p2顺时针方向
	return (p2.x - p1.x) * (p3.x - p1.y) - (p2.y - p1.y) * (p3.y - p1.y);
}
template<typename T>
T Max3(const T &a, const T &b, const T &c){
	return max(a,max(b,c));
}
template<typename T>
T Min3(const T &a, const T &b, const T &c){
	return min(a,min(b,c));
}

inline BOOL FLTEQ(const float &a, const float &b){
	return fabs(a-b)<EPSILON;
}
//Test if AABB b intersects plane p
BOOL TestAABBPlane(const d3d::BoundingBox &b, const d3d::Plane &p);
//AABB与三角形之间的相交测试
BOOL TestTriangleAABB(D3DXVECTOR3 v0, D3DXVECTOR3 v1, D3DXVECTOR3 v2, const d3d::BoundingBox &b);
BOOL IntersectSegmentPlane(const D3DXVECTOR3 &a, const D3DXVECTOR3 &b, const d3d::Plane &p,D3DXVECTOR3 &q);
BOOL PointInTriangle2D(const D3DXVECTOR2 &p, const D3DXVECTOR2 t[3]);
BOOL TestSegmentIntersect2D(const D3DXVECTOR2 &p1_start, const D3DXVECTOR2 &p1_end, const D3DXVECTOR2 &p2_start, const D3DXVECTOR2 &p2_end);
BOOL TestCoPlaneTriangle2D(const D3DXVECTOR2 t1[3],const D3DXVECTOR2 t2[3]);
BOOL TestTriangleTriangle(const D3DXVECTOR3 t1[3],const D3DXVECTOR3 t2[3]);
#endif