
// SpriteTextureRes.h
//
// Copyright (c) 2014 Michael Imamura.
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

#pragma once

#include "Res.h"

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
	namespace Display {
		class Texture;
	}
	namespace Parcel {
		class ObjStream;
	}
}

namespace HoverRace {
namespace Display {

/**
 * Loads a legacy 8-bit sprite as a texture.
 * @author Michael Imamura
 */
class SpriteTextureRes : public Res<Texture>
{
	typedef Res<Texture> SUPER;

public:
	SpriteTextureRes(const std::string &recordName,
		Parcel::ObjStream &archive, bool flipped = false);
	virtual ~SpriteTextureRes() { }

public:
	std::string GetId() const override { return id; }

	std::unique_ptr<std::istream> Open() const override
	{
		throw ResLoadExn("Attempted to open internal texture.");
	}

	bool IsGenerated() const override { return true; }
	const ImageData *GetImageData() override { return &imageData; }

private:
	std::string id;
	ImageData imageData;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
