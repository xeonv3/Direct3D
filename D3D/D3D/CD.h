#ifndef __D3DCOLLISIONDETECTIONH__
#define __D3DCOLLISIONDETECTIONH__
#include "CG.h"
#include "d3dUtility.h"
#include <vector>
const float ENDEP = 1.0f;
struct AABB{
	d3d::BoundingBox bb;
	std::vector<int> faces;
	AABB *left, *right;
	AABB(){}
	AABB(const d3d::BoundingBox& b):bb(b){}
};
BOOL TestBoxIntersect(const d3d::BoundingBox &b1,const d3d::BoundingBox &b2);
void ConstructAABB(AABB *r,ID3DXMesh* Mesh);

bool TestCollision(AABB *o,AABB *e,ID3DXMesh* Mesh1,ID3DXMesh* Mesh2);
void GetCollisionFaces(AABB *o,AABB *e,std::vector<std::pair<int,int> > *tripair, ID3DXMesh *Mesh1,ID3DXMesh *Mesh2);
//===================================================================================
//===========下面这两个函数调用很重要，碰撞检测只要使用这个函数即可==================
//===================================================================================
void TravelAABBTree(AABB *r,void (*fun)(AABB *b));
bool CollisionDetection(ID3DXMesh *Mesh1,ID3DXMesh *Mesh2,std::vector<std::pair<int,int> > *tripair);
////x-aab
//bool F[128][128][128];
////bool DP[128][128][128][8][8][8];
//int mxx,mxy,mxz;
//float ux,uy,uz;
//enum SpritAxis{X,Y,Z};
//const float JD = 1.0f;


////void dp(){
////	for(int i = 0; i <= mxx; ++i){
////		for(int j = 0; j <= mxy; ++j){
////			for(int k = 0; k <= mxz; ++k){
////				dp[i][j][k][0][0][0] = F[i][j][k];
////			}
////		}
////	}
////	for(int n = 0; (1<<n) <= mxx; ++n){
////		for(int m = 0; (1<<m) <= mxy; ++m){
////			for(int q = 0; (1<<q) <= mxz; ++q){
////				if(n == 0 && m == 0 && q == 0) continue;
////				for(int i = 0; i+(1<<n)<= mxx; ++i){
////					for(int j = 0; j+(1<<m)<=mxy;++j){
////						for(int k = 0; k+(1<<q)<=mxz;++k){
////							if(n == 0 && m == 0){
////								dp[i][j][k][n][m][q] = dp[i][j][k][n][m][q-1] || d[i-(1<<(q-1))][j][k][n][m][q-1];
////							}else if(n == 0){
////								dp[i][j][k][n][m][q] = dp[i][j][k][n][m-1][q] || dp[i][j-(1<<(m-1))][k][n][m-1][q];
////							}else{
////								dp[i][j][k][n][m][q] = dp[i][j][k][n-1][m][q] || dp[i-(1<<(q-1))][j][k][n-1][m][q];
////							}
////						}
////					}
////				}
////			}
////		}
////	}
////}
//
////bool query(int x1,int x2,int y1,int y2,int z1,int z2){
////	int kx = log(x2-x1)/log(2.0);
////	int ky = log(y2-y1)/log(2.0);
////	int kz = log(z2-z1)/log(2.0);
////	return dp[x1][y1][z1][kx][ky][kz] ||
////		dp[x2-(1<<kx)+1][y1][z1][kx][ky][kz] ||
////		dp[x1][y2-(1<<ky)+1][z1][kx][ky][kz] ||
////		dp[x1][y1][z2-(1<<kz)+1][kx][ky][kz] ||
////		dp[x2-(1<<kx)+1][y2-(1<<ky)+1][z1][kx][ky][kz] ||
////		dp[x2-(1<<kx)+1][y1][z2-(1<<kz)+1][kx][ky][kz] ||
////		dp[x1][y2-(1<<ky)+1][z2-(1<<kz)+1][kx][ky][kz] ||
////		dp[x2-(1<<kx)+1][y2-(1<<ky)+1][z2-(1<<kz)+1][kx][ky][kz];
////}
//
//void initCD(){
//	root = new BVT();
//	aabb = new BVT();
//	root->bb = boundingBox;
//	aabb->bb = boundingBox;
//	for(int i = 0; i < Mesh->GetNumFaces(); ++i){
//		root->faces.push_back(i);
//		aabb->faces.push_back(i);
//	}
//	root->left = aabb->left = NULL;
//	root->right = aabb->right = NULL;
//	OutFile.open("out.txt");
//	createBVT(root,0,0,0);
//	ux = (boundingBox._max.x - boundingBox._min.x) / mxx;
//	uy = (boundingBox._max.y - boundingBox._min.y) / mxy;
//	uz = (boundingBox._max.z - boundingBox._min.z) / mxz;
//	OutFile << mxx << " " << mxy << " " << mxz << std::endl;
//	OutFile.close();
//	//dp();
//}
//
////void CD(BVT *t1,BVT *t2){
////
////}
#endif