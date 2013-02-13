
// ClientApp.cpp
// Experimental game shell.
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

#include "StdAfx.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#ifdef WITH_SDL_PANGO
#	include <SDL_Pango.h>
#endif

#include "../../engine/Exception.h"
#include "../../engine/MainCharacter/MainCharacter.h"
#include "../../engine/Model/Track.h"
#include "../../engine/Parcel/TrackBundle.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/FuzzyLogic.h"
#include "../../engine/Util/DllObjectFactory.h"
#include "../../engine/Util/Str.h"
#include "../../engine/Util/WorldCoordinates.h"
#include "../../engine/VideoServices/SoundServer.h"
#include "../../engine/VideoServices/VideoBuffer.h"

#include "Control/Controller.h"
#include "Control/UiHandler.h"
#include "HoverScript/ClientScriptCore.h"
#include "HoverScript/GamePeer.h"
#include "HoverScript/HighConsole.h"
#include "HoverScript/SessionPeer.h"
#include "HoverScript/SysEnv.h"
#include "ClientSession.h"
#include "GameScene.h"
#include "HighObserver.h"
#include "Rulebook.h"
#include "Scene.h"

#ifdef _WIN32
#	include "resource.h"
#endif

#include "ClientApp.h"

using namespace HoverRace::Client::HoverScript;
using namespace HoverRace::Util;
namespace SoundServer = HoverRace::VideoServices::SoundServer;
using HoverRace::Client::Control::InputEventController;

namespace {
	/**
	 * Draws a color grid to debug the color palette.
	 * @param surface The destination surface (must be an 8-bit surface).
	 */
	void DrawPalette(SDL_Surface *surface)
	{
		if (SDL_MUSTLOCK(surface)) {
			if (SDL_LockSurface(surface) < 0) {
				throw HoverRace::Exception("Unable to lock surface");
			}
		}

		MR_UInt8 *buf = static_cast<MR_UInt8*>(surface->pixels);
		for (int y = 0; y < 256; y++, buf += surface->pitch) {
			if ((y % 16) == 0) continue;
			MR_UInt8 *cur = buf;
			for (int x = 0; x < 256; x++, cur++) {
				if ((x % 16) == 0) continue;
				*cur = ((y >> 4) << 4) + (x >> 4);
			}
		}

		if (SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}

		SDL_Flip(surface);
	}
}

