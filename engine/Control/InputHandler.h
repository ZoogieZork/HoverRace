
// InputHandler.h
// Base class for input handlers.
//
// Copyright (c) 2010 Michael Imamura.
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

#include <SDL2/SDL_keycode.h>

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
namespace Control {

class MR_DllDeclare InputHandler
{
	public:
		InputHandler() { }
		virtual ~InputHandler() { }

	public:
		virtual bool KeyPressed(SDL_Keycode kc, unsigned int text) { return true; }
		virtual bool KeyReleased(SDL_Keycode kc, unsigned int text) { return true; }

		/*TODO
		virtual bool MouseMoved(const OIS::MouseState &state) { return true; }
		virtual bool MousePressed(const OIS::MouseState &state, OIS::MouseButtonID id) { return true; }
		virtual bool MouseReleased(const OIS::MouseState &state, OIS::MouseButtonID id) { return true; }

		virtual bool ButtonPressed(const OIS::JoyStickState &state, int button) { return true; }
		virtual bool ButtonReleased(const OIS::JoyStickState &state, int button) { return true; }
		virtual bool AxisMoved(const OIS::JoyStickState &state, int axis) { return true; }
		virtual bool PovMoved(const OIS::JoyStickState &state, int pov) { return true; }
		*/
};
typedef std::shared_ptr<InputHandler> InputHandlerPtr;

} // namespace Control
} // namespace HoverRace

#undef MR_DllDeclare
