#ifndef __d3dIntersecH__
#define __d3dIntersecH__
#include"d3dUtility.h"
#define EPSILON 1e-5
inline float Dot(const D3DXVECTOR3 &p, const D3DXVECTOR3 &q){
	return p.x * q.x + p.y * q.y + p.z * q.z;
}
inline D3DXVECTOR3 Cross(const D3DXVECTOR3 &p, const D3DXVECTOR3 &q){
	return D3DXVECTOR3(p.y*q.z - p.z*q.y,p.z*q.x-p.x*q.z,p.x*q.y - p.y*q.x);
}
template<typename T>
T Max3(const T &a, const T &b, const T &c){
	return max(a,max(b,c));
}
template<typename T>
T Min3(const T &a, const T &b, const T &c){
	return min(a,min(b,c));
}

BOOL FLTEQ(const float &a, const float &b){
	return fabs(a-b)<EPSILON;
}
//Test if AABB b intersects plane p
BOOL TestAABBPlane(const d3d::BoundingBox &b, const d3d::Plane &p){
	D3DXVECTOR3 c = (b._max + b._min) * 0.5f;
	D3DXVECTOR3 e = b._max - c;

	float r = e.x * abs(p.n.x) + e.y * abs(p.n.y) + e.z * abs(p.n.z);
	
	float s = Dot(p.n,c)-p.d;
	return abs(s) <= r;
}
//AABB与三角形之间的相交测试
BOOL TestTriangleAABB(D3DXVECTOR3 &v0, D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, const d3d::BoundingBox &b){
	float p0,p1,p2,r;

	//compute box center and extents
	D3DXVECTOR3 c = (b._min + b._max) * 0.5f;
	D3DXVECTOR3 e = b._max - c;

	v0 = v0 -  c;
	v1 = v1 - c;
	v2 = v2 - c;

	//Test the three axes corresponding to the face normals of AABB b
	if(Max3(v0.x,v1.x,v2.x) < -e.x || Min3(v0.x,v1.x,v2.x) > e.x) return 0;
	if(Max3(v0.y,v1.y,v2.y) < -e.y || Min3(v0.y,v1.y,v2.y) > e.y) return 0;
	if(Max3(v0.z,v1.z,v2.z) < -e.z || Min3(v0.z,v1.z,v2.z) > e.z) return 0;

	//compute edge vectors for triangle
	D3DXVECTOR3 f0 = v1 - v0, f1 = v2 - v1, f2 = v0 - v2;
	//Test axes a00...a22
	D3DXVECTOR3 a[9] = {D3DXVECTOR3(0,-f0.z,f0.y),D3DXVECTOR3(0,-f1.z,f1.y),D3DXVECTOR3(0,-f2.z,f2.y),
		D3DXVECTOR3(f0.z,0,-f0.x),D3DXVECTOR3(f1.z,0,-f1.x),D3DXVECTOR3(f2.z,0,-f2.x),
		D3DXVECTOR3(-f0.y,f0.x,0),D3DXVECTOR3(-f1.y,f1.x,0),D3DXVECTOR3(-f2.y,f2.x,0)
	};
	for(int i = 0; i < 9; ++i){
		float p0 = Dot(v0,a[i]);
		float p1 = Dot(v1,a[i]);
		float p2 = Dot(v2,a[i]);
		if(max(-Max3(p0,p1,p2),Min3(p0,p1,p2)) > r) return 0;
	}

	//Test separating axis correspoinding to triangle face normal
	d3d::Plane p;
	p.n = Cross(f0,f1);
	p.d = Dot(p.n,v0);
	return TestAABBPlane(b,p);
}
BOOL IntersectSegmentPlane(const D3DXVECTOR3 &a, const D3DXVECTOR3 &b, const d3d::Plane &p,D3DXVECTOR3 &q){
	D3DXVECTOR3 ab = b - a;
	float t = (p.d - Dot(p.n,a)) / Dot(p.n,ab);
	if(t >= 0.0f && t <= 1.0f){
		q = a + t * ab;
		return true;
	}
	return false;
}
BOOL PointInTriangle(const D3DXVECTOR3 &p, const D3DXVECTOR3 t[3]){
	
}
BOOL TestSegmentIntersect(const D3DXVECTOR3 &p, const D3DXVECTOR3 &q, const D3DXVECTOR3 &u, const D3DXVECTOR3 &v){
	
}
BOOL TestCoPlaneTriangle(const D3DXVECTOR3 t1[3],const D3DXVECTOR3 t2[3],const D3DXVECTOR3 &n){ //参数表示两个三角形的顶点及其法向量
	//投影至最大面积的轴平面
	int e[3][2] = {{1,0},{2,0},{2,1}};//边索引
	if(n.x >= n.y && n.x >= n.z){
		//测试边边是否相交
		for(int i = 0; i < 3; ++i){
			for(int j = 0; j < 3; ++j){
				if(TestSegmentIntersect(t1[i][0],t1[i][1],t2[j][0],t2[j][1])) return true;
			}
		}
		return PointInTriangle(t1[0],t2) || PointInTriangle(t2[0],t1);
		//测试点是否在三角形内
	}else if(n.y >= n.x && n.y >= n.z){
		
	}else{
		
	}
}
BOOL TestTriangleTriangle(const D3DXVECTOR3 t1[3],const D3DXVECTOR3 t2[3]){
	D3DXVECTOR3 n1 = Cross(t1[1]-t1[0],t1[2]-t1[0]);
	D3DXVECTOR3 n2 = Cross(t2[1]-t2[0],t2[2]-t2[0]);
	float d1 = Dot(n1,t1[0]), d2 = Dot(n2,t2[0]);
	d3d::Plane p1(n1,d1),p2(n2,d2);
	D3DXVECTOR3 u,v,m,n;
	BOOL bu,bv,bm,bn;
	bu = IntersectSegmentPlane(t1[1],t1[0],p2,u);
	bv = IntersectSegmentPlane(t1[2],t1[0],p2,v);

	if(!bu) bu = IntersectSegmentPlane(t1[2],t1[1],p2,u);
	else if(!bv) bv = IntersectSegmentPlane(t1[2],t1[1],p2,v);
	if(!bu || !bv){//共面或平行
		//共面三角形相交测试
		if(FLTEQ(Dot(t1[0],n1),Dot(t2[0],n2))){
			return TestCoPlaneTriangle(t1,t2,n1);
		}
		return false;
	}

	bm = IntersectSegmentPlane(t2[1],t2[0],p2,m);
	bn = IntersectSegmentPlane(t2[2],t2[0],p2,n);

	if(!bm) bm = IntersectSegmentPlane(t2[2],t2[1],p2,m);
	else if(!bn) bn = IntersectSegmentPlane(t2[2],t2[1],p2,n);
	if(!bm || !bn){
		//因为上面已经测试了共面
		return false;
	}

	D3DXVECTOR3 uv = v - u;
	float fu = Dot(uv,u), fv = Dot(uv,v), fm = Dot(uv,m), fn = Dot(uv,n);
	return fu <= fn && fm <= fv;
}
#endif