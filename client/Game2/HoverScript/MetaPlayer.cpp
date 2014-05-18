
// MetaPlayer.cpp
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

#include "StdAfx.h"

#include "../../../engine/Script/Core.h"
#include "../../../engine/Util/Log.h"

#include "PlayerPeer.h"

#include "MetaPlayer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {
namespace HoverScript {

namespace {
	class Wrapper : public MetaPlayer, public luabind::wrap_base
	{
		typedef MetaPlayer SUPER;
		public:
			Wrapper(std::shared_ptr<PlayerPeer> player) : SUPER(player) { }
			virtual ~Wrapper() { }
	};
}

MetaPlayer::MetaPlayer(std::shared_ptr<PlayerPeer> player) :
	player(std::move(player))
{
}

MetaPlayer::~MetaPlayer()
{
}

/**
 * Register this peer in an environment.
 */
void MetaPlayer::Register(Script::Core *scripting)
{
	using namespace luabind;
	lua_State *L = scripting->GetState();

	module(L)[
		class_<MetaPlayer, Wrapper>("MetaPlayer")
			.def(constructor<std::shared_ptr<PlayerPeer>>())
	];
}

}  // namespace HoverScript
}  // namespace Client
}  // namespace HoverRace