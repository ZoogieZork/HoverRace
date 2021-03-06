
// Core.cpp
//
// Copyright (c) 2009, 2010, 2014 Michael Imamura.
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

#include <iostream>

#include <luabind/luabind.hpp>

#include "../Util/Config.h"
#include "../Util/Log.h"
#include "../Util/OS.h"
#include "../Util/Str.h"
#include "../Util/yaml/MapNode.h"
#include "../Util/yaml/ScalarNode.h"
#include "../Util/yaml/SeqNode.h"
#include "../Util/yaml/Parser.h"
#include "Help/HelpHandler.h"
#include "Help/Class.h"
#include "Help/Event.h"
#include "Help/Method.h"
#include "Peer.h"

#include "Core.h"

using namespace HoverRace::Script;
using HoverRace::Util::OS;
namespace Log = HoverRace::Util::Log;
namespace Str = HoverRace::Util::Str;

#define REG_LUA_LIB(st, name, fn) \
	lua_pushcfunction((st), (fn)); \
	lua_pushstring((st), (name)); \
	lua_call((st), 1, 0)

#define UNDEF_LUA_GLOBAL(state, name) \
	lua_pushnil(state); \
	lua_setglobal((state), (name))

#define DISALLOW_LUA_GLOBAL(state, name) \
	lua_pushstring((state), (name)); \
	lua_pushcclosure((state), Core::LSandboxedFunction, 1); \
	lua_setglobal((state), (name))

