#ifndef GPSCORE_H
#define GPSCORE_H

namespace Ms {
class GPScore
{
public:

    void setTitle(const QString& title) { _title = title; }
    QString title() const { return _title; }
    void setSubTitle(const QString& subTitle) { _subTitle = subTitle; }
    QString subTitle() const { return _subTitle; }
    void setArtist(const QString& artist) { _artist = artist; }
    QString artist() const { return _artist; }
    void setAlbum(const QString& album) { _album = album; }
    QString album() const { return _album; }
    void setPoet(const QString& poet) { _poet = poet; }
    QString poet() const { return _poet; }
    void setComposer(const QString& composer) { _composer = composer; }
    QString composer() const { return _composer; }

private:
    QString _title;
    QString _subTitle;
    QString _artist;
    QString _album;
    QString _poet;
    QString _composer;
};
}

#endif // GPSCORE_H
