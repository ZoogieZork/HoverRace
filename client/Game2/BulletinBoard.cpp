
// BulletinBoard.cpp
//
// Copyright (c) 2014, 2015 Michael Imamura.
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

#include "../../engine/Display/ClickRegion.h"
#include "../../engine/Display/FillBox.h"
#include "../../engine/Display/FlexGrid.h"
#include "../../engine/Display/Label.h"

#include "Announcement.h"

#include "BulletinBoard.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace Client {

namespace {
	OS::timestamp_t BULLETIN_TIMEOUT = 3000;
}

/// Displays a single announcement.
class BulletinBoard::Bulletin : public Display::Container /*{{{*/
{
	typedef Display::Container SUPER;
public:
	Bulletin(BulletinBoard *board, Display::Display &display,
		std::shared_ptr<Announcement> ann);
	virtual ~Bulletin() { }

public:
	Announcement *GetAnnouncement() const { return ann.get(); }
	bool IsExpired() const { return expired; }

protected:
	void Layout() override;

public:
	void Advance(OS::timestamp_t tick);

private:
	BulletinBoard *board;
	std::shared_ptr<Announcement> ann;
	bool expired;
	OS::timestamp_t expiration;
	std::shared_ptr<Display::FillBox> bg;
	std::shared_ptr<Display::ClickRegion> clickBox;
	std::shared_ptr<Display::Label> labelLbl;
	std::shared_ptr<Display::FlexGrid> contentGrid;
}; //}}}

BulletinBoard::BulletinBoard(Display::Display &display,
	Display::uiLayoutFlags_t layoutFlags) :
	SUPER(display, Vec2(580, 720), false, layoutFlags)
{
}

BulletinBoard::~BulletinBoard()
{
}

/**
 * Post an announcement to the bulletin board.
 * @param ann The announcement (may not be @c nullptr).
 */
void BulletinBoard::Announce(std::shared_ptr<Announcement> ann)
{
	bulletins.emplace_front(NewChild<Bulletin>(this, display, ann));
	RequestLayout();
}

void BulletinBoard::Layout()
{
	Vec2 pos{ 0, 0 };
	for (auto &bulletin : bulletins) {
		if (bulletin->IsVisible() && !bulletin->IsExpired()) {
			bulletin->SetPos(pos);
			pos.y += bulletin->GetSize().y;
		}
	}
}

void BulletinBoard::Advance(Util::OS::timestamp_t tick)
{
	for (auto &bulletin : bulletins) {
		bulletin->Advance(tick);
		if (bulletin->IsExpired()) {
			RemoveChild(bulletin);
			//TODO: Remove from list of bulletins.
		}
	}
}

//{{{ Bulletin /////////////////////////////////////////////////////////////////

BulletinBoard::Bulletin::Bulletin(BulletinBoard *board,
	Display::Display &display, std::shared_ptr<Announcement> announcement) :
	SUPER(display, Vec2(580, 0), false),
	board(board), ann(std::move(announcement)),
	expired(false), expiration(0)
{
	using namespace Display;

	const auto &s = display.styles;

	// The initial height of the Bulletin is zero and it starts hidden so that
	// it's not displayed until the first layout.
	SetVisible(false);

	bg = NewChild<FillBox>(580, 100, s.announcementBg);

	clickBox = NewChild<ClickRegion>(display, Vec2(580, 100));
	clickBox->GetClickedSignal().connect(std::bind(&Announcement::OnClick,
		ann.get()));

	labelLbl = NewChild<Label>(420, ann->GetLabel(),
		s.announcementHeadFont, s.announcementHeadFg);
	labelLbl->SetPos(120, 20);

	auto icon = ann->CreateIcon(display, *this);
	icon->SetPos(40, 20);
	icon->SetSize(60, 60);

	//TODO: Player avatar.

	contentGrid = NewChild<FlexGrid>(display);
	contentGrid->SetMargin(3, 0);
	contentGrid->SetPos(120, 60);
	contentGrid->SetFixedWidth(420);
	ann->CreateContents(display, *contentGrid);
}

void BulletinBoard::Bulletin::Layout()
{
	SetVisible(true);

	Vec3 gridSize = contentGrid->Measure();
	Vec2 fullSize{ GetSize().x, 60 + gridSize.y + 10 };
	if (GetSize() != fullSize) {
		SetSize(fullSize);
		bg->SetSize(fullSize);
		clickBox->SetSize(fullSize);
		board->OnBulletinSizeUpdated();
	}
}

void BulletinBoard::Bulletin::Advance(OS::timestamp_t tick)
{
	if (!expired) {
		if (expiration == 0) {
			expiration = tick + BULLETIN_TIMEOUT;
		}
		else if (tick > expiration) {
			expired = true;
		}
	}
}

//}}} Bulletin

}  // namespace HoverScript
}  // namespace Client
