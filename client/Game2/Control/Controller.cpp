// Controller.cpp
//
// This file contains the definition for the HoverRace::Client::Controller
// class.  It contains all the code to control input devices; joysticks, 
// keyboards, and the like.

#include "StdAfx.h"

#ifdef WITH_SDL_OIS_INPUT
#	include "SDL/SDLInputManager.h"
#endif

//#include "InputHandler.h"
#include "UiHandler.h"

#include "Controller.h"

#include "ActionPerformers.h"
//#include "ConsoleActions.h"
#include "ControlAction.h"
#include "ObserverActions.h"

#include <sstream>

using namespace HoverRace;
using namespace HoverRace::Client::Control;
using namespace HoverRace::Util;
using HoverRace::Client::HoverScript::HighConsole;
using HoverRace::Client::Observer;
using namespace std;

InputEventController::actions_t::ui_t::ui_t() :
	menuOk(std::make_shared<Action<voidSignal_t>>(_("OK"), 0)),
	menuCancel(std::make_shared<Action<voidSignal_t>>(_("Cancel"), 0))
	{ }

InputEventController::InputEventController(Util::OS::wnd_t mainWindow, UiHandlerPtr uiHandler) :
	uiHandler(uiHandler)
{
	captureNextInput = false;
	captureOldHash = 0;
	captureMap = "";
	nextAvailableDisabledHash = 0;

	InitInputManager(mainWindow);
	LoadConfig();
	LoadMenuMap();
	LoadConsoleMap();
}

InputEventController::~InputEventController()
{
}

void InputEventController::Poll() 
{
	// We assume that all pending input events are at the front of the queue.

	SDL_Event events[16];
	int numEvents;

	while ((numEvents = SDL_PeepEvents(events, sizeof(events) / sizeof(SDL_Event),
		SDL_GETEVENT, SDL_KEYDOWN, SDL_MULTIGESTURE) > 0))
	{
		for (int i = 0; i < numEvents; i++) {
			SDL_Event &evt = events[i];
			switch (evt.type) {
				case SDL_KEYUP: OnKeyPressed(evt.key); break;
				case SDL_KEYDOWN: OnKeyReleased(evt.key); break;
				default:
					std::ostringstream oss;
					oss << "Unhandled input event type: " << evt.type << "\n";
					OutputDebugString(oss.str().c_str());
			}
		}
	}

	/*
	try {
		if(kbd)
			kbd->capture();
		if(mouse)
			mouse->capture();
		if(joys) {
			for(int i = 0; i < numJoys; i++)
				joys[i]->capture();
		}
	} catch(Exception&) { // this should not happen, hence our error message
		ASSERT(FALSE);
	}
	*/
}

bool InputEventController::OnKeyPressed(const SDL_KeyboardEvent& arg)
{
	// make left and right modifiers equal
	SDL_Keycode kc = arg.keysym.sym;

	if (kc == SDLK_RSHIFT)
		kc = SDLK_LSHIFT;
	else if (kc == SDLK_RCTRL)
		kc = SDLK_LCTRL;
	else if (kc == SDLK_RGUI)
		kc = SDLK_LGUI;

	HandleEvent(HashKeyboardEvent(kc), 1);
	return true;
}

bool InputEventController::OnKeyReleased(const SDL_KeyboardEvent& arg)
{
	// make left and right modifiers equal
	SDL_Keycode kc = arg.keysym.sym;

	if (kc == SDLK_RSHIFT)
		kc = SDLK_LSHIFT;
	else if (kc == SDLK_RCTRL)
		kc = SDLK_LCTRL;
	else if (kc == SDLK_RGUI)
		kc = SDLK_LGUI;

	// value will be 0 (since the key was released)
	HandleEvent(HashKeyboardEvent(kc), 0);
	return true;
}

