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
#include <vector>

//
// Globals
//

IDirect3DDevice9* Device = 0; 

const int Width  = 640;
const int Height = 480;

ID3DXMesh*                      Mesh = 0;
std::vector<D3DMATERIAL9>       Mtrls(0);
std::vector<IDirect3DTexture9*> Textures(0);

d3d::BoundingBox    boundingBox;
ID3DXMesh* BoxMesh;

//x-aab
float uL;
bool F[512][512][512];

struct Vertex{
	float _x,_y,_z;
	Vertex(float x,float y,float z):_x(x),_y(y),_z(z){}
};
const int HCNT = 100000000;
struct Node{
	int key;
	Node* next;
}nodes[HCNT];
Node* Hash[HCNT];
int pn;
void initHash(){
	pn = 0;
	memset(Hash,0,sizeof(Hash));
}
void insert(int x,int y,int z,int id){
	int h =x*1024*1024+y*1024+z;
	int p = h % HCNT;
	while(Hash[p] != NULL && Hash[p]->key != h){
		++p;
		if(p == HCNT) p = 0;
	}
	if(Hash[p] == NULL){
		nodes[pn].key = h;
		Hash[p] = nodes+pn++;
		nodes[pn].key = id;
		nodes[pn].next = NULL;
		Hash[p]->next = nodes + pn++;
	}else{
		for(Node* i = Hash[p]->next; i != NULL; i = i->next){
			if(i->key == id) return;
		}
		nodes[pn].key = id;
		nodes[pn].next = Hash[p]->next;
		Hash[p]->next = nodes+pn++;
	}
}
// Prototypes
//
bool ComputeBoundingBox(ID3DXMesh* mesh, d3d::BoundingBox*    box);
void PreProcess();
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
	// Compute Bounding Sphere and Bounding Box.
	//
	ComputeBoundingBox(Mesh, &boundingBox);
	PreProcess();
	D3DXCreateBox(
		Device,
		uL,
		uL,
		uL,
		&BoxMesh,
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
	// Set camera.
	//

	D3DXVECTOR3 pos(4.0f, 12.0f, -20.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

	D3DXMATRIX V;
	D3DXMatrixLookAtLH(
		&V,
		&pos,
		&target,
		&up);

	Device->SetTransform(D3DTS_VIEW, &V);

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

}

bool Display(float timeDelta)
{
	if( Device )
	{
		//
		// Update: Rotate the mesh.
		//

		/*static float y = 0.0f;
		D3DXMATRIX yRot;
		D3DXMatrixRotationY(&yRot, y);
		y += timeDelta;

		if( y >= 6.28f )
		y = 0.0f;

		D3DXMATRIX World = yRot;

		Device->SetTransform(D3DTS_WORLD, &World);*/

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
		D3DMATERIAL9 blue = d3d::RED_MTRL;
		blue.Diffuse.a = 0.1f; // 10% opacity

		Device->SetMaterial(&blue);
		Device->SetTexture(0, 0); // disable texture

		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		D3DXMATRIX bx;
		float centx = (boundingBox._max.x + boundingBox._min.x)/2.0;
		float centy = (boundingBox._max.y + boundingBox._min.y)/2.0;
		float centz = (boundingBox._max.z + boundingBox._min.z)/2.0;
		for(int i = 0; i < 256; ++i){
			for(int j = 0; j  < 256; ++j){
				for(int k = 0; k < 256; ++k){
					if(F[i][j][k]){
						D3DXMatrixTranslation(&bx,boundingBox._min.x + i*uL - centx,boundingBox._min.y + j*uL - centy,boundingBox._min.z + k*uL - centz);	
						Device->SetTransform(D3DTS_WORLD,&bx);
						BoxMesh->DrawSubset(0);	
					}
				}
			}
		}
		D3DXMatrixTranslation(&bx,0,0,0);	
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
float Vproduct(Vertex *a,Vertex *b){
	return a->_x * b->_x + a->_y * b->_y + a->_z * b->_z;
}
bool TestSegPlane(Vertex *p0,Vertex* d,Vertex *n, int dd){
	float t = (dd-Vproduct(p0,n))/Vproduct(d,n);
	return t >= 0 && t <= 1;
}

bool SegCube(Vertex *p0,Vertex *p1,int i,int j,int k){
		Vertex d(p1->_x - p0->_x,p1->_y-p0->_y,p1->_z-p0->_z);
		float centx = (boundingBox._max.x + boundingBox._min.x)/2.0;
		float centy = (boundingBox._max.y + boundingBox._min.y)/2.0;
		float centz = (boundingBox._max.z + boundingBox._min.z)/2.0;
		Vertex p(boundingBox._min.x + i*uL - centx,boundingBox._min.y + j*uL - centy,boundingBox._min.z + k*uL - centz);	
		Vertex pp(p._x + uL, p._y + uL,p._z + uL);
		Vertex n[6] = {
			Vertex(-1,0,0),Vertex(0,-1,0),Vertex(0,0,-1),
			Vertex(1,0,0),Vertex(0,1,0),Vertex(0,0,1)
		};
		for(int i = 0; i < 3; ++i){
			if(TestSegPlane(p0,&d,&n[i],Vproduct(&n[i],&p))) return true;
		}
		for(int i = 3; i < 6; ++i){
			if(TestSegPlane(p0,&d,&n[i],Vproduct(&n[i],&pp))) return true;
		}
}
bool TriangleCube(Vertex *p0,Vertex *p1, Vertex *p2,int i,int j,int k){
	return SegCube(p0,p1,i,j,k) || SegCube(p0,p2,i,j,k) || SegCube(p1,p2,i,j,k);
}
//得到单位包围盒的长度
float GetBoxLength(d3d::BoundingBox* bb){
	float minL = bb->_max.x-bb->_min.x;
	minL = min(minL,bb->_max.y-bb->_min.y);
	minL = min(minL,bb->_max.z-bb->_min.z);
	return minL /30.0;
}
void GetYiShe(ID3DXMesh* mesh,d3d::BoundingBox* bb){
	WORD* id = 0;
	Vertex* v = 0;
	mesh->LockIndexBuffer(0, (void**)&id);
	mesh->LockVertexBuffer(0, (void**)&v);
	for(int i = 0; i < mesh->GetNumFaces(); ++i){
		for(int j=3*i;j<3*i+3; ++j){
			int ix = (v[id[j]]._x - bb->_min.x)/uL;
			int iy = (v[id[j]]._y - bb->_min.y)/uL;
			int iz = (v[id[j]]._z - bb->_min.z)/uL;
			if(ix < 0) ix = 0; 
			if(iy < 0) iy = 0;
			if(iz < 0) iz = 0;
			if(F[ix][iy][iz] == false){
				F[ix][iy][iz] = true;
				insert(ix,iy,iz,i);
			}

		}
	}
	mesh->UnlockVertexBuffer();
	mesh->UnlockIndexBuffer();
}
void PreProcess(){
	memset(F,0,sizeof(F));
	initHash();
	uL = GetBoxLength(&boundingBox);
	GetYiShe(Mesh,&boundingBox);
}