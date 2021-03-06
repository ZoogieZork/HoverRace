
// SettingsScene.cpp
//
// Copyright (c) 2014, 2015 Michael Imamura.
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

#include "../../engine/Display/Button.h"
#include "../../engine/Display/Container.h"
#include "../../engine/Display/Label.h"
#include "MessageScene.h"

#include "SettingsScene.h"

namespace HoverRace {
namespace Client {

/// Default size for settings sliders.
Vec2 SettingsScene::SLIDER_SIZE = Vec2(300, 20);


SettingsScene::SettingsScene(Display::Display &display, GameDirector &director,
	const std::string &parentTitle, const std::string &title,
	const std::string &name) :
	SUPER(display, director, JoinTitles(parentTitle, title), name),
	curRow(0), needsLoadFromConfig(true)
{
	using namespace Display;
	using Alignment = UiViewModel::Alignment;

	SupportCancelAction();
	SupportExtraAction(_("Reset to defaults"));

	auto root = GetContentRoot();

	auto mainGrid = root->NewChild<FlexGrid>(display);
	mainGrid->SetPos(640, 60);
	mainGrid->SetMargin(0, 30);
	mainGrid->SetAlignment(Alignment::N);

	auto settingsGridCell = mainGrid->At(0, 0).NewChild<FlexGrid>(display);
	settingsGridCell->SetAlignment(Alignment::N);
	settingsGrid = settingsGridCell->GetContents();

	size_t col = 0;
	{
		auto &cell = settingsGrid->GetColumnDefault(col++);
		cell.SetAlignment(Alignment::E);
	}
	{
		auto &cell = settingsGrid->GetColumnDefault(col++);
		cell.SetAlignment(Alignment::W);
	}

	auto saveCell = mainGrid->At(1, 0).
		NewChild<Button>(display, _("Save Settings"));
	saveCell->SetAlignment(Alignment::N);
	saveConn = saveCell->GetContents()->GetClickedSignal().connect(
		std::bind(&SettingsScene::OnOk, this));

	settingsGrid->RequestFocus();
}

/**
 * Add a new setting row.
 * @param label The label for the setting.
 * @return A reference to the content cell for the row.
 */
Display::FlexGrid::CellProxy SettingsScene::AddSetting(const std::string &label)
{
	using namespace Display;

	if (!label.empty()) {
		const auto &s = display.styles;
		settingsGrid->At(curRow, 0).
			NewChild<Label>(label, s.bodyFont, s.bodyFg);
	}

	return settingsGrid->At(curRow++, 1);
}

void SettingsScene::OnExtra()
{
	auto confirmScene = std::make_shared<MessageScene>(display, director,
		_("Reset to defaults"),
		_("Reset all settings on this screen to their default values?"),
		true);
	confirmOkConn = confirmScene->GetOkSignal().connect([&]() {
		ResetToDefaults();
		LoadFromConfig();
	});
	director.RequestPushScene(confirmScene);
}

void SettingsScene::PrepareRender()
{
	if (needsLoadFromConfig) {
		LoadFromConfig();
		needsLoadFromConfig = false;
	}

	SUPER::PrepareRender();
}

}  // namespace Client
}  // namespace HoverRace
