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
#include"CD.h"
//
// Globals
//
std::ofstream OutFile;

IDirect3DDevice9* Device = 0; 

const int Width  = 640;
const int Height = 480;

ID3DXMesh *Mesh1 = 0, *Mesh2;
std::vector<D3DMATERIAL9> Mtrls1(0),Mtrls2(0);
std::vector<IDirect3DTexture9*> Textures1(0),Textures2(0);

d3d::BoundingBox    boundingBox;
ID3DXMesh* BoxMesh;

bool LoadMesh(LPCWSTR filename, ID3DXMesh *&mesh,std::vector<D3DMATERIAL9> &Mtrl,std::vector<IDirect3DTexture9*> &Texture);
bool ComputeBoundingBox(ID3DXMesh* mesh, d3d::BoundingBox*    box);
//
// Framework functions
//
bool Setup()
{

	if(!LoadMesh(TEXT("bigship1.x"),Mesh1,Mtrls1,Textures1)) return false;
	if(!LoadMesh(TEXT("chair.x"),Mesh2,Mtrls2,Textures2)) return false;
	// Compute Bounding Cube and Bounding Box.
	//
	ComputeBoundingBox(Mesh1, &boundingBox);
	D3DXCreateBox(
		Device,
		boundingBox._max.x - boundingBox._min.x,
		boundingBox._max.y - boundingBox._min.y,
		boundingBox._max.z - boundingBox._min.z,
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
	d3d::Release<ID3DXMesh*>(Mesh1);
	d3d::Release<ID3DXMesh*>(Mesh2);

	for(int i = 0; i < Textures1.size(); i++){
		d3d::Release<IDirect3DTexture9*>( Textures1[i] );	
	}
	for(int i = 0; i < Textures2.size(); ++i){
		d3d::Release<IDirect3DTexture9*>( Textures2[i] );
	}

	d3d::Release<ID3DXMesh*>(BoxMesh);

}

bool Display(float timeDelta)
{
	if( Device )
	{
		//
		// Set camera.
		//
		static float angle = 3.14f;
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
		angle += timeDelta;
		if( angle >= 6.28f )
			angle = 0.0f;

		// Render
		//
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
		//Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		Device->BeginScene();

		// draw the mesh	
		for(int i = 0; i < Mtrls1.size(); i++)
		{
			Device->SetMaterial( &Mtrls1[i] );
			Device->SetTexture(0, Textures1[i]);
			Mesh1->DrawSubset(i);

		}
		D3DXMATRIX bx;
		D3DXMatrixTranslation(&bx,0,5,0);
		Device->SetTransform(D3DTS_WORLD,&bx);
		for(int i = 0; i < Mtrls2.size(); ++i){
			Device->SetMaterial( &Mtrls2[i]);
			Device->SetTexture(0,Textures2[i]);
			Mesh2->DrawSubset(i);
		}
		D3DXMatrixTranslation(&bx,0,0,0);
		Device->SetTransform(D3DTS_WORLD,&bx);
		//
		// Draw bounding volume in blue and at 10% opacity
		D3DMATERIAL9 blue = d3d::BLUE_MTRL;
		blue.Diffuse.a = 0.3f; // 30% opacity

		Device->SetMaterial(&blue);
		Device->SetTexture(0, 0); // disable texture

		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		//BoxMesh->DrawSubset(0);	

		//D3DXMATRIX bx;
		//for(int i = 0; i <= mxx; ++i){
		//	for(int j = 0; j  <= mxy; ++j){
		//		for(int k = 0; k <= mxz; ++k){
		//			if(F[i][j][k]){
		//				D3DXMatrixTranslation(&bx,i*ux,j*uy,k*uz);	
		//				Device->SetTransform(D3DTS_WORLD,&bx);
		//				CubeMesh->DrawSubset(0);
		//			}
		//		}
		//	}
		//}
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
		else if( wParam == VK_RETURN){
			//Test CD
			if(CollisionDetection(Mesh1,Mesh2,NULL)){
				MessageBox(0, TEXT("Collison Detected!!"), 0, 0);
			}else{
				MessageBox(0, TEXT("Non-Collison!!"), 0, 0);
			}
			
		}
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

bool LoadMesh(LPCWSTR filename, ID3DXMesh *&mesh,std::vector<D3DMATERIAL9> &Mtrl,std::vector<IDirect3DTexture9*> &Texture){
	HRESULT hr = 0;
	//
	// Load the XFile data.
	//
	ID3DXBuffer* adjBuffer  = 0;
	ID3DXBuffer* mtrlBuffer = 0;
	DWORD        numMtrls   = 0;

	hr = D3DXLoadMeshFromX(  
		filename,
		D3DXMESH_MANAGED,
		Device,
		&adjBuffer,
		&mtrlBuffer,
		0,
		&numMtrls,
		&mesh);

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
			Mtrl.push_back( mtrls[i].MatD3D );

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
				Texture.push_back( tex );
			}
			else
			{
				// no texture for the ith subset
				Texture.push_back( 0 );
			}
		}
	}
	d3d::Release<ID3DXBuffer*>(mtrlBuffer); // done w/ buffer

	//
	// Optimize the mesh.
	//

	hr = mesh->OptimizeInplace(		
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
	return true;
}
