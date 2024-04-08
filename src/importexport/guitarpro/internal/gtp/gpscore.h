#ifndef MU_IMPORTEXPORT_GPSCORE_H
#define MU_IMPORTEXPORT_GPSCORE_H

#include "types/string.h"

namespace mu::iex::guitarpro {
class GPScore
{
public:

    void setTitle(const muse::String& title) { _title = title; }
    muse::String title() const { return _title; }
    void setSubTitle(const muse::String& subTitle) { _subTitle = subTitle; }
    muse::String subTitle() const { return _subTitle; }
    void setArtist(const muse::String& artist) { _artist = artist; }
    muse::String artist() const { return _artist; }
    void setAlbum(const muse::String& album) { _album = album; }
    muse::String album() const { return _album; }
    void setPoet(const muse::String& poet) { _poet = poet; }
    muse::String poet() const { return _poet; }
    void setComposer(const muse::String& composer) { _composer = composer; }
    muse::String composer() const { return _composer; }
    void setMultiVoice(bool multiVoice) { _multiVoice = multiVoice; }
    bool multiVoice() const { return _multiVoice; }

private:
    muse::String _title;
    muse::String _subTitle;
    muse::String _artist;
    muse::String _album;
    muse::String _poet;
    muse::String _composer;
    bool _multiVoice = false;
};
} // namespace mu::iex::guitarpro

#endif // MU_IMPORTEXPORT_GPSCORE_H
