// FreeElementBase.h
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

//
// The class defined in this file can be used as a base class for
// free elements
//
//

#pragma once

#include "../Model/MazeElement.h"
#include "ResourceLib.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace ObjFacTools {

class FreeElementBase : public HoverRace::Model::FreeElement
{
	protected:
		const ResActor *mActor;
		int mCurrentSequence;
		int mCurrentFrame;

	public:
		MR_DllDeclare FreeElementBase(const Util::ObjectFromFactoryId & pId);
		MR_DllDeclare ~FreeElementBase();

		// Rendering stuff
		MR_DllDeclare void Render(VideoServices::Viewport3D *pDest, MR_SimulationTime pTime);

};

}  // namespace ObjFacTools
}  // namespace HoverRace

#undef MR_DllDeclare