/*TODO

bool InputEventController::mouseMoved(const MouseEvent& arg)
{
	// value will be how far the mouse moved
	// examine the axes to find the maximum
	int x, y, z, ax, ay, az;

	if(arg.state.X.absOnly)
		x = arg.state.X.abs;
	else
		x = arg.state.X.rel;

	if(arg.state.Y.absOnly)
		y = arg.state.Y.abs;
	else
		y = arg.state.Y.rel;

	if(arg.state.Z.absOnly)
		z = arg.state.Z.abs;
	else
		z = arg.state.Z.rel;

	// find magnitudes
	ax = abs(x);
	ay = abs(y);
	az = abs(z);

	// send up to three events if necessary
	if(ax > 0)
		HandleEvent(HashMouseAxisEvent(arg, AXIS_X, (x > 0) ? 1 : 0), ax);
	if(ay > 0)
		HandleEvent(HashMouseAxisEvent(arg, AXIS_Y, (y > 0) ? 1 : 0), ay);
	if(az > 0)
		HandleEvent(HashMouseAxisEvent(arg, AXIS_Z, (z > 0) ? 1 : 0), az);

	return true;
}

bool InputEventController::mousePressed(const MouseEvent& arg, MouseButtonID id)
{
	HandleEvent(HashMouseButtonEvent(arg, id), 1);
	return true;
}

bool InputEventController::mouseReleased(const MouseEvent& arg, MouseButtonID id)
{
	HandleEvent(HashMouseButtonEvent(arg, id), 0);
	return true;
}

bool InputEventController::buttonPressed(const JoyStickEvent& arg, int button)
{
	HandleEvent(HashJoystickButtonEvent(arg, button), 1);
	return true;
}

bool InputEventController::buttonReleased(const JoyStickEvent& arg, int button)
{
	HandleEvent(HashJoystickButtonEvent(arg, button), 0);
	return true;
}

bool InputEventController::axisMoved(const JoyStickEvent& arg, int axis)
{
	int value;
	if(arg.state.mAxes[axis].absOnly)
		value = arg.state.mAxes[axis].abs;
	else
		value = arg.state.mAxes[axis].rel;

	HandleEvent(HashJoystickAxisEvent(arg, axis, (value > 0) ? 1 : 0), value);
	return true;
}

bool InputEventController::povMoved(const JoyStickEvent& arg, int pov)
{
	HandleEvent(HashJoystickPovEvent(arg, pov, arg.state.mPOV[pov].direction), 0);
	return true;
}

*/

int InputEventController::GetNextAvailableDisabledHash()
{
	while(actionMap.count(++nextAvailableDisabledHash) == 1)
	{
		if(nextAvailableDisabledHash >= 16384)
			nextAvailableDisabledHash = 0; // only allowed to use 14 bits for hash
	}
	
	return nextAvailableDisabledHash;
}

void InputEventController::HandleEvent(int hash, int value)
{
	// if we are setting controls, we need to update our action map
	if(captureNextInput) {
		RebindKey(captureMap, captureOldHash, hash);

		captureNextInput = false; // don't do this again
		captureMap = "";
		captureOldHash = 0;

		return;
	}

	// fire the action bound to the given input hash code
	if(actionMap.count(hash) == 1)
		(*actionMap[hash])(value);
}

void InputEventController::CaptureNextInput(int oldhash, string mapname)
{
	// the next time the user gives any input, it will be bound to the control
	// that is currently bound to oldhash
	captureNextInput = true;
	captureOldHash = oldhash;
	captureMap = mapname;
}

bool InputEventController::IsCapturing()
{
	return captureNextInput;
}

void InputEventController::StopCapture()
{
	captureNextInput = false;
	captureOldHash = 0;
}

void InputEventController::DisableCaptureInput()
{
	if(!captureNextInput)
		return;

	RebindKey(captureMap, captureOldHash, GetNextAvailableDisabledHash());
	
	captureOldHash = 0;
	captureMap = "";
	captureNextInput = false;
}

void InputEventController::ClearActionMap()
{
	// remove all active bindings
	actionMap.clear();
	activeMaps.clear();
}

bool InputEventController::AddActionMap(string mapname)
{
	if(allActionMaps.count(mapname) != 1)
		return false;

	for(ActionMap::iterator it = allActionMaps[mapname].begin(); it != allActionMaps[mapname].end(); it++)
		actionMap[it->first] = it->second; // add to active controls
	activeMaps.push_back(mapname);
	return true;
}

