
// Controller.h
//
// Copyright (c) 2010 Ryan Curtin.
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

#include <vector>
#include <string>

#include <SDL2/SDL.h>

#include "Action.h"
#include "ControlAction.h"

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
	namespace MainCharacter {
		class MainCharacter;
	}
}

namespace HoverRace {
namespace Control {

class InputHandler;
typedef std::shared_ptr<InputHandler> InputHandlerPtr;

/***
 * Tracks actions which are either engaged (@c true) or disengaged (@c false).
 * In other words, whether the key or button is currently pressed or not.
 * @todo Analog instead of digital (well, the ones that can, at least).
 */
struct MR_DllDeclare ControlState {
	// TODO: make these inputs analog, not digital
	bool motorOn;
	bool jump;
	bool brake;
	bool fire;
	bool weapon;
	bool lookBack;
	bool right;
	bool left;
};

/**
 * Translates input events into actions.
 *
 * The app class (the one running the SDL event loop), passes along input events
 * to this class, which then fires the mapped action.  Listeners can subscribe
 * to the actions via the InputEventController#actions struct.
 *
 * <pre>
 *                           [Util::Config]
 *                                |
 *                                |
 *   [app] --(SDL_Event)--> [InputEventController] --(actions)--> [listeners]
 * </pre>
 *
 * Typically, to listen to an action, the listener must call the appropriate
 * Add* function to enable the group of actions then call Action#Connect on the
 * desired action to bind a function to the action.  In subclasses of
 * Client::Scene this is done in Client::Scene#AttachController.
 *
 * Example:
 *
 * <pre>
 *   controller.AddCameraMaps();
 *   cameraZoomInConn = controller.actions.camera.zoomIn->Connect(
 *     std::bind(&GameScene::OnCameraZoom, this, 1));
 * </pre>
 *
 * Action#Connect returns a boost::signals2::connection.  Be sure to call
 * disconnect() on this connection to clean up.  In subclasses of Client::Scene
 * this is done in Client::Scene#DetachController.
 *
 * @author Ryan Curtin
 * @author Michael Imamura
 */
class MR_DllDeclare InputEventController
{
public:
	InputEventController();
	~InputEventController();

	// Typedef for the maps of hashes to controls
	using ActionMap = std::map<int, ControlActionPtr>;

	enum axis_t {
		AXIS_X = 1,
		AXIS_Y,
		AXIS_Z,
		AXIS_WHEEL_X,
		AXIS_WHEEL_Y,
	};

	enum class ActionMapId : size_t
	{
		CAMERA = 0x01,
		CONSOLE = 0x02,
		CONSOLE_TOGGLE = 0x04,
		MENU = 0x08,
		PLAYER = 0x10,
	};

	using VoidActionPtr = std::shared_ptr<Action<voidSignal_t>>;
	using ValueActionPtr = std::shared_ptr<Action<valueSignal_t>>;
	using StringActionPtr =
		std::shared_ptr<Action<stringSignal_t, const std::string&>>;
	using TextControlActionPtr =
		std::shared_ptr<Action<textControlSignal_t, TextControl::key_t>>;
	using Vec2ActionPtr = std::shared_ptr<Action<vec2Signal_t, const Vec2&>>;
	using MouseClickActionPtr =
		std::shared_ptr<Action<mouseClickSignal_t, const Mouse::Click&>>;

	// event handlers
	bool OnKeyPressed(const SDL_KeyboardEvent &arg);
	bool OnKeyReleased(const SDL_KeyboardEvent &arg);
	bool OnTextInput(const SDL_TextInputEvent &evt);
	bool OnMouseMoved(const SDL_MouseMotionEvent &evt);
	bool OnMousePressed(const SDL_MouseButtonEvent &evt);
	bool OnMouseReleased(const SDL_MouseButtonEvent &evt);
	bool OnMouseWheel(const SDL_MouseWheelEvent &evt);
	/*TODO
	bool buttonPressed(const JoyStickEvent &arg, int button);
	bool buttonReleased(const JoyStickEvent &arg, int button);
	bool axisMoved(const JoyStickEvent &arg, int axis);
	bool povMoved(const JoyStickEvent &arg, int pov);
	*/

