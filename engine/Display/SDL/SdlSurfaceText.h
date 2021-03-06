
// SdlSurfaceText.h
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include <SDL2/SDL.h>

#include "../UiFont.h"

#include "SdlDisplay.h"

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
namespace SDL {

/**
 * A fragment of text rendered on an SDL surface.
 *
 * This class does not manage the SDL surface itself, since the surface may be
 * shared amongst multiple instances (e.g. to only use one texture for
 * multiple text fragments).  Instead, this class handles rendering the text
 * onto a surface and keeps track of the properties needed for a display
 * renderer or post-processor to use the rendered text.
 *
 * Note that since this class deals directly with surfaces, no UI scaling
 * is taken into account; users of this class must pre-scale all values
 * that are passed in (if necessary).
 *
 * @author Michael Imamura
 */
class MR_DllDeclare SdlSurfaceText
{
public:
	SdlSurfaceText(SdlDisplay &display);
	SdlSurfaceText(SdlDisplay &display, const UiFont &font,
		const Color color = COLOR_WHITE);
	~SdlSurfaceText() { }

	SdlSurfaceText &operator=(const SdlSurfaceText&) = delete;

public:
	// Properties to control rendering.
	void SetFont(const UiFont &font) { this->font = font; }
	void SetColor(const Color color) { this->color = color; }
	void SetWrapWidth(int wrapWidth) { this->wrapWidth = wrapWidth; }
	void SetFixedScale(bool fixedScale) { this->fixedScale = fixedScale; }

public:
	int MeasureLineHeight();
	SDL_Surface *RenderToNewSurface(const std::string &s);
	SDL_Surface *RenderToSurface(SDL_Surface *dest, int x, int y,
		const std::string &s);

public:
	// Properties available after rendering.
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }

private:
	SdlDisplay &display;

	UiFont font;
	Color color;
	int wrapWidth;
	bool fixedScale;

	int width;
	int height;
};

}  // namespace SDL
}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
