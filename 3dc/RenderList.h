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

#include <stdint.h>
#include <vector>
#include "renderStates.h"
#include "renderer.h"

struct RenderItem
{
	uint32_t	vertStart;
	uint32_t	vertEnd;

	uint32_t	indexStart;
	uint32_t	indexEnd;

	// various render states can be set with the below struct members
	// and then this render item can be sorted with the sortKey member
	union
	{
		struct
		{
			// should be equal in size to sortKey
			unsigned texID          : 16;
			unsigned transType      : 8;
			unsigned filterType     : 3;
			unsigned texAddressType : 2;
			unsigned zWrite         : 2;
			unsigned unused         : 1;
		};

		uint32_t sortKey;
	};

	bool operator < (const RenderItem& rhs) const {return sortKey < rhs.sortKey;}
};

class RenderList
{
	private:
		size_t		capacity;
		size_t		listIndex;	// used as list size

		uint32_t	vertexCount;
		uint32_t	indexCount;

		std::vector<RenderItem> Items;

		bool useIndicesOffset;

		void RenderList::AddIndices(uint16_t *indexArray, uint32_t a, uint32_t b, uint32_t c, uint32_t n);

	public:
		RenderList(size_t size);
		~RenderList();

	void Reset();
	void RenderList::Init(size_t size);
	size_t RenderList::GetCapacity() const { return capacity; }
	uint32_t RenderList::GetVertexCount() const { return vertexCount; }
	uint32_t RenderList::GetIndexCount() const { return indexCount; }
	uint32_t RenderList::GetSize() const { return indexCount; }
	void RenderList::Sort();
	void RenderList::AddTriangle(uint16_t *indexArray, uint32_t a, uint32_t b, uint32_t c, uint32_t n);
	void RenderList::AddItem(uint32_t numVerts, texID_t textureID, enum TRANSLUCENCY_TYPE translucencyMode, enum FILTERING_MODE_ID filteringMode = FILTERING_BILINEAR_ON, enum TEXTURE_ADDRESS_MODE textureAddress = TEXTURE_WRAP, enum ZWRITE_ENABLE = ZWRITE_ENABLED);
	void RenderList::CreateIndices(uint16_t *indexArray, uint32_t numVerts);
	void RenderList::CreateOrthoIndices(uint16_t *indexArray);
	void RenderList::Draw();

	// test
	void RenderList::EnableIndicesOffset();
	void RenderList::IncrementIndexCount(uint32_t nI);
};
