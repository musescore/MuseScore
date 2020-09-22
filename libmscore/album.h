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

//---------------------------------------------------------
//   AlbumExcerpt
///     This class is used as a temporary representation of loaded excerpts (constructed on load).
///     When the user enters Album-mode for the first time and the combinedScore is created
///     instances of this class are used to reconstruct the excerpts/parts of the combinedScore.
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
///     Represents an item of an Album.
///     Each AlbumItem can have only one Score and that Score cannot change.
///
///     Every AlbumItem is part of an Album. Use Album::createItem to create them.
//---------------------------------------------------------

class AlbumItem : public QObject
{
    Q_OBJECT

public:
    // AlbumItems are created by Album::createItem, don't create them manually.
    AlbumItem(Album& album, XmlReader& reader);
    AlbumItem(Album& album, MasterScore* m_score, bool enabled = true);
    ~AlbumItem();

    void setEnabled(bool b);
    bool enabled() const;
    int setScore(MasterScore* score);
    MasterScore* score() const;
    const QFileInfo& fileInfo() const;
    void addAlbumSectionBreak();
    bool removeAlbumSectionBreak();
    void addAlbumPageBreak();
    bool removeAlbumPageBreak();
    void readAlbumItem(XmlReader& reader);
    void writeAlbumItem(XmlWriter& writer);

    int duration() const;
    bool checkReadiness() const;

    Album& album;

signals:
    void durationChanged();

private slots:
    void updateDuration();

private:
    LayoutBreak* getSectionBreak() const;

    MasterScore* m_score { nullptr };
    QFileInfo m_fileInfo { "-" };

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
    static bool scoreInActiveAlbum(MasterScore* score);

    Album();

    AlbumItem* addScore(MasterScore* score, bool enabled = true);
    void removeScore(MasterScore* score);
    void removeScore(int index);
    void swap(int indexA, int indexB);

    void addAlbumSectionBreaks();
    void addAlbumPageBreaks();
    void removeAlbumSectionBreaks();
    void removeAlbumPageBreaks();
    void applyDefaultPauseToSectionBreaks();

    QStringList composers() const;
    QStringList lyricists() const;
    QStringList scoreTitles() const;

    bool loadFromFile(const QString& path, bool legacy = false);
    void readLegacyAlbum(XmlReader& reader);
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

    MasterScore* createCombinedScore();
    MasterScore* getCombinedScore() const;
    std::vector<AlbumItem*> albumItems() const;
    std::vector<MasterScore*> albumScores() const;

    void updateContents();

    const QString& albumTitle() const;
    void setAlbumTitle(const QString& newTitle);
    const QFileInfo& fileInfo() const;
    static bool albumModeActive();
    static void setAlbumModeActive(bool b);
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
    int defaultPause() const;
    void setDefaultPause(qreal s);

    const QString& exportedScoreFolder() const;
    bool exporting() const;

public slots:
    void setAlbumLayoutMode(LayoutMode lm);
    void updateFrontCover();

private:
    AlbumItem* createItem(XmlReader& reader);
    AlbumItem* createItem(MasterScore* score, bool enabled);

    std::vector<std::unique_ptr<AlbumItem> > m_albumItems {};
    std::vector<std::unique_ptr<AlbumExcerpt> > m_albumExcerpts {};
    QString m_albumTitle                            { "" };
    QFileInfo m_fileInfo                            {};
    std::unique_ptr<MasterScore> m_combinedScore    { nullptr };
    bool m_albumModeActive                          { false };

    bool m_titleAtTheBottom                         { true };
    bool m_drawFrontCover                           { true };
    bool m_generateContents                         { false };
    bool m_addPageBreaksEnabled                     { false };
    bool m_includeAbsolutePaths                     { false };
    qreal m_defaultPause                            { 3 };

    const QString m_exportedScoreFolder             { "Scores" };
    bool m_exporting                                { false };
};
}     // namespace Ms

#endif // ALBUM_H
