//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// File: boundingvolumes.cpp
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Demonstrates how to use D3DXComputeBoundingSphere and D3DXComputeBoundingBox.
//
//      -The spacebar key switches between rendering the mesh's bounding sphere and box.
//          
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <fstream>
#include<cmath>
#include <vector>
#include<set>
#include<algorithm>
//
// Globals
//
std::ofstream OutFile;

IDirect3DDevice9* Device = 0; 

const int Width  = 640;
const int Height = 480;

ID3DXMesh*                      Mesh = 0;
std::vector<D3DMATERIAL9>       Mtrls(0);
std::vector<IDirect3DTexture9*> Textures(0);

d3d::BoundingBox    boundingBox;
ID3DXMesh* BoxMesh;
ID3DXMesh* CubeMesh;
struct BVT{
	d3d::BoundingBox bb;
	std::vector<int> faces;
	BVT *left, *right;
	BVT(){}
	BVT(const d3d::BoundingBox& b):bb(b){}
}*root,*aabb;//root树使用我的新算法构造的BVT,aabb使用旧有算法构造的BVT
//x-aab
bool F[128][128][128];
//bool DP[128][128][128][8][8][8];
int mxx,mxy,mxz;
float ux,uy,uz;
enum SpritAxis{X,Y,Z};
const float JD = 1.0f;
struct Vertex{
	float _x,_y,_z;
	Vertex(float x,float y,float z):_x(x),_y(y),_z(z){}
};
Vertex operator-(const Vertex &a, const Vertex &b){
	return Vertex(a._x-b._x,a._y-b._y,a._z-b._z);
}

bool ComputeBoundingBox(ID3DXMesh* mesh, d3d::BoundingBox*    box);
void initCD();
//
// Framework functions
//
bool Setup()
{
	HRESULT hr = 0;

	//
	// Load the XFile data.
	//
	ID3DXBuffer* adjBuffer  = 0;
	ID3DXBuffer* mtrlBuffer = 0;
	DWORD        numMtrls   = 0;

	hr = D3DXLoadMeshFromX(  
		TEXT("bigship1.x"),
		D3DXMESH_MANAGED,
		Device,
		&adjBuffer,
		&mtrlBuffer,
		0,
		&numMtrls,
		&Mesh);

	if(FAILED(hr))
	{
		::MessageBox(0, TEXT("D3DXLoadMeshFromX() - FAILED"), 0, 0);
		return false;
	}

	//
	// Extract the materials, load textures.
	//

	if( mtrlBuffer != 0 && numMtrls != 0 )
	{
		D3DXMATERIAL* mtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();

		for(int i = 0; i < numMtrls; i++)
		{
			// the MatD3D property doesn't have an ambient value set
			// when its loaded, so set it now:
			mtrls[i].MatD3D.Ambient = mtrls[i].MatD3D.Diffuse;

			// save the ith material
			Mtrls.push_back( mtrls[i].MatD3D );

			// check if the ith material has an associative texture
			if( mtrls[i].pTextureFilename != 0 )
			{
				// yes, load the texture for the ith subset
				IDirect3DTexture9* tex = 0;
				D3DXCreateTextureFromFile(
					Device,
					(LPCWSTR)mtrls[i].pTextureFilename,
					&tex);

				// save the loaded texture
				Textures.push_back( tex );
			}
			else
			{
				// no texture for the ith subset
				Textures.push_back( 0 );
			}
		}
	}
	d3d::Release<ID3DXBuffer*>(mtrlBuffer); // done w/ buffer

	//
	// Optimize the mesh.
	//

	hr = Mesh->OptimizeInplace(		
		D3DXMESHOPT_ATTRSORT |
		D3DXMESHOPT_COMPACT  |
		D3DXMESHOPT_VERTEXCACHE,
		(DWORD*)adjBuffer->GetBufferPointer(),
		0, 0, 0);

	d3d::Release<ID3DXBuffer*>(adjBuffer); // done w/ buffer

	if(FAILED(hr))
	{
		::MessageBox(0, TEXT("OptimizeInplace() - FAILED"), 0, 0);
		return false;
	}

	//
	// Compute Bounding Cube and Bounding Box.
	//
	ComputeBoundingBox(Mesh, &boundingBox);
	D3DXCreateBox(
		Device,
		boundingBox._max.x - boundingBox._min.x,
		boundingBox._max.y - boundingBox._min.y,
		boundingBox._max.z - boundingBox._min.z,
		&BoxMesh,
		0);

	initCD();

	D3DXCreateBox(
		Device,
		ux,
		uy,
		uz,
		&CubeMesh,
		0);
	//
	// Set texture filters.
	//
	Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	// 
	// Set Lights.
	//
	D3DXVECTOR3 dir(1.0f, -1.0f, 1.0f);
	D3DXCOLOR col(1.0f, 1.0f, 1.0f, 1.0f);
	D3DLIGHT9 light = d3d::InitDirectionalLight(&dir, &col);

	Device->SetLight(0, &light);
	Device->LightEnable(0, true);
	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);



	//
	// Set projection matrix.
	//

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI * 0.5f, // 90 - degree
		(float)Width / (float)Height,
		1.0f,
		1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	return true;
}

