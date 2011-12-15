// Copyright (C) 2010 Barry Duncan. All Rights Reserved.
// The original author of this code can be contacted at: bduncan22@hotmail.com

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// DEVELOPERS AND CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "d3_func.h"
#include "fmvCutscenes.h"
#include "chnkload.hpp" // c++ header which ignores class definitions/member functions if __cplusplus is not defined ?
#include "logString.h"
#include "configFile.h"
#include "Fonts.h"
#include <XInput.h> // XInput API
#include "AvP_UserProfile.h"
#include "console.h"

extern void RenderListInit();
extern void RenderListDeInit();
extern void ThisFramesRenderingHasBegun(void);
extern void ThisFramesRenderingHasFinished(void);
extern HWND hWndMain;
extern int WindowMode;
extern void ChangeWindowsSize(uint32_t width, uint32_t height);
extern bool IsDemoVersion();

extern AVPIndexedFont IntroFont_Light;

// size of vertex and index buffers
const uint32_t kMaxVertices = 4096;
const uint32_t kMaxIndices  = 9216;

static HRESULT LastError;
texID_t NO_TEXTURE = 0;
texID_t MISSING_TEXTURE = 1;

// keep track of set render states
static bool	D3DAlphaBlendEnable;
D3DBLEND D3DSrcBlend;
D3DBLEND D3DDestBlend;
RENDERSTATES CurrentRenderStates;
bool D3DAlphaTestEnable = FALSE;
static bool D3DStencilEnable;
D3DCMPFUNC D3DStencilFunc;
static D3DCMPFUNC D3DZFunc;

int	VideoModeColourDepth;
int	NumAvailableVideoModes;
D3DINFO d3d;
bool usingStencil = false;

std::string shaderPath;
std::string videoModeDescription;

bool CreateVolatileResources();
bool ReleaseVolatileResources();
bool SetRenderStateDefaults();
void ToggleWireframe();

const int kMaxTextureStages = 8;
std::vector<texID_t> setTextureArray;

// Alien FOV - 115
// Marine & Predator FOV - 77
const int kDefaultFOV = 77;

// byte order macros for A8R8G8B8 d3d texture
enum
{
	BO_BLUE,
	BO_GREEN,
	BO_RED,
	BO_ALPHA
};

// list of allowed display formats
const D3DFORMAT DisplayFormats[] =
{
	D3DFMT_X8R8G8B8,
	D3DFMT_A8R8G8B8,
	D3DFMT_R8G8B8/*,
	D3DFMT_A1R5G5B5,
	D3DFMT_X1R5G5B5,
	D3DFMT_R5G6B5*/
};

// 16 bit formats
const D3DFORMAT DisplayFormats16[] =
{
	D3DFMT_R8G8B8/*,
	D3DFMT_A1R5G5B5,
	D3DFMT_X1R5G5B5,
	D3DFMT_R5G6B5*/
};

// 32 bit formats
const D3DFORMAT DisplayFormats32[] =
{
	D3DFMT_X8R8G8B8,
	D3DFMT_A8R8G8B8
};

// list of allowed depth buffer formats
const D3DFORMAT DepthFormats[] =
{
	D3DFMT_D32,
	D3DFMT_D24S8,
	D3DFMT_D24X8,
	D3DFMT_D24FS8,
	D3DFMT_D16
};

// TGA header structure
#pragma pack(1)
struct TGA_HEADER
{
	char		idlength;
	char		colourmaptype;
	char		datatypecode;
	int16_t		colourmaporigin;
	int16_t 	colourmaplength;
	char		colourmapdepth;
	int16_t		x_origin;
	int16_t		y_origin;
	int16_t		width;
	int16_t		height;
	char		bitsperpixel;
	char		imagedescriptor;
};
#pragma pack()

static TGA_HEADER TgaHeader = {0};

bool IsPowerOf2(int i)
{
	if ((i & -i) == i) {
		return true;
	}
	else return false;
}

int NearestSuperiorPow2(int i)
{
	int x = ((i - 1) & i);
	return x ? NearestSuperiorPow2(x) : i << 1;
}

uint32_t XPercentToScreen(float percent)
{
	return ((((float)ScreenDescriptorBlock.SDB_Width) / 100) * percent);
}

uint32_t YPercentToScreen(float percent)
{
	return ((((float)ScreenDescriptorBlock.SDB_Height) / 100) * percent);
}

uint32_t R_GetScreenWidth()
{
	return ScreenDescriptorBlock.SDB_Width;
}

bool ReleaseVolatileResources()
{
	Tex_ReleaseDynamicTextures();

	SAFE_DELETE(d3d.mainVB);
	SAFE_DELETE(d3d.mainIB);

	SAFE_DELETE(d3d.particleVB);
	SAFE_DELETE(d3d.particleIB);

	SAFE_DELETE(d3d.decalVB);
	SAFE_DELETE(d3d.decalIB);

	SAFE_DELETE(d3d.orthoVB);
	SAFE_DELETE(d3d.orthoIB);

	return true;
}

bool CheckPointIsInFrustum(D3DXVECTOR3 *point)
{
	// check if point is in front of each plane
	for (int i = 0; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&d3d.frustumPlanes[i], point) < 0.0f)
		{
			// its outside
			return false;
		}
	}

	// return here if the point is entirely within
	return true;
}

// call per frame after we've updated view and projection matrixes
void BuildFrustum()
{
	D3DXMATRIX viewProjection;
	D3DXMatrixMultiply(&viewProjection, &d3d.matView, &d3d.matProjection);

	// Left plane
	d3d.frustumPlanes[0].a = viewProjection._14 + viewProjection._11;
	d3d.frustumPlanes[0].b = viewProjection._24 + viewProjection._21;
	d3d.frustumPlanes[0].c = viewProjection._34 + viewProjection._31;
	d3d.frustumPlanes[0].d = viewProjection._44 + viewProjection._41;

	// Right plane
	d3d.frustumPlanes[1].a = viewProjection._14 - viewProjection._11;
	d3d.frustumPlanes[1].b = viewProjection._24 - viewProjection._21;
	d3d.frustumPlanes[1].c = viewProjection._34 - viewProjection._31;
	d3d.frustumPlanes[1].d = viewProjection._44 - viewProjection._41;

	// Top plane
	d3d.frustumPlanes[2].a = viewProjection._14 - viewProjection._12;
	d3d.frustumPlanes[2].b = viewProjection._24 - viewProjection._22;
	d3d.frustumPlanes[2].c = viewProjection._34 - viewProjection._32;
	d3d.frustumPlanes[2].d = viewProjection._44 - viewProjection._42;

	// Bottom plane
	d3d.frustumPlanes[3].a = viewProjection._14 + viewProjection._12;
	d3d.frustumPlanes[3].b = viewProjection._24 + viewProjection._22;
	d3d.frustumPlanes[3].c = viewProjection._34 + viewProjection._32;
	d3d.frustumPlanes[3].d = viewProjection._44 + viewProjection._42;

	// Near plane
	d3d.frustumPlanes[4].a = viewProjection._13;
	d3d.frustumPlanes[4].b = viewProjection._23;
	d3d.frustumPlanes[4].c = viewProjection._33;
	d3d.frustumPlanes[4].d = viewProjection._43;

	// Far plane
	d3d.frustumPlanes[5].a = viewProjection._14 - viewProjection._13;
	d3d.frustumPlanes[5].b = viewProjection._24 - viewProjection._23;
	d3d.frustumPlanes[5].c = viewProjection._34 - viewProjection._33;
	d3d.frustumPlanes[5].d = viewProjection._44 - viewProjection._43;

	// Normalize planes
	for (int i = 0; i < 6; i++)
	{
		D3DXPlaneNormalize(&d3d.frustumPlanes[i], &d3d.frustumPlanes[i]);
	}
}

