#ifndef GPSCORE_H
#define GPSCORE_H

namespace mu::engraving {
class GPScore
{
public:

    void setTitle(const mu::String& title) { _title = title; }
    mu::String title() const { return _title; }
    void setSubTitle(const mu::String& subTitle) { _subTitle = subTitle; }
    mu::String subTitle() const { return _subTitle; }
    void setArtist(const mu::String& artist) { _artist = artist; }
    mu::String artist() const { return _artist; }
    void setAlbum(const mu::String& album) { _album = album; }
    mu::String album() const { return _album; }
    void setPoet(const mu::String& poet) { _poet = poet; }
    mu::String poet() const { return _poet; }
    void setComposer(const mu::String& composer) { _composer = composer; }
    mu::String composer() const { return _composer; }

private:
    mu::String _title;
    mu::String _subTitle;
    mu::String _artist;
    mu::String _album;
    mu::String _poet;
    mu::String _composer;
};
}

#endif // GPSCORE_H
