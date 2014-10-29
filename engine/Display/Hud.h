
// Hud.h
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

#include "../Util/OS.h"
#include "../Exception.h"
#include "HudDecor.h"

#include "Container.h"

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
	}
	namespace MainCharacter {
		class MainCharacter;
	}
	namespace Model {
		class Track;
	}
	namespace Player {
		class Player;
	}
}

namespace HoverRace {
namespace Display {

/**
 * The container for the heads-up display.
 * @author Michael Imamura
 */
class MR_DllDeclare Hud : public Container
{
	typedef Container SUPER;

public:
	struct Props
	{
		enum {
			PLAYER = SUPER::Props::NEXT_,
			TRACK,
			NEXT_,  ///< First index for subclasses.
		};
	};
	struct HudAlignment
	{
		enum type {
			ABOVE,  ///< Centered in the top-half of the screen.
			BELOW,  ///< Centered in the bottom-half of the screen.
			N,      ///< Center-north, stacked left-to-right.
			NNE,    ///< Northeast corner, stacked right-to-left.
			NE,     ///< Northeast corner, only top is visible.
			ENE,    ///< Northeast corner, stacked top-to-bottom.
			E,      ///< Center-east, stacked top-to-bottom.
			ESE,    ///< Southeast corner, stacked bottom-to-top.
			SE,     ///< Southeast corner, only top is visible.
			SSE,    ///< Southeast corner, stacked right-to-left.
			S,      ///< Center-south, stacked left-to-right.
			SSW,    ///< Southwest corner, stacked left-to-right.
			SW,     ///< Southwest corner, only top is visible.
			WSW,    ///< Southwest corner, stacked bottom-to-top.
			W,      ///< Center-west, stacked top-to-bottom.
			WNW,    ///< Northwest corner, stacked top-to-bottom.
			NW,     ///< Northwest corner, only top is visible.
			NNW,    ///< Northwest corner, stacked left-to-right.
		};
		static const size_t NUM = NNW + 1;

		/**
		 * Safely convert from an int.
		 * @param t The int value.
		 * @return The HudAlignment enum value.
		 * @throws Exception The int value is does not map to a valid
		 *                   enum value.
		 */
		static type FromInt(int t) {
			if (t < 0 || t > NNW) {
				std::ostringstream oss;
				oss << "Invalid HUD alignment: " << t;
				throw Exception(oss.str());
			}
			return static_cast<type>(t);
		}

		static bool IsCorner(type t) {
			return t == NW || t == NE || t == SE || t == SW;
		}

		static Alignment AlignmentFor(type t) {
			switch (t) {
				case ABOVE:
					return Alignment::S;
				case BELOW:
					return Alignment::N;
				case N:
					return Alignment::N;
				case NNE:
				case NE:
				case ENE:
					return Alignment::NE;
				case E:
					return Alignment::E;
				case ESE:
				case SE:
				case SSE:
					return Alignment::SE;
				case S:
					return Alignment::S;
				case SSW:
				case SW:
				case WSW:
					return Alignment::SW;
				case W:
					return Alignment::W;
				case WNW:
				case NW:
				case NNW:
				default:
					return Alignment::NW;
			}
		}
	};  // HudAlignment

	enum class HudCell {
		FILL,   ///< Fill the screen.
		NE,     ///< Upper-right quadrant.
		E,      ///< Right half.
		SE,     ///< Lower-right quadrant.
		SW,     ///< Lower-left quadrant.
		W,      ///< Left half.
		NW,     ///< Upper-left quadrant.
	};
protected:
	struct MR_DllDeclare HudChild
	{
		HudChild(std::shared_ptr<HudDecor> decor);
		HudChild(const HudChild&) = delete;
		HudChild(HudChild &&other);

		HudChild &operator=(const HudChild&) = delete;
		HudChild &operator=(HudChild &&other);

		std::shared_ptr<HudDecor> decor;
		boost::signals2::scoped_connection sizeChangedConn;
	};

public:
	Hud(Display &display, std::shared_ptr<Player::Player> player,
		std::shared_ptr<Model::Track> track,
		uiLayoutFlags_t layoutFlags=0);
	virtual ~Hud() { }

/* Using the view for Container.
public:
	virtual void AttachView(Display &disp) { AttachViewDynamic(disp, this); }
*/

public:
	/**
	 * Append a child element to the end of the list.
	 * @param alignment Where to place the child element in the HUD.
	 * @param child The child element; must be a subclass of UiViewModel.
	 * @return The child element, wrapped in a @c std::shared_ptr.
	 */
	template<typename T>
	typename std::enable_if<std::is_base_of<HudDecor, T>::value, std::shared_ptr<T>>::type
	AddHudChild(HudAlignment::type alignment, T *child)
	{
		std::shared_ptr<T> sharedChild = AddChild(child);
		sharedChild->SetPlayer(player);
		sharedChild->SetTrack(track);

		// For corner elems, replace the elem instead of adding.
		if (HudAlignment::IsCorner(alignment)) {
			auto &elems = hudChildren[alignment];
			if (!elems.empty()) {
				RemoveChild(elems.back().decor);
				elems.clear();
			}
		}

		sharedChild->SetAlignment(HudAlignment::AlignmentFor(alignment));

		auto &children = hudChildren[alignment];
		children.emplace_back(sharedChild);

		// Trigger layout when the child changes size.
		children.back().sizeChangedConn =
			sharedChild->GetSizeChangedSignal().connect(
				std::bind(&Hud::RequestLayout, this));

		RequestLayout();
		return sharedChild;
	}

	virtual void Clear()
	{
		for (auto &children : hudChildren) {
			children.clear();
		}
		SUPER::Clear();
	}

protected:
	template<typename Fn>
	void ForEachHudChild(Fn fn)
	{
		for (auto &children : hudChildren) {
			for (auto &child : children) {
				fn(child.decor);
			}
		}
	}

public:
	void SetPlayer(std::shared_ptr<Player::Player> player);

	Model::Track *GetTrack() const { return track.get(); }
	std::shared_ptr<Model::Track> ShareTrack() const { return track; }
	void SetTrack(std::shared_ptr<Model::Track> track);

	HudCell GetCell() const { return cell; }
	void SetCell(HudCell cell);

public:
	void OnScreenSizeChanged();

private:
	void LayoutStacked(HudAlignment::type align,
		double startX, double startY,
		double scaleX, double scaleY);
	void LayoutCorner(HudAlignment::type alignCorner,
		HudAlignment::type alignH, HudAlignment::type alignV,
		double startX, double startY,
		double scaleX, double scaleY);
protected:
	virtual void Layout();

public:
	void Advance(Util::OS::timestamp_t tick);

private:
	std::shared_ptr<Model::Track> track;
	std::shared_ptr<Player::Player> player;
	HudCell cell;
	typedef std::vector<HudChild> hudChildList_t;
	std::array<hudChildList_t, HudAlignment::NUM> hudChildren;
	boost::signals2::scoped_connection displayConfigChangedConn;
};

}  // namespace Display
}  // namespace HoverRace

#undef MR_DllDeclare
