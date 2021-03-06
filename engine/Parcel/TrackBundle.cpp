
// TrackBundle.cpp
//
// Copyright (c) 2010, 2013, 2014 Michael Imamura.
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

#include <boost/algorithm/string.hpp>

#include "../Display/SpriteTextureRes.h"
#include "../Display/Texture.h"
#include "../Model/Track.h"
#include "../Model/TrackEntry.h"
#include "../Parcel/RecordFile.h"
#include "../Util/Config.h"
#include "../Util/InspectMapNode.h"
#include "ObjStream.h"

#include "TrackBundle.h"

using HoverRace::Util::Config;
using HoverRace::Util::OS;

namespace HoverRace {
namespace Parcel {

TrackBundle::TrackBundle(const OS::path_t &dir, BundlePtr subBundle) :
	SUPER(dir, subBundle)
{
}

TrackBundle::~TrackBundle()
{
}

RecordFilePtr TrackBundle::OpenParcel(const std::string &name, bool writing) const
{
	return SUPER::OpenParcel(
		boost::ends_with(name, Config::TRACK_EXT) ? name : name + Config::TRACK_EXT,
		writing);
}

/**
 * Load a track by name.
 * @param name The name of the track.  The ".trk" suffix may be omitted.
 * @return The track or @c nullptr if the track does not exist.
 * @throws ObjStreamExn The track failed to load.
 */
Model::TrackPtr TrackBundle::OpenTrack(const std::string &name) const
{
	RecordFilePtr recFile(OpenParcel(name));
	return !recFile ?
		Model::TrackPtr() :
		std::make_shared<Model::Track>(name, recFile);
}

/**
 * Load a track by track header.
 * @param entry The entry for the track (may not be @c nullptr, see OpenTrackEntry).
 * @return The track or @c nullptr if the track does not exist.
 * @throws ObjStreamExn The track failed to load.
 */
Model::TrackPtr TrackBundle::OpenTrack(const std::shared_ptr<const Model::TrackEntry> &entry) const
{
	return OpenTrack(entry->name);
}

std::shared_ptr<Display::Res<Display::Texture>> TrackBundle::LoadMap(
	std::shared_ptr<const Model::TrackEntry> entry) const
{
	RecordFilePtr recFile(OpenParcel(entry->name));
	if (!recFile || recFile->GetNbRecords() < 4) {
		return std::shared_ptr<Display::Res<Display::Texture>>();
	}

	//TODO: Refactor into shared code with Track::LoadMap().

	recFile->SelectRecord(3);
	ObjStreamPtr archivePtr(recFile->StreamIn());
	ObjStream &archive = *archivePtr;

	// Ignore track bounds.
	MR_Int32 i;
	archive >> i >> i >> i >> i;
	
	return std::make_shared<Display::SpriteTextureRes>(
		"map:" + entry->name, archive);
}


/**
 * Load a track header.
 * @param name The name of the track.  The ".trk" suffix may be omitted.
 * @return The track or @c NULL if the track does not exist.
 * @throws ObjStreamExn The track failed to load.
 */
Model::TrackEntryPtr TrackBundle::OpenTrackEntry(const std::string &name) const
{
	RecordFilePtr recFile(OpenParcel(name));
	if (!recFile) {
		return Model::TrackEntryPtr();
	}
	else {
		recFile->SelectRecord(0);
		Model::TrackEntryPtr retv = std::make_shared<Model::TrackEntry>();
		if (boost::ends_with(name, Config::TRACK_EXT)) {
			// Trim off ".trk".
			retv->name.assign(name, 0, name.length() - Config::TRACK_EXT.length());
		}
		else {
			retv->name = name;
		}
		retv->Serialize(*recFile->StreamIn());
		return retv;
	}
}

/**
 * Check if a track is available.
 * @param name The name of the track.  The ".trk" suffix may be omitted.
 * @return @c eTrackAvail if the track is available, @c eTrackNotFound otherwise.
 */
MR_TrackAvail TrackBundle::CheckAvail(const std::string &name) const
{
	try {
		return OpenTrack(name) ? eTrackNotFound : eTrackAvail;
	}
	catch (ObjStreamExn&) {
		return eTrackNotFound;
	}
}

}  // namespace Parcel
}  // namespace HoverRace