bool R_BeginScene()
{
	// check for lost device
	LastError = d3d.lpD3DDevice->TestCooperativeLevel();
	if (FAILED(LastError))
	{
		// release dynamic vertex and index buffers and dynamic textures
		ReleaseVolatileResources();

		// disable XInput
		XInputEnable(false);

		while (1)
		{
			CheckForWindowsMessages();

			if (D3DERR_DEVICENOTRESET == LastError)
			{
				OutputDebugString("Releasing resources for a device reset..\n");

				if (FAILED(d3d.lpD3DDevice->Reset(&d3d.d3dpp)))
				{
					OutputDebugString("Couldn't reset device\n");
				}
				else
				{
					OutputDebugString("We have reset the device. recreating resources..\n");
					CreateVolatileResources();

					SetTransforms();

					// re-enable XInput
					XInputEnable(true);
					break;
				}
			}
			else if (D3DERR_DEVICELOST == LastError)
			{
				OutputDebugString("D3D device lost\n");
			}
			else if (D3DERR_DRIVERINTERNALERROR == LastError)
			{
				// handle this a lot better (exit the game etc)
				Con_PrintError("need to close avp as a display adapter error occured");
				return false;
			}
			Sleep(50);
		}
	}

	LastError = d3d.lpD3DDevice->BeginScene();
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	LastError = d3d.lpD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_EndScene()
{
	LastError = d3d.lpD3DDevice->EndScene();
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_CreateVertexBuffer(class VertexBuffer &vertexBuffer)
{
	D3DPOOL vbPool;
	DWORD	vbUsage;

	switch (vertexBuffer.usage)
	{
		case USAGE_STATIC:
			vbUsage = 0;
			vbPool  = D3DPOOL_MANAGED;
			break;
		case USAGE_DYNAMIC:
			vbUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
			vbPool  = D3DPOOL_DEFAULT;
			break;
		default:
			// error and return
			Con_PrintError("Unknown vertex buffer usage type");
			return false;
			break;
	}

	LastError = d3d.lpD3DDevice->CreateVertexBuffer(vertexBuffer.sizeInBytes, vbUsage, 0, vbPool, &vertexBuffer.vertexBuffer.vertexBuffer, NULL);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't create vertex buffer");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_ReleaseVertexBuffer(class VertexBuffer &vertexBuffer)
{
	if (vertexBuffer.vertexBuffer.vertexBuffer)
	{
		vertexBuffer.vertexBuffer.vertexBuffer->Release();
	}

	return true;
}

bool R_CreateIndexBuffer(class IndexBuffer &indexBuffer)
{
	D3DPOOL ibPool;
	DWORD	ibUsage;

	switch (indexBuffer.usage)
	{
		case USAGE_STATIC:
			ibUsage = 0;
			ibPool  = D3DPOOL_MANAGED;
			break;
		case USAGE_DYNAMIC:
			ibUsage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
			ibPool  = D3DPOOL_DEFAULT;
			break;
		default:
			// error and return
			Con_PrintError("Unknown index buffer usage type");
			return false;
			break;
	}
	
	// create index buffer
	LastError = d3d.lpD3DDevice->CreateIndexBuffer(indexBuffer.sizeInBytes, ibUsage, D3DFMT_INDEX16, ibPool, &indexBuffer.indexBuffer.indexBuffer, NULL);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't create index buffer");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_ReleaseIndexBuffer(class IndexBuffer &indexBuffer)
{
	if (indexBuffer.indexBuffer.indexBuffer)
	{
		indexBuffer.indexBuffer.indexBuffer->Release();
	}

	return true;
}

uint32_t R_GetNumVideoModes()
{
	return d3d.Driver[d3d.CurrentDriver].NumModes;
}

void R_NextVideoMode()
{
	uint32_t numModes = R_GetNumVideoModes();

	if (++d3d.CurrentVideoMode >= numModes)
	{
		d3d.CurrentVideoMode = 0;
	}
}

void R_PreviousVideoMode()
{
	uint32_t numModes = R_GetNumVideoModes();

	if (--d3d.CurrentVideoMode < 0)
	{
		d3d.CurrentVideoMode = numModes - 1;
	}
}

std::string& R_GetVideoModeDescription()
{
	videoModeDescription = Util::IntToString(d3d.Driver[d3d.CurrentDriver].DisplayMode[d3d.CurrentVideoMode].Width)
						   + "x" +
						   Util::IntToString(d3d.Driver[d3d.CurrentDriver].DisplayMode[d3d.CurrentVideoMode].Height);

	return videoModeDescription;
}

char* R_GetDeviceName()
{
	return d3d.Driver[d3d.CurrentDriver].AdapterInfo.Description;
}

void R_SetCurrentVideoMode()
{
	uint32_t currentWidth  = d3d.Driver[d3d.CurrentDriver].DisplayMode[d3d.CurrentVideoMode].Width;
	uint32_t currentHeight = d3d.Driver[d3d.CurrentDriver].DisplayMode[d3d.CurrentVideoMode].Height;

	// set the new values in the config file
	Config_SetInt("[VideoMode]", "Width" , currentWidth);
	Config_SetInt("[VideoMode]", "Height", currentHeight);

	// save the changes to the config file
	Config_Save();

	// and actually change the resolution on the device
	R_ChangeResolution(currentWidth, currentHeight);
}

bool R_SetTexture(uint32_t stage, texID_t textureID)
{
	// check that the stage value is within range
	if (stage > kMaxTextureStages-1)
	{
		Con_PrintError("Invalid texture stage: " + Util::IntToString(stage) + " set for texture: " + Tex_GetName(textureID));
		return false;
	}

	// check if the texture is already set
	if (setTextureArray[stage] == textureID)
	{
		// already set, just return
		return true;
	}

	// we need to set it
	LastError = d3d.lpD3DDevice->SetTexture(stage, Tex_GetTexture(textureID));
	if (FAILED(LastError))
	{
		Con_PrintError("Unable to set texture: " + Tex_GetName(textureID));
		return false;
	}
	else
	{
		// ok, update set texture array
		setTextureArray[stage] = textureID;
	}

	return true;
}

void R_UnsetTexture(texID_t textureID)
{
	// unbind this texture if it's set to any stage
	for (uint32_t i = 0; i < kMaxTextureStages; i++)
	{
		if (setTextureArray[i] == textureID)
		{
			setTextureArray[i] = NO_TEXTURE;
		}
	}
}

bool R_DrawPrimitive(uint32_t numPrimitives)
{
	LastError = d3d.lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, numPrimitives);
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_LockVertexBuffer(class VertexBuffer &vertexBuffer, uint32_t offsetToLock, uint32_t sizeToLock, void **data, enum R_USAGE usage)
{
	DWORD vbFlags = 0;
	if (usage == USAGE_DYNAMIC)
	{
		vbFlags = D3DLOCK_DISCARD;
	}

	LastError = vertexBuffer.vertexBuffer.vertexBuffer->Lock(offsetToLock, sizeToLock, data, vbFlags);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't vertex index buffer");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	vertexBuffer.isLocked = true;

	return true;
}

bool R_DrawIndexedPrimitive(uint32_t baseVertexIndex, uint32_t minIndex, uint32_t numVerts, uint32_t startIndex, uint32_t numPrimitives)
{
	LastError = d3d.lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
			baseVertexIndex,               // BaseVertexIndex - Offset from the start of the vertex buffer to the first vertex
			minIndex,               // MinIndex - Minimum vertex index for vertices used during this call. This is a zero based index relative to BaseVertexIndex.
			numVerts,        // total num verts in VB
			startIndex,
			numPrimitives);

	if (FAILED(LastError))
	{
		Con_PrintError("Can't draw indexed primitive");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_LockIndexBuffer(class IndexBuffer &indexBuffer, uint32_t offsetToLock, uint32_t sizeToLock, uint16_t **data, enum R_USAGE usage)
{
	DWORD ibFlags = 0;
	if (usage == USAGE_DYNAMIC)
	{
		ibFlags = D3DLOCK_DISCARD;
	}

	LastError = indexBuffer.indexBuffer.indexBuffer->Lock(offsetToLock, sizeToLock, (void**)data, ibFlags);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't lock index buffer");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_SetVertexBuffer(class VertexBuffer &vertexBuffer)
{
	LastError = d3d.lpD3DDevice->SetStreamSource(0, vertexBuffer.vertexBuffer.vertexBuffer, 0, vertexBuffer.stride);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't set vertex buffer");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_SetIndexBuffer(class IndexBuffer &indexBuffer)
{
	LastError = d3d.lpD3DDevice->SetIndices(indexBuffer.indexBuffer.indexBuffer);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't set index buffer");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_UnlockVertexBuffer(class VertexBuffer &vertexBuffer)
{
	LastError = vertexBuffer.vertexBuffer.vertexBuffer->Unlock();
	if (FAILED(LastError))
	{
		Con_PrintError("Can't unlock vertex buffer");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	vertexBuffer.isLocked = false;

	return true;
}

bool R_UnlockIndexBuffer(class IndexBuffer &indexBuffer)
{
	LastError = indexBuffer.indexBuffer.indexBuffer->Unlock();
	if (FAILED(LastError))
	{
		Con_PrintError("Can't unlock index buffer");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	indexBuffer.isLocked = false;

	return true;
}

bool CreateVolatileResources()
{
	Tex_ReloadDynamicTextures();

	// main
	d3d.mainVB = new VertexBuffer;
	d3d.mainVB->Create(kMaxVertices*5, sizeof(D3DLVERTEX), USAGE_DYNAMIC);

	d3d.mainIB = new IndexBuffer;
	d3d.mainIB->Create((kMaxIndices*5) * 3, USAGE_DYNAMIC);

	// orthographic projected quads
	d3d.orthoVB = new VertexBuffer;
	d3d.orthoVB->Create(kMaxVertices, sizeof(ORTHOVERTEX), USAGE_DYNAMIC);

	d3d.orthoIB = new IndexBuffer;
	d3d.orthoIB->Create(kMaxIndices * 3, USAGE_DYNAMIC);

	// particle vertex buffer
	d3d.particleVB = new VertexBuffer;
	d3d.particleVB->Create(kMaxVertices*6, sizeof(PARTICLEVERTEX), USAGE_DYNAMIC);

	d3d.particleIB = new IndexBuffer;
	d3d.particleIB->Create((kMaxIndices*6) * 3, USAGE_DYNAMIC);

	// decal buffers
	d3d.decalVB = new VertexBuffer;
	// (MAX_NO_OF_DECALS + MAX_NO_OF_FIXED_DECALS) * 4 * 2 (as we have mirrored decals so we redraw everything)
	d3d.decalVB->Create(8192 * 2, sizeof(DECAL_VERTEX), USAGE_DYNAMIC);

	d3d.decalIB = new IndexBuffer;
	// (MAX_NO_OF_DECALS + MAX_NO_OF_FIXED_DECALS) * 6 * 2 (as we have mirrored decals so we redraw everything)
	d3d.decalIB->Create(12288 * 2, USAGE_STATIC);

	SetRenderStateDefaults();

	// going to clear texture stages too
	for (int stage = 0; stage < kMaxTextureStages; stage++)
	{
		LastError = d3d.lpD3DDevice->SetTexture(stage, Tex_GetTexture(NO_TEXTURE));
		if (FAILED(LastError))
		{
			Con_PrintError("Unable to reset texture stage: " + stage);
			return false;
		}

		setTextureArray[stage] = NO_TEXTURE;
	}

	return true;
}

bool R_LockTexture(Texture &texture, uint8_t **data, uint32_t *pitch, enum TextureLock lockType)
{
	D3DLOCKED_RECT lock;
	uint32_t lockFlag;

	switch (lockType)
	{
		case TextureLock_Normal:
			lockFlag = 0;
			break;
		case TextureLock_Discard:
			lockFlag = D3DLOCK_DISCARD;
			break;
		default:
			// error and return
			break;
	}

	LastError = texture.texture->LockRect(0, &lock, NULL, lockFlag);
	if (FAILED(LastError))
	{
		Con_PrintError("Unable to lock texture");
		*data = 0;
		*pitch = 0;
		return false;
	}
	else
	{
		*data = static_cast<uint8_t*>(lock.pBits);
		*pitch = lock.Pitch;
		return true;
	}
}

bool R_UnlockTexture(Texture &texture)
{
	LastError = texture.texture->UnlockRect(0);
	if (FAILED(LastError))
	{
		Con_PrintError("Unable to unlock texture");
		return false;
	}

	return true;
}

void ColourFillBackBuffer(int FillColour)
{
	d3d.lpD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, FillColour, 1.0f, 0);
}

// console command : set a new field of view value
void SetFov()
{
	if (Con_GetNumArguments() == 0)
	{
		Con_PrintMessage("usage: r_setfov 75");
		return;
	}

	d3d.fieldOfView = Util::StringToInt(Con_GetArgument(0));

	SetTransforms();
}

void CheckWireFrameMode(int shouldBeOn)
{
	if (shouldBeOn)
		shouldBeOn = 1;

	if (CurrentRenderStates.WireFrameModeIsOn != shouldBeOn)
	{
		CurrentRenderStates.WireFrameModeIsOn = shouldBeOn;
		if (shouldBeOn)
		{
			d3d.lpD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
		else
		{
			d3d.lpD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		}
	}
}

void R_SetFov(uint32_t fov)
{
	d3d.fieldOfView = fov;
}

void FlushD3DZBuffer()
{
	d3d.lpD3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
}

extern bool frustumCull;

void ToggleFrustumCull()
{
	frustumCull = !frustumCull;

	if (frustumCull)
	{
		Con_PrintMessage("Frustum culling is now ON");
	}
	else
	{
		Con_PrintMessage("Frustum culling is now OFF");
	}
}

void PrintD3DMatrix(const char* name, D3DXMATRIX &mat)
{
	char buf[300];
	sprintf(buf, "Printing D3D Matrix: - %s\n"
	"\t %f \t %f \t %f \t %f\n"
	"\t %f \t %f \t %f \t %f\n"
	"\t %f \t %f \t %f \t %f\n"
	"\t %f \t %f \t %f \t %f\n", name,
	mat._11, mat._12, mat._13, mat._14,
	mat._21, mat._22, mat._23, mat._24,
	mat._31, mat._32, mat._33, mat._34,
	mat._41, mat._42, mat._43, mat._44);

	OutputDebugString(buf);
}

// called from Scrshot.cpp
void CreateScreenShotImage()
{
	SYSTEMTIME systemTime;
	LPDIRECT3DSURFACE9 frontBuffer = NULL;

	std::ostringstream fileName(GetSaveFolderPath());

	// get current system time
	GetSystemTime(&systemTime);

	//	creates filename from date and time, adding a prefix '0' to seconds value
	//	otherwise 9 seconds appears as '9' instead of '09'
	bool prefixSeconds = false;
	if (systemTime.wYear < 10)
		prefixSeconds = true;

	fileName << "AvP_" << systemTime.wDay << "-" << systemTime.wMonth << "-" << systemTime.wYear << "_" << systemTime.wHour << "-" << systemTime.wMinute << "-";

	if (systemTime.wSecond < 10)
	{
		fileName << "0" << systemTime.wSecond;
	}
	else
	{
		fileName << systemTime.wSecond;
	}

	fileName << ".jpg";

	// create surface to copy screen to
	if (FAILED(d3d.lpD3DDevice->CreateOffscreenPlainSurface(ScreenDescriptorBlock.SDB_Width, ScreenDescriptorBlock.SDB_Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &frontBuffer, NULL)))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		OutputDebugString("Couldn't create screenshot surface\n");
		return;
	}

	// copy front buffer screen to surface
	if (FAILED(d3d.lpD3DDevice->GetFrontBufferData(0, frontBuffer)))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		OutputDebugString("Couldn't get a copy of the front buffer\n");
		SAFE_RELEASE(frontBuffer);
		return;
	}

	// save surface to image file
	if (FAILED(D3DXSaveSurfaceToFile(fileName.str().c_str(), D3DXIFF_JPG, frontBuffer, NULL, NULL)))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		OutputDebugString("Save Surface to file failed\n");
	}

	// release surface
	SAFE_RELEASE(frontBuffer);
}

/*
 * This function is responsible for creating the large sized font texture, as used in the game menu system. Originally this
 * texture was only handled by DirectDraw, and has a resolution of 30 x 7392, which cannot be loaded as a standard Direct3D texture.
 * The original image (IntroFont.RIM) is a bitmap font, containing one letter per row (width of 30px). The below function takes this
 * bitmap font and rearranges it into a square texture, now containing more letters per row (as a standard bitmap font would be)
 * which can be used by Direct3D without issue.
 */
bool R_CreateTallFontTexture(AVPTEXTURE &tex, enum TextureUsage usageType, Texture &texture)
{
	// default colour format
	D3DFORMAT colourFormat = D3DFMT_A8R8G8B8;

	D3DPOOL texturePool;
	uint32_t textureUsage;

	// ensure this will be NULL unless we successfully create it
	texture.texture = 0;

	switch (usageType)
	{
		case TextureUsage_Normal:
		{
			texturePool  = D3DPOOL_MANAGED;
			textureUsage = 0;
			break;
		}
		case TextureUsage_Dynamic:
		{
			texturePool  = D3DPOOL_DEFAULT;
			textureUsage = D3DUSAGE_DYNAMIC;
			break;
		}
		default:
		{
			Con_PrintError("Invalid texture usageType value in R_CreateTallFontTexture");
			return false;
		}
	}

	uint32_t width, height;
	uint32_t padWidth, padHeight;
	uint32_t charWidth, charHeight;

	if (IsDemoVersion())
	{	
		width  = 240;
		height = 160;

		padWidth  = 256;
		padHeight = 256;

		charWidth  = 30;
		charHeight = 21;
	}
	else
	{
		width  = 450;
		height = 495;

		padWidth  = 512;
		padHeight = 512;

		charWidth  = 30;
		charHeight = 33;
	}

	uint32_t numTotalChars = tex.height / charHeight;

	LastError = d3d.lpD3DDevice->CreateTexture(padWidth, padHeight, 1, textureUsage, colourFormat, texturePool, &texture.texture, NULL);
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	D3DLOCKED_RECT lock;

	LastError = texture.texture->LockRect(0, &lock, NULL, NULL);
	if (FAILED(LastError))
	{
		texture.texture->Release();
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	uint8_t *destPtr, *srcPtr;

	srcPtr = (uint8_t*)tex.buffer;

	D3DCOLOR padColour = D3DCOLOR_ARGB(255, 255, 0, 255);

	// lets pad the whole thing black first
	for (uint32_t y = 0; y < padHeight; y++)
	{
		destPtr = (((uint8_t*)lock.pBits) + y*lock.Pitch);

		for (uint32_t x = 0; x < padWidth; x++)
		{
			// >> 3 for red and blue in a 16 bit texture, 2 for green
			*(D3DCOLOR*)destPtr = D3DCOLOR_RGBA(padColour, padColour, padColour, padColour);

			destPtr += sizeof(uint32_t); // move over an entire 4 byte pixel
		}
	}

	for (uint32_t i = 0; i < numTotalChars; i++)
	{
		uint32_t row = i / IntroFont_Light.nRows;       // get row
		uint32_t column = i % IntroFont_Light.nColumns; // get column from remainder value

		uint32_t offset = ((column * charWidth) * sizeof(uint32_t)) + ((row * charHeight) * lock.Pitch);

		destPtr = (((uint8_t*)lock.pBits + offset));

		for (uint32_t y = 0; y < charHeight; y++)
		{
			destPtr = (((uint8_t*)lock.pBits + offset) + (y*lock.Pitch));

			for (uint32_t x = 0; x < charWidth; x++)
			{
				if (srcPtr[0] == 0x00 && srcPtr[1] == 0x00 && srcPtr[2] == 0x00)
				{
					*(D3DCOLOR*)destPtr = D3DCOLOR_RGBA(srcPtr[0], srcPtr[1], srcPtr[2], 0x00);
				}
				else
				{
					*(D3DCOLOR*)destPtr = D3DCOLOR_RGBA(srcPtr[0], srcPtr[1], srcPtr[2], 0xff);
				}

				destPtr += sizeof(uint32_t); // move over an entire 4 byte pixel
				srcPtr  += sizeof(uint32_t);
			}
		}
	}

	LastError = texture.texture->UnlockRect(0);
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}
	
	// fill out newTexture struct
	texture.bitsPerPixel = 32;
	texture.width  = width;
	texture.height = height;
	texture.realWidth  = padWidth;
	texture.realHeight = padHeight;
	texture.usage = usageType;

	return true;
}

static BYTE VD_USAGEtoD3DDECLUSAGE(VD_USAGE usage)
{
	BYTE d3dUsage;

	switch (usage)
	{
		case VDUSAGE_POSITION:
			d3dUsage = D3DDECLUSAGE_POSITION;
			break;
		case VDUSAGE_BLENDWEIGHT:
			d3dUsage = D3DDECLUSAGE_BLENDWEIGHT;
			break;
		case VDUSAGE_BLENDINDICES:
			d3dUsage = D3DDECLUSAGE_BLENDINDICES;
			break;
		case VDUSAGE_NORMAL:
			d3dUsage = D3DDECLUSAGE_NORMAL;
			break;
		case VDUSAGE_PSIZE:
			d3dUsage = D3DDECLUSAGE_PSIZE;
			break;
		case VDUSAGE_TEXCOORD:
			d3dUsage = D3DDECLUSAGE_TEXCOORD;
			break;
		case VDUSAGE_TANGENT:
			d3dUsage = D3DDECLUSAGE_TANGENT;
			break;
		case VDUSAGE_BINORMAL:
			d3dUsage = D3DDECLUSAGE_BINORMAL;
			break;
		case VDUSAGE_TESSFACTOR:
			d3dUsage = D3DDECLUSAGE_TESSFACTOR;
			break;
		case VDUSAGE_POSITIONT:
			d3dUsage = D3DDECLUSAGE_POSITIONT;
			break;
		case VDUSAGE_COLOR:
			d3dUsage = D3DDECLUSAGE_COLOR;
			break;
		case VDUSAGE_FOG:
			d3dUsage = D3DDECLUSAGE_FOG;
			break;
		case VDUSAGE_DEPTH:
			d3dUsage = D3DDECLUSAGE_DEPTH;
			break;
		case VDUSAGE_SAMPLE:
			d3dUsage = D3DDECLUSAGE_SAMPLE;
			break;
		default:
			assert (1==0);
			break;
	}

	return d3dUsage;
}

static BYTE VD_TYPEtoD3DDECLTYPE(VD_TYPE type)
{
	BYTE d3dType;

	switch (type)
	{
		case VDTYPE_FLOAT1:
			d3dType = D3DDECLTYPE_FLOAT1;
			break;
		case VDTYPE_FLOAT2:
			d3dType = D3DDECLTYPE_FLOAT2;
			break;
		case VDTYPE_FLOAT3:
			d3dType = D3DDECLTYPE_FLOAT3;
			break;
		case VDTYPE_FLOAT4:
			d3dType = D3DDECLTYPE_FLOAT4;
			break;
		case VDTYPE_COLOR:
			d3dType = D3DDECLTYPE_D3DCOLOR;
			break;
		default:
			assert (1==0);
			break;
	}

	return d3dType;
}

bool R_CreateVertexDeclaration(class VertexDeclaration *vertexDeclaration)
{
	// take a local copy of this to aid in keeping the code below a bit tidier..
	size_t elementsSize = vertexDeclaration->elements.size();

	std::vector<D3DVERTEXELEMENT9> d3dElement;
	d3dElement.resize(elementsSize+1); // +1 for DECL_END()

	for (size_t i = 0; i < elementsSize; i++)
	{
		d3dElement[i].Stream = vertexDeclaration->elements[i].stream;
		d3dElement[i].Offset = vertexDeclaration->elements[i].offset;
		d3dElement[i].Type   = VD_TYPEtoD3DDECLTYPE(vertexDeclaration->elements[i].type);
		d3dElement[i].Method = D3DDECLMETHOD_DEFAULT; // TODO
		d3dElement[i].Usage  = VD_USAGEtoD3DDECLUSAGE(vertexDeclaration->elements[i].usage);
		d3dElement[i].UsageIndex = vertexDeclaration->elements[i].usageIndex;
	}

	// add D3DDECL_END() which is {0xFF,0,D3DDECLTYPE_UNUSED,0,0,0}
	d3dElement[elementsSize].Stream = 0xFF;
	d3dElement[elementsSize].Offset = 0;
	d3dElement[elementsSize].Type   = D3DDECLTYPE_UNUSED;
	d3dElement[elementsSize].Method = 0;
	d3dElement[elementsSize].Usage  = 0;
	d3dElement[elementsSize].UsageIndex = 0;

	// try and create it now
	LastError = d3d.lpD3DDevice->CreateVertexDeclaration(&d3dElement[0], &vertexDeclaration->declaration);
	if (FAILED(LastError))
	{
		Con_PrintError("Could not create vertex declaration");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool R_SetVertexDeclaration(class VertexDeclaration &declaration)
{
	LastError = d3d.lpD3DDevice->SetVertexDeclaration(declaration.declaration);
	if (FAILED(LastError))
	{
		Con_PrintError("Could not set vertex declaration");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool R_ReleaseVertexDeclaration(class VertexDeclaration &declaration)
{
	LastError = declaration.declaration->Release();
	if (FAILED(LastError))
	{
		Con_PrintError("Could not release vertex declaration");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}
	return true;
}

bool R_CreateVertexShader(const std::string &fileName, r_VertexShader &vertexShader, VertexDeclaration *vertexDeclaration)
{
	LPD3DXBUFFER pErrors = NULL;
	LPD3DXBUFFER pCode = NULL;
	std::string actualPath = shaderPath + fileName;

	// test that the path to the file is valid first (d3dx doesn't give a specific error message for this)
	std::ifstream fileOpenTest(actualPath.c_str(), std::ifstream::in | std::ifstream::binary);
	if (!fileOpenTest.good())
	{
		LogErrorString("Can't open vertex shader file '" + actualPath + "'", __LINE__, __FILE__);
		return false;
	}
	// close the file
	fileOpenTest.close();

	// set up vertex shader
	LastError = D3DXCompileShaderFromFile(actualPath.c_str(), //filepath
						NULL,            // macro's
						NULL,            // includes
						"vs_main",       // main function
						"vs_2_0",        // shader profile
						0,               // flags
						&pCode,          // compiled operations
						&pErrors,        // errors
						&vertexShader.constantTable); // constants

	if (FAILED(LastError))
	{
		OutputDebugString(DXGetErrorString(LastError));
		OutputDebugString(DXGetErrorDescription(LastError));

		LogErrorString("D3DXCompileShaderFromFile failed for '" + actualPath + "'", __LINE__, __FILE__);

		if (pErrors)
		{
			// shader didn't compile for some reason
			LogErrorString("Shader compile errors found for '" + actualPath + "'", __LINE__, __FILE__);
			LogErrorString("\n" + std::string((const char*)pErrors->GetBufferPointer()));

			pErrors->Release();
		}

		return false;
	}

	LastError = d3d.lpD3DDevice->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &vertexShader.shader);
	if (FAILED(LastError))
	{
		LogErrorString("CreateVertexShader failed for '" + actualPath + "'", __LINE__, __FILE__);
		return false;
	}

	// no longer need pCode
	pCode->Release();

	// now find out how many constant registers our shader uses
	D3DXCONSTANTTABLE_DESC constantTableDesc;
	LastError = vertexShader.constantTable->GetDesc(&constantTableDesc);
	if (FAILED(LastError))
	{
		LogErrorString("CreateVertexShader failed getting constant table description for '" + actualPath + "'", __LINE__, __FILE__);
		return false;
	}

	// we're going to store handles to each register in our std::vector so make it the right size
	vertexShader.constantsArray.resize(constantTableDesc.Constants);

	std::stringstream message;
	message << "Processing constants for vertex shader '" << fileName << "' which has " << constantTableDesc.Constants << " constant(s)\n";
	OutputDebugString(message.str().c_str());

	D3DXCONSTANT_DESC constantDesc;

	// we just want to get one constantDesc at a time
	uint32_t constArraySize = 1;

    // loop, getting and storing a handle to each shader constant
	for (uint32_t i = 0; i < constantTableDesc.Constants; i++)
	{
		vertexShader.constantsArray[i] = vertexShader.constantTable->GetConstant(NULL, i);
		vertexShader.constantTable->GetConstantDesc(vertexShader.constantsArray[i], &constantDesc, &constArraySize);

		assert(constArraySize == 1);

		std::stringstream constantInfo;
		constantInfo << "\t Name: " << constantDesc.Name << ", Register Index: " << constantDesc.RegisterIndex << /*", Register Index: " << constantDesc.RegisterIndex <<*/ "\n";
		OutputDebugString(constantInfo.str().c_str());
	}

	return true;
}

bool R_CreatePixelShader(const std::string &fileName, r_PixelShader &pixelShader)
{
	LPD3DXBUFFER pErrors = NULL;
	LPD3DXBUFFER pCode = NULL;
	std::string actualPath = shaderPath + fileName;

    // test that the path to the file is valid first (d3dx doesn't give a specific error message for this)
	std::ifstream fileOpenTest(actualPath.c_str(), std::ifstream::in | std::ifstream::binary);
	if (!fileOpenTest.good())
	{
		LogErrorString("Can't open pixel shader file '" + actualPath + "'", __LINE__, __FILE__);
		return false;
	}
	// close the file
	fileOpenTest.close();

	// set up pixel shader
	LastError = D3DXCompileShaderFromFile(actualPath.c_str(), //filepath
						NULL,            // macro's
						NULL,            // includes
						"ps_main",       // main function
						"ps_2_0",        // shader profile
						0,               // flags
						&pCode,          // compiled operations
						&pErrors,        // errors
						&pixelShader.constantTable);    // constants

	if (FAILED(LastError))
	{
		OutputDebugString(DXGetErrorString(LastError));
		OutputDebugString(DXGetErrorDescription(LastError));

		LogErrorString("D3DXCompileShaderFromFile failed for '" + actualPath + "'", __LINE__, __FILE__);

		if (pErrors)
		{
			// shader didn't compile for some reason
			OutputDebugString((const char*)pErrors->GetBufferPointer());

			LogErrorString("Shader compile errors found for '" + actualPath + "'", __LINE__, __FILE__);
			LogErrorString("\n" + std::string((const char*)pErrors->GetBufferPointer()));

			pErrors->Release();
		}

		return false;
	}

	LastError = d3d.lpD3DDevice->CreatePixelShader((DWORD*)pCode->GetBufferPointer(), &pixelShader.shader);
	if (FAILED(LastError))
	{
		LogErrorString("CreatePixelShader failed for '" + actualPath + "'", __LINE__, __FILE__);
		return false;
	}

	// no longer need pCode
	pCode->Release();

	// now find out how many constant registers our shader uses
	D3DXCONSTANTTABLE_DESC constantTableDesc;
	pixelShader.constantTable->GetDesc(&constantTableDesc);
	if (FAILED(LastError))
	{
		LogErrorString("CreatePixelShader failed getting constant table description for '" + actualPath + "'", __LINE__, __FILE__);
		return false;
	}

	// we're going to store handles to each register in our std::vector so make it the right size
	pixelShader.constantsArray.resize(constantTableDesc.Constants);

	std::stringstream message;
	message << "Processing constants for pixel shader '" << fileName << "' which has " << constantTableDesc.Constants << " constant(s)\n";
	OutputDebugString(message.str().c_str());

	D3DXCONSTANT_DESC constantDesc;

	// we just want to get one constantDesc at a time
	uint32_t constArraySize = 1;

	// loop, getting and storing a handle to each shader constant
	for (uint32_t i = 0; i < constantTableDesc.Constants; i++)
	{
		pixelShader.constantsArray[i] = pixelShader.constantTable->GetConstant(NULL, i);
		pixelShader.constantTable->GetConstantDesc(pixelShader.constantsArray[i], &constantDesc, &constArraySize);

		assert(constArraySize == 1);

		std::stringstream constantInfo;
		constantInfo << "\t Name: " << constantDesc.Name << ", Register Index: " << constantDesc.RegisterIndex << /*", Register Index: " << constantDesc.RegisterIndex <<*/ "\n";
		OutputDebugString(constantInfo.str().c_str());
	}

	return true;
}

bool R_SetVertexShader(r_VertexShader &vertexShader)
{
	LastError = d3d.lpD3DDevice->SetVertexShader(vertexShader.shader);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't set vertex shader " + vertexShader.shaderName);
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_UnsetVertexShader()
{
	LastError = d3d.lpD3DDevice->SetVertexShader(NULL);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't set vertex shader to NULL");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_SetPixelShaderConstant(r_PixelShader &pixelShader, uint32_t registerIndex, enum SHADER_CONSTANT type, const void *constantData)
{
	uint32_t sizeInBytes = 0;

	assert(registerIndex < pixelShader.constantsArray.size());

	switch (type)
	{
		case CONST_INT:
			sizeInBytes = 4; // sizeof(float);
			break;

		case CONST_FLOAT:
			sizeInBytes = 4; // sizeof(float);
			break;

		case CONST_VECTOR3:
			sizeInBytes = 12; // sizeof(float) * 3;
			break;

		case CONST_VECTOR4:
			sizeInBytes = 16; // sizeof(float) * 4;
			break;

		case CONST_MATRIX:
			sizeInBytes = 64; // sizeof(float) * 16;
			break;

		default:
			LogErrorString("Unknown shader constant type");
			return false;
			break;
	}

	LastError = pixelShader.constantTable->SetValue(d3d.lpD3DDevice, pixelShader.constantsArray[registerIndex], constantData, sizeInBytes);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't SetValue for pixel shader " + pixelShader.shaderName);
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_SetVertexShaderConstant(r_VertexShader &vertexShader, uint32_t registerIndex, enum SHADER_CONSTANT type, const void *constantData)
{
	uint32_t sizeInBytes = 0;

	assert(registerIndex < vertexShader.constantsArray.size());

	switch (type)
	{
		case CONST_INT:
			sizeInBytes = 4; // sizeof(float);
			break;

		case CONST_FLOAT:
			sizeInBytes = 4; // sizeof(float);
			break;

		case CONST_VECTOR3:
			sizeInBytes = 12; // sizeof(float) * 3;
			break;

		case CONST_VECTOR4:
			sizeInBytes = 16; // sizeof(float) * 4;
			break;

		case CONST_MATRIX:
			sizeInBytes = 64; // sizeof(float) * 16;
			break;

		default:
			LogErrorString("Unknown shader constant type");
			return false;
			break;
	}

	LastError = vertexShader.constantTable->SetValue(d3d.lpD3DDevice, vertexShader.constantsArray[registerIndex], constantData, sizeInBytes);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't SetValue for vertex shader " + vertexShader.shaderName);
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_SetPixelShader(r_PixelShader &pixelShader)
{
	LastError = d3d.lpD3DDevice->SetPixelShader(pixelShader.shader);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't set pixel shader " + pixelShader.shaderName);
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

bool R_UnsetPixelShader()
{
	LastError = d3d.lpD3DDevice->SetPixelShader(NULL);
	if (FAILED(LastError))
	{
		Con_PrintError("Can't set pixel shader to NULL");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	return true;
}

// removes pure red colour from a texture. used to remove red outline grid on small font texture.
// we remove the grid as it can sometimes bleed onto text when we use texture filtering. maybe add params for passing width/height?
void DeRedTexture(Texture &texture)
{
	uint8_t *srcPtr = NULL;
	uint8_t *destPtr = NULL;
	uint32_t pitch = 0;

	// lock texture
	if (!R_LockTexture(texture, &srcPtr, &pitch, TextureLock_Normal))
	{
		Con_PrintError("DeRedTexture call failed - can't lock texture");
		return;
	}

	// loop, setting all full red pixels to black
	for (uint32_t y = 0; y < texture.height; y++)
	{
		destPtr = (srcPtr + y*pitch);

		for (uint32_t x = 0; x < texture.width; x++)
		{
			if ((destPtr[BO_RED] == 255) && (destPtr[BO_BLUE] == 0) && (destPtr[BO_GREEN] == 0))
			{
				destPtr[BO_RED] = 0; // set to black
			}

			destPtr += sizeof(uint32_t); // move over an entire 4 byte pixel
		}
	}

	R_UnlockTexture(texture);
}

bool R_CreateTextureFromFile(const std::string &fileName, Texture &texture)
{
	D3DXIMAGE_INFO imageInfo;

	// ensure this will be NULL unless we successfully create it
	texture.texture = NULL;

	LastError = D3DXCreateTextureFromFileEx(d3d.lpD3DDevice,
		fileName.c_str(),
		D3DX_DEFAULT,			// width
		D3DX_DEFAULT,			// height
		1,						// mip levels
		0,						// usage
		D3DFMT_UNKNOWN,			// format
		D3DPOOL_MANAGED,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		&imageInfo,
		NULL,
		&texture.texture
		);

	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		texture.texture = NULL;
		return false;
	}

	// check to see if D3D resized our texture
	D3DSURFACE_DESC surfaceDescription;
	texture.texture->GetLevelDesc(0, &surfaceDescription);

	texture.realWidth  = surfaceDescription.Width;
	texture.realHeight = surfaceDescription.Height;

	// save the original width and height
	texture.width  = imageInfo.Width;
	texture.height = imageInfo.Height;
	texture.bitsPerPixel = 32; // this should be ok?
	texture.usage = TextureUsage_Normal;

	return true;
}

bool R_CreateTextureFromAvPTexture(AVPTEXTURE &AvPTexture, enum TextureUsage usageType, Texture &texture)
{
	D3DPOOL texturePool;
	uint32_t textureUsage;
	LPDIRECT3DTEXTURE9 d3dTexture = NULL;

	// ensure this will be NULL unless we successfully create it
	texture.texture = NULL;

	switch (usageType)
	{
		case TextureUsage_Normal:
		{
			texturePool  = D3DPOOL_MANAGED;
			textureUsage = 0;
			break;
		}
		case TextureUsage_Dynamic:
		{
			texturePool  = D3DPOOL_DEFAULT;
			textureUsage = D3DUSAGE_DYNAMIC;
			break;
		}
		default:
		{
			Con_PrintError("Invalid texture usageType value in R_CreateTextureFromAvPTexture()");
			return false;
		}
	}

	// fill tga header
	TgaHeader.idlength = 0;
	TgaHeader.x_origin = AvPTexture.width;
	TgaHeader.y_origin = AvPTexture.height;
	TgaHeader.colourmapdepth	= 0;
	TgaHeader.colourmaplength	= 0;
	TgaHeader.colourmaporigin	= 0;
	TgaHeader.colourmaptype		= 0;
	TgaHeader.datatypecode		= 2;		// RGB
	TgaHeader.bitsperpixel		= 32;
	TgaHeader.imagedescriptor	= 0x20;		// set origin to top left
	TgaHeader.width = AvPTexture.width;
	TgaHeader.height = AvPTexture.height;

	// size of raw image data
	uint32_t imageSize = AvPTexture.height * AvPTexture.width * sizeof(uint32_t);

	// create new buffer for header and image data
	uint8_t *buffer = new uint8_t[sizeof(TGA_HEADER) + imageSize];

	// copy header and image data to buffer
	memcpy(buffer, &TgaHeader, sizeof(TGA_HEADER));

	uint8_t *imageData = buffer + sizeof(TGA_HEADER);

	// loop, converting RGB to BGR for D3DX function
	for (uint32_t i = 0; i < imageSize; i+=4)
	{
		// ARGB
		// BGR
		imageData[i+2] = AvPTexture.buffer[i];
		imageData[i+1] = AvPTexture.buffer[i+1];
		imageData[i]   = AvPTexture.buffer[i+2];
		imageData[i+3] = AvPTexture.buffer[i+3];
	}

	if (FAILED(D3DXCreateTextureFromFileInMemoryEx(d3d.lpD3DDevice,
		buffer,
		sizeof(TGA_HEADER) + imageSize,
		AvPTexture.width,
		AvPTexture.height,
		1, // mips
		textureUsage,
		D3DFMT_A8R8G8B8,
		texturePool,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		NULL,
		0,
		&d3dTexture)))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		delete[] buffer;
		texture.texture = NULL;
		return false;
	}

	// check to see if D3D resized our texture
	D3DSURFACE_DESC surfaceDescription;
	d3dTexture->GetLevelDesc(0, &surfaceDescription);

	texture.realWidth  = surfaceDescription.Width;
	texture.realHeight = surfaceDescription.Height;

	// set texture struct members
	texture.bitsPerPixel = 32; // set to 32 for now
	texture.width   = AvPTexture.width;
	texture.height  = AvPTexture.height;
	texture.usage   = usageType;
	texture.texture = d3dTexture;

	delete[] buffer;
	return true;
}

bool IsPowerOfTwo(uint32_t value)
{
	return ((value & (value - 1)) == 0);
}

uint32_t RoundUpToNextPowerOfTwo(uint32_t value)
{
	value--;
	value |= value >> 1;  // handle  2 bit numbers
	value |= value >> 2;  // handle  4 bit numbers
	value |= value >> 4;  // handle  8 bit numbers
	value |= value >> 8;  // handle 16 bit numbers
	value |= value >> 16; // handle 32 bit numbers
	value++;

	return value;
}

bool R_CreateTexture(uint32_t width, uint32_t height, uint32_t bitsPerPixel, enum TextureUsage usageType, Texture &texture)
{
	D3DPOOL texturePool;
	D3DFORMAT textureFormat;
	uint32_t textureUsage;
	LPDIRECT3DTEXTURE9 d3dTexture = NULL;

	// ensure this will be NULL unless we successfully create it
	texture.texture = NULL;

	switch (usageType)
	{
		case TextureUsage_Normal:
		{
			texturePool  = D3DPOOL_MANAGED;
			textureUsage = 0;
			break;
		}
		case TextureUsage_Dynamic:
		{
			texturePool  = D3DPOOL_DEFAULT;
			textureUsage = D3DUSAGE_DYNAMIC;
			break;
		}
		default:
		{
			Con_PrintError("Invalid texture usageType value in R_CreateTexture()");
			return false;
		}
	}

	switch (bitsPerPixel)
	{
		case 32:
		{
			textureFormat = D3DFMT_A8R8G8B8;
			break;
		}
		case 16:
		{
			textureFormat = D3DFMT_A1R5G5B5;
			break;
		}
		case 8:
		{
			textureFormat = D3DFMT_L8;
			break;
		}
		default:
		{
			Con_PrintError("Invalid bitsPerPixel value in R_CreateTexture()");
			return false;
		}
	}

	uint32_t realWidth  = width;
	uint32_t realHeight = height;

	D3DXCheckTextureRequirements(d3d.lpD3DDevice, &realWidth, &realHeight, NULL, textureUsage, &textureFormat, texturePool);

	// create the d3d9 texture
	LastError = d3d.lpD3DDevice->CreateTexture(realWidth, realHeight, 1, textureUsage, textureFormat, texturePool, &d3dTexture, NULL);
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	// set texture struct members
	texture.bitsPerPixel = bitsPerPixel;
	texture.width  = width;
	texture.height = height;
	texture.realWidth  = realWidth;
	texture.realHeight = realHeight;
	texture.usage   = usageType;
	texture.texture = d3dTexture;
	texture.isValid = true;

	return true;
}

void R_ReleaseTexture(Texture &texture)
{
	if (texture.texture)
	{
		texture.texture->Release();
		texture.texture = NULL;
	}
}

void R_ReleaseVertexShader(r_VertexShader &vertexShader)
{
	if (vertexShader.shader)
	{
		vertexShader.shader->Release();
		vertexShader.shader = 0;
	}

	// release the constant table too
	if (vertexShader.constantTable)
	{
		vertexShader.constantTable->Release();
		vertexShader.constantTable = 0;
	}
}

void R_ReleasePixelShader(r_PixelShader &pixelShader)
{
	if (pixelShader.shader)
	{
		pixelShader.shader->Release();
		pixelShader.shader = 0;
	}

	// release the constant table too
	if (pixelShader.constantTable)
	{
		pixelShader.constantTable->Release();
		pixelShader.constantTable = 0;
	}
}

bool R_ChangeResolution(uint32_t width, uint32_t height)
{
	// don't bother resetting device if we're already using the requested settings
	if ((width == d3d.d3dpp.BackBufferWidth) && (height == d3d.d3dpp.BackBufferHeight))
	{
		return true;
	}

	ThisFramesRenderingHasFinished();

	ReleaseVolatileResources();

	d3d.d3dpp.BackBufferWidth  = width;
	d3d.d3dpp.BackBufferHeight = height;

	// change the window size
	ChangeWindowsSize(width, height);

	// try reset the device
	LastError = d3d.lpD3DDevice->Reset(&d3d.d3dpp);
	if (FAILED(LastError))
	{
		// log an error message
		std::stringstream sstream;
		sstream << "Can't set resolution " << width << " x " << height << ". Setting default safe values";
		Con_PrintError(sstream.str());

		OutputDebugString(DXGetErrorString(LastError));
		OutputDebugString(DXGetErrorDescription(LastError));
		OutputDebugString("\n");

		// this'll occur if the resolution width and height passed aren't usable on this device
		if (D3DERR_INVALIDCALL == LastError)
		{
			// set some default, safe resolution?
			width  = 800;
			height = 600;

			ChangeWindowsSize(width, height);

			d3d.d3dpp.BackBufferWidth  = width;
			d3d.d3dpp.BackBufferHeight = height;

			// try reset again, if it doesnt work, bail out
			LastError = d3d.lpD3DDevice->Reset(&d3d.d3dpp);
			if (FAILED(LastError))
			{
				LogDxError(LastError, __LINE__, __FILE__);
				return false;
			}
		}
		else
		{
			LogDxError(LastError, __LINE__, __FILE__);
			return false;
		}
	}

	d3d.D3DViewport.Width  = width;
	d3d.D3DViewport.Height = height;

	LastError = d3d.lpD3DDevice->SetViewport(&d3d.D3DViewport);
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
	}

	ScreenDescriptorBlock.SDB_Width     = width;
	ScreenDescriptorBlock.SDB_Height    = height;
//	ScreenDescriptorBlock.SDB_Depth		= colourDepth;
	ScreenDescriptorBlock.SDB_Size      = width*height;
	ScreenDescriptorBlock.SDB_CentreX   = width/2;
	ScreenDescriptorBlock.SDB_CentreY   = height/2;

	ScreenDescriptorBlock.SDB_ProjX     = width/2;
	ScreenDescriptorBlock.SDB_ProjY     = height/2;

	ScreenDescriptorBlock.SDB_ClipLeft  = 0;
	ScreenDescriptorBlock.SDB_ClipRight = width;
	ScreenDescriptorBlock.SDB_ClipUp    = 0;
	ScreenDescriptorBlock.SDB_ClipDown  = height;

	// aspect ratio
	d3d.aspectRatio = (float)width / (float)height;

	CreateVolatileResources();

	SetTransforms();

	d3d.effectSystem->Reset();

	ThisFramesRenderingHasBegun();

	return true;
}

// need to redo all the enumeration code here, as it's not very good..
bool InitialiseDirect3D()
{
	// clear log file first, then write header text
	ClearLog();
	Con_PrintMessage("Starting to initialise Direct3D9");

	// grab the users selected resolution from the config file
	uint32_t width       = Config_GetInt("[VideoMode]", "Width", 800);
	uint32_t height      = Config_GetInt("[VideoMode]", "Height", 600);
//	uint32_t colourDepth = Config_GetInt("[VideoMode]", "ColourDepth", 32);
	bool useTripleBuffering = Config_GetBool("[VideoMode]", "UseTripleBuffering", false);
	bool useVSync = Config_GetBool("[VideoMode]", "UseVSync", true);
	shaderPath = Config_GetString("[VideoMode]", "ShaderPath", "shaders/");

	// set some defaults
	uint32_t defaultDevice = D3DADAPTER_DEFAULT;
	bool windowed = false;
	D3DFORMAT SelectedDepthFormat = D3DFMT_D24S8;
	D3DFORMAT SelectedAdapterFormat = D3DFMT_X8R8G8B8;
	D3DFORMAT SelectedBackbufferFormat = D3DFMT_X8R8G8B8; // back buffer format

	if (WindowMode == WindowModeSubWindow)
		windowed = true;

	// Zero d3d structure
	ZeroMemory(&d3d, sizeof(D3DINFO));

	// Set up Direct3D interface object
	d3d.lpD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d.lpD3D)
	{
		Con_PrintError("Could not create Direct3D9 object");
		return false;
	}

	// Get the number of devices/video cards in the system
	d3d.NumDrivers = d3d.lpD3D->GetAdapterCount();

	Con_PrintMessage("\t Found " + Util::IntToString(d3d.NumDrivers) + " video adapter(s)");

	// Get adapter information for all available devices (vid card name, etc)
	for (uint32_t driverIndex = 0; driverIndex < d3d.NumDrivers; driverIndex++)
	{
		LastError = d3d.lpD3D->GetAdapterIdentifier(driverIndex, /*D3DENUM_WHQL_LEVEL*/0, &d3d.Driver[driverIndex].AdapterInfo);
		if (FAILED(LastError))
		{
			LogDxError(LastError, __LINE__, __FILE__);
			return false;
		}
	}

	Con_PrintMessage("\t Using device: " + std::string(d3d.Driver[defaultDevice].AdapterInfo.Description) + " on display " + std::string(d3d.Driver[defaultDevice].AdapterInfo.DeviceName));

	// chose an adapter format. take from desktop if windowed.
	if (windowed)
	{
		D3DDISPLAYMODE d3ddm;
		LastError = d3d.lpD3D->GetAdapterDisplayMode(defaultDevice, &d3ddm);
		if (FAILED(LastError))
		{
			Con_PrintError("GetAdapterDisplayMode call failed");
			LogDxError(LastError, __LINE__, __FILE__);
			return false;
		}
		else
		{
			SelectedAdapterFormat = d3ddm.Format;
		}
	}
	else
	{
		// i assume we shouldn't be guessing this?
		SelectedAdapterFormat = D3DFMT_X8R8G8B8;
	}

	// count number of display formats in our array
	uint32_t numDisplayFormats = sizeof(DisplayFormats) / sizeof(DisplayFormats[0]);
	uint32_t numFomats = 0;

	// loop through all the devices, getting the list of formats available for each
	for (uint32_t thisDevice = 0; thisDevice < d3d.NumDrivers; thisDevice++)
	{
		d3d.Driver[thisDevice].NumModes = 0;

		for (uint32_t displayFormatIndex = 0; displayFormatIndex < numDisplayFormats; displayFormatIndex++)
		{
			// get the number of display moves available on this adapter for this particular format
			uint32_t numAdapterModes = d3d.lpD3D->GetAdapterModeCount(thisDevice, DisplayFormats[displayFormatIndex]);

			for (uint32_t modeIndex = 0; modeIndex < numAdapterModes; modeIndex++)
			{
				D3DDISPLAYMODE DisplayMode;

				// does this adapter support the requested format?
				d3d.lpD3D->EnumAdapterModes(thisDevice, DisplayFormats[displayFormatIndex], modeIndex, &DisplayMode);

				// Filter out low-resolution modes
				if (DisplayMode.Width < 640 || DisplayMode.Height < 480)
					continue;

				uint32_t j = 0;

				// Check if the mode already exists (to filter out refresh rates)
				for (; j < d3d.Driver[thisDevice].NumModes; j++)
				{
					if ((d3d.Driver[thisDevice].DisplayMode[j].Width  == DisplayMode.Width) &&
						(d3d.Driver[thisDevice].DisplayMode[j].Height == DisplayMode.Height) &&
						(d3d.Driver[thisDevice].DisplayMode[j].Format == DisplayMode.Format))
							break;
				}

				// If we found a new mode, add it to the list of modes
				if (j == d3d.Driver[thisDevice].NumModes) // we made it all the way throught the list and didn't find a match
				{
					assert (DisplayMode.Width != 0);
					assert (DisplayMode.Height != 0);

					d3d.Driver[thisDevice].DisplayMode[d3d.Driver[thisDevice].NumModes].Width       = DisplayMode.Width;
					d3d.Driver[thisDevice].DisplayMode[d3d.Driver[thisDevice].NumModes].Height      = DisplayMode.Height;
					d3d.Driver[thisDevice].DisplayMode[d3d.Driver[thisDevice].NumModes].Format      = DisplayMode.Format;
					d3d.Driver[thisDevice].DisplayMode[d3d.Driver[thisDevice].NumModes].RefreshRate = DisplayMode.RefreshRate;

					d3d.Driver[thisDevice].NumModes++;

					uint32_t f = 0;

					// Check if the mode's format already exists
					for (; f < numFomats; f++ )
					{
						if (DisplayMode.Format == d3d.Driver[thisDevice].Formats[f])
							break;
					}

					// If the format is new, add it to the list
					if (f == numFomats)
						d3d.Driver[thisDevice].Formats[numFomats++] = DisplayMode.Format;
				}
			}
		}
	}

	// set CurrentVideoMode variable to index display mode that matches user requested settings
	for (uint32_t i = 0; i < d3d.Driver[d3d.CurrentDriver].NumModes; i++)
	{
		if ((width == d3d.Driver[d3d.CurrentDriver].DisplayMode[i].Width)
		 &&(height == d3d.Driver[d3d.CurrentDriver].DisplayMode[i].Height))
		{
			d3d.CurrentVideoMode = i;
			break;
		}
	}

	// check that the resolution and colour depth the user wants is supported
	bool gotOne = false;
	bool gotValidFormats = false;

	for (uint32_t i = 0; i < (sizeof(DisplayFormats) / sizeof(DisplayFormats[0])); i++)
	{
		for (uint32_t j = 0; j < d3d.Driver[defaultDevice].NumModes; j++)
		{
			// found a usable mode
			if ((d3d.Driver[defaultDevice].DisplayMode[j].Width == width) && (d3d.Driver[defaultDevice].DisplayMode[j].Height == height))
			{
				// try find a matching depth buffer format
				for (uint32_t d = 0; d < (sizeof(DepthFormats) / sizeof(DepthFormats[0])); d++)
				{
					LastError = d3d.lpD3D->CheckDeviceFormat( d3d.CurrentDriver,
												D3DDEVTYPE_HAL,
												SelectedAdapterFormat,
												D3DUSAGE_DEPTHSTENCIL,
												D3DRTYPE_SURFACE,
												DepthFormats[d]);

					// if the format wont work with this depth buffer, try another format
					if (FAILED(LastError))
						continue;

					LastError = d3d.lpD3D->CheckDepthStencilMatch( d3d.CurrentDriver,
											D3DDEVTYPE_HAL,
											SelectedAdapterFormat,
											DisplayFormats[i],
											DepthFormats[d]);

					// we got valid formats
					if (!FAILED(LastError))
					{
						SelectedDepthFormat = DepthFormats[d];
						SelectedBackbufferFormat = DisplayFormats[i];
						gotValidFormats = true;
						gotOne = true;

						// fix this..
						goto gotValidFormats;
					}
				}
			}
		}
	}

	// we jump to here when we have picked all valid formats
	gotValidFormats:

	if (!gotOne)
	{
		Con_PrintError("Couldn't find match for user requested resolution");

		// set some default values?
		width = 800;
		height = 600;
		SelectedBackbufferFormat = DisplayFormats32[0]; // use the first in the list
	}

	// set up the presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory (&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.hDeviceWindow = hWndMain;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = SelectedDepthFormat;
	d3dpp.BackBufferFormat = SelectedBackbufferFormat;

	d3dpp.Windowed = (windowed) ? TRUE : FALSE;

	d3dpp.BackBufferWidth  = width;
	d3dpp.BackBufferHeight = height;

	// setting this to interval one will cap the framerate to monitor refresh
	// the timer goes a bit mad if this isnt capped!
	if (useVSync)
	{
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		Con_PrintMessage("V-Sync is on");
	}
	else
	{
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		Con_PrintMessage("V-Sync is off");
	}

	// resize the win32 window to match our d3d backbuffer size
	ChangeWindowsSize(d3dpp.BackBufferWidth, d3dpp.BackBufferHeight);

	d3dpp.BackBufferCount = 1;

	if (useTripleBuffering)
	{
		d3dpp.BackBufferCount = 2;
		d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
		Con_PrintMessage("Using triple buffering");
	}

	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	// store the index of the driver we want to use
	d3d.CurrentDriver = defaultDevice;

#if 0 // NVidia PerfHUD
	UINT adapter_to_use;

	for (UINT Adapter = 0; Adapter < d3d.lpD3D->GetAdapterCount(); Adapter++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		HRESULT Res;

	Res = d3d.lpD3D->GetAdapterIdentifier(Adapter, 0, &Identifier);

	if (strstr(Identifier.Description, "PerfHUD") != 0)
	{
		adapter_to_use = Adapter;

		d3d.lpD3D->CreateDevice(Adapter, D3DDEVTYPE_REF, hWndMain,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3d.lpD3DDevice);
	}
	else
	{
		LastError = d3d.lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWndMain,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3d.lpD3DDevice);
	}
}
#endif

// #define USEREFDEVICE

#ifdef USEREFDEVICE
	LastError = d3d.lpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWndMain,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3d.lpD3DDevice);
#else

	// create D3DCREATE_PUREDEVICE
	LastError = d3d.lpD3D->CreateDevice(defaultDevice, D3DDEVTYPE_HAL, hWndMain,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE, &d3dpp, &d3d.lpD3DDevice);

	if (FAILED(LastError))
	{
		LastError = d3d.lpD3D->CreateDevice(defaultDevice, D3DDEVTYPE_HAL, hWndMain,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3d.lpD3DDevice);
	}
	if (FAILED(LastError))
	{
		LastError = d3d.lpD3D->CreateDevice(defaultDevice, D3DDEVTYPE_HAL, hWndMain,
			D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp, &d3d.lpD3DDevice);
	}
	if (FAILED(LastError))
	{
		LastError = d3d.lpD3D->CreateDevice(defaultDevice, D3DDEVTYPE_HAL, hWndMain,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3d.lpD3DDevice);
	}
#endif
	if (FAILED(LastError))
	{
		Con_PrintError("Could not create Direct3D device");
		LogDxError(LastError, __LINE__, __FILE__);
		return false;
	}

	// get device capabilities
	D3DCAPS9 d3dCaps;
	d3d.lpD3DDevice->GetDeviceCaps(&d3dCaps);

	// set this to true initially
	d3d.supportsShaders = true;

	// check pixel shader support
	if (d3dCaps.PixelShaderVersion < (D3DPS_VERSION(2,0)))
	{
		Con_PrintError("Device does not support Pixel Shader version 2.0 or greater");
		d3d.supportsShaders = false;
	}

	// check vertex shader support
	if (d3dCaps.VertexShaderVersion < (D3DVS_VERSION(2,0)))
	{
		Con_PrintError("Device does not support Vertex Shader version 2.0 or greater");
		d3d.supportsShaders = false;
	}

	// check and remember if we have dynamic texture support
	if (d3dCaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES)
	{
		d3d.supportsDynamicTextures = true;
	}
	else
	{
		d3d.supportsDynamicTextures = false;
		Con_PrintError("Device does not support D3DUSAGE_DYNAMIC");
	}

	// check texture support
	if (d3dCaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		Con_PrintError("Device requires square textures");
	}

	if (d3dCaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		Con_PrintError("Device requires power of two textures");

		// check conditonal support
		if (d3dCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
		{
			Con_PrintError("Device has conditional power of two textures support");
		}
	}

	// check max texture size
	Con_PrintMessage("Max texture size: " + Util::IntToString(d3dCaps.MaxTextureWidth));

	// Log resolution set
	Con_PrintMessage("\t Resolution set: " + Util::IntToString(d3dpp.BackBufferWidth) + " x " + Util::IntToString(d3dpp.BackBufferHeight));

	// Log format set
	switch (d3dpp.BackBufferFormat)
	{
		case D3DFMT_A8R8G8B8:
			Con_PrintMessage("\t Format set: 32bit - D3DFMT_A8R8G8B8");
			break;
		case D3DFMT_X8R8G8B8:
			Con_PrintMessage("\t Format set: 32bit - D3DFMT_X8R8G8B8");
			break;
		case D3DFMT_R8G8B8:
			Con_PrintMessage("\t Format set: 32bit - D3DFMT_R8G8B8");
			break;
		case D3DFMT_A1R5G5B5:
			Con_PrintMessage("\t Format set: 16bit - D3DFMT_A1R5G5B5");
			break;
		case D3DFMT_R5G6B5:
			Con_PrintMessage("\t Format set: 16bit - D3DFMT_R5G6B5");
			break;
		case D3DFMT_X1R5G5B5:
			Con_PrintMessage("\t Format set: 16bit - D3DFMT_X1R5G5B5");
			break;
		default:
			Con_PrintMessage("\t Format set: Unknown");
			break;
	}

	// Log depth buffer format set
	switch (d3dpp.AutoDepthStencilFormat)
	{
		case D3DFMT_D24S8:
			Con_PrintMessage("\t Depth Format set: 24bit and 8bit stencil - D3DFMT_D24S8");
			break;
		case D3DFMT_D24X8:
			Con_PrintMessage("\t Depth Format set: 24bit and 0bit stencil - D3DFMT_D24X8");
			break;
		case D3DFMT_D24FS8:
			Con_PrintMessage("\t Depth Format set: 24bit and 8bit stencil - D3DFMT_D32");
			break;
		case D3DFMT_D32:
			Con_PrintMessage("\t Depth Format set: 32bit and 0bit stencil - D3DFMT_D32");
			break;
		default:
			Con_PrintMessage("\t Depth Format set: Unknown");
			break;
	}

	ZeroMemory (&d3d.D3DViewport, sizeof(d3d.D3DViewport));
	d3d.D3DViewport.X = 0;
	d3d.D3DViewport.Y = 0;
	d3d.D3DViewport.Width = width;
	d3d.D3DViewport.Height = height;
	d3d.D3DViewport.MinZ = 0.0f;
	d3d.D3DViewport.MaxZ = 1.0f;

	LastError = d3d.lpD3DDevice->SetViewport(&d3d.D3DViewport);
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
	}

	ScreenDescriptorBlock.SDB_Width     = width;
	ScreenDescriptorBlock.SDB_Height    = height;
//	ScreenDescriptorBlock.SDB_Depth		= colourDepth;
	ScreenDescriptorBlock.SDB_Size      = width*height;
	ScreenDescriptorBlock.SDB_CentreX   = width/2;
	ScreenDescriptorBlock.SDB_CentreY   = height/2;

	ScreenDescriptorBlock.SDB_ProjX     = width/2;
	ScreenDescriptorBlock.SDB_ProjY     = height/2;

	ScreenDescriptorBlock.SDB_ClipLeft  = 0;
	ScreenDescriptorBlock.SDB_ClipRight = width;
	ScreenDescriptorBlock.SDB_ClipUp    = 0;
	ScreenDescriptorBlock.SDB_ClipDown  = height;

	// use an offset for hud items to account for tv safe zones. just use width for now. 15%?
	if (0)
	{
		ScreenDescriptorBlock.SDB_SafeZoneWidthOffset = (width / 100) * 15;
		ScreenDescriptorBlock.SDB_SafeZoneHeightOffset = (height / 100) * 15;
	}
	else
	{
		ScreenDescriptorBlock.SDB_SafeZoneWidthOffset  = 0;
		ScreenDescriptorBlock.SDB_SafeZoneHeightOffset = 0;
	}

	// save a copy of the presentation parameters for use later (device reset, resolution/depth change)
	d3d.d3dpp = d3dpp;

	// set field of view (this needs to be set differently for alien but 77 seems ok for marine and predator
	d3d.fieldOfView = kDefaultFOV;

	// set aspect ratio
	d3d.aspectRatio = (float)width / (float)height;

	SetTransforms();

	Con_AddCommand("r_togglewireframe", ToggleWireframe);
	Con_AddCommand("r_setfov", SetFov);
	Con_AddCommand("r_texlist", Tex_ListTextures);
	Con_AddCommand("fcull", ToggleFrustumCull);

	// create vertex declarations
	d3d.mainDecl = new VertexDeclaration;
	d3d.mainDecl->Add(0, VDTYPE_FLOAT3, VDMETHOD_DEFAULT, VDUSAGE_POSITION, 0);
	d3d.mainDecl->Add(0, VDTYPE_COLOR,  VDMETHOD_DEFAULT, VDUSAGE_COLOR,    0);
	d3d.mainDecl->Add(0, VDTYPE_COLOR,  VDMETHOD_DEFAULT, VDUSAGE_COLOR,    1);
	d3d.mainDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD, 0);
	d3d.mainDecl->Create();

	d3d.orthoDecl = new VertexDeclaration;
	d3d.orthoDecl->Add(0, VDTYPE_FLOAT3, VDMETHOD_DEFAULT, VDUSAGE_POSITION, 0);
	d3d.orthoDecl->Add(0, VDTYPE_COLOR,  VDMETHOD_DEFAULT, VDUSAGE_COLOR,    0);
	d3d.orthoDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD, 0);
	d3d.orthoDecl->Create();

	d3d.decalDecl = new VertexDeclaration;
	d3d.decalDecl->Add(0, VDTYPE_FLOAT3, VDMETHOD_DEFAULT, VDUSAGE_POSITION, 0);
	d3d.decalDecl->Add(0, VDTYPE_COLOR,  VDMETHOD_DEFAULT, VDUSAGE_COLOR,    0);
	d3d.decalDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD, 0);
	d3d.decalDecl->Create();

	d3d.fmvDecl = new VertexDeclaration;
	d3d.fmvDecl->Add(0, VDTYPE_FLOAT3, VDMETHOD_DEFAULT, VDUSAGE_POSITION, 0);
	d3d.fmvDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD, 0);
	d3d.fmvDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD, 1);
	d3d.fmvDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD, 2);
	d3d.fmvDecl->Create();

	d3d.tallTextDecl = new VertexDeclaration;
	d3d.tallTextDecl->Add(0, VDTYPE_FLOAT3, VDMETHOD_DEFAULT, VDUSAGE_POSITION, 0);
	d3d.tallTextDecl->Add(0, VDTYPE_COLOR,  VDMETHOD_DEFAULT, VDUSAGE_COLOR,    0);
	d3d.tallTextDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD, 0);
	d3d.tallTextDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD, 1);
	d3d.tallTextDecl->Create();

	d3d.particleDecl = new VertexDeclaration;
	d3d.particleDecl->Add(0, VDTYPE_FLOAT3, VDMETHOD_DEFAULT, VDUSAGE_POSITION, 0);
	d3d.particleDecl->Add(0, VDTYPE_COLOR,  VDMETHOD_DEFAULT, VDUSAGE_COLOR,    0);
	d3d.particleDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD, 0);
	d3d.particleDecl->Create();

	// rhw pretransformed
	d3d.rhwDecl = new VertexDeclaration;
	d3d.rhwDecl->Add(0, VDTYPE_FLOAT4, VDMETHOD_DEFAULT, VDUSAGE_POSITION, 0);
	d3d.rhwDecl->Add(0, VDTYPE_COLOR,  VDMETHOD_DEFAULT, VDUSAGE_COLOR,     0);
	d3d.rhwDecl->Add(0, VDTYPE_FLOAT2, VDMETHOD_DEFAULT, VDUSAGE_TEXCOORD,  0);
	d3d.rhwDecl->Create();

	r_Texture whiteTexture;
	r_Texture missingTexture;

	// create a hot pink texture to use when a texture resource can't be loaded
	{
		LastError = d3d.lpD3DDevice->CreateTexture(1, 1, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &missingTexture, NULL);
		if (FAILED(LastError))
		{
			Con_PrintError("Could not create missing texture signifying texture");
			LogDxError(LastError, __LINE__, __FILE__);
		}

		D3DLOCKED_RECT lock;
		LastError = missingTexture->LockRect(0, &lock, NULL, 0);
		if (FAILED(LastError))
		{
			Con_PrintError("Could not lock missing texture signifying texture");
			LogDxError(LastError, __LINE__, __FILE__);
		}
		else
		{
			// set pixel to hot pink
			//memset(lock.pBits, 255, lock.Pitch);
			(*(reinterpret_cast<uint32_t*>(lock.pBits))) = D3DCOLOR_XRGB(255, 0, 255);

			missingTexture->UnlockRect(0);

			// should we just add it even if it fails?
			MISSING_TEXTURE = Tex_AddTexture("missing", missingTexture, 1, 1, 32, TextureUsage_Normal);
		}
	}

	// create a 1x1 resolution white texture to set to shader for sampling when we don't want to texture an object (eg what was NULL texture in fixed function pipeline)
	{
		LastError = d3d.lpD3DDevice->CreateTexture(1, 1, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &whiteTexture, NULL);
		if (FAILED(LastError))
		{
			Con_PrintError("Could not create white texture for shader sampling");
			LogDxError(LastError, __LINE__, __FILE__);
		}

		D3DLOCKED_RECT lock;
		LastError = whiteTexture->LockRect(0, &lock, NULL, 0);
		if (FAILED(LastError))
		{
			Con_PrintError("Could not lock white texture for shader sampling");
			LogDxError(LastError, __LINE__, __FILE__);
		}
		else
		{
			// set pixel to white
			(*(reinterpret_cast<uint32_t*>(lock.pBits))) = D3DCOLOR_XRGB(255, 255, 255);

			whiteTexture->UnlockRect(0);

			// should we just add it even if it fails?
			NO_TEXTURE = Tex_AddTexture("white", whiteTexture, 1, 1, 32, TextureUsage_Normal);
		}
	}

	setTextureArray.resize(kMaxTextureStages);

	d3d.effectSystem = new EffectManager;

	d3d.mainEffect  = d3d.effectSystem->Add("main", "vertex.vsh", "pixel.psh", d3d.mainDecl);
	d3d.orthoEffect = d3d.effectSystem->Add("ortho", "orthoVertex.vsh", "orthoPixel.psh", d3d.orthoDecl);
	d3d.decalEffect = d3d.effectSystem->Add("decal", "decal.vsh", "decal.psh", d3d.decalDecl);
	d3d.fmvEffect   = d3d.effectSystem->Add("fmv", "fmvVertex.vsh", "fmvPixel.psh", d3d.fmvDecl);
	d3d.tallTextEffect = d3d.effectSystem->Add("tallText", "tallText.vsh", "tallText.psh", d3d.tallTextDecl);
	d3d.particleEffect = d3d.effectSystem->Add("particle", "particle.vsh", "particle.psh", d3d.particleDecl);
	d3d.rhwEffect   = d3d.effectSystem->Add("rhw", "rhw.vsh", "rhw.psh", d3d.rhwDecl);

	// we should bail out if the shaders can't be loaded
	if ((d3d.mainEffect  == kNullShaderID)    ||
		(d3d.orthoEffect == kNullShaderID)    ||
		(d3d.decalEffect == kNullShaderID)    ||
		(d3d.particleEffect == kNullShaderID) ||
		(d3d.fmvEffect   == kNullShaderID)    || // make this optional?
		(d3d.tallTextEffect == kNullShaderID))   // make this optional?
	{
		return false;
	}

	// create vertex and index buffers
	CreateVolatileResources();

	RenderListInit();

	Con_PrintMessage("Initialised Direct3D9 succesfully");

	return true;
}

void SetTransforms()
{
	// create an identity matrix
	D3DXMatrixIdentity(&d3d.matIdentity);

	D3DXMatrixIdentity(&d3d.matView);

	// set up orthographic projection matrix
	D3DXMatrixOrthoLH(&d3d.matOrtho, 2.0f, -2.0f, 1.0f, 10.0f);

	// set up projection matrix
	D3DXMatrixPerspectiveFovLH(&d3d.matProjection, D3DXToRadian(d3d.fieldOfView), (float)ScreenDescriptorBlock.SDB_Width / (float)ScreenDescriptorBlock.SDB_Height, 64.0f, 1000000.0f);

	// set up a viewport transform matrix
	d3d.matViewPort = d3d.matIdentity;

	d3d.matViewPort._11 = (float)(ScreenDescriptorBlock.SDB_Width / 2);
	d3d.matViewPort._22 = (float)((-ScreenDescriptorBlock.SDB_Height) / 2);
	d3d.matViewPort._33 = (1.0f - 0.0f);
	d3d.matViewPort._41 = (0.0f + d3d.matViewPort._11); // dwX + dwWidth / 2
	d3d.matViewPort._42 = (float)(ScreenDescriptorBlock.SDB_Height / 2) + 0;
	d3d.matViewPort._43 = 0.0f; // minZ
	d3d.matViewPort._44 = 1.0f;
}

void FlipBuffers()
{
	LastError = d3d.lpD3DDevice->Present(NULL, NULL, NULL, NULL);
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
	}
}

