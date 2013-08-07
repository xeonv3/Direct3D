#include "CD.h"

BOOL TestBoxIntersect(const d3d::BoundingBox &a,const d3d::BoundingBox &b){
	if(a._max.x < b._min.x || a._min.x > b._max.x) return false;
	if(a._max.y < b._min.y || a._min.y > b._max.y) return false;
	if(a._max.z < b._min.z || a._min.z > b._max.z) return false;
	return true;
}
void ConstructAABB(AABB *r,ID3DXMesh* Mesh){
	if(r == NULL) return;
	D3DXVECTOR3 dl = r->bb._max - r->bb._min;
	if(dl.x < ENDEP && dl.y < ENDEP && dl.z < ENDEP){
		r->left = NULL;
		r->right = NULL;
		return;
	}
	r->left = new AABB(r->bb);
	r->right = new AABB(r->bb);
	if(dl.x >= dl.y && dl.x >= dl.z){		
		r->right->bb._min.x = r->left->bb._max.x = (r->bb._max.x + r->bb._min.x)* 0.5f;
	}else if(dl.y >= dl.x && dl.y >= dl.z){
		r->right->bb._min.y = r->left->bb._max.y = (r->bb._max.y + r->bb._min.y) * 0.5f;
	}else{
		r->right->bb._min.z = r->left->bb._max.z = (r->bb._max.z + r->bb._min.z) * 0.5f;
	}
	WORD* id = 0;
	D3DXVECTOR3* v = 0;
	Mesh->LockIndexBuffer(0, (void**)&id);
	Mesh->LockVertexBuffer(0, (void**)&v);
	d3d::BoundingBox lb,rb;
	for(std::vector<int>::iterator it = r->faces.begin(); it != r->faces.end(); ++it){
		int i = *it;
		D3DXVECTOR3 A=v[id[3*i]], B = v[id[3*i+1]], C = v[id[3*i+2]];
		if(TestTriangleAABB(A,B,C,r->left->bb)){
			r->left->faces.push_back(i);
			if(lb._max.x < A.x) lb._max.x = A.x;
			if(lb._min.x > A.x) lb._min.x = A.x;
			if(lb._max.y < A.y) lb._max.y = A.y;
			if(lb._min.y > A.y) lb._min.y = A.y;
			if(lb._max.z < A.z) lb._max.z = A.z;
			if(lb._min.z > A.z) lb._min.z = A.z;

			if(lb._max.x < B.x) lb._max.x = B.x;
			if(lb._min.x > B.x) lb._min.x = B.x;
			if(lb._max.y < B.y) lb._max.y = B.y;
			if(lb._min.y > B.y) lb._min.y = B.y;
			if(lb._max.z < B.z) lb._max.z = B.z;
			if(lb._min.z > B.z) lb._min.z = B.z;

			if(lb._max.x < C.x) lb._max.x = C.x;
			if(lb._min.x > C.x) lb._min.x = C.x;
			if(lb._max.y < C.y) lb._max.y = C.y;
			if(lb._min.y > C.y) lb._min.y = C.y;
			if(lb._max.z < C.z) lb._max.z = C.z;
			if(lb._min.z > C.z) lb._min.z = C.z;
		}
		if(TestTriangleAABB(A,B,C,r->right->bb)){
			if(rb._max.x < A.x) rb._max.x = A.x;
			if(rb._min.x > A.x) rb._min.x = A.x;
			if(rb._max.y < A.y) rb._max.y = A.y;
			if(rb._min.y > A.y) rb._min.y = A.y;
			if(rb._max.z < A.z) rb._max.z = A.z;
			if(rb._min.z > A.z) rb._min.z = A.z;

			if(rb._max.x < B.x) rb._max.x = B.x;
			if(rb._min.x > B.x) rb._min.x = B.x;
			if(rb._max.y < B.y) rb._max.y = B.y;
			if(rb._min.y > B.y) rb._min.y = B.y;
			if(rb._max.z < B.z) rb._max.z = B.z;
			if(rb._min.z > B.z) rb._min.z = B.z;

			if(rb._max.x < C.x) rb._max.x = C.x;
			if(rb._min.x > C.x) rb._min.x = C.x;
			if(rb._max.y < C.y) rb._max.y = C.y;
			if(rb._min.y > C.y) rb._min.y = C.y;
			if(rb._max.z < C.z) rb._max.z = C.z;
			if(rb._min.z > C.z) rb._min.z = C.z;
			r->right->faces.push_back(i);
		}
	}
	if(r->left->bb._min.x < lb._min.x) r->left->bb._min.x = lb._min.x;
	if(r->left->bb._max.x > lb._max.x) r->left->bb._max.x = lb._max.x;
	if(r->left->bb._min.y < lb._min.y) r->left->bb._min.y = lb._min.y;
	if(r->left->bb._max.y > lb._max.y) r->left->bb._max.y = lb._max.y;
	if(r->left->bb._min.z < lb._min.z) r->left->bb._min.z = lb._min.z;
	if(r->left->bb._max.z > lb._max.z) r->left->bb._max.z = lb._max.z;

	if(r->right->bb._min.x < lb._min.x) r->right->bb._min.x = lb._min.x;
	if(r->right->bb._max.x > lb._max.x) r->right->bb._max.x = lb._max.x;
	if(r->right->bb._min.y < lb._min.y) r->right->bb._min.y = lb._min.y;
	if(r->right->bb._max.y > lb._max.y) r->right->bb._max.y = lb._max.y;
	if(r->right->bb._min.z < lb._min.z) r->right->bb._min.z = lb._min.z;
	if(r->right->bb._max.z > lb._max.z) r->right->bb._max.z = lb._max.z;
	r->faces.clear();//只保留叶子结点中物体三角面片，这样可以让空间少一些
	Mesh->UnlockVertexBuffer();
	Mesh->UnlockIndexBuffer();
	if(r->left->faces.empty()){delete r->left; r->left = NULL;}
	if(r->right->faces.empty()){delete r->left; r->right = NULL;}
	ConstructAABB(r->left,Mesh);
	ConstructAABB(r->right,Mesh);
}
//对AABB树的结点遍历
//传入回调函数
void TravelAABBTree(AABB *r,void (*fun)(AABB*b)){
	if(r == NULL) return;
	fun(r);
	TravelAABBTree(r->left,fun);
	TravelAABBTree(r->right,fun);
}
//测试是否发生碰撞
bool TestCollision(AABB *o,AABB *e,ID3DXMesh* Mesh1,ID3DXMesh* Mesh2){
	if(o == NULL || e == NULL) return false;
	if(!TestBoxIntersect(o->bb,e->bb)) return false;
	if(o->left == NULL && o->right == NULL){
		if(e->left == NULL && e->right == NULL){
			WORD* id1 = 0; WORD* id2 = 0;
			D3DXVECTOR3* v1 = 0; D3DXVECTOR3* v2 = 0;
			Mesh1->LockIndexBuffer(0, (void**)&id1); Mesh2->LockIndexBuffer(0, (void**)&id2);
			Mesh1->LockVertexBuffer(0, (void**)&v1); Mesh2->LockVertexBuffer(0, (void**)&v2);
			for(std::vector<int>::iterator it = o->faces.begin(); it != o->faces.end(); ++it){
				for(std::vector<int>::iterator jt = e->faces.begin(); jt != e->faces.end(); ++jt){
					int i = *it,j = *jt;
					D3DXVECTOR3 t1[3] = {v1[id1[3*i]],v1[id1[3*i+1]],v1[id1[3*i+2]]};
					D3DXVECTOR3 t2[3] = {v2[id2[3*j]],v2[id2[3*j+1]],v2[id2[3*j+2]]};
					if(TestTriangleTriangle(t1,t2)){
						return true;
					}
				}
			}
			Mesh1->UnlockVertexBuffer();
			Mesh2->UnlockIndexBuffer();

		}else{
			if(TestCollision(o,e->left,Mesh1,Mesh2)) return true;
			if(TestCollision(o,e->right,Mesh1,Mesh2)) return true;
		}
	}else{
		if(TestCollision(o->left,e,Mesh1,Mesh2)) return true;
		if(TestCollision(o->right,e,Mesh1,Mesh2)) return true;
	}
	return false;
}