const vector<string>& InputEventController::GetActiveMaps()
{
	return activeMaps;
}

InputEventController::ActionMap& InputEventController::GetActionMap(string key)
{
	return allActionMaps[key];
}

vector<string> InputEventController::GetAvailableMaps()
{
	vector<string> maps;
	for(map<string, ActionMap>::iterator it = allActionMaps.begin(); it != allActionMaps.end(); it++)
		maps.push_back(it->first);

	return maps;
}

void InputEventController::AddPlayerMaps(int numPlayers, MainCharacter::MainCharacter** mcs)
{
	// iterate over each actionMap
	// start with the highest player, so the lowest-numbered player's controls override any
	// potentially conflicting controls (this should not happen)
	for(int i = numPlayers - 1; i >= 0; i--) {
		if(mcs[i] == NULL) // player controls do not exist for this player; do not add it
			continue;

		stringstream str;
		str << _("Player") << " " << (i + 1);
		string mapname = str.str();
		if(allActionMaps.count(mapname) != 1)
			continue; // was not loaded for some reason...

		for(ActionMap::iterator it = allActionMaps[mapname].begin(); it != allActionMaps[mapname].end(); it++) {
			PlayerEffectAction* perf = dynamic_cast<PlayerEffectAction*>(it->second.get());
			if(perf != NULL)
				perf->SetMainCharacter(mcs[i]);
			actionMap[it->first] = it->second; // add to active controls
		}
		activeMaps.push_back(mapname);
	}

	AddActionMap(_("Console"));
}

void InputEventController::AddObserverMaps(Observer** obs, int numObs)
{
	for(ActionMap::iterator it = allActionMaps[_("Camera")].begin(); it != allActionMaps[_("Camera")].end(); it++) {
		ObserverAction* x = dynamic_cast<ObserverAction*>(it->second.get());
		if(x != NULL)
			x->SetObservers(obs, numObs);
	}

	AddActionMap(_("Camera"));
}

void InputEventController::AddMenuMaps()
{
	AddActionMap(_("Menu"));
}

void InputEventController::SetConsole(HighConsole* hc)
{
	/*TODO
	for(ActionMap::iterator it = allActionMaps["console-keys"].begin(); it != allActionMaps["console-keys"].end(); it++) {
		ConsoleAction* x = dynamic_cast<ConsoleAction*>(it->second.get());
		if(x != NULL)
			x->SetHighConsole(hc);
	}

	for(ActionMap::iterator it = allActionMaps[_("Console")].begin(); it != allActionMaps[_("Console")].end(); it++) {
		ConsoleAction* x = dynamic_cast<ConsoleAction*>(it->second.get());
		if(x != NULL)
			x->SetHighConsole(hc);
	}
	*/
}

