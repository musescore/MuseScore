#ifndef MU_IMPORTEXPORT_GPSCORE_H
#define MU_IMPORTEXPORT_GPSCORE_H

#include "types/string.h"

namespace mu::iex::guitarpro {
class GPScore
{
public:

    void setTitle(const String& title) { _title = title; }
    String title() const { return _title; }
    void setSubTitle(const String& subTitle) { _subTitle = subTitle; }
    String subTitle() const { return _subTitle; }
    void setArtist(const String& artist) { _artist = artist; }
    String artist() const { return _artist; }
    void setAlbum(const String& album) { _album = album; }
    String album() const { return _album; }
    void setPoet(const String& poet) { _poet = poet; }
    String poet() const { return _poet; }
    void setComposer(const String& composer) { _composer = composer; }
    String composer() const { return _composer; }
    void setMultiVoice(bool multiVoice) { _multiVoice = multiVoice; }
    bool multiVoice() const { return _multiVoice; }

private:
    String _title;
    String _subTitle;
    String _artist;
    String _album;
    String _poet;
    String _composer;
    bool _multiVoice = false;
};
} // namespace mu::iex::guitarpro

#endif // MU_IMPORTEXPORT_GPSCORE_H