//返回发生碰撞的面片对
void GetCollisionFaces(AABB *o,AABB *e,std::vector<std::pair<int,int> > *tripair,ID3DXMesh *Mesh1,ID3DXMesh *Mesh2){
	if(o == NULL || e == NULL) return;
	if(!TestBoxIntersect(o->bb,e->bb)) return;
	if(o->left == NULL && o->right == NULL){
		if(e->left == NULL && e->right == NULL){
			WORD* id1 = 0; WORD* id2 = 0;
			D3DXVECTOR3* v1 = 0; D3DXVECTOR3* v2 = 0;
			Mesh1->LockIndexBuffer(0, (void**)&id1); Mesh2->LockIndexBuffer(0, (void**)&id2);
			Mesh1->LockVertexBuffer(0, (void**)&v1); Mesh2->LockVertexBuffer(0, (void**)&v2);
			for(std::vector<int>::iterator it = o->faces.begin(); it != o->faces.end(); ++it){
				for(std::vector<int>::iterator jt = e->faces.begin(); jt != e->faces.end(); ++jt){
					int i = *it,j = *jt;
					D3DXVECTOR3 t1[3] = {v1[id1[3*i]],v1[id1[3*i+1]],v1[id1[3*i+2]]};
					D3DXVECTOR3 t2[3] = {v2[id2[3*j]],v2[id2[3*j+1]],v2[id2[3*j+2]]};
					if(TestTriangleTriangle(t1,t2)){
						tripair->push_back(std::make_pair(i,j));
					}
				}
			}
			Mesh1->UnlockVertexBuffer();
			Mesh2->UnlockIndexBuffer();

		}else{
			TestCollision(o,e->left,Mesh1,Mesh2);
			TestCollision(o,e->right,Mesh1,Mesh2);
		}
	}else{
		TestCollision(o->left,e,Mesh1,Mesh2);
		TestCollision(o->right,e,Mesh1,Mesh2);
	}
}
/*
Mesh1 Mesh2分别代表需要碰撞检测的面片
tripair 表示需要返回发生碰撞的三角面对,若不需要返回，可以填NULL
bool 代表是否发生碰撞
*/
bool CollisionDetection(ID3DXMesh *Mesh1,ID3DXMesh *Mesh2,std::vector<std::pair<int,int> > *tripair){
	AABB o,e;

	HRESULT hr = 0;
	BYTE* v = 0;

	Mesh1->LockVertexBuffer(0, (void**)&v);
	hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		Mesh1->GetNumVertices(),
		D3DXGetFVFVertexSize(Mesh1->GetFVF()),
		&o.bb._min,
		&o.bb._max);

	Mesh2->UnlockVertexBuffer();
	if( FAILED(hr) )return false;
	for(int i = 0; i < Mesh1->GetNumFaces(); ++i){
		o.faces.push_back(i);
	}

	Mesh2->LockVertexBuffer(0, (void**)&v);
	hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		Mesh2->GetNumVertices(),
		D3DXGetFVFVertexSize(Mesh2->GetFVF()),
		&e.bb._min,
		&e.bb._max);
	Mesh2->UnlockVertexBuffer();
	if( FAILED(hr) )return false;
	for(int i = 0; i < Mesh2->GetNumFaces(); ++i){
		e.faces.push_back(i);
	}
	ConstructAABB(&o,Mesh1);
	ConstructAABB(&e,Mesh2);
	if(tripair == NULL){
		return TestCollision(&o,&e,Mesh1,Mesh2);
	}else{
		GetCollisionFaces(&o,&e,tripair,Mesh1,Mesh2);
		return !tripair->empty();
	}
}