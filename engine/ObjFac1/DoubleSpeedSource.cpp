// DoubleSpeedSource.cpp
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

#include "DoubleSpeedSource.h"

namespace HoverRace {
namespace ObjFac1 {

const MR_Int32 cSourceRay = 20000;				  // the source have a diameter of 40 meters
const MR_Int32 cSourceHeight = 2000;			  // 2 meters

MR_Int32 DoubleSpeedSource::ZMin() const
{
	return mPosition.mZ;
}

MR_Int32 DoubleSpeedSource::ZMax() const
{
	return mPosition.mZ + cSourceHeight;
}

MR_Int32 DoubleSpeedSource::AxisX() const
{
	return mPosition.mX;
}

MR_Int32 DoubleSpeedSource::AxisY() const
{
	return mPosition.mY;
}

MR_Int32 DoubleSpeedSource::RayLen() const
{
	return cSourceRay;
}

DoubleSpeedSource::DoubleSpeedSource(const Util::ObjectFromFactoryId &pId) :
	Model::FreeElement(pId)
{
	mContactEffectList.push_back(&mSpeedEffect);
}

DoubleSpeedSource::~DoubleSpeedSource()
{
}

const MR_ContactEffectList *DoubleSpeedSource::GetEffectList()
{
	return &mContactEffectList;
}

const Model::ShapeInterface *DoubleSpeedSource::GetReceivingContactEffectShape()
{
	return this;
}

}  // namespace ObjFac1
}  // namespace HoverRace