void ReleaseDirect3D()
{
	Font_Release();

	Tex_DeInit();

	// release dynamic vertex buffers, index buffers and textures
	ReleaseVolatileResources();

	// release vertex declarations
	delete d3d.mainDecl;
	delete d3d.orthoDecl;
	delete d3d.decalDecl;
	delete d3d.fmvDecl;
	delete d3d.tallTextDecl;
	delete d3d.particleDecl;
	delete d3d.rhwDecl;

	// clean up render list classes
	RenderListDeInit();

	delete d3d.effectSystem;

	// release device
	SAFE_RELEASE(d3d.lpD3DDevice);
	LogString("Releasing Direct3D9 device...");

	// release object
	SAFE_RELEASE(d3d.lpD3D);
	LogString("Releasing Direct3D9 object...");

	// release Direct Input stuff
	ReleaseDirectKeyboard();
	ReleaseDirectMouse();
	ReleaseDirectInput();

	// find a better place to put this
	Config_Save();
}

void ReleaseAvPTexture(AVPTEXTURE *texture)
{
	delete[] texture->buffer;
	delete texture;
}

void ChangeTranslucencyMode(enum TRANSLUCENCY_TYPE translucencyRequired)
{
	if (CurrentRenderStates.TranslucencyMode == translucencyRequired)
		return;

	CurrentRenderStates.TranslucencyMode = translucencyRequired;

	switch (CurrentRenderStates.TranslucencyMode)
	{
	 	case TRANSLUCENCY_OFF:
		{
			if (TRIPTASTIC_CHEATMODE || MOTIONBLUR_CHEATMODE)
			{
				if (D3DAlphaBlendEnable != TRUE)
				{
					d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					D3DAlphaBlendEnable = TRUE;
				}
				if (D3DSrcBlend != D3DBLEND_INVSRCALPHA)
				{
					d3d.lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA);
					D3DSrcBlend = D3DBLEND_INVSRCALPHA;
				}
				if (D3DDestBlend != D3DBLEND_SRCALPHA)
				{
					d3d.lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
					D3DDestBlend = D3DBLEND_SRCALPHA;
				}
			}
			else
			{
				if (D3DAlphaBlendEnable != FALSE)
				{
					d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
					D3DAlphaBlendEnable = FALSE;
				}
				if (D3DSrcBlend != D3DBLEND_ONE)
				{
					d3d.lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					D3DSrcBlend = D3DBLEND_ONE;
				}
				if (D3DDestBlend != D3DBLEND_ZERO)
				{
					d3d.lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					D3DDestBlend = D3DBLEND_ZERO;
				}
			}
			break;
		}
	 	case TRANSLUCENCY_NORMAL:
		{
			if (D3DAlphaBlendEnable != TRUE)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				D3DAlphaBlendEnable = TRUE;
			}
			if (D3DSrcBlend != D3DBLEND_SRCALPHA)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				D3DSrcBlend = D3DBLEND_SRCALPHA;
			}
			if (D3DDestBlend != D3DBLEND_INVSRCALPHA)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				D3DDestBlend = D3DBLEND_INVSRCALPHA;
			}
			break;
		}
	 	case TRANSLUCENCY_COLOUR:
		{
			if (D3DAlphaBlendEnable != TRUE)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				D3DAlphaBlendEnable = TRUE;
			}
			if (D3DSrcBlend != D3DBLEND_ZERO)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				D3DSrcBlend = D3DBLEND_ZERO;
			}
			if (D3DDestBlend != D3DBLEND_SRCCOLOR)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
				D3DDestBlend = D3DBLEND_SRCCOLOR;
			}
			break;
		}
	 	case TRANSLUCENCY_INVCOLOUR:
		{
			if (D3DAlphaBlendEnable != TRUE)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				D3DAlphaBlendEnable = TRUE;
			}
			if (D3DSrcBlend != D3DBLEND_ZERO)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				D3DSrcBlend = D3DBLEND_ZERO;
			}
			if (D3DDestBlend != D3DBLEND_INVSRCCOLOR)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
				D3DDestBlend = D3DBLEND_INVSRCCOLOR;
			}
			break;
		}
  		case TRANSLUCENCY_GLOWING:
		{
			if (D3DAlphaBlendEnable != TRUE)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				D3DAlphaBlendEnable = TRUE;
			}
			if (D3DSrcBlend != D3DBLEND_SRCALPHA)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				D3DSrcBlend = D3DBLEND_SRCALPHA;
			}
			if (D3DDestBlend != D3DBLEND_ONE)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				D3DDestBlend = D3DBLEND_ONE;
			}
			break;
		}
  		case TRANSLUCENCY_DARKENINGCOLOUR:
		{
			if (D3DAlphaBlendEnable != TRUE)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				D3DAlphaBlendEnable = TRUE;
			}
			if (D3DSrcBlend != D3DBLEND_INVDESTCOLOR)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
				D3DSrcBlend = D3DBLEND_INVDESTCOLOR;
			}

			if (D3DDestBlend != D3DBLEND_ZERO)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
				D3DDestBlend = D3DBLEND_ZERO;
			}
			break;
		}