namespace HoverRace {
namespace Client {

class ClientApp::UiInput : public Control::UiHandler
{
	virtual void OnConsole()
	{
		throw UnimplementedExn("ClientApp::UiInput::OnConsole()");
	}
};

ClientApp::ClientApp() :
	SUPER(),
	uiInput(std::make_shared<UiInput>()),
	scene(NULL)
{
	Config *cfg = Config::GetInstance();

	// Engine initialization.
	MR_InitTrigoTables();
	MR_InitFuzzyModule();
	SoundServer::Init();
	DllObjectFactory::Init();
	MainCharacter::MainCharacter::RegisterFactory();

	if (SDL_Init(SDL_INIT_VIDEO) == -1)
		throw Exception("SDL initialization failed");
#	ifdef WITH_SDL_PANGO
		SDLPango_Init();
#	endif

	// Create the system console and execute the init script.
	// This allows the script to modify the configuration (e.g. for unit tests).
	scripting = (new ClientScriptCore())->Reset();
	gamePeer = new GamePeer(scripting, this);
	sysEnv = new SysEnv(scripting, gamePeer);
	OS::path_t &initScript = cfg->runtime.initScript;
	if (!initScript.empty()) {
		sysEnv->RunScript(initScript);
	}

	// With SDL we can only get the desktop resolution before the first call to
	// SDL_SetVideoMode().
	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	int desktopWidth = videoInfo->current_w;
	int desktopHeight = videoInfo->current_h;

	// Create the main window and SDL surface.
	if (SDL_SetVideoMode(cfg->video.xRes, cfg->video.yRes, 0,
		SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE) == NULL)
	{
		throw Exception("Unable to create video surface");
	}

	// Set window position and icon (platform-dependent).
	SDL_SysWMinfo wm;
	SDL_VERSION(&wm.version);
	if (SDL_GetWMInfo(&wm) != 0) {
#		ifdef _WIN32
			HWND hwnd = mainWnd = wm.window;
			SetWindowPos(hwnd, HWND_TOP,
				cfg->video.xPos, cfg->video.yPos, 0, 0,
				SWP_NOSIZE);

			// Set icon.
			// On Windows, the icon is embedded as a resource.
			HMODULE hmod = GetModuleHandleW(NULL);
			LPWSTR iconRes = MAKEINTRESOURCEW(IDI_HOVER_ICON);
			HANDLE ico;
			ico = LoadImageW(hmod, iconRes, IMAGE_ICON,
				GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
			if (ico != NULL)
				SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(ico));
			ico = LoadImageW(hmod, iconRes, IMAGE_ICON,
				GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
			if (ico != NULL)
				SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(ico));
#		else
			mainWnd = wm.info.x11.window;
			// On non-Win32 we prefer to let the window manager decide the
			// position of the window.
			/*TODO
			SDL_WM_SetIcon(SDL_LoadBMP(cfg->GetMediaPath("icon.bmp").file_string().c_str()), 0);
			*/
#		endif
	}

	videoBuf = new VideoServices::VideoBuffer();
	videoBuf->OnDesktopModeChange(desktopWidth, desktopHeight);

	AssignPalette();

	controller = new InputEventController(mainWnd, uiInput);

	RefreshTitleBar();
}

ClientApp::~ClientApp()
{
	delete scene;

	delete sysEnv;
	delete gamePeer;
	delete scripting;
	delete videoBuf;
	delete controller;

	// Engine shutdown.
	DllObjectFactory::Clean(FALSE);
	SoundServer::Close();

	SDL_Quit();
}

void ClientApp::RefreshTitleBar()
{
	Config *cfg = Config::GetInstance();

	std::ostringstream oss;
	oss << PACKAGE_NAME " " << cfg->GetVersion();
	if (cfg->IsPrerelease()) {
		oss << " (" << pgettext("Version", "testing") << ')';
	}
	if (cfg->runtime.silent) {
		oss << " (" << _("silent mode") << ')';
	}

	SDL_WM_SetCaption(oss.str().c_str(), NULL);
}

void ClientApp::RenderScene()
{
	VideoServices::VideoBuffer::Lock lock(videoBuf);

	if (scene == NULL) {
		videoBuf->Clear();
	}
	else {
		scene->Render();
	}
}

void ClientApp::MainLoop()
{
	bool quit = false;
	SDL_Event evt;

	// Fire all on_init handlers and check if a new session was requested.
	gamePeer->OnInit();
	RulebookPtr rules = gamePeer->RequestedNewSession();
	if (rules != NULL) {
		NewLocalSession(rules);
	}

	SDL_Surface *surface = SDL_GetVideoSurface();
	videoBuf->OnWindowResChange();

#	ifdef WITH_SDL_OIS_INPUT
		std::vector<SDL_Event> deferredEvents;
#	endif

	while (!quit) {
		OS::timestamp_t tick = OS::Time();

		while (SDL_PollEvent(&evt) && !quit) {
			switch (evt.type) {
				case SDL_QUIT:
					quit = true;
					break;

				case SDL_VIDEORESIZE:
					if ((surface = SDL_SetVideoMode(evt.resize.w, evt.resize.h,
						8, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE))
						== NULL)
					{
						throw Exception("Unable to resize video surface");
					}
					videoBuf->OnWindowResChange();
					AssignPalette();
					break;

				// Certain SDL events need to be processed by the SDL-OIS
				// bridge, so we need to save them for later.
#				ifdef WITH_SDL_OIS_INPUT
					case SDL_KEYDOWN:
					case SDL_KEYUP:
							deferredEvents.push_back(evt);
						break;
#				endif
			}
			//TODO: Check for resize event and call NotifyWindowResChange().
		}
		if (quit) break;

#		ifdef WITH_SDL_OIS_INPUT
			BOOST_FOREACH(SDL_Event &evt, deferredEvents) {
				SDL_PushEvent(&evt);
			}
			deferredEvents.clear();
#		endif
		controller->Poll();

		if (scene != NULL) {
			scene->Advance(tick);
			//TODO: Check for scene change notification.
		}

		RenderScene();
	}
}

void ClientApp::NewLocalSession(RulebookPtr rules)
{
	//TODO: Confirm ending the current session.

	// Shut down the current session (if any).
	if (scene != NULL) {
		delete scene;
		scene = NULL;
	}

	//TODO: Prompt the user for a track name.
	try {
		scene = new GameScene(this, videoBuf, controller, scripting, gamePeer, rules);
	}
	catch (Parcel::ObjStreamExn&) {
		throw;
	}

	AssignPalette();
}

void ClientApp::RequestShutdown()
{
	throw UnimplementedExn("ClientApp::RequestShutdown()");
}

void ClientApp::SignalServerHasChanged()
{
	throw UnimplementedExn("ClientApp::SignalServerHasChanged()");
}

void ClientApp::ChangeAutoUpdates(bool newSetting)
{
	throw UnimplementedExn("ClientApp::ChangeAutoUpdates(bool)");
}

void ClientApp::AssignPalette()
{
	Config *cfg = Config::GetInstance();

	videoBuf->CreatePalette();
	videoBuf->AssignPalette();
}

Control::InputEventController *ClientApp::ReloadController()
{
	//TODO: Notify current scene that controller is changing.
	delete controller;
	return (controller = new InputEventController(mainWnd, uiInput));
}

}  // namespace HoverScript
}  // namespace Client