	void ProcessInputEvent(const SDL_Event &evt);
	void HandleEvent(int hash, int value);

	/**
	 * Capture the next user input event and reassign hash.
	 *
	 * This function tells the InputEventController to capture the next user
	 * input event and assign the action currently residing at 'oldhash' to the
	 * hash of the new input.  Behavior is undefined if there is no action
	 * assigned to the old hash, so don't screw it up!  This is meant to be
	 * called by the control assignment dialog box.
	 *
	 * @param oldhash Old hash.
	 * @param mapname String representing the name of the map.
	 */
	void CaptureNextInput(int oldhash, std::string mapname);

	/**
	 * Check if the controller is currently capturing input.
	 *
	 * It can be used to check whether or not an input has been captured.
	 *
	 * @return @c true if in capture mode, @c false otherwise.
	 * @see CaptureNextInput(int, std::string)
	 */
	bool IsCapturing();

	/**
	 * Exit capture mode.
	 */
	void StopCapture();

	/**
	 * Assign the next disabled hash to the current capture control.
	 *
	 * Nothing will be done if the InputEventController is not in capture mode.
	 * This function will also disable capture mode.
	 */
	void DisableCaptureInput();

	/**
	 * Clears all of the active control bindings.
	 *
	 * Does not delete the bindings but simply removes them from the active
	 * action map.
	 */
	void ClearActionMap();

	/**
	 * Add an action map into the current action map.  The available maps are
	 * referenced by string.  Maps include:
	 *
	 * "player1" ... "player4"
	 *
	 * @param mapname The action map name.
	 * @param mapId The action map ID.
	 * @return @c false if the map is not found
	 */
	bool AddActionMap(const std::string &mapname, ActionMapId mapId);

	/**
	 * Check if a control map is active.
	 * @param id The map to look up.
	 * @return @c true if active, @c false if inactive.
	 */
	bool IsMapActive(ActionMapId id)
	{
		return (activeMaps | static_cast<size_t>(id)) != 0;
	}

	/**
	 * Return the map with the given key.
	 */
	ActionMap& GetActionMap(std::string key);

	/**
	 * Return a vector containing the names of all the available maps.
	 */
	std::vector<std::string> GetAvailableMaps();

	/**
	 * Update player mappings to point to correct MainCharacter objects, then
	 * add them to the active action map.
	 *
	 * If @c nullptr is passed as any of the pointers that map will not be
	 * added.  The "console-toggle" map will also be added.
	 *
	 * @param numPlayers The number of players to update.
	 * @param mcs A pointer to the list of MainCharacter objects.
	 */
	void AddPlayerMaps(int numPlayers, MainCharacter::MainCharacter** mcs);

	void AddCameraMaps();
	void AddMenuMaps();
	void AddConsoleToggleMaps();
	void AddConsoleMaps();

	VoidActionPtr Hotkey(const std::string &key);

	static std::string HashToString(int hash);
	SDL_Keycode StringToKey(const std::string &s);

private:
	template<class T>
	static void AssignAction(ActionMap &cmap, int hash, const T &action)
	{
		cmap[hash] = action;
		action->SetPrimaryTrigger(hash);
	}

public:
	void LoadCameraMap();
	void LoadMenuMap();
	void LoadConsoleToggleMap();
	void LoadConsoleMap();

	void SaveConfig();
	void ReloadConfig();
	void LoadConfig();

private:
	// Auxiliary functions
	void RebindKey(std::string mapname, int oldhash, int newhash);

