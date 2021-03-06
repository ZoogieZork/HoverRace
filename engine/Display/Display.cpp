
// Display.cpp
//
// Copyright (c) 2013-2015 Michael Imamura.
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

#include "../Util/Config.h"

#include "Display.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Display {

namespace {
	const double MIN_VIRT_WIDTH = 1280.0;
	const double MIN_VIRT_HEIGHT = 720.0;
}

void Display::OnDisplayConfigChanged()
{
	const auto &vidCfg = Config::GetInstance()->video;
	int w = vidCfg.xRes;
	int h = vidCfg.yRes;
	double wd = static_cast<double>(w);
	double hd = static_cast<double>(h);

	// Recalculate the UI coordinates.
	double newUiScale;
	double uiScaleW = wd / MIN_VIRT_WIDTH;
	double uiScaleH = hd / MIN_VIRT_HEIGHT;
	if (uiScaleW < uiScaleH) {
		newUiScale = uiScaleW;
		uiOffset.x = 0;
		uiOffset.y = floor((hd - (MIN_VIRT_HEIGHT * uiScaleW)) / 2);
	}
	else {
		newUiScale = uiScaleH;
		uiOffset.x = floor((wd - (MIN_VIRT_WIDTH * uiScaleH)) / 2);
		uiOffset.y = 0;
	}

	// Save the new UI scale before firing displayConfigChanged.
	// This way, handlers that need to know both the new dimensions and scale
	// can do so, even though we only pass the dimensions.
	bool scaleChanged = uiScale != newUiScale;
	if (scaleChanged) {
		uiScale = newUiScale;
	}

	uiScreenSize.x = 1280.0;
	uiScreenSize.y = 720.0;
	uiScreenSize += (uiOffset * 2.0) / uiScale;

	FireDisplayConfigChangedSignal(w, h);

	if (scaleChanged) {
		FireUiScaleChangedSignal(uiScale);
	}
}

void Display::FireDisplayConfigChangedSignal(int width, int height) const
{
	displayConfigChangedSignal(width, height);
}

void Display::FireUiScaleChangedSignal(double scale) const
{
	uiScaleChangedSignal(scale);
}

//{{{ styles_t /////////////////////////////////////////////////////////////////

Display::styles_t::styles_t() :
	gridMargin(0, 0), gridPadding(0, 0)
{
	Reload();
}

void Display::styles_t::Reload()
{
	const Config *cfg = Config::GetInstance();
	const std::string &monospaceFontName = cfg->GetDefaultMonospaceFontName();

	// Hack fix for broken alpha blending when HW acceleration is off.
	// Only full-brightness, fully-opaque white is affected.
	const Color WHITE(cfg->runtime.noAccel ? 0xfffefefe : COLOR_WHITE);

	bodyFont.Set(30, 0);
	bodyFg = 0xffbfbfbf;
	bodyHeadFont.Set(30, 0);
	bodyHeadFg = WHITE;
	bodyAsideFont.Set(20, 0);
	bodyAsideFg = 0xffbfbfbf;

	announcementHeadFont.Set(30);
	announcementHeadFg = WHITE;
	announcementBodyFont.Set(20);
	announcementBodyFg = 0xff7f7f7f;
	announcementSymbolFg = 0xbfffffff;
	announcementBg = 0xbf000000;

	// 100% HUD scale.
	hudNormalFont.Set(30, UiFont::BOLD);
	hudNormalHeadFont.Set(20, 0);
	// 50% HUD scale.
	hudSmallFont.Set(20, UiFont::BOLD);
	hudSmallHeadFont.Set(15, 0);

	consoleFont.Set(monospaceFontName, 30);
	consoleFg = WHITE;
	consoleCursorFg = 0xffbfbfbf;
	consoleBg = 0xbf000000;

	formFont = bodyFont;
	formFg = WHITE;
	formDisabledFg = 0x7fffffff;

	dialogBg = 0xcc000000;

	gridMargin.x = 6;
	gridMargin.y = 6;
	gridPadding.x = 1;
	gridPadding.y = 1;

	buttonBg = 0x3f00007f;
	buttonFocusedBg = 0xff00007f;
	buttonDisabledBg = 0x3f7f7f7f;
	buttonPressedBg = 0x7f00007f;

	headingFont.Set(40, UiFont::BOLD);
	headingFg = WHITE;
}

//}}} styles_t

}  // namespace Display
}  // namespace HoverRace
