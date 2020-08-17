//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef ALBUM_H
#define ALBUM_H

namespace Ms {
class Album;
class MasterScore;
class XmlReader;
class XmlWriter;
class Excerpt;
class LayoutBreak;
enum class LayoutMode : char;

using std::unique_ptr;

//---------------------------------------------------------
//   AlbumExcerpt
//---------------------------------------------------------

struct AlbumExcerpt
{
    AlbumExcerpt(XmlReader& reader);
    void writeAlbumExcerpt(XmlWriter& writer) const;

    QString title;
    QList<int> partIndices;
    QMultiMap<int, int> tracks;
};

//---------------------------------------------------------
//   AlbumItem
//---------------------------------------------------------

class AlbumItem : public QObject
{
    Q_OBJECT

public:
    // AlbumItems are created by Album::createItem, don't create them manually.
    AlbumItem(Album& album, XmlReader& reader);
    AlbumItem(Album& album, MasterScore* score, bool enabled = true);
    ~AlbumItem();

    void setEnabled(bool b);
    bool enabled() const;
    int setScore(MasterScore* score);
    void addAlbumSectionBreak();
    bool removeAlbumSectionBreak();
    void addAlbumPageBreak();
    bool removeAlbumPageBreak();
    void readAlbumItem(XmlReader& reader);
    void writeAlbumItem(XmlWriter& writer);

    int duration() const;

    Album& album;
    MasterScore* score      { nullptr }; // make reference? (probably can't cause I am not reading while loading)
    QFileInfo fileInfo      { "-" };

signals:
    void durationChanged();

private slots:
    void updateDuration();

private:
    LayoutBreak* getSectionBreak() const;
    bool checkReadiness() const;

    bool m_enabled { true };
    bool m_extraPageBreak { false };
    bool m_extraSectionBreak { false };
    qreal m_pauseDuration { -1 };
    int m_duration { -1 };
};

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

class Album : public QObject
{
    Q_OBJECT

public:
    static Album* activeAlbum;
    static bool scoreInActiveAlbum(MasterScore* score); // I also have MasterScore::partOfActiveAlbum

    Album();

    AlbumItem* addScore(MasterScore* score, bool enabled = true);
    void removeScore(MasterScore* score);
    void removeScore(int index);
    void swap(int indexA, int indexB);

    void addAlbumSectionBreaks();
    void addAlbumPageBreaks();
    void removeAlbumSectionBreaks();
    void removeAlbumPageBreaks();

    QStringList composers() const;
    QStringList lyricists() const;
    QStringList scoreTitles() const;

    bool loadFromFile(const QString& path);
    void readAlbum(XmlReader& reader);
    void readExcerpts(XmlReader& reader);
    bool saveToFile();
    bool saveToFile(const QString& path);
    bool saveToFile(QIODevice* f);
    void writeAlbum(XmlWriter& writer) const;
    static void importAlbum(const QString& compressedFilePath, QDir destinationFolder);
    bool exportAlbum(QIODevice* f, const QFileInfo& info);

    bool checkPartCompatibility() const;
    bool checkPartCompatibility(MasterScore* score);
    void removeAlbumExcerpts();
    static Excerpt* prepareMovementExcerpt(Excerpt* masterExcerpt, MasterScore* score);
    static Excerpt* createMovementExcerpt(Excerpt* e);

    MasterScore* createDominant();
    MasterScore* getDominant() const;
    std::vector<AlbumItem*> albumItems() const;

    void updateContents();

    const QString& albumTitle() const;
    void setAlbumTitle(const QString& newTitle);
    const QFileInfo& fileInfo() const;
    bool albumModeActive() const;
    void setAlbumModeActive(bool b);
    bool titleAtTheBottom() const;
    void setTitleAtTheBottom(bool titleAtTheBottom);
    bool drawFrontCover() const;
    void setDrawFrontCover(bool b);
    bool generateContents() const;
    void setGenerateContents(bool enabled);
    bool addPageBreaksEnabled() const;
    void setAddPageBreaksEnabled(bool enabled);
    bool includeAbsolutePaths() const;
    void setIncludeAbsolutePaths(bool enabled);
    int defaultPlaybackDelay() const;
    void setDefaultPlaybackDelay(int ms);

    const QString& exportedScoreFolder() const;
    bool exporting() const;

public slots:
    void setAlbumLayoutMode(LayoutMode lm);
    void updateFrontCover();

private:
    AlbumItem* createItem(XmlReader& reader);
    AlbumItem* createItem(MasterScore* score, bool enabled);

    std::vector<unique_ptr<AlbumItem> > m_albumItems {};
    std::vector<unique_ptr<AlbumExcerpt> > m_albumExcerpts {};
    QString m_albumTitle                            { "" };
    QFileInfo m_fileInfo                            {};
    MasterScore* m_dominantScore                    { nullptr }; // unique ptr?
    bool m_albumModeActive                          { false };

    bool m_titleAtTheBottom                         { true };
    bool m_drawFrontCover                           { true };
    bool m_generateContents                         { false };
    bool m_addPageBreaksEnabled                     { false };
    bool m_includeAbsolutePaths                     { false };
    int m_defaultPlaybackDelay                      { 3000 };

    const QString m_exportedScoreFolder             { "Scores" };
    bool m_exporting                                { false };
};
}     // namespace Ms

#endif // ALBUM_H