/*
		case TRANSLUCENCY_JUSTSETZ:
		{
			if (D3DAlphaBlendEnable != TRUE)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				D3DAlphaBlendEnable = TRUE;
			}
			if (D3DSrcBlend != D3DBLEND_ZERO)
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
				D3DSrcBlend = D3DBLEND_ZERO;
			}
			if (D3DDestBlend != D3DBLEND_ONE) 
			{
				d3d.lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				D3DDestBlend = D3DBLEND_ONE;
			}
		}
*/
		default: break;
	}
}

void ChangeZWriteEnable(enum ZWRITE_ENABLE zWriteEnable)
{
	if (CurrentRenderStates.ZWriteEnable == zWriteEnable)
		return;

	if (zWriteEnable == ZWRITE_ENABLED)
	{
		LastError = d3d.lpD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		if (FAILED(LastError))
		{
			OutputDebugString("D3DRS_ZWRITEENABLE D3DZB_TRUE failed\n");
		}
	}
	else if (zWriteEnable == ZWRITE_DISABLED)
	{
		LastError = d3d.lpD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		if (FAILED(LastError))
		{
			OutputDebugString("D3DRS_ZWRITEENABLE D3DZB_FALSE failed\n");
			OutputDebugString("DISABLING Z WRITES\n");
		}
	}
	else
	{
		LOCALASSERT("Unrecognized ZWriteEnable mode"==0);
	}

	CurrentRenderStates.ZWriteEnable = zWriteEnable;
}