void InputEventController::SaveConfig()
{
	Config *cfg = Config::GetInstance();

	/* We must save each functor's hash */
	for(int i = 0; i < cfg->MAX_PLAYERS; i++) {
		// use map playerX
		stringstream str;
		str << _("Player") << " " << (i + 1);
		ActionMap& playerMap = allActionMaps[str.str()];

		for(ActionMap::iterator it = playerMap.begin(); it != playerMap.end(); it++) {
			// tedious, unfortunately
			switch(it->second->GetListOrder()) {
				case 0: // throttle
					cfg->controls_hash[i].motorOn = it->first;
					break;
				case 1: // brake
					cfg->controls_hash[i].brake = it->first;
					break;
				case 2: // turn left
					cfg->controls_hash[i].left = it->first;
					break;
				case 3: // turn right
					cfg->controls_hash[i].right = it->first;
					break;
				case 4: // jump
					cfg->controls_hash[i].jump = it->first;
					break;
				case 5: // powerup
					cfg->controls_hash[i].fire = it->first;
					break;
				case 6: // change item
					cfg->controls_hash[i].weapon = it->first;
					break;
				case 7: // look back
					cfg->controls_hash[i].lookBack = it->first;
					break;
			}
		}
	}

	// now save console map
	for(ActionMap::iterator it = allActionMaps[_("Console")].begin(); it != allActionMaps[_("Console")].end(); it++) {
		switch(it->second->GetListOrder()) {
			case 0: // toggle console
				cfg->ui.console_toggle = it->first;
				break;
			case 1: // page up
				cfg->ui.console_up = it->first;
				break;
			case 2: // page down
				cfg->ui.console_down = it->first;
				break;
			case 3: // top
				cfg->ui.console_top = it->first;
				break;
			case 4: // bottom
				cfg->ui.console_bottom = it->first;
				break;
			case 5: // help
				cfg->ui.console_help = it->first;
				break;
		}
	}

	// now save camera map
	for(ActionMap::iterator it = allActionMaps[_("Camera")].begin(); it != allActionMaps[_("Camera")].end(); it++) {
		switch(it->second->GetListOrder()) {
			case 0: // zoom in
				cfg->camera_hash.zoomIn = it->first;
				break;
			case 1: // zoom out
				cfg->camera_hash.zoomOut = it->first;
				break;
			case 2: // pan up
				cfg->camera_hash.panUp = it->first;
				break;
			case 3: // pan down
				cfg->camera_hash.panDown = it->first;
				break;
			case 4: // reset
				cfg->camera_hash.reset = it->first;
				break;
		}
	}
}

void InputEventController::ReloadConfig()
{
	allActionMaps.clear();
	activeMaps.clear();
	actionMap.clear();
	LoadConfig();
	LoadConsoleMap();
}

void InputEventController::LoadConfig()
{
	// use the cfg_controls_t structure to load functors
	Config *cfg = Config::GetInstance();

	/* now we need to load the values */
	for(int i = 0; i < cfg->MAX_PLAYERS; i++) {
		// use map playerX
		stringstream str;
		str << _("Player") << " " << (i + 1);
		ActionMap& playerMap = allActionMaps[str.str()];

		playerMap[cfg->controls_hash[i].motorOn].reset(new EngineAction(_("Throttle"), 0, NULL));
		playerMap[cfg->controls_hash[i].brake].reset(new BrakeAction(_("Brake"), 1, NULL));
		playerMap[cfg->controls_hash[i].left].reset(new TurnLeftAction(_("Turn Left"), 2, NULL));
		playerMap[cfg->controls_hash[i].right].reset(new TurnRightAction(_("Turn Right"), 3, NULL));
		playerMap[cfg->controls_hash[i].jump].reset(new JumpAction(_("Jump"), 4, NULL));
		playerMap[cfg->controls_hash[i].fire].reset(new PowerupAction(_("Fire"), 5, NULL));
		playerMap[cfg->controls_hash[i].weapon].reset(new ChangeItemAction(_("Item"), 6, NULL));
		playerMap[cfg->controls_hash[i].lookBack].reset(new LookBackAction(_("Look Back"), 7, NULL));
	}

	/* load camera map */
	ActionMap& cameraMap = allActionMaps[_("Camera")];

	cameraMap[cfg->camera_hash.zoomIn].reset(new ObserverZoomAction(_("Zoom In"), 0, NULL, 0, 1));
	cameraMap[cfg->camera_hash.zoomOut].reset(new ObserverZoomAction(_("Zoom Out"), 1, NULL, 0, -1));
	cameraMap[cfg->camera_hash.panUp].reset(new ObserverTiltAction(_("Pan Up"), 2, NULL, 0, 1));
	cameraMap[cfg->camera_hash.panDown].reset(new ObserverTiltAction(_("Pan Down"), 3, NULL, 0, -1));
	cameraMap[cfg->camera_hash.reset].reset(new ObserverResetAction(_("Reset Camera"), 4, NULL, 0));
}

void InputEventController::InitInputManager(Util::OS::wnd_t mainWindow)
{
	//FIXME
}

