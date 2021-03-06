
// Counter.h
//
// Copyright (c) 2013, 2014 Michael Imamura.
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

#include "HudDecor.h"

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
	namespace Display {
		class Display;
		class FillBox;
		class Label;
	}
}

namespace HoverRace {
namespace Display {

/**
 * HUD lap / score / whatever counter.
 * @author Michael Imamura
 */
class MR_DllDeclare Counter : public HudDecor
{
	typedef HudDecor SUPER;

private:
	static const double NO_TOTAL;

public:
	Counter(Display &display, const std::string &title, double initValue,
		double total=NO_TOTAL);
	virtual ~Counter() { }

protected:
	void OnHudRescaled(const Vec2 &hudScale) override;

public:
	/// Retrieve the current value.
	double GetValue() const { return value; }
	void SetValue(double value);

	/**
	 * Retrieve the total value.
	 * @note Use IsTotalHidden() to determine if the total is hidden.
	 * @return The value.
	 */
	double GetTotal() const { return total; }
	void SetTotal(double total);

	bool IsTotalHidden() const;
	void HideTotal();

protected:
	void Layout() override;
public:
	void Advance(Util::OS::timestamp_t tick) override;

private:
	double value;
	double total;
	bool valueChanged;
	std::shared_ptr<FillBox> bg;
	std::shared_ptr<Label> titleLbl;
	std::shared_ptr<Label> valueLbl;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