namespace HoverRace {
namespace Script {

const std::string Core::DEFAULT_CHUNK_NAME("=lua");

Core::Core() :
	NIL(nullptr), curHelpHandler(NULL)
{
	state = luaL_newstate();

	//TODO: Set panic handler.
}

Core::~Core()
{
	delete NIL;
	lua_close(state);
}

/**
 * Reset changes to the global environment.
 * When creating a new instance, this member function must be called at least
 * once before executing any scripts.
 * This is used for fixing accidental changes to globals.
 * Note that this will effectively deactivate the security sandbox.
 * Call ActivateSandbox() to reactivate if necessary.
 * The state returned by GetState() is otherwise unchanged.
 * @return The same instance.
 */
Core *Core::Reset()
{
	// Register a "safe" set of standard libraries.
	// Load standard libraries, then remove references to libraries which we
	// don't want any scripts to have access to.
	// This is to be compatible with LuaJIT.
	luaL_openlibs(state);
	UNDEF_LUA_GLOBAL(state, "debug");
	UNDEF_LUA_GLOBAL(state, "io");
	UNDEF_LUA_GLOBAL(state, "os");
	UNDEF_LUA_GLOBAL(state, "package");

	// Override the print function so we can reroute the output.
	lua_pushlightuserdata(state, this);
	lua_pushcclosure(state, Core::LPrint, 1);
	lua_setglobal(state, "print");

	// Remove the metatable protection from the global table.
	// We don't need to do the same to the string metatable, since
	// luaopen_string handles that for us.
	lua_rawgeti(state, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	lua_pushnil(state);
	lua_setmetatable(state, 1);
	lua_pop(state, 1);

	// Startup Luabind.
	luabind::open(state);
	luabind::set_pcall_callback(&Core::ErrorFunc);
	Peer::Register(this);

	if (NIL) delete NIL;
	NIL = new luabind::object();

	return this;
}

/**
 * Activate the security sandbox.
 * This guards against methods a script may use to access the filesystem
 * or break out of the sandbox.
 */
void Core::ActivateSandbox()
{
	if (!lua_checkstack(state, 2))
		throw ScriptExn("Out of stack space");

	// Create a reusable metatable protector.
	// This prevents modification of the metatable.
	lua_newtable(state);
	lua_pushboolean(state, 1);
	lua_setfield(state, -2, "__metatable");
	int metatableProtector = luaL_ref(state, LUA_REGISTRYINDEX);

	// Protect the metatable for the global table.
	// We do this instead of just destroying/replacing the "_G" global so that
	// users can still query the table for a list of keys, etc.
	lua_rawgeti(state, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	lua_rawgeti(state, LUA_REGISTRYINDEX, metatableProtector);
	lua_setmetatable(state, -2);
	lua_pop(state, 1);

	// Protect the shared metatable for strings.
	lua_pushstring(state, "");
	lua_getmetatable(state, -1);
	lua_pushboolean(state, 1);
	lua_setfield(state, -2, "__metatable");
	lua_pop(state, 2);

	// Clean up the registry.
	luaL_unref(state, LUA_REGISTRYINDEX, metatableProtector);

	// Mostly based on the list at: http://lua-users.org/wiki/SandBoxes
	DISALLOW_LUA_GLOBAL(state, "collectgarbage");
	DISALLOW_LUA_GLOBAL(state, "dofile");
	DISALLOW_LUA_GLOBAL(state, "getfenv");
	DISALLOW_LUA_GLOBAL(state, "load");
	DISALLOW_LUA_GLOBAL(state, "loadfile");
	DISALLOW_LUA_GLOBAL(state, "loadstring");
	DISALLOW_LUA_GLOBAL(state, "module");
	DISALLOW_LUA_GLOBAL(state, "rawequal");
	DISALLOW_LUA_GLOBAL(state, "rawget");
	DISALLOW_LUA_GLOBAL(state, "rawset");
	DISALLOW_LUA_GLOBAL(state, "require");
	DISALLOW_LUA_GLOBAL(state, "setfenv");
}

/**
 * The error function for Luabind.
 * We use the default Lua function for non-Luabind calls.
 * @param L Lua state.
 * @return
 */
int Core::ErrorFunc(lua_State *L)
{
	// Basically straight from the Luabind documentation.
	lua_Debug d;
	lua_getstack(L, 1, &d);
	lua_getinfo(L, "Sln", &d);
	std::string err = lua_tostring(L, -1);
	lua_pop(L, 1);
	std::stringstream msg;
	msg << d.short_src << ':' << d.currentline;

	if (d.name != 0) {
		msg << '(' << d.namewhat << ' ' << d.name << ')';
	}
	msg << ' ' << err << '\n';

	// Generate the traceback and append it to the error.
	luaL_traceback(L, L, nullptr, 1);
	msg << lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, msg.str().c_str());
	return 1;
}

/**
 * Redirect output to a stream.
 * @param out The output stream (wrapped in a shared pointer).
 *            May be @c NULL to use the system default.
 * @return A handle for removing the stream later.
 */
Core::OutHandle Core::AddOutput(std::shared_ptr<std::ostream> out)
{
	outs.push_back(out);
	return --(outs.end());
}

void Core::RemoveOutput(const OutHandle &handle)
{
	outs.erase(handle);
}

/**
 * Retrieve the full scripting version string (name and version).
 * @return The string (never empty).
 */
std::string Core::GetVersionString() const
{
	return LUA_VERSION;
}

void Core::PrintFromStack(lua_State *state, int n)
{
	// Initial stack: args...

	if (n > 0) {
		lua_getglobal(state, "print");  // args... fn
		lua_insert(state, -(n + 1));  // fn args...
		if (lua_pcall(state, n, 0, 0) != 0) {
			throw ScriptExn(PopError(state));
		}
	}
}

/**
 * Print a message to all registered output streams.
 * @param s The message to broadcast (a newline will be appended automatically).
 */
void Core::Print(const std::string &s)
{
	for (auto &out : outs) {
		*out << s << std::endl;
	}
}

/**
 * Compile a chunk of code.
 * Upon successful execution, the compiled chunk will be pushed to the stack.
 * @param chunk The chunk to compile.
 * @throw IncompleteExn If the code does not complete a statement; i.e.,
 *                      expecting more tokens.  Callers can catch this
 *                      to keep reading more data to finish the statement.
 * @throw ScriptExn The code failed to compile.
 */
void Core::Compile(const Chunk &chunk)
{
	auto &src = chunk.src;
	int status = luaL_loadbuffer(state, src.c_str(), src.length(),
		chunk.name.c_str());

	if (status != 0) {
		std::string msg = PopError(state);
		if (msg.find("<eof>") != std::string::npos) {
			// Incomplete chunk.
			// Callers can use this to keep reading more data.
			throw IncompleteExn(msg);
		}
		else {
			// Other compilation error.
			throw ScriptExn(msg);
		}
	}
}

/**
 * Pop a function off the stack and execute it.
 * @param numParams The number of params being passed to the function.
 * @param helpHandler Optional callback for when a script requests API help.
 * @return The number of return values from the function that remain
 *         on the stack.
 * @throw ScriptExn The code signaled an error while executing.
 */
int Core::Call(int numParams, Help::HelpHandler *helpHandler)
{
	int initStack = lua_gettop(state);
	if (initStack == 0) {
		throw ScriptExn("No function on stack.");
	}
	initStack -= (1 + numParams);

	// Execute the chunk.
	curHelpHandler = helpHandler;
	int status = lua_pcall(state, numParams, LUA_MULTRET, 0);
	curHelpHandler = NULL;
	if (status != 0) {
		throw ScriptExn(PopError(state));
	}

	return lua_gettop(state) - initStack;
}

void Core::PrintStack()
{
	int num = lua_gettop(state);
	std::ostringstream oss;
	for (int i = 1; i <= num; ++i) {
		oss << i << ": ";
		int type = lua_type(state, i);
		switch (type) {
			case LUA_TBOOLEAN: oss << "bool<" << (lua_toboolean(state, i) ? "true" : "false") << '>'; break;
			case LUA_TFUNCTION: oss << "function"; break;
			case LUA_TLIGHTUSERDATA: oss << "lightuserdata"; break;
			case LUA_TNIL: oss << "nil"; break;
			case LUA_TNUMBER: oss << "number<" << lua_tonumber(state, i) << '>'; break;
			case LUA_TSTRING: oss << "string<" << lua_tostring(state, i) << '>'; break;
			case LUA_TTABLE: oss << "table"; break;
			case LUA_TTHREAD: oss << "thread"; break;
			case LUA_TUSERDATA: oss << "userdata"; break;
			default: oss << "unknown type: " << type; break;
		}
		oss << std::endl;
	}
	std::string s = oss.str();
	for (auto &out : outs) {
		*out << s << std::flush;
	}
}

/**
 * Request the documentation for a scripting API class.
 * This must only be called from a script peer as the result of a Lua invocation!
 * @param className The name of the class (may not be blank).
 */
void Core::ReqHelp(const std::string &className)
{
	ReqHelp(className, "");
}

/**
 * Request the documentation for a scripting API class or method.
 * This must only be called from a script peer as the result of a Lua invocation!
 * @param className The name of the class (may not be blank).
 * @param methodName The name of the method within the class (may be blank to
 *                   retrieve the class documentation instead of the method
 *                   documentation).
 */
void Core::ReqHelp(const std::string &className, const std::string &methodName)
{
	if (curHelpHandler == NULL) return;

	// Find the class documentation in the cache.
	helpClasses_t::const_iterator iter = helpClasses.find(className);
	if (iter == helpClasses.end()) {
		// Not found, try to load from disk, then look again.
		LoadClassHelp(className);
		iter = helpClasses.find(className);
		if (iter == helpClasses.end()) {
			luaL_error(state, "Class \"%s\" %s", className.c_str(),
				"has no documentation");
			return;
		}
	}
	Help::ClassPtr cls = iter->second;

	if (methodName.empty()) {
		curHelpHandler->HelpClass(*cls);
	}
	else {
		Help::MethodPtr method = cls->GetMethod(methodName);
		if (!method) {
			luaL_error(state, "Class \"%s\" %s \"%s\"",
				className.c_str(), "has no method", methodName.c_str());
			return;
		}
		else {
			curHelpHandler->HelpMethod(*cls, *method);
		}
	}
}

void Core::LoadClassHelp(const std::string &className)
{
	Util::Config *cfg = Util::Config::GetInstance();
	OS::path_t filename = cfg->GetScriptHelpPath(className);

	FILE *in = OS::FOpen(filename, "rb");
	if (in == NULL) {
		Log::Warn("Class help file not found: %s", (const char*)Str::PU(filename));
		return;
	}

	yaml::Parser *parser = NULL;
	try {
		parser = new yaml::Parser(in);
		yaml::Node *node = parser->GetRootNode();

		yaml::MapNode *root = dynamic_cast<yaml::MapNode*>(node);
		if (root == NULL) {
			std::string filenamestr = (const char*)Str::PU(filename);
			throw yaml::ParserExn(
				(filenamestr + ": Expected root node to be a map.").c_str());
		}
		Help::ClassPtr cls = std::make_shared<Help::Class>(className);
		cls->Load(root);
		helpClasses.insert(helpClasses_t::value_type(className, cls));

		delete parser;
	}
	catch (yaml::EmptyDocParserExn&) {
		// Ignore.
	}
	catch (yaml::ParserExn &ex) {
		Log::Error("%s", ex.what());
		if (parser != NULL) delete parser;
		fclose(in);
	}

	fclose(in);
}

/**
 * Pop the error message off the stack.
 *
 * Only call this if you KNOW that there is an error on the top of the stack.
 *
 * @param state The Lua context.
 * @return The error as a string.
 */
std::string Core::PopError(lua_State *state)
{
	size_t len;
	const char *s = lua_tolstring(state, -1, &len);
	std::string msg(s, len);
	lua_pop(state, 1);
	return msg;
}

int Core::LPrint(lua_State *state)
{
	Core *self = static_cast<Core*>(lua_touserdata(state, lua_upvalueindex(1)));

	int numParams = lua_gettop(state);

	// Note: This aims to be pretty close to the print() function from the Lua
	//       base lib, including allowing the global tostring() to be
	//       replaced (e.g. to support additional types).

	lua_getglobal(state, "tostring");

	for (int i = 1; i <= numParams; ++i) {
		// Call the global "tostring" function
		lua_pushvalue(state, -1);
		lua_pushvalue(state, i);
		lua_call(state, 1, 1);

		// Get the result.
		const char *s = lua_tostring(state, -1);
		if (s == NULL) {
			// tostring() returned a non-string result.
			// This can happen if the tostring function is replaced.
			ASSERT(false);
			lua_pop(state, 1);
			continue;
		}
		for (auto &oss : self->outs) {
			if (i > 1) *oss << '\t';
			*oss << s;
		}
		lua_pop(state, 1);
	}
	for (auto &oss : self->outs) {
		*oss << std::endl;
	}

	return 0;
}

int Core::LSandboxedFunction(lua_State *state)
{
	const char *name = lua_tostring(state, lua_upvalueindex(1));
	return luaL_error(state, "%s: %s",
		"Disallowed access to protected function", name);
}

//{{{ StackRestore ////////////////////////////////////////////////////////////

Core::StackRestore::StackRestore(lua_State *state) :
	state(state), initStack(lua_gettop(state))
{
}

Core::StackRestore::~StackRestore()
{
	lua_settop(state, initStack);
}

//}}} StackRestore

}  // namespace Script
}  // namespace HoverRace
