
// RoomList.cpp
// The server room list.
//
// Copyright (c) 2009 Michael Imamura.
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

#include "../../engine/Net/Agent.h"
#include "../../engine/Net/NetExn.h"
#include "../../engine/Util/OS.h"

#include "RoomList.h"

using namespace HoverRace;
using namespace HoverRace::Client;
using HoverRace::Util::OS;

RoomList::RoomList() :
	selectedRoom(NULL), curBanner(NULL), curBannerIdx(0)
{
}

RoomList::~RoomList()
{
	for (rooms_t::iterator iter = rooms.begin();
		iter != rooms.end(); ++iter)
	{
		delete *iter;
	}

	for (banners_t::iterator iter = banners.begin();
		iter != banners.end(); ++iter)
	{
		delete *iter;
	}
}

/**
 * Load the roomlist from a URL.
 * @param url The URL.
 * @param cancelFlag Optional callback to cancel the transfer.
 * @throw NetExn An error occurred during the transfer.
 * @throw CanceledExn The transfer was canceled.
 */
void RoomList::LoadFromUrl(const std::string &url, Net::CancelFlagPtr cancelFlag)
{
	Net::Agent agent(url);
	//TODO: Set max size.

	std::stringstream io;
	agent.Get(io, cancelFlag);

	LoadFromStream(io);
}

void RoomList::LoadFromStream(std::istream &in)
{
	std::string ris;

	std::getline(in, ris);
	if (ris != "SERVER LIST") throw Net::NetExn("Invalid format: Missing preamble");

	in.exceptions(std::istream::badbit | std::istream::failbit);

	bool foundScoreServer = false;
	for (;;) {
		int type;
		try {
			in >> type;
		}
		catch (std::istream::failure&) {
			// End of stream.
			break;
		}

		try {
			switch (type) {
				case 0:  // Score server
					in >> scoreServer;
					foundScoreServer = true;
					break;

				case 1:  // Room entry
					rooms.push_back(new Server());
					in >> *(rooms.back());
					break;

				case 2:  // Room w/ ladder server.
					// Currently unused.
					break;

				case 8:  // Public banner
					banners.push_back(new Banner());
					in >> *(banners.back());
					break;

				case 9:  // Registered banner
					// Currently ignored.
					break;
			}
			// Consume trailing characters in line.
			std::getline(in, ris);
		}
		catch (std::istream::failure&) {
			throw Net::NetExn("Parse error");
		}
	}

	// Verify data.
	if (!foundScoreServer) {
		throw Net::NetExn("No score server");
	}
	if (rooms.empty()) {
		throw Net::NetExn("No rooms available");
	}

	if (!banners.empty()) {
		curBanner = banners.front();
		curBannerIdx = 0;
	}
}

/**
 * Set the currently-selected room index.
 * If an invalid @p index is provided, then the currently-selected room is
 * set to @c NULL.
 * @param index The index.
 */
void RoomList::SetSelectedRoom(size_t index)
{
	selectedRoom = (index < rooms.size()) ? rooms.at(index) : NULL;
}

/**
 * Rotate to the next banner in the banner list.
 * Will have no effect if there are no banners.
 * @return The next banner in the list (@c NULL if banner list is empty).
 */
RoomList::Banner *RoomList::NextBanner()
{
	return (curBanner =
		banners.empty() ? NULL : banners.at((++curBannerIdx) % banners.size()));
}

RoomList::Banner *RoomList::PeekNextBanner() const
{
	return banners.empty() ? NULL : banners.at((curBannerIdx + 1) % banners.size());
}

std::istream &HoverRace::Client::operator>>(std::istream &in, RoomList::IpAddr &ip)
{
	// InternetRoom uses IPv4 addresses packed into an unsigned long.
	// We store both the string and unsigned long versions for now
	// to ease in the transition.

	in >> ip.s;

	unsigned long i = 0;
	unsigned int nibble = 0;
	unsigned int components = 1;
	for (std::string::iterator iter = ip.s.begin();
		iter != ip.s.end(); ++iter)
	{
		char c = *iter;
		if (c >= '0' && c <= '9') {
			nibble = (nibble * 10) + (c - '0');
		}
		else if (c == '.') {
			i = (i << 8) + nibble;
			nibble = 0;
			++components;
		}
		else {
			throw Net::NetExn(boost::str(boost::format(
				"Invalid character in IP address: 0x%02x") % (int)c));
		}
	}
	i = (i << 8) + nibble;

	if (components != 4) {
		throw Net::NetExn("Invalid IP address in roomlist");
	}

	ip.ud = i;

	return in;
}

std::istream &HoverRace::Client::operator>>(std::istream &in, RoomList::Server &server)
{
	in >> server.name >> server.addr >> server.port >> server.path;
	return in;
}

std::istream &HoverRace::Client::operator>>(std::istream &in, RoomList::Banner &banner)
{
	in >> (RoomList::Server&)banner >> banner.delay >> banner.clickUrl;

	banner.indirectClick = (banner.clickUrl.find("//") == std::string::npos);

	return in;
}
