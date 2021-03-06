
// Roster.cpp
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

#include "../../engine/Player/Player.h"

#include "Roster.h"

namespace HoverRace {
namespace Client {

Roster::Roster()
{
}

void Roster::AddPlayer(std::shared_ptr<Player::Player> player)
{
	players.emplace_back(player);
	playerAddedSignal(player);
}

}  // namespace Client
}  // namespace HoverRace
