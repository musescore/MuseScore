#ifndef GPSCORE_H
#define GPSCORE_H

namespace Ms {
class GPScore
{
public:

    void setTitle(QString title) { _title = title; }
    QString title() const { return _title; }
    void setSubTitle(QString subTitle) { _subTitle = subTitle; }
    QString subTitle() const { return _subTitle; }
    void setArtist(QString artist) { _artist = artist; }
    QString artist() const { return _artist; }
    void setAlbum(QString album) { _album = album; }
    QString album() const { return _album; }

private:
    QString _title;
    QString _subTitle;
    QString _artist;
    QString _album;
};
}

#endif // GPSCORE_H