void ChangeTextureAddressMode(uint32_t samplerIndex, enum TEXTURE_ADDRESS_MODE textureAddressMode)
{
	if (CurrentRenderStates.TextureAddressMode[samplerIndex] == textureAddressMode)
		return;

	CurrentRenderStates.TextureAddressMode[samplerIndex] = textureAddressMode;

	if (textureAddressMode == TEXTURE_WRAP)
	{
		// wrap texture addresses
		LastError = d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		if (FAILED(LastError))
		{
			OutputDebugString("D3DSAMP_ADDRESSU Wrap fail\n");
		}

		LastError = d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		if (FAILED(LastError))
		{
			OutputDebugString("D3DSAMP_ADDRESSV Wrap fail\n");
		}

		LastError = d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		if (FAILED(LastError))
		{
			OutputDebugString("D3DSAMP_ADDRESSW Wrap fail\n");
		}
	}
	else if (textureAddressMode == TEXTURE_CLAMP)
	{
		// clamp texture addresses
		LastError = d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		if (FAILED(LastError))
		{
			OutputDebugString("D3DSAMP_ADDRESSU Clamp fail\n");
		}

		LastError = d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		if (FAILED(LastError))
		{
			OutputDebugString("D3DSAMP_ADDRESSV Clamp fail\n");
		}

		LastError = d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);
		if (FAILED(LastError))
		{
			OutputDebugString("D3DSAMP_ADDRESSW Clamp fail\n");
		}
	}
}