void InputEventController::RebindKey(string mapname, int oldhash, int newhash)
{
	// check map exists
	if(allActionMaps.count(mapname) > 0) {
		ActionMap& map = allActionMaps[mapname];
		// check anything exists at old hash
		if(map.count(oldhash) > 0) {
			ControlActionPtr tmp = map[oldhash];
			map.erase(oldhash);
			// check if we need to disable 
			if(map.count(newhash) > 0) {
				ControlActionPtr tmp2 = map[newhash];
				map.erase(newhash);
				map[GetNextAvailableDisabledHash()] = tmp2;
			}
			map[newhash] = tmp; // bind old action to new control
		}
	}
}

void InputEventController::LoadMenuMap()
{
	ActionMap& cmap = allActionMaps[_("Menu")];
	cmap.clear();
	
	Config* config = Config::GetInstance();

	AssignAction(cmap, config->ui.menu_ok, actions.ui.menuOk);
	AssignAction(cmap, config->ui.menu_cancel, actions.ui.menuCancel);
}

void InputEventController::LoadConsoleMap()
{
	/*TODO
	// add console-keys map, which is all keys
	// console-keys map is special because it won't appear in the configuration panel,
	// so we don't need to internationalize the name of it.
	for(int i = (int) OIS::KC_ESCAPE; i < (int) OIS::KC_MEDIASELECT; i++) {
		allActionMaps["console-keys"][HashKeyboardEvent((OIS::KeyCode) i)].reset(
			new ConsoleKeyAction("" /* no name necessary /, 0 /* no ordering necessary /,
					NULL, (OIS::KeyCode) i));
	}

	ActionMap& cmap = allActionMaps[_("Console")];
	cmap.clear();
	
	Config* config = Config::GetInstance();

	cmap[config->ui.console_toggle].reset(new ConsoleToggleAction(_("Toggle Console"), 0, NULL, this));
	cmap[config->ui.console_up].reset(new ConsoleScrollAction(_("Page Up"), 1, NULL, -HighConsole::SCROLL_SPEED));
	cmap[config->ui.console_down].reset(new ConsoleScrollAction(_("Page Down"), 2, NULL, HighConsole::SCROLL_SPEED));
	cmap[config->ui.console_top].reset(new ConsoleScrollTopAction(_("Top"), 3, NULL));
	cmap[config->ui.console_bottom].reset(new ConsoleScrollBottomAction(_("Bottom"), 4, NULL));
	cmap[config->ui.console_help].reset(new ConsoleHelpAction(_("Help"), 5, NULL));
	*/
}

// 0x0000xx00; xx = keycode
int InputEventController::HashKeyboardEvent(const SDL_Keycode& key)
{
	return (key % 256) << 8;
}

/*TODO
// 0x004xx000; xx = mouse button
int InputEventController::HashMouseButtonEvent(const MouseEvent& arg, MouseButtonID id)
{
	return (0x00400000 | ((id % 256) << 12));
}

// 0x005xy000; x = axis id, y = direction
int InputEventController::HashMouseAxisEvent(const MouseEvent& arg, int axis, int direction)
{
	return (0x00500000 | ((axis % 16) << 16) | ((direction % 16) << 12));
}

// 0x008xxyy0; x = joystick id, y = button id
int InputEventController::HashJoystickButtonEvent(const JoyStickEvent& arg, int button)
{
	return (0x00800000 | ((arg.device->getID() % 256) << 12) | ((button % 256) << 4));
}

// 0x009xxyy0; x = joystick id, y = slider id
int InputEventController::HashJoystickSliderEvent(const JoyStickEvent& arg, int slider)
{
	return (0x00900000 | ((arg.device->getID() % 256) << 12) | ((slider % 256) << 4));
}

// 0x00Axxyz0; x = joystick id, y = direction, z = pov
int InputEventController::HashJoystickPovEvent(const JoyStickEvent& arg, int pov, int direction)
{
	return (0x00A00000 | ((arg.device->getID() % 256) << 12) |
						 ((direction % 16) << 8) | ((pov % 16) << 4));
}

// 0x00Bxxyz0; x = joystick id, y = axis id, z = direction
int InputEventController::HashJoystickAxisEvent(const JoyStickEvent& arg, int axis, int direction)
{
	return (0x00B00000 | ((arg.device->getID() % 256) << 12) |
						 ((axis % 16) << 8) | ((direction % 16) << 4));
}
*/

