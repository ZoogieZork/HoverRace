
// GameSelectScene.h
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

#pragma once

#include "../../engine/Model/TrackList.h"
#include "GameDirector.h"

#include "DialogScene.h"

namespace HoverRace {
	namespace Client {
		class Rulebook;
		class RulebookLibrary;
	}
	namespace Display {
		class Container;
		class Display;
	}
}

namespace HoverRace {
namespace Client {

/**
 * Select options for a new game (track, rules, etc.).
 *
 * This specific scene only handles selecting a rulebook; it will use
 * TrackSelectScene to select the track and configure the game rules.
 *
 * @author Michael Imamura
 */
class GameSelectScene : public DialogScene
{
	using SUPER = DialogScene;

public:
	GameSelectScene(Display::Display &display, GameDirector &director,
		RulebookLibrary &rulebookLibrary, bool multiplayer);
	virtual ~GameSelectScene();

private:
	void OnRulebookSelected(std::shared_ptr<const Rulebook> rulebook);

protected:
	void OnOk() override;
	void OnCancel() override;

protected:
	void OnPhaseTransition(double progress) override;
	void OnStateTransition(double progress) override;

public:
	using okSignal_t = boost::signals2::signal<void(std::shared_ptr<Rules>, std::shared_ptr<Display::Res<Display::Texture>>)>;
	okSignal_t &GetOkSignal() { return okSignal; }

	using cancelSignal_t = boost::signals2::signal<void()>;
	cancelSignal_t &GetCancelSignal() { return cancelSignal; }

public:
	void Render() override;

private:
	Display::Display &display;
	GameDirector &director;
	bool trackSelected;  ///< Are we exiting because a track was selected?

	std::shared_ptr<Display::Container> rulebookPanel;

	okSignal_t okSignal;
	cancelSignal_t cancelSignal;
};

}  // namespace Client
}  // namespace HoverRace