void Cleanup()
{
	d3d::Release<ID3DXMesh*>(Mesh);

	for(int i = 0; i < Textures.size(); i++)
		d3d::Release<IDirect3DTexture9*>( Textures[i] );


	d3d::Release<ID3DXMesh*>(BoxMesh);
	d3d::Release<ID3DXMesh*>(CubeMesh);

}

bool Display(float timeDelta)
{
	if( Device )
	{
		//
		// Set camera.
		//
		static float angle = 2.0f;
		D3DXVECTOR3 pos(cosf(angle)*-20.0f, 0.0f, sinf(angle)*-20.0f);
		D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

		D3DXMATRIX V;
		D3DXMatrixLookAtLH(
			&V,
			&pos,
			&target,
			&up);

		Device->SetTransform(D3DTS_VIEW, &V);
		//angle += timeDelta;
		//if( angle >= 6.28f )
			//angle = 0.0f;

		D3DXMATRIX mx;
		D3DXMatrixTranslation(&mx,-boundingBox._min.x,-boundingBox._min.y,-boundingBox._min.z);
		D3DXMATRIX World = mx;

		Device->SetTransform(D3DTS_WORLD, &mx);
		//
		// Render
		//
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
		Device->BeginScene();

		// draw the mesh
		for(int i = 0; i < Mtrls.size(); i++)
		{
			Device->SetMaterial( &Mtrls[i] );
			Device->SetTexture(0, Textures[i]);
			Mesh->DrawSubset(i);
		}
		//
		// Draw bounding volume in blue and at 10% opacity
		//D3DMATERIAL9 blue = d3d::BLUE_MTRL;
		//blue.Diffuse.a = 0.5f; // 30% opacity

		//Device->SetMaterial(&blue);
		//Device->SetTexture(0, 0); // disable texture

		//Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		//Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		//Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		//BoxMesh->DrawSubset(0);	

		D3DXMATRIX bx;
		for(int i = 0; i <= mxx; ++i){
			for(int j = 0; j  <= mxy; ++j){
				for(int k = 0; k <= mxz; ++k){
					if(F[i][j][k]){
						D3DXMatrixTranslation(&bx,i*ux,j*uy,k*uz);	
						Device->SetTransform(D3DTS_WORLD,&bx);
						CubeMesh->DrawSubset(0);
					}
				}
			}
		}
		D3DXMatrixTranslation(&bx,-boundingBox._min.x,-boundingBox._min.y,-boundingBox._min.z);	
		Device->SetTransform(D3DTS_WORLD,&bx);
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}

//
// WndProc
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, TEXT("InitD3D() - FAILED"), 0, 0);
		return 0;
	}

	if(!Setup())
	{
		::MessageBox(0, TEXT("Setup() - FAILED"), 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop( Display );

	Cleanup();

	Device->Release();

	return 0;
}


bool ComputeBoundingBox(ID3DXMesh* mesh, d3d::BoundingBox* box)
{
	HRESULT hr = 0;

	BYTE* v = 0;
	mesh->LockVertexBuffer(0, (void**)&v);

	hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		mesh->GetNumVertices(),
		D3DXGetFVFVertexSize(mesh->GetFVF()),
		&box->_min,
		&box->_max);

	mesh->UnlockVertexBuffer();

	if( FAILED(hr) )
		return false;

	return true;
}
int TestSegmentAABB(const D3DXVECTOR3 &p0, const D3DXVECTOR3 &p1, d3d::BoundingBox b){
	D3DXVECTOR3 c = (b._min + b._max) * 0.5f;
	D3DXVECTOR3 e = b._max - c;
	D3DXVECTOR3 m = (p0 + p1) * 0.5f;
	D3DXVECTOR3 d = p1 - m;
	m = m - c;

	float adx = abs(d.x);
	if(abs(m.x) > e.x + adx) return 0;
	float ady = abs(d.y);
	if(abs(m.y) > e.y + ady) return 0;
	float adz = abs(d.z);
	if(abs(m.z) > e.z + adz) return 0;

	adx += d3d::EPSILON; ady += d3d::EPSILON; adz += d3d::EPSILON;

	if(abs(m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady) return 0;
	if(abs(m.z * d.z - m.x * d.z) > e.x * adz + e.z * adx) return 0;
	if(abs(m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx) return 0;
	return 1;
}
void createBVT(BVT *r,int ix,int iy,int iz){
	float dx = r->bb._max.x - r->bb._min.x;
	float dy = r->bb._max.y - r->bb._min.y;
	float dz = r->bb._max.z - r->bb._min.z;
	float maxL = dx; maxL = max(maxL,max(dy,dz));
	float sL = maxL/2.0;
	if(sL < JD){
		//OutFile<< ix <<" "<< iy << " " << iz  << ":"<< r->faces.size() << std::endl;
		F[ix][iy][iz] = !(r->faces.empty());
		r->left = NULL;
		r->right = NULL;
		mxx = max(mxx,ix);
		mxy = max(mxy,iy);
		mxz = max(mxz,iz);
		return;
	}
	r->left = new BVT(r->bb);
	r->right = new BVT(r->bb);
	SpritAxis SA;
	if(d3d::FloatCmp(dx,maxL) == 0){
		SA = X;
		sL += r->bb._min.x;
		r->left->bb._max.x = sL;
		r->right->bb._min.x = sL;
		ix<<=1;
	}else if(d3d::FloatCmp(dy,maxL)==0){
		SA = Y;
		sL += r->bb._min.y;
		r->left->bb._max.y = sL;
		r->right->bb._min.y = sL;
		iy<<=1;
	}else{
		SA = Z;
		sL += r->bb._min.z;
		r->left->bb._max.z = sL;
		r->right->bb._min.z = sL;
		iz<<=1;
	}
	WORD* id = 0;
	D3DXVECTOR3* v = 0;
	Mesh->LockIndexBuffer(0, (void**)&id);
	Mesh->LockVertexBuffer(0, (void**)&v);
	for(std::vector<int>::iterator it = r->faces.begin(); it != r->faces.end(); ++it){
		int i = *it;
		bool f1 = true,f2 = true;
		for(int j = 3*i; j<3*i+3 && (f1 || f2); ++j){
			if(f1 && r->left->bb.isPointInside(v[id[j]])){
				r->left->faces.push_back(i);
				f1 = false;
			}
			if(f2 && r->right->bb.isPointInside(v[id[j]])){
				f2 = false;
				r->right->faces.push_back(i);
			}
		}
		D3DXVECTOR3 A = v[id[3*i]], B = v[id[3*i+1]], C = v[id[3*i+2]]; 
		
		if(f1 && (TestSegmentAABB(A,B,r->left->bb) || TestSegmentAABB(B,C,r->left->bb) || TestSegmentAABB(A,C,r->left->bb))){
			r->left->faces.push_back(i);
		}
		if(f2 && (TestSegmentAABB(A,B,r->right->bb) || TestSegmentAABB(B,C,r->right->bb) || TestSegmentAABB(A,C,r->right->bb))){
			r->right->faces.push_back(i);
		}
	}
	Mesh->UnlockVertexBuffer();
	Mesh->UnlockIndexBuffer();
	createBVT(r->left,ix,iy,iz);
	createBVT(r->right,ix+(SA == X),iy+(SA == Y),iz+(SA == Z));
}
//使用旧有算法构造AABB
void createBVT2(BVT *r){
	float dx = r->bb._max.x - r->bb._min.x;
	float dy = r->bb._max.y - r->bb._min.y;
	float dz = r->bb._max.z - r->bb._min.z;
	float maxL = dx; maxL = max(maxL,max(dy,dz));
	float sL = maxL/2.0;
	if(sL < JD){
		r->left = NULL;
		r->right = NULL;
		return;
	}
	r->left = new BVT(r->bb);
	r->right = new BVT(r->bb);
	WORD* id = 0;
	D3DXVECTOR3* v = 0;
	Mesh->LockIndexBuffer(0, (void**)&id);
	Mesh->LockVertexBuffer(0, (void**)&v);
	for(std::vector<int>::iterator it = r->faces.begin(); it != r->faces.end(); ++it){
		int i = *it;
		bool f1 = true,f2 = true;
		for(int j = 3*i; j<3*i+3 && (f1 || f2); ++j){
			if(f1 && r->left->bb.isPointInside(v[id[j]])){
				r->left->faces.push_back(i);
				if(r->left->bb._min.x < v[id[j]].x){
					r->left->bb._min.x = v[id[j]].x;
				}
				if(r->left->bb._max.x > v[id[j]].x){
					r->left->bb._max.x = v[id[j]].x;
				}
				if(r->left->bb._min.y < v[id[j]].y){
					r->left->bb._min.y = v[id[j]].y;
				}
				if(r->left->bb._max.y > v[id[j]].y){
					r->left->bb._max.y = v[id[j]].y;
				}
				if(r->left->bb._min.z < v[id[j]].z){
					r->left->bb._min.z = v[id[j]].z;
				}
				if(r->left->bb._max.z > v[id[j]].z){
					r->left->bb._max.z = v[id[j]].z;
				}
				f1 = false;
			}
			if(f2 && r->right->bb.isPointInside(v[id[j]])){
				f2 = false;
				if(r->right->bb._min.x < v[id[j]].x){
					r->right->bb._min.x = v[id[j]].x;
				}
				if(r->right->bb._max.x > v[id[j]].x){
					r->right->bb._max.x = v[id[j]].x;
				}
				if(r->right->bb._min.y < v[id[j]].y){
					r->right->bb._min.y = v[id[j]].y;
				}
				if(r->right->bb._max.y > v[id[j]].y){
					r->right->bb._max.y = v[id[j]].y;
				}
				if(r->right->bb._min.z < v[id[j]].z){
					r->right->bb._min.z = v[id[j]].z;
				}
				if(r->right->bb._max.z > v[id[j]].z){
					r->right->bb._max.z = v[id[j]].z;
				}
				r->right->faces.push_back(i);
			}
		}
		D3DXVECTOR3 A = v[id[3*i]], B = v[id[3*i+1]], C = v[id[3*i+2]]; 
		
		if(f1 && (TestSegmentAABB(A,B,r->left->bb) || TestSegmentAABB(B,C,r->left->bb) || TestSegmentAABB(A,C,r->left->bb))){
			r->left->faces.push_back(i);
		}
		if(f2 && (TestSegmentAABB(A,B,r->right->bb) || TestSegmentAABB(B,C,r->right->bb) || TestSegmentAABB(A,C,r->right->bb))){
			r->right->faces.push_back(i);
		}
	}
	Mesh->UnlockVertexBuffer();
	Mesh->UnlockIndexBuffer();
	createBVT2(r->left);
	createBVT2(r->right);
}
//void dp(){
//	for(int i = 0; i <= mxx; ++i){
//		for(int j = 0; j <= mxy; ++j){
//			for(int k = 0; k <= mxz; ++k){
//				dp[i][j][k][0][0][0] = F[i][j][k];
//			}
//		}
//	}
//	for(int n = 0; (1<<n) <= mxx; ++n){
//		for(int m = 0; (1<<m) <= mxy; ++m){
//			for(int q = 0; (1<<q) <= mxz; ++q){
//				if(n == 0 && m == 0 && q == 0) continue;
//				for(int i = 0; i+(1<<n)<= mxx; ++i){
//					for(int j = 0; j+(1<<m)<=mxy;++j){
//						for(int k = 0; k+(1<<q)<=mxz;++k){
//							if(n == 0 && m == 0){
//								dp[i][j][k][n][m][q] = dp[i][j][k][n][m][q-1] || d[i-(1<<(q-1))][j][k][n][m][q-1];
//							}else if(n == 0){
//								dp[i][j][k][n][m][q] = dp[i][j][k][n][m-1][q] || dp[i][j-(1<<(m-1))][k][n][m-1][q];
//							}else{
//								dp[i][j][k][n][m][q] = dp[i][j][k][n-1][m][q] || dp[i-(1<<(q-1))][j][k][n-1][m][q];
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//}

//bool query(int x1,int x2,int y1,int y2,int z1,int z2){
//	int kx = log(x2-x1)/log(2.0);
//	int ky = log(y2-y1)/log(2.0);
//	int kz = log(z2-z1)/log(2.0);
//	return dp[x1][y1][z1][kx][ky][kz] ||
//		dp[x2-(1<<kx)+1][y1][z1][kx][ky][kz] ||
//		dp[x1][y2-(1<<ky)+1][z1][kx][ky][kz] ||
//		dp[x1][y1][z2-(1<<kz)+1][kx][ky][kz] ||
//		dp[x2-(1<<kx)+1][y2-(1<<ky)+1][z1][kx][ky][kz] ||
//		dp[x2-(1<<kx)+1][y1][z2-(1<<kz)+1][kx][ky][kz] ||
//		dp[x1][y2-(1<<ky)+1][z2-(1<<kz)+1][kx][ky][kz] ||
//		dp[x2-(1<<kx)+1][y2-(1<<ky)+1][z2-(1<<kz)+1][kx][ky][kz];
//}

void initCD(){
	root = new BVT();
	aabb = new BVT();
	root->bb = boundingBox;
	aabb->bb = boundingBox;
	for(int i = 0; i < Mesh->GetNumFaces(); ++i){
		root->faces.push_back(i);
		aabb->faces.push_back(i);
	}
	root->left = aabb->left = NULL;
	root->right = aabb->right = NULL;
	OutFile.open("out.txt");
	createBVT(root,0,0,0);
	ux = (boundingBox._max.x - boundingBox._min.x) / mxx;
	uy = (boundingBox._max.y - boundingBox._min.y) / mxy;
	uz = (boundingBox._max.z - boundingBox._min.z) / mxz;
	OutFile << mxx << " " << mxy << " " << mxz << std::endl;
	OutFile.close();
	//dp();
}

//void CD(BVT *t1,BVT *t2){
//
//}