
// Rule.h
//
// Copyright (c) 2013 Michael Imamura.
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

#include <luabind/object.hpp>

namespace HoverRace {
namespace Client {

/**
 * Base class for a user-configurable rule in a Rulebook.
 * @author Michael Imamura
 */
class Rule
{
	public:
		Rule(const std::string &label="") : label(label) { }
		virtual ~Rule() { }

	public:
		virtual luabind::object GetDefault() const = 0;

	private:
		std::string label;
};

}  // namespace Client
}  // namespace HoverRace
