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
#ifdef WIN32
#include <d3dx9.h>
#endif
#include "logString.h"
#include "stdint.h"
#include "font2.h"
#include "textureManager.h"

static HRESULT LastError;

extern "C" {
extern void New_D3D_HUDQuad_Output(int textureID, int x, int y, int width, int height, int *uvArray, uint32_t colour, enum FILTERING_MODE_ID filteringType);
}

struct Font
{
	enum type;
	int textureWidth;
	int textureHeight;
	uint32_t	textureID;
	D3DXIMAGE_INFO imageInfo;
	int fontWidths[256];
	int blockWidth;
	int blockHeight;
};

static Font Fonts[NUM_FONT_TYPES];

void Font_Release()
{
/*
	for (int i = 0; i < NUM_FONT_TYPES; i++)
	{
		if (Fonts[i].texture)
		{
			Fonts[i].texture->Release();
			Fonts[i].texture = NULL;
		}
	}
*/
}

void Font_Init()
{
	D3DTEXTURE texture;
/*
	// load the small font texture
	if (Fonts[FONT_SMALL].texture)
	{
		Fonts[FONT_SMALL].texture->Release();
		Fonts[FONT_SMALL].texture = NULL;
	}
*/
	LastError = D3DXCreateTextureFromFileEx
	(
		d3d.lpD3DDevice, 
		"aa_font_grid_test.png", 
		D3DX_DEFAULT,			// width
		D3DX_DEFAULT,			// height
		1,						// mip levels
		0,						// usage	
		D3DFMT_UNKNOWN,			// format
		D3DPOOL_MANAGED,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		&Fonts[FONT_SMALL].imageInfo,
		NULL,
		&texture
	);
	
	if (FAILED(LastError))
	{
		LogDxError(LastError, __LINE__, __FILE__);
		return;
	}
	
	// get the font widths
	D3DLOCKED_RECT lock;
	uint8_t *srcPtr = NULL;
	int c;
	Fonts[FONT_SMALL].blockWidth = Fonts[FONT_SMALL].imageInfo.Width / 16;
	Fonts[FONT_SMALL].blockHeight = Fonts[FONT_SMALL].imageInfo.Height / 16;
	
	LastError = texture->LockRect(0, &lock, NULL, NULL );
	if (FAILED(LastError)) 
	{
		texture->Release();
		LogDxError(LastError, __LINE__, __FILE__);
		return;
	}

	srcPtr = static_cast<uint8_t*> (lock.pBits);

	Fonts[FONT_SMALL].fontWidths[32] = 12; // size of space character

	for (c = 33; c < 255; c++) 
	{
		int x, y;

//		int x1 = 1+((c-32)&15) * Fonts[FONT_SMALL].blockWidth;
//		int y1 = 1+((c-32)>>4) * Fonts[FONT_SMALL].blockHeight;

		int row = (int)((c-32) / 16);
		int column = (c-32) % 16;

		int x2 = column * Fonts[FONT_SMALL].blockWidth;
		int y2 = row * Fonts[FONT_SMALL].blockHeight;

		Fonts[FONT_SMALL].fontWidths[c] = Fonts[FONT_SMALL].blockWidth + 1;

		for (x = x2 + Fonts[FONT_SMALL].blockWidth; x > x2; x--)
		{
			int blank = 1;

			for (y = y2; y < y2 + Fonts[FONT_SMALL].blockHeight; y++)
			{
				uint8_t *s = &srcPtr[((x * 4) + y * lock.Pitch)];

				if ((s[2] >= 0x80))// && (s[1] >= 240) && (s[2] >= 240))
				{
					blank = 0;
					break;
				}
			}

			if (blank)
			{
				Fonts[FONT_SMALL].fontWidths[c]--;
			}
			else
			{
				break;
			}
		}
	}

	LastError = texture->UnlockRect(0);
	if (FAILED(LastError)) 
	{
		LogDxError(LastError, __LINE__, __FILE__);
		return;
	}

	Fonts[FONT_SMALL].textureID = Tex_AddTexture(texture, Fonts[FONT_SMALL].imageInfo.Width, Fonts[FONT_SMALL].imageInfo.Height);
/*
	char buf[100];
	for (int i = 0; i < 256; i++)
	{
		sprintf(buf, "Fonts[FONT_SMALL].fontWidths[%d] == %d\n", i, Fonts[FONT_SMALL].fontWidths[i]);
		OutputDebugString(buf);
	}
*/
}

extern "C" {
extern char AAFontWidths[256];
}

int Font_DrawText(const char* text, int x, int y, int colour, int fontType)
{
	return 0;

	float RecipW = (1.0f / Fonts[FONT_SMALL].imageInfo.Width);
	float RecipH = (1.0f / Fonts[FONT_SMALL].imageInfo.Height);

	int sixtyThree = 32;

	while (*text)
	{
		char c = *text++;

//		int charWidth = Fonts[FONT_SMALL].fontWidths[c];
		int charWidth = AAFontWidths[c] * 2;

		c = c - 32;

		int row = (int)(c / 16); // get row 
		int column = c % 16;	 // get column from remainder value

		int tex_x = column * Fonts[FONT_SMALL].blockWidth;
		int tex_y = row * Fonts[FONT_SMALL].blockHeight;

		int uvArray[8];

		uvArray[0] = tex_x;
		uvArray[1] = tex_y + Fonts[FONT_SMALL].blockWidth;
		uvArray[2] = tex_x;
		uvArray[3] = tex_y;
		uvArray[4] = tex_x + Fonts[FONT_SMALL].blockWidth;
		uvArray[5] = tex_y + Fonts[FONT_SMALL].blockHeight;
		uvArray[6] = tex_x + Fonts[FONT_SMALL].blockWidth;
		uvArray[7] = tex_y;

		New_D3D_HUDQuad_Output(Fonts[FONT_SMALL].textureID, x, y, sixtyThree, sixtyThree, &uvArray[0], colour, FILTERING_BILINEAR_OFF);

		if (/*widthSpaced*/1)
		{
			x += charWidth;
		}
		else
		{
			x += Fonts[FONT_SMALL].blockWidth;
		}
	}

	return 0;
}

int Font_GetTextLength(const char* text)
{
	int width = 0;

	while (text && *text) 
	{
		width += Fonts[FONT_SMALL].fontWidths[*text];
		text++;
	}

	return width;
}