	// Hashing scheme (we have 32 bits but won't always use them):
	// disabled control
	// [000000000000000000][aaaaaaaaaaaa]
	//   a: next available disabled id
	// keyboard event
	// [00000000][00][000000][aaaaaaaa][00000000]: keycode
	//	 a: int keycode
	// [00000000][00][01][aaaaaaaaaaaa][00000000]: scancode as keycode
	//	 a: int scancode as keycode
	// mouse event
	// [00000000][01][00][aaaaaaaa][000000000000]: button press
	//   a: button id
	// [00000000][01][01][aaaa][bbbb][000000000000]: axis move
	//	 a: axis id
	//   b: direction
	// joystick event
	// [00000000][10][00][aaaaaaaa][bbbbbbbb][0000]: button press
	//   a: joystick id
	//   b: button id
	// [00000000][10][01][aaaaaaaa][bbbbbbbb][0000]: slider move
	//   a: joystick id
	//   b: slider id
	// [00000000][10][10][aaaaaaaa][bbbb][cccc][0000]: pov move
	//   a: joystick id
	//   b: direction
	//   c: pov id
	// [00000000][10][11][aaaaaaaa][bbbb][cccc][0000]: axis move
	//   a: joystick id
	//   b: axis id
	//   c: direction
	int GetNextAvailableDisabledHash();

public:
	static int HashKeyboardEvent(const SDL_Keycode& arg);
	static int HashMouseButtonEvent(const SDL_MouseButtonEvent& arg);
	static int HashMouseAxisEvent(axis_t axis, int direction);
	/*TODO
	static int HashJoystickAxisEvent(const JoyStickEvent& arg, int axis, int direction);
	static int HashJoystickSliderEvent(const JoyStickEvent& arg, int slider);
	static int HashJoystickButtonEvent(const JoyStickEvent& arg, int button);
	static int HashJoystickPovEvent(const JoyStickEvent& arg, int pov, int direction);
	*/

private:
	/**
	 * We store several different action maps which we can choose from.
	 * They are referenced by string.  See ClearActionMap(), AddActionMap().
	 */
	ActionMap actionMap;
	size_t activeMaps;
	std::map<std::string, ActionMap> allActionMaps;
	std::unordered_map<SDL_Keycode, VoidActionPtr> hotkeys;

	int nextAvailableDisabledHash;

	bool captureNextInput;
	int  captureOldHash; ///< stores the value of the hash we will be replacing when capturing input
	std::string captureMap; ///< name of the map we are capturing for

public:
	struct actions_t
	{
		struct ui_t
		{
			ui_t();
			VoidActionPtr menuOk;
			VoidActionPtr menuCancel;
			VoidActionPtr menuExtra;  ///< Scene-specific extra action.
			VoidActionPtr menuUp;
			VoidActionPtr menuDown;
			VoidActionPtr menuLeft;
			VoidActionPtr menuRight;
			VoidActionPtr menuNext;
			VoidActionPtr menuPrev;

			// Console scrolling.
			VoidActionPtr consoleUp;
			VoidActionPtr consoleDown;
			VoidActionPtr consoleTop;
			VoidActionPtr consoleBottom;
			VoidActionPtr consolePrevCmd;
			VoidActionPtr consoleNextCmd;

			// Text input mode.
			StringActionPtr text;  ///< Text input.
			/// Text input control key (param is TextControl::key_t).
			TextControlActionPtr control;

			// Low-level UI widget controls.
			Vec2ActionPtr mouseMoved;
			MouseClickActionPtr mousePressed;
			MouseClickActionPtr mouseReleased;
		} ui;
		struct sys_t
		{
			sys_t();
			VoidActionPtr consoleToggle;
		} sys;
		struct camera_t
		{
			camera_t();
			VoidActionPtr zoomIn;
			VoidActionPtr zoomOut;
			VoidActionPtr panUp;
			VoidActionPtr panDown;
			VoidActionPtr reset;
		} camera;
	} actions;
};

} // namespace Control
} // namespace HoverRace

#undef MR_DllDeclare
