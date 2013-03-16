
// TestLabScene.cpp
//
// Copyright (c) 2013 Michael Imamura.
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

#include "StdAfx.h"

#include "../../engine/Display/Display.h"
#include "../../engine/Display/Label.h"
#include "../../engine/VideoServices/FontSpec.h"
#include "../../engine/VideoServices/VideoBuffer.h"
#include "../../engine/Util/Config.h"

#include "TestLabScene.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

TestLabScene::TestLabScene(Display::Display &display) :
	SUPER("Test Lab"),
	display(display)
{
	Config *cfg = Config::GetInstance();
	std::string fontName = cfg->GetDefaultFontName();

	Display::Label *lbl;

	lbl = AddElem(new Display::Label("Red 20 Normal",
		Display::UiFont(fontName, 20),
		Display::Color(0xff, 0xff, 0x00, 0x00)));
	lbl->SetPos(0, 20);
	lbl = AddElem(new Display::Label("Yellow 25 Italic",
		Display::UiFont(fontName, 25, Display::UiFont::ITALIC),
		Display::Color(0xff, 0xff, 0xff, 0x00)));
	lbl->SetPos(0, 40);
	lbl = AddElem(new Display::Label("Magenta 30 Bold+Italic",
		Display::UiFont(fontName, 30, Display::UiFont::BOLD | Display::UiFont::ITALIC),
		Display::Color(0xff, 0xff, 0x00, 0xff)));
	lbl->SetPos(0, 65);
}

TestLabScene::~TestLabScene()
{
}

void TestLabScene::OnPhaseChanged(Phase::phase_t oldPhase)
{
	// Act like the starting and stopping phases don't even exist.
	switch (GetPhase()) {
		case Phase::STARTING:
			SetPhase(Phase::RUNNING);
			break;
		case Phase::STOPPING:
			SetPhase(Phase::STOPPED);
			break;
	}
}

void TestLabScene::Advance(Util::OS::timestamp_t)
{
}

void TestLabScene::PrepareRender()
{
	std::for_each(elems.begin(), elems.end(),
		std::mem_fn(&Display::ViewModel::PrepareRender));
}

void TestLabScene::Render()
{
	std::for_each(elems.begin(), elems.end(),
		std::mem_fn(&Display::ViewModel::Render));
}

}  // namespace HoverScript
}  // namespace Client
