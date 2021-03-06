// Viewport2D.cpp
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.
//

#include "VideoBuffer.h"

#include "Viewport2D.h"

namespace HoverRace {
namespace VideoServices {

Viewport2D::Viewport2D()
{

	mVideoBuffer = NULL;
	mBuffer = NULL;
	mLineLen = 0;
	mXPitch = 0;
	mYPitch = 0;

	mXRes = 0;
	mYRes = 0;

}

Viewport2D::~Viewport2D()
{
}

void Viewport2D::OnMetricsChange(int /*pMetrics */ )
{
}

int Viewport2D::GetXRes() const
{
	return mXRes;
}

int Viewport2D::GetYRes() const
{
	return mYRes;
}

MR_UInt8 *Viewport2D::GetBuffer()
{
	return mBuffer;
}

int Viewport2D::GetLineLen() const
{
	return mLineLen;
}

void Viewport2D::Setup(VideoBuffer * pBuffer, int pX0, int pY0, int pSizeX, int pSizeY, int pMetrics)
{
	ASSERT(pBuffer != NULL);

	mVideoBuffer = pBuffer;

	if(pSizeX != mXRes) {
		mXRes = pSizeX;
		pMetrics |= eXSize;
	}

	if(pSizeY != mYRes) {
		mYRes = pSizeY;
		pMetrics |= eYSize;
	}

	VideoBuffer::pixelMeter_t pixelMeter = pBuffer->GetPixelMeter();

	if (pixelMeter.first != mXPitch) {
		mXPitch = pixelMeter.first;
		pMetrics |= eXPitch | eBuffer;
	}

	if (pixelMeter.second != mYPitch) {
		mYPitch = pixelMeter.second;
		pMetrics |= eYPitch | eBuffer;
	}

	if (pBuffer->GetPitch() != mLineLen) {
		mLineLen = pBuffer->GetPitch();
		pMetrics |= eBuffer;
	}

	MR_UInt8 *lNewBuffer = pBuffer->GetBuffer() + pX0 + pY0 * mLineLen;

	if(lNewBuffer != mBuffer) {
		mBuffer = lNewBuffer;
		pMetrics |= eBuffer;
	}

	OnMetricsChange(pMetrics);
}

void Viewport2D::Clear(MR_UInt8 pColor)
{
	MR_UInt8 *lBuffer = mBuffer;

	for(int lCounter = 0; lCounter < mYRes; lCounter++) {
		memset(lBuffer, pColor, mXRes);
		lBuffer += mLineLen;
	}
}

void Viewport2D::DrawPoint(int pX, int pY, MR_UInt8 pColor)
{
	if(pX >= 0 && pY >= 0 && pX < mXRes && pY < mYRes) {
		mBuffer[pY * mLineLen + pX] = pColor;
	}

}

void Viewport2D::DrawHorizontalLine(int pY, int pX0, int pX1, MR_UInt8 pColor)
{
	int lTemp;

	// Horizontal line
	if((pY >= 0) && (pY < mYRes)) {
		if(pX0 > pX1) {
			lTemp = pX0;
			pX0 = pX1;
			pX1 = lTemp;
		}

		if((pX0 < mXRes) && (pX1 >= 0)) {
			if(pX0 < 0) {
				pX0 = 0;
			}

			if(pX1 >= mXRes) {
				pX1 = mXRes - 1;
			}

			memset(mBuffer + mLineLen * pY + pX0, pColor, pX1 - pX0 + 1);
		}
	}
}

void Viewport2D::DrawLine(int pX0, int pY0, int pX1, int pY1, MR_UInt8 pColor)
{
	int lTemp;

	if(pY0 == pY1) {
		// Horizontal line
		DrawHorizontalLine(pY0, pX0, pX1, pColor);
	}
	else {
		if(pY0 > pY1) {
			lTemp = pY0;
			pY0 = pY1;
			pY1 = lTemp;

			lTemp = pX0;
			pX0 = pX1;
			pX1 = lTemp;
		}

		// Return if the line is not on screen

		if((pY1 < 0) || (pY0 >= mYRes) || ((pX0 < 0) && (pX1 < 0)) || ((pX0 >= mXRes) && (pX1 >= mXRes))
		) {
			return;
		}

		int lDXDY = ((pX1 - pX0) << 8) / (pY1 - pY0);
		int lXS = pX0 << 8;
		int lXE = lXS + lDXDY / 2;
		int lY = pY0;

		while(lY < pY1) {
			int lXStart = lXS >> 8;
			int lXEnd = lXE >> 8;

			if(lXEnd > lXStart) {
				lXEnd--;
			}
			else if(lXEnd < lXStart) {
				lXEnd++;
			}

			DrawHorizontalLine(lY, lXStart, lXEnd, pColor);

			// Prepare the next line;
			lXS = lXE;
			lXE += lDXDY;
			lY++;
		}

		// Draw the last line
		DrawHorizontalLine(lY, lXS >> 8, pX1, pColor);

	}
}

// HighLevel 2D functionalities
void Viewport2D::DrawHorizontalMeter(int pX0, int pXLen, int pY0, int pYHeight, int pXM, MR_UInt8 pColor, MR_UInt8 pBackColor)
{

	// Warning, no coordinate check
	MR_UInt8 *lDest = mBuffer + pX0 + pY0 * mLineLen;

	for(int lLine = 0; lLine < pYHeight; lLine++) {
		int lPixel;

		for(lPixel = 0; lPixel < pXM; lPixel++) {
			lDest[lPixel] = pColor;
		}

		for(; lPixel < pXLen; lPixel++) {
			lDest[lPixel] = pBackColor;
		}
		lDest += mLineLen;
	}
}

}  // namespace VideoServices
}  // namespace HoverRace
