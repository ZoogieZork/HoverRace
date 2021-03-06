// PowerUp.cpp
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

#include "PowerUp.h"
#include "ObjFac1Res.h"
#include "../Model/ConcreteShape.h"
#include "../Model/ObstacleCollisionReport.h"

using HoverRace::ObjFacTools::ResourceLib;

namespace HoverRace {
namespace ObjFac1 {

const MR_Int32 cPowerUpRay = 550;
const MR_Int32 cPowerUpHalfHeight = 550;

MR_Int32 PowerUp::ZMin() const
{
	return mPosition.mZ - cPowerUpHalfHeight;
}

MR_Int32 PowerUp::ZMax() const
{
	return mPosition.mZ + cPowerUpHalfHeight;
}

MR_Int32 PowerUp::AxisX() const
{
	return mPosition.mX;
}

MR_Int32 PowerUp::AxisY() const
{
	return mPosition.mY;
}

MR_Int32 PowerUp::RayLen() const
{
	return cPowerUpRay;
}

PowerUp::PowerUp(const Util::ObjectFromFactoryId &pId, ResourceLib* resourceLib) :
	SUPER(pId)
{
	mEffectList.push_back(&mPowerUpEffect);
	mActor = resourceLib->GetActor(MR_PWRUP);

	mOrientation = 0;
	mPowerUpEffect.mElementPermId = -1;
}

PowerUp::~PowerUp()
{
}

BOOL PowerUp::AssignPermNumber(int pNumber)
{
	mPowerUpEffect.mElementPermId = pNumber;
	return TRUE;
}

const MR_ContactEffectList *PowerUp::GetEffectList()
{
	return &mEffectList;
}

const Model::ShapeInterface *PowerUp::GetReceivingContactEffectShape()
{
	return this;
}

const Model::ShapeInterface *PowerUp::GetGivingContactEffectShape()
{
	// return this;
	return NULL;
}

// Simulation
int PowerUp::Simulate(MR_SimulationTime pDuration, Model::Level * /*pLevel */ , int pRoom)
{
	// Just rotate on ourself
	if(pDuration != 0) {
		mOrientation = MR_NORMALIZE_ANGLE(mOrientation + pDuration);
	}

	return pRoom;
}

/*
void PowerUp::ApplyEffect( const MR_ContactEffect* pEffect,  MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, Model::Level* pLevel )
{
   MR_ContactEffect* lEffect = (MR_ContactEffect*)pEffect;
   const MR_PhysicalCollision* lPhysCollision = dynamic_cast<MR_PhysicalCollision*>(lEffect);

   if( (lPhysCollision != NULL)&&pValidDirection )
   {

	  if( lPhysCollision->mWeight < MR_PhysicalCollision::eInfiniteWeight )
	  {
		 if( mLived >= cIgnitionTime )
		 {
			// Hitted a free object
			// Must died
			mLived = cLifeTime+cStopTime;
		 }
	  }
	  else
	  {
		 // Hitted structure..make a perfect bounce
		 MR_Angle lDiff = MR_NORMALIZE_ANGLE( pHorizontalDirection-mOrientation+MR_PI );

		 if( (lDiff < (MR_PI/2) ) || ( lDiff > (MR_PI+MR_PI/2)) )
		 {
			mOrientation = MR_NORMALIZE_ANGLE( pHorizontalDirection + lDiff );
			mBounceSoundEvent = TRUE;
		 }
	  }
   }
}
*/

// State broadcast

class MR_PowerUpState
{
	public:
		MR_Int32 mPosX;							  // 4    4
		MR_Int32 mPosY;							  // 4    8
		MR_Int32 mPosZ;							  // 4   12

};

Model::ElementNetState PowerUp::GetNetState() const
{
	static MR_PowerUpState lsState;				  // Static is ok because the variable will be used immediatly

	Model::ElementNetState lReturnValue;

	lReturnValue.mDataLen = sizeof(lsState);
	lReturnValue.mData = (MR_UInt8 *) & lsState;

	lsState.mPosX = mPosition.mX;
	lsState.mPosY = mPosition.mY;
	lsState.mPosZ = mPosition.mZ;

	return lReturnValue;
}

void PowerUp::SetNetState(int /*pDataLen */ , const MR_UInt8 * pData)
{

	const MR_PowerUpState *lState = (const MR_PowerUpState *) pData;

	mPosition.mX = lState->mPosX;
	mPosition.mY = lState->mPosY;
	mPosition.mZ = lState->mPosZ;

	mOrientation = 0;

}

}  // namespace ObjFac1
}  // namespace HoverRace