void ChangeFilteringMode(uint32_t samplerIndex, enum FILTERING_MODE_ID filteringRequired)
{
	if (CurrentRenderStates.FilteringMode[samplerIndex] == filteringRequired)
		return;

	CurrentRenderStates.FilteringMode[samplerIndex] = filteringRequired;

	switch (CurrentRenderStates.FilteringMode[samplerIndex])
	{
		case FILTERING_BILINEAR_OFF:
		{
			d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			break;
		}
		case FILTERING_BILINEAR_ON:
		{
			d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3d.lpD3DDevice->SetSamplerState(samplerIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			break;
		}
		default:
		{
			LOCALASSERT("Unrecognized filtering mode"==0);
			OutputDebugString("Unrecognized filtering mode\n");
			break;
		}
	}
}

void ToggleWireframe()
{
	if (CurrentRenderStates.WireFrameModeIsOn)
	{
		CheckWireFrameMode(0);
	}
	else
	{
		CheckWireFrameMode(1);
	}
}

bool SetRenderStateDefaults()
{
	const int kNumStages = 8; // TODO, dont hardcode this?

	for (int i = 0; i < kNumStages; i++)
	{
		d3d.lpD3DDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3d.lpD3DDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3d.lpD3DDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	}

#if 0 // anisotropic
	d3d.lpD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	d3d.lpD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	d3d.lpD3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

	d3d.lpD3DDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 16);
#endif

/*
	d3d.lpD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	d3d.lpD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	d3d.lpD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);

	d3d.lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);
	d3d.lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	d3d.lpD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);

	d3d.lpD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_DISABLE);
	d3d.lpD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);

	d3d.lpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	d3d.lpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	d3d.lpD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
*/
	float alphaRef = 0.5f;
	d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHAREF,			*((DWORD*)&alphaRef));//(DWORD)0.5);
	d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHAFUNC,		D3DCMP_GREATER);
	d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);

	d3d.lpD3DDevice->SetRenderState(D3DRS_CULLMODE,			D3DCULL_NONE);
	d3d.lpD3DDevice->SetRenderState(D3DRS_CLIPPING,			TRUE);
	d3d.lpD3DDevice->SetRenderState(D3DRS_LIGHTING,			FALSE);
	d3d.lpD3DDevice->SetRenderState(D3DRS_SPECULARENABLE,	TRUE);
	
	{
		// set transparency to TRANSLUCENCY_OFF
		d3d.lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		D3DAlphaBlendEnable = FALSE;
		
		d3d.lpD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		D3DSrcBlend = D3DBLEND_ONE;
		
		d3d.lpD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		D3DDestBlend = D3DBLEND_ZERO;

		// make sure render state tracking reflects above setting
		CurrentRenderStates.TranslucencyMode = TRANSLUCENCY_OFF;
	}

	{
		// enable bilinear filtering (FILTERING_BILINEAR_ON)
		for (int i = 0; i < kNumStages; i++)
		{
			d3d.lpD3DDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3d.lpD3DDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

			// make sure render state tracking reflects above setting
			CurrentRenderStates.FilteringMode[i] = FILTERING_BILINEAR_ON;
		}
	}

	{
		// set texture addressing mode to clamp (TEXTURE_CLAMP)
		for (int i = 0; i < kNumStages; i++)
		{
			d3d.lpD3DDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			d3d.lpD3DDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			d3d.lpD3DDevice->SetSamplerState(i, D3DSAMP_ADDRESSW, D3DTADDRESS_CLAMP);

			// make sure render state tracking reflects above setting
			CurrentRenderStates.TextureAddressMode[i] = TEXTURE_CLAMP;
		}
	}

	{
		// enable z-buffer
		d3d.lpD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

		// enable z writes (already on by default)
		d3d.lpD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

		// make sure render state tracking reflects above setting
		CurrentRenderStates.ZWriteEnable = ZWRITE_ENABLED;

		// set less + equal z buffer test
		d3d.lpD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		D3DZFunc = D3DCMP_LESSEQUAL;
	}

	CurrentRenderStates.FogIsOn = 0;
	CurrentRenderStates.WireFrameModeIsOn = 0;

	return true;
}
