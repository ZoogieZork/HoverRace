
/* config-win32.h
	Win32 counterpart to the config.h generated from Linux build. */

#include "ntverp.h"

#define PACKAGE "hoverrace"
#define PACKAGE_NAME "HoverRace"
#define PLATFORM_NAME "Win32"

#define ENABLE_NLS 1
#define LOCALEDIR "../share/locale"

#define HAVE_LUA 1

#define HAVE_LUABIND 1
#define LUABIND_DYNAMIC_LINK 1

// Use wide (Unicode) paths.
#define WITH_WIDE_PATHS 1

#define WITH_OBJSTREAM 1

// Uncomment this to use SDL_Mixer instead of OpenAL.
//#define WITH_SDL_MIXER

#define WITH_CHRONO_TIMESTAMP