string InputEventController::HashToString(int hash)
{
	switch((hash & 0x00C00000) >> 22) {
		case 0: // keyboard event
			if((hash & 0x0000FF00) == 0)
				return "Disabled";
			else
				//TODO
				return "Unknown";
		/*TODO
		case 1: // mouse event
			if((hash & 0x00300000) == 0) {
				// OIS has no nice "getAsString" for mice so we have to do it by hand
				switch((hash & 0x000FF000) >> 12) {
					case MB_Left:
						return _("Left Mouse Btn");
					case MB_Right:
						return _("Right Mouse Btn");
					case MB_Middle:
						return _("Middle Mouse Btn");
					case MB_Button3:
						return _("Mouse Button 3");
					case MB_Button4:
						return _("Mouse Button 4");
					case MB_Button5:
						return _("Mouse Button 5");
					case MB_Button6:
						return _("Mouse Button 6");
					case MB_Button7:
						return _("Mouse Button 7");
					// apparently there is no support for more than 7 mouse buttons
					default:
						return _("Unknown Mouse Button");
				}
			} else {
				switch((hash & 0x000F0000) >> 16) {
					case AXIS_X:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Mouse X+ Axis");
						else
							return _("Mouse X- Axis");
					case AXIS_Y:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Mouse Y+ Axis");
						else
							return _("Mouse Y- Axis");
					case AXIS_Z:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Mouse Z+ Axis");
						else
							return _("Mouse Z- Axis");
					default:
						if(((hash & 0x0000F000) >> 12) == 1)
							return _("Unknown Mouse Axis +");
						else
							return _("Unknown Mouse Axis -");
				}
			}
		case 2: // joystick event
			char joynum[100];
			sprintf(joynum, "%s %d", _("Joystick"), (hash & 0x000FF000) >> 12);

			if((hash & 0x00300000) == 0) {
				// joystick button
				char ret[100];
				sprintf(ret, "%s %s %d\0", joynum, _("Button"), (hash & 0x00000FF0) >> 4);
				return ret;
			} else if(((hash & 0x00300000) >> 20) == 1) {
				// joystick slider
				char slider[100];
				sprintf(slider, "%s %s %d", joynum, _("Slider"), (hash & 0x00000FF0) >> 4);
				return slider;
			} else if(((hash & 0x00300000) >> 20) == 2) {
				// joystick pov
				char *povnum;
				switch((hash & 0x000000F0) >> 4) { // try to use longer gettext phrases
					case 1:
						povnum = _("POV 1");
						break;
					case 2:
						povnum = _("POV 2");
						break;
					case 3:
						povnum = _("POV 3");
						break;
					case 4:
						povnum = _("POV 4");
						break;
				}
				// now the direction
				char *dir;
				switch((hash & 0x00000F00) >> 8) {
					case Pov::Centered:
						dir = _("Center");
						break;
					case Pov::North:
						dir = _("North");
						break;
					case Pov::East:
						dir = _("East");
						break;
					case Pov::West:
						dir = _("West");
						break;
					case Pov::South:
						dir = _("South");
						break;
					case Pov::NorthEast:
						dir = _("Northeast");
						break;
					case Pov::NorthWest:
						dir = _("Northwest");
						break;
					case Pov::SouthEast:
						dir = _("Southeast");
						break;
					case Pov::SouthWest:
						dir = _("Southwest");
						break;
					default:
						dir = _("Unknown direction");
						break;
				}
				char ret[150];
				sprintf(ret, "%s %s %s\0", joynum, povnum, dir);
				return ret;
			} else {
				// joystick axis
				char joyax[150];
				sprintf(joyax, "%s %s %d%c",
					joynum,
					_("Axis"),
					(hash & 0x00000F00) >> 8,
					(((hash & 0x000000F0) >> 4) == 1) ? '+' : '-');
				return joyax;
			}
		*/
	}

	return _("Unknown");
}
