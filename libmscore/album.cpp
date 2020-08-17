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

#include "album.h"
#include "part.h"
#include "excerpt.h"
#include "layoutbreak.h"
#include "score.h"
#include "system.h"
#include "page.h"
#include "box.h"
#include "xml.h"
#include "musescoreCore.h"
#include "measure.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"

namespace Ms {
//---------------------------------------------------------
//   AlbumExcerpt
//---------------------------------------------------------

AlbumExcerpt::AlbumExcerpt(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "partIndex") {
            partIndices.push_back(reader.readInt());
        } else if (tag == "key") {
            int k = reader.readInt();
            reader.readNextStartElement();
            int v = reader.readInt();
            tracks.insert(k, v);
        } else if (tag == "title") {
            title = reader.readElementText();
        }
    }
}

//---------------------------------------------------------
//   writeAlbumExcerpt
//---------------------------------------------------------

void AlbumExcerpt::writeAlbumExcerpt(XmlWriter& writer) const
{
    writer.stag("Excerpt");
    writer.tag("title", title);
    for (auto partIndex : partIndices) {
        writer.tag("partIndex", partIndex);
    }
    for (auto k : tracks.uniqueKeys()) {
        writer.tag("key", k);
        for (auto v : tracks.values(k)) {
            writer.tag("track", v);
        }
    }
    writer.etag();
}

//---------------------------------------------------------
//   AlbumItem
//---------------------------------------------------------

AlbumItem::AlbumItem(Album& album, XmlReader& reader)
    : album(album)
{
    readAlbumItem(reader);
}

AlbumItem::AlbumItem(Album& album, MasterScore* score, bool enabled)
    : album(album)
{
    m_enabled = enabled;
    setScore(score);
}

AlbumItem::~AlbumItem()
{
    if (m_score) {
        if (!m_score->requiredByMuseScore()) { // delete the score if it is only used by this Album
            delete m_score;
        }
    }
}

//---------------------------------------------------------
//   setEnabled
//---------------------------------------------------------

void AlbumItem::setEnabled(bool b)
{
    if (!checkReadiness()) {
        return;
    }
    m_enabled = b;
    m_score->setEnabled(b);
    if (album.getCombinedScore()) {
        if (m_score->albumExcerpts().size()) {
            for (auto e : m_score->albumExcerpts()) {
                static_cast<MasterScore*>(e->partScore())->setEnabled(b);
            }
        }
        album.getCombinedScore()->doLayout();
        album.getCombinedScore()->update();
    }
}

//---------------------------------------------------------
//   enabled
//---------------------------------------------------------

bool AlbumItem::enabled() const
{
    if (!checkReadiness()) {
        return false;
    }
    return m_enabled;
}

//---------------------------------------------------------
//   setScore
///     You can call this function once per AlbumItem (if you give a valid score).
///     This function prepares the AlbumItem for use. It is not automatically called
///     on creation when loading an Album because the load code is part of mscore.
//---------------------------------------------------------

int AlbumItem::setScore(MasterScore* score)
{
    // if you want to change the score, create a new AlbumItem
    if (m_score != nullptr) {
        qDebug() << "The AlbumItem already has a Score, don't set a new one. Create a new AlbumItem.";
        return -1;
    }
    // don't set an empty score
    if (score == nullptr) {
        qDebug() << "You are trying to set an empty score.";
        return -1;
    }

    m_score = score;
    setEnabled(m_enabled);
    if (!score->importedFilePath().isEmpty()) {
        m_fileInfo.setFile(score->importedFilePath());
    }

    addAlbumSectionBreak();
    if (album.addPageBreaksEnabled()) {
        addAlbumPageBreak();
    }

    updateDuration();
    connect(score, &MasterScore::durationChanged, this, &AlbumItem::updateDuration);
    connect(score, &MasterScore::composerChanged, &album, &Album::updateFrontCover);
    connect(score, &MasterScore::lyricistChanged, &album, &Album::updateFrontCover);
    return 0;
}

//---------------------------------------------------------
//   score
//---------------------------------------------------------

MasterScore* AlbumItem::score() const
{
    return m_score;
}

//---------------------------------------------------------
//   fileInfo
//---------------------------------------------------------

const QFileInfo& AlbumItem::fileInfo() const
{
    return m_fileInfo;
}

//---------------------------------------------------------
//   addAlbumSectionBreak
//---------------------------------------------------------

void AlbumItem::addAlbumSectionBreak()
{
    if (!checkReadiness()) {
        return;
    }
    if (!m_score->lastMeasure()->sectionBreak()) { // add only if there isn't one
        LayoutBreak* lb = new LayoutBreak(m_score);
        lb->setLayoutBreakType(LayoutBreak::Type::SECTION);
        m_score->lastMeasure()->add(lb);
        if (m_pauseDuration >= 0) {
            lb->setPause(m_pauseDuration);
        } else {
            lb->setPause(album.defaultPause());
            m_pauseDuration = lb->pause();
        }
        m_extraSectionBreak = true;
        m_score->update();
    }
}

//---------------------------------------------------------
//   removeAlbumSectionBreak
//---------------------------------------------------------

bool AlbumItem::removeAlbumSectionBreak()
{
    if (!checkReadiness()) {
        return false;
    }
    if (m_extraSectionBreak && m_score->lastMeasure()) {
        m_pauseDuration = getSectionBreak()->pause();
        m_score->lastMeasure()->remove(getSectionBreak());
        m_extraSectionBreak = false;
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   addAlbumPageBreak
//---------------------------------------------------------

void AlbumItem::addAlbumPageBreak()
{
    if (!checkReadiness()) {
        return;
    }
    if (!m_score->lastMeasure()->pageBreak()) { // add only if there isn't one
        LayoutBreak* lb = new LayoutBreak(m_score);
        lb->setLayoutBreakType(LayoutBreak::Type::PAGE);
        m_score->lastMeasure()->add(lb);
        m_extraPageBreak = true;
        m_score->update();
    }
}

//---------------------------------------------------------
//   removeAlbumPageBreak
//---------------------------------------------------------

bool AlbumItem::removeAlbumPageBreak()
{
    if (!checkReadiness()) {
        return false;
    }
    if (m_extraPageBreak && m_score->lastMeasure()) {
        for (auto& e : m_score->lastMeasure()->el()) {
            if (e->isLayoutBreak() && toLayoutBreak(e)->isPageBreak()) {
                m_score->lastMeasure()->remove(e);
                m_extraPageBreak = false;
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------
//   readAlbumItem
//---------------------------------------------------------

void AlbumItem::readAlbumItem(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "name") {
            reader.readElementText();
        } else if (tag == "path") {
            m_fileInfo.setFile(reader.readElementText());
            album.setIncludeAbsolutePaths(true);
        } else if (tag == "relativePath") {
            if (!m_fileInfo.exists()) {
                QDir dir(album.fileInfo().dir());
                QString relativePath = reader.readElementText();
                m_fileInfo.setFile(dir, relativePath);
            } else {
                reader.readElementText();
            }
        } else if (tag == "enabled") {
            m_enabled = reader.readBool();
        } else if (tag == "pauseDuration") {
            m_pauseDuration = reader.readDouble();
        } else {
            reader.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   writeAlbumItem
//---------------------------------------------------------

void AlbumItem::writeAlbumItem(XmlWriter& writer)
{
    writer.stag("Score");
    writer.tag("alias", "");
    if (album.includeAbsolutePaths()) {
        writer.tag("path", m_fileInfo.absoluteFilePath());
    }
    if (!album.exporting()) {
        QDir dir(album.fileInfo().dir());
        QString relativePath = dir.relativeFilePath(m_fileInfo.absoluteFilePath());
        writer.tag("relativePath", relativePath);
    } else {
        writer.tag("relativePath", album.exportedScoreFolder() + QDir::separator() + m_score->title() + ".mscx");
    }
    writer.tag("enabled", m_enabled);
    if (auto lb = getSectionBreak()) {
        m_pauseDuration = lb->pause();
    }
    writer.tag("pauseDuration", m_pauseDuration);
    writer.etag();
}

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

int AlbumItem::duration() const
{
    if (!checkReadiness()) {
        return -1;
    }
    return m_duration;
}

//---------------------------------------------------------
//   updateDuration
//---------------------------------------------------------

void AlbumItem::updateDuration()
{
    if (!checkReadiness()) {
        return;
    }
    m_duration = m_score->duration();
    emit durationChanged();
}

//---------------------------------------------------------
//   getSectionBreak
//---------------------------------------------------------

LayoutBreak* AlbumItem::getSectionBreak() const
{
    if (!m_score) {
        return nullptr;
    }
    return m_score->lastMeasure()->sectionBreakElement();
}

//---------------------------------------------------------
//   checkReadiness
///     Used to disallow any actions until a Score has been loaded
//---------------------------------------------------------

bool AlbumItem::checkReadiness() const
{
    if (!m_score) {
        qDebug() << "You need to load a score before you use an AlbumItem.";
        Q_ASSERT(false);
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

Album* Album::activeAlbum = nullptr;

//---------------------------------------------------------
//   scoreInActiveAlbum
//---------------------------------------------------------

bool Album::scoreInActiveAlbum(MasterScore* score)
{
    if (!activeAlbum) {
        return false;
    }

    for (auto& x : activeAlbum->m_albumItems) {
        if (x->score() == score) {
            return true;
        }
    }
    if (score == activeAlbum->m_combinedScore.get()) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

Album::Album()
{
    QSettings s;
    m_titleAtTheBottom = s.value("album/titleAtTheBottomAllowed").toBool();
    m_drawFrontCover = s.value("album/drawFrontCover").toBool();
    m_generateContents = s.value("album/generateContents").toBool();
    m_addPageBreaksEnabled = s.value("album/pageBreaks").toBool();
    m_defaultPause = s.value("album/defaultPauseDuration").toDouble();
}

//---------------------------------------------------------
//   createItem
//---------------------------------------------------------

AlbumItem* Album::createItem(XmlReader& reader)
{
    AlbumItem* a = new AlbumItem(*this, reader);
    m_albumItems.push_back(std::unique_ptr<AlbumItem>(a));
    return a;
}

AlbumItem* Album::createItem(MasterScore* score, bool enabled)
{
    AlbumItem* a = new AlbumItem(*this, score, enabled);
    m_albumItems.push_back(std::unique_ptr<AlbumItem>(a));
    return a;
}

//---------------------------------------------------------
//   addScore
//---------------------------------------------------------

AlbumItem* Album::addScore(MasterScore* score, bool enabled)
{
    if (!score) {
        qDebug() << "There is no score to add to album...";
        return nullptr;
    }

    if (checkPartCompatibility() && !checkPartCompatibility(score)) { // check part compatibility
        QMessageBox msgBox;
        msgBox.setWindowTitle(QObject::tr("Incompatible parts"));
        msgBox.setText(QObject::tr("The parts of your new score are incompatible with the rest of the album."));
        msgBox.setDetailedText(QObject::tr("The parts of your new score (%1) are incompatible with the rest of the album. "
                                           "That means that adding this score will disable the 'Parts' functionality for your album. "
                                           "You can remove this score to restore this functionality. ").arg(score->realTitle()));
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(
            QMessageBox::Cancel | QMessageBox::Ignore
            );
        auto response = msgBox.exec();
        if (response == QMessageBox::Cancel) {
            return nullptr;
        } else {
            if (m_combinedScore) {
                while (m_combinedScore->excerpts().size()) {
                    m_combinedScore->removeExcerpt(m_combinedScore->excerpts().first());
                }
            }
        }
    }

    AlbumItem* a = createItem(score, enabled);

    if (m_combinedScore) { // add the new score as a movement to the main score and its parts
        m_combinedScore->addMovement(score);
        m_combinedScore->update();
        m_combinedScore->doLayout(); // position the movements correctly
        if (m_combinedScore->excerpts().size() > 0) {
            // add movement to excerpts
            for (Excerpt* e : m_combinedScore->excerpts()) {
                Excerpt* ee = createMovementExcerpt(prepareMovementExcerpt(e, score));
                static_cast<MasterScore*>(e->partScore())->addMovement(static_cast<MasterScore*>(ee->partScore()));
            }
        }
    }
    return a;
}

//---------------------------------------------------------
//   removeScore
//---------------------------------------------------------

void Album::removeScore(MasterScore* score)
{
    for (int i = 0; i < int(m_albumItems.size()); i++) {
        if (m_albumItems.at(i)->score() == score) {
            removeScore(i);
            break;
        }
    }
}

void Album::removeScore(int index)
{
    if (m_combinedScore) {
        // remove the movement from the combinedScore
        if (m_combinedScore->excerpts().size() > 0) {
            // remove movement from excerpts
            for (Excerpt* e : m_combinedScore->excerpts()) {
                static_cast<MasterScore*>(e->partScore())->removeMovement(index + 1);
            }
        }
        m_combinedScore->removeMovement(index + 1);
    }
    m_albumItems.erase(m_albumItems.begin() + index);
}

//---------------------------------------------------------
//   swap
///     Swap the specified AlbumItems and relayout the combinedScore
//---------------------------------------------------------

void Album::swap(int indexA, int indexB)
{
    std::swap(m_albumItems.at(indexA), m_albumItems.at(indexB));
    if (m_combinedScore) {
        int offset = m_combinedScore->firstRealMovement();
        std::swap(m_combinedScore->movements()->at(indexA + offset), m_combinedScore->movements()->at(indexB + offset));
        m_combinedScore->doLayout(); // position the movements correctly
    }
}

//---------------------------------------------------------
//   addAlbumSectionBreaks
//---------------------------------------------------------

void Album::addAlbumSectionBreaks()
{
    for (auto& aItem : m_albumItems) {
        aItem->addAlbumSectionBreak();
    }
}

//---------------------------------------------------------
//   addAlbumPageBreaks
//---------------------------------------------------------

void Album::addAlbumPageBreaks()
{
    for (auto& aItem : m_albumItems) {
        aItem->addAlbumPageBreak();
    }
}

//---------------------------------------------------------
//   removeAlbumSectionBreaks
//---------------------------------------------------------

void Album::removeAlbumSectionBreaks()
{
    for (auto& item : m_albumItems) {
        item->removeAlbumSectionBreak();
    }
}

//---------------------------------------------------------
//   removeAlbumPageBreaks
//---------------------------------------------------------

void Album::removeAlbumPageBreaks()
{
    for (auto& item : m_albumItems) {
        item->removeAlbumPageBreak();
    }
}

//---------------------------------------------------------
//   applyDefaultPauseToSectionBreaks
//---------------------------------------------------------

void Album::applyDefaultPauseToSectionBreaks()
{
    for (auto& score : albumScores()) {
        if (score->lastMeasure()->sectionBreak()) {
            score->lastMeasure()->sectionBreakElement()->setPause(m_defaultPause);
        }
    }
}

//---------------------------------------------------------
//   composers
//---------------------------------------------------------

QStringList Album::composers() const
{
    QStringList composers;

    for (auto& item : m_albumItems) {
        if (!item->checkReadiness()) {
            continue;
        }
        QString composer = item->score()->composer();
        if (!composers.contains(composer, Qt::CaseSensitivity::CaseInsensitive) && !composer.isEmpty()) {
            composers.push_back(composer);
        }
    }

    return composers;
}

//---------------------------------------------------------
//   lyricists
//---------------------------------------------------------

QStringList Album::lyricists() const
{
    QStringList lyricists;

    for (auto& item : m_albumItems) {
        if (!item->checkReadiness()) {
            continue;
        }
        QString lyricist = item->score()->lyricist();
        if (!lyricists.contains(lyricist) && !lyricist.isEmpty()) {
            lyricists.push_back(lyricist);
        }
    }

    return lyricists;
}

//---------------------------------------------------------
//   scoreTitles
//---------------------------------------------------------

QStringList Album::scoreTitles() const
{
    QStringList scoreTitles;

    for (auto& item : m_albumItems) {
        if (!item->checkReadiness()) {
            continue;
        }
        if (!item->score()->textMovement()) {
            QString title = item->score()->realTitle();
            title = title.isEmpty() ? item->score()->title() : title;
            scoreTitles.push_back(title);
        }
    }

    return scoreTitles;
}

//---------------------------------------------------------
//   checkPartCompatibility
///     Check if the scores in the Album have compatible parts
//---------------------------------------------------------

bool Album::checkPartCompatibility() const
{
    if (!m_albumItems.size()) {
        return true;
    }

    MasterScore* firstMovement = m_albumItems.at(0)->score();
    int partCount = firstMovement->parts().size();
    // check number of parts
    for (auto& ms : albumScores()) {
        if (partCount < ms->parts().size()) {
            return false;
        }
    }
    // check part names
    for (int i = 0; i < partCount; i++) {
        for (auto& ms : albumScores()) {
            // compare instrument IDs, does it work if the instrument changes?
            if (ms->parts().at(i)->instrumentId().compare(firstMovement->parts().at(i)->instrumentId(),
                                                          Qt::CaseSensitivity::CaseInsensitive)) {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------
//   checkPartCompatibility
///     Check if the given score breaks part compatibility
//---------------------------------------------------------

bool Album::checkPartCompatibility(MasterScore* score)
{
    if (!m_albumItems.size()) {
        return true;
    }

    // check if the new score breaks compatibility
    MasterScore* firstMovement = m_albumItems.at(0)->score();
    int partCount = firstMovement->parts().size();
    // check number of parts
    if (partCount < score->parts().size()) {
        return false;
    }
    // check part names
    for (int i = 0; i < partCount; i++) {
        // compare instrument IDs, does it work if the instrument changes?
        if (score->parts().at(i)->instrumentId().compare(firstMovement->parts().at(i)->instrumentId(),
                                                         Qt::CaseSensitivity::CaseInsensitive)) {
            return false;
        }
    }
    // if it does not check if the album has lost it already
    // in the only case where this function is used the call to
    // checkPartCompatibility is superfluous, I added it as a safety net
    // for future use
    return checkPartCompatibility();
}

//---------------------------------------------------------
//   removeAlbumExcerpts
//---------------------------------------------------------

void Album::removeAlbumExcerpts()
{
    while (m_combinedScore->excerpts().size()) {
        m_combinedScore->removeExcerpt(m_combinedScore->excerpts().first());
    }
    for (auto& score : albumScores()) {
        for (auto& excerpt : score->albumExcerpts()) {
            score->removeExcerpt(excerpt, true);
        }
    }
}

//---------------------------------------------------------
//   prepareMovementExcerpt
//---------------------------------------------------------

Excerpt* Album::prepareMovementExcerpt(Excerpt* masterExcerpt, MasterScore* score)
{
    Excerpt* e = new Excerpt(score);
    for (auto& part : masterExcerpt->parts()) {
        int index = masterExcerpt->oscore()->parts().indexOf(part);
        e->parts().append(score->parts().at(index));
    }
    e->tracks() = masterExcerpt->tracks();
    e->setTitle(masterExcerpt->title());
    return e;
}

//---------------------------------------------------------
//   createMovementExcerpt
///     Create an Excerpt whose partScore will be used as a movement in
///     an Excerpt of the combinedScore.
//---------------------------------------------------------

Excerpt* Album::createMovementExcerpt(Excerpt* e)
{
    if (e->partScore()) {
        return e;
    }
    if (e->parts().isEmpty()) {
        qDebug("no parts");
        return e;
    }

    MasterScore* nscore = new MasterScore(e->oscore());
    e->setPartScore(nscore);
    nscore->setName(e->oscore()->title() + "_albumPart_" + e->oscore()->excerpts().size());
    qDebug() << " + Add part: " << e->title();
    e->oscore()->addExcerpt(e, true);
    Excerpt::createExcerpt(e);

    // a new excerpt is created in AddExcerpt, make sure the parts are filed
    for (auto& ee : e->oscore()->albumExcerpts()) {
        if (ee->partScore() == nscore && ee != e) {
            ee->parts().clear();
            ee->parts().append(e->parts());
        }
    }
    nscore->undoChangeStyleVal(MSQE_Sid::Sid::spatium, 25.016); // hack: normally it's 25 but it draws crazy stuff with that
                                                                // if you disable this the ScoreView becomes Picasso
    return e;
}

//---------------------------------------------------------
//   createCombinedScore
///     Creates the combinedScore (a score that uses all scores in the Album as movements).
///     In addition to the movements this score can have a front cover page and contents pages.
///     After adding all scores as movements excerpts/parts are added (if they exist in the savefile).
///     That last section is mostly copied from the ExcerptsDialog class.
//---------------------------------------------------------

MasterScore* Album::createCombinedScore()
{
    if (m_combinedScore) {
        qDebug() << "There is a combined score already...";
        return m_combinedScore.get();
    }

    //
    // clone the first score and use the clone as the main/combined score and as the front cover.
    //
    m_combinedScore = std::unique_ptr<MasterScore>(m_albumItems.at(0)->score()->clone());
    m_combinedScore->setMasterScore(m_combinedScore.get());
    m_combinedScore->setName("Temporary Album Score");
    m_combinedScore->style().resetAllStyles(m_combinedScore.get());
    // remove all systems/measures other than the first one (that is used for the front cover).
    while (m_combinedScore->systems().size() > 1) {
        for (auto& x : m_combinedScore->systems().last()->measures()) {
            m_combinedScore->removeElement(x);
        }
        m_combinedScore->systems().removeLast();
    }
    m_combinedScore->setTextMovement(true);

    // add the album's scores as movements and layout the combined score
    for (auto& item : m_albumItems) {
        m_combinedScore->addMovement(item->score());
    }
    if (m_drawFrontCover) {
        updateFrontCover();
    }
    if (m_generateContents) {
        m_combinedScore->setfirstRealMovement(2);
        updateContents();
    } else {
        m_combinedScore->setfirstRealMovement(1);
    }
    m_combinedScore->setLayoutAll();
    m_combinedScore->update();

    //
    // parts - excerpts
    //
    if (m_combinedScore->excerpts().isEmpty()) {
        for (auto& e : m_albumExcerpts) {
            //
            // prepare Excerpts
            //
            Excerpt* ne = new Excerpt(m_combinedScore.get());
            ne->setTitle(e->title);
            for (auto& partIndex : e->partIndices) {
                ne->parts().append(m_combinedScore->parts().at(partIndex));
            }
            ne->setTracks(e->tracks);
            //
            // create Excerpts
            //
            MasterScore* nscore = new MasterScore(ne->oscore());
            ne->setPartScore(nscore);
            nscore->setName(ne->oscore()->title() + "_part_" + ne->oscore()->excerpts().size());
            m_combinedScore->addExcerpt(ne);
            Excerpt::createExcerpt(ne);

            // a new excerpt is created in AddExcerpt, make sure the parts are filed
            for (auto& ee : ne->oscore()->excerpts()) {
                if (ee->partScore() == nscore && ee != ne) {
                    ee->parts().clear();
                    ee->parts().append(ne->parts());
                }
            }

            for (auto& m : *m_combinedScore->movements()) {
                if (m == m_combinedScore.get()) {
                    continue;
                }
                Excerpt* ee = createMovementExcerpt(prepareMovementExcerpt(ne, m));
                nscore->addMovement(static_cast<MasterScore*>(ee->partScore()));
            }

            nscore->setLayoutAll();
            nscore->undoChangeStyleVal(MSQE_Sid::Sid::spatium, 25.016); // hack: normally it's 25 but it draws crazy stuff with that
                                                                        // if you disable this the ScoreView becomes Picasso
            nscore->update();
        }
    }

    return m_combinedScore.get();
}

//---------------------------------------------------------
//   getCombinedScore
//---------------------------------------------------------

MasterScore* Album::getCombinedScore() const
{
    return m_combinedScore.get();
}

//---------------------------------------------------------
//   loadFromFile
///     Load a .msca or .album file.
//---------------------------------------------------------

bool Album::loadFromFile(const QString& path, bool legacy)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open filestream to open album: " << path;
        return false;
    }

    m_fileInfo.setFile(path);
    XmlReader reader(&f);
    reader.setDevice(&f);
    if (legacy) {
        readLegacyAlbum(reader);
    } else {
        readAlbum(reader);
    }
    readExcerpts(reader);
    f.close();
    return true;
}

//---------------------------------------------------------
//   readLegacyAlbum
//---------------------------------------------------------

void Album::readLegacyAlbum(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "name") {
            m_albumTitle = reader.readElementText();
        } else if (tag == "Score") {
            createItem(reader);
        }
    }
}

//---------------------------------------------------------
//   readAlbum
//---------------------------------------------------------

void Album::readAlbum(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "name") {
            m_albumTitle = reader.readElementText();
        } else if (tag == "Score") {
            createItem(reader);
        } else if (tag == "drawFrontCover") {
            m_drawFrontCover = reader.readBool();
        } else if (tag == "generateContents") {
            m_generateContents = reader.readBool();
        } else if (tag == "addPageBreaks") {
            m_addPageBreaksEnabled = reader.readBool();
        } else if (tag == "titleAtTheBottom") {
            setTitleAtTheBottom(reader.readBool());
        } else if (tag == "playbackDelay") {
            m_defaultPause = reader.readDouble();
        }
    }
}

//---------------------------------------------------------
//   readExcerpts
//---------------------------------------------------------

void Album::readExcerpts(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());
        if (tag == "Excerpt") {
            m_albumExcerpts.push_back(std::unique_ptr<AlbumExcerpt>(new AlbumExcerpt(reader)));
        }
    }
}

//---------------------------------------------------------
//   saveToFile
//---------------------------------------------------------

bool Album::saveToFile()
{
    return saveToFile(m_fileInfo.absoluteFilePath());
}

bool Album::saveToFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open filestream to save album: " << path;
        return false;
    }
    m_fileInfo.setFile(path);

    XmlWriter writer(nullptr, &f);
    writer.header();
    writer.stag(QStringLiteral("museScore version=\"" MSC_VERSION "\""));
    writeAlbum(writer);
    writer.etag();

    f.close();
    return true;
}

// used for exporting
bool Album::saveToFile(QIODevice* f)
{
    bool b = m_includeAbsolutePaths;
    m_includeAbsolutePaths = false;
    m_exporting = true;

    XmlWriter writer(nullptr, f);
    writer.header();
    writer.stag(QStringLiteral("museScore version=\"" MSC_VERSION "\""));
    writeAlbum(writer);
    writer.etag();

    m_includeAbsolutePaths = b;
    m_exporting = false;
    return true;
}

//---------------------------------------------------------
//   writeAlbum
//---------------------------------------------------------

void Album::writeAlbum(XmlWriter& writer) const
{
    writer.stag("Album");
    writer.tag("name", m_albumTitle);
    writer.tag("drawFrontCover", m_drawFrontCover);
    writer.tag("generateContents", m_generateContents);
    writer.tag("addPageBreaks", m_addPageBreaksEnabled);
    writer.tag("titleAtTheBottom", m_titleAtTheBottom);
    writer.tag("playbackDelay", m_defaultPause);
    for (auto& aItem : m_albumItems) {
        aItem->writeAlbumItem(writer);
    }
    writer.etag();
    writer.stag("Excerpts");
    if (m_combinedScore) {
        for (auto& e : m_combinedScore->excerpts()) {
            e->writeForAlbum(writer);
        }
    } else {
        for (auto& e : m_albumExcerpts) {
            e->writeAlbumExcerpt(writer);
        }
    }
    writer.etag();
}

//---------------------------------------------------------
//   importAlbum
///     Extract the contents of the .mscaz file in the specified directory
//---------------------------------------------------------

void Album::importAlbum(const QString& compressedFilePath, QDir destinationFolder)
{
    destinationFolder.mkdir("Scores");
    MQZipReader uz(compressedFilePath, QIODevice::ReadWrite);
    uz.extractAll(destinationFolder.path());
}

//---------------------------------------------------------
//   exportAlbum
///     Mostly copied from .mscz
//---------------------------------------------------------

bool Album::exportAlbum(QIODevice* f, const QFileInfo& info)
{
    MQZipWriter uz(f);
    QBuffer dbuf;

    QString fn = info.completeBaseName() + ".msca";
    dbuf.open(QIODevice::ReadWrite);
    saveToFile(&dbuf);
    dbuf.seek(0);
    uz.addFile(fn, dbuf.data());
    dbuf.close();

    QFileDevice* fd = dynamic_cast<QFileDevice*>(f);
    if (fd) { // if is file (may be buffer)
        fd->flush();     // flush to preserve score data in case of
    }
    // any failures on the further operations.

    for (auto& x : m_albumItems) {
        QString path = m_exportedScoreFolder + QDir::separator() + x->score()->title() + ".mscx";
        dbuf.open(QIODevice::ReadWrite);
        x->score()->Score::saveFile(&dbuf, false);
        dbuf.seek(0);
        uz.addFile(path, dbuf.data());
        dbuf.close();
        fd->flush();
    }

    fd->flush();
    uz.close();
    return true;
}

//---------------------------------------------------------
//   setAlbumLayoutMode
//---------------------------------------------------------

void Album::setAlbumLayoutMode(LayoutMode lm)
{
    for (auto& x : m_albumItems) {
        x->score()->setLayoutMode(lm);
        x->score()->doLayout();
    }
    if (m_combinedScore) {
        m_combinedScore->doLayout();
    }
}

//---------------------------------------------------------
//   albumItems
//---------------------------------------------------------

std::vector<AlbumItem*> Album::albumItems() const
{
    std::vector<AlbumItem*> ai {};
    for (auto& x : m_albumItems) {
        ai.push_back(x.get());
    }
    return ai;
}

//---------------------------------------------------------
//   albumScores
//---------------------------------------------------------

std::vector<MasterScore*> Album::albumScores() const
{
    std::vector<MasterScore*> scores {};
    for (auto& x : m_albumItems) {
        scores.push_back(x->score());
    }
    return scores;
}

//---------------------------------------------------------
//   updateFrontCover
//---------------------------------------------------------

void Album::updateFrontCover()
{
    if (!getCombinedScore()) {
        return;
    }

    VBox* box = toVBox(getCombinedScore()->measures()->first());
    qreal pageHeight = getCombinedScore()->pages().at(0)->height();
    qreal scoreSpatium = getCombinedScore()->spatium();

    // the front cover has its own page
    if (m_drawFrontCover) {
        box->setOffset(0, pageHeight * 0.1);
        box->setBoxHeight(Spatium(pageHeight * 0.8 / scoreSpatium));
    } else { // the front cover is presented as a VBox before the first movement
        box->setOffset(0, 0);
        box->setBoxHeight(Spatium(pageHeight * 0.05 / scoreSpatium));
    }

    // make sure that we have these 3 text fields
    MeasureBase* measure = getCombinedScore()->measures()->first();
    measure->clearElements();
    Text* s = new Text(getCombinedScore(), Tid::ALBUM_FRONT_TITLE);
    s->setPlainText("");
    measure->add(s);
    s = new Text(getCombinedScore(), Tid::ALBUM_FRONT_COMPOSER);
    s->setPlainText("");
    measure->add(s);
    s = new Text(getCombinedScore(), Tid::ALBUM_FRONT_LYRICIST);
    s->setPlainText("");
    measure->add(s);

    for (auto& x : getCombinedScore()->measures()->first()->el()) {
        if (x && x->isText()) {
            Text* t = toText(x);

            if (t->tid() == Tid::ALBUM_FRONT_TITLE) {
                t->setPlainText(albumTitle());
            } else if (t->tid() == Tid::ALBUM_FRONT_COMPOSER) {
                if (m_drawFrontCover) {
                    t->setPlainText(composers().join("\n"));
                }
            } else if (t->tid() == Tid::ALBUM_FRONT_LYRICIST) {
                if (m_drawFrontCover) {
                    t->setPlainText(lyricists().join("\n"));
                }
            }
        }
    }
}

//---------------------------------------------------------
//   updateContents
///     Updates (or creates or deletes) the movement and content responsible
///     for the Contents pages.
//---------------------------------------------------------

void Album::updateContents()
{
    if (!getCombinedScore()) {
        return;
    }

    //
    // delete the contents pages-movement
    //
    if (!generateContents()) {
        if (m_combinedScore->movements()->at(1)->textMovement()) {
            MasterScore* contentsMovement = m_combinedScore->movements()->at(1);
            m_combinedScore->removeMovement(1);
            delete contentsMovement;
        }
        return;
    }

    qreal pageWidth = getCombinedScore()->pages().at(0)->width();
    qreal pageHeight = getCombinedScore()->pages().at(0)->height();
    qreal scoreSpatium = getCombinedScore()->spatium();
    int charWidth = pageWidth / scoreSpatium;

    //
    // if there is no contents movement create one
    //
    if (!getCombinedScore()->movements()->at(1)->textMovement()) {
        MasterScore* ms = m_albumItems.at(0)->score()->clone();
        ms->setName("Contents");
        ms->setTextMovement(true);
        getCombinedScore()->insertMovement(ms, 1);

        while (ms->systems().size() > 1) {
            for (auto& x : ms->systems().last()->measures()) {
                ms->removeElement(x);
            }
            ms->systems().removeLast();
        }

        // make sure that we have these 2 text fields
        MeasureBase* measure = ms->measures()->first();
        measure->clearElements();
        Text* s = new Text(ms, Tid::ALBUM_CONTENTS_TITLE);
        s->setPlainText("Contents");
        measure->add(s);
        s = new Text(ms, Tid::ALBUM_CONTENTS_TEXT);
        s->setPlainText("");
        measure->add(s);
    }

    //
    // add content to Contents ;-)
    //
    MasterScore* ms = getCombinedScore()->movements()->at(1);
    MeasureBase* measure = ms->measures()->first();

    Text* t = nullptr;
    for (auto& x : measure->el()) {
        if (x && x->isText()) {
            t = toText(x);
            if (t->tid() == Tid::ALBUM_CONTENTS_TEXT) {
                break;
            }
        }
    }

    if (!t) {
        return;
    }

    QString str("");
    int i = 0;
    for (auto& x : scoreTitles()) {
        // add line to the current page
        QString temp(x);
        temp.append(QString(".").repeated(charWidth - x.length()));
        temp += QString::number(albumItems().at(i)->score()->pageIndexInAlbum());
        temp += "\n";
        str += temp;
        i++;
        t->setPlainText(str);
        // TODO: find a more efficient way to calculate whether the page is full
        ms->doLayout(); // a relayout is called for every title

        // if this page is full make a new VBox (or use the next one) and continue
        if (t->pageBoundingRect().height() + t->pageBoundingRect().y() > pageHeight * 0.8) {
            t = nullptr;
            if (measure->next()) {
                measure = measure->next();
                for (auto& x : measure->el()) {
                    if (x && x->isText()) {
                        t = toText(x);
                        break;
                    }
                }
                if (!t) {
                    qDebug() << "Error: could not generate Contents page";
                }
            } else {
                VBox* newMeasure = new VBox(ms);
                newMeasure->clearElements();
                Text* s = new Text(ms, Tid::ALBUM_CONTENTS_TITLE);
                s->setPlainText("Contents");
                newMeasure->add(s);
                s = new Text(ms, Tid::ALBUM_CONTENTS_TEXT);
                s->setPlainText("");
                newMeasure->add(s);
                ms->measures()->add(newMeasure);
                t = s;
            }
            str = QString("");
        }
    }
    ms->doLayout();
}

//---------------------------------------------------------
//   albumTitle
//---------------------------------------------------------

const QString& Album::albumTitle() const
{
    return m_albumTitle;
}

//---------------------------------------------------------
//   setAlbumTitle
//---------------------------------------------------------

void Album::setAlbumTitle(const QString& newTitle)
{
    m_albumTitle = newTitle;
    updateFrontCover();
}

//---------------------------------------------------------
//   fileInfo
//---------------------------------------------------------

const QFileInfo& Album::fileInfo() const
{
    return m_fileInfo;
}

//---------------------------------------------------------
//   albumModeActive
//---------------------------------------------------------

bool Album::albumModeActive()
{
    if (!activeAlbum) {
        return false;
    }
    return activeAlbum->m_albumModeActive;
}

//---------------------------------------------------------
//   setAlbumModeActive
//---------------------------------------------------------

void Album::setAlbumModeActive(bool b)
{
    if (!activeAlbum) {
        return;
    }
    activeAlbum->m_albumModeActive = b;
}

//---------------------------------------------------------
//   titleAtTheBottom
//---------------------------------------------------------

bool Album::titleAtTheBottom() const
{
    return m_titleAtTheBottom;
}

//---------------------------------------------------------
//   setTitleAtTheBottom
//---------------------------------------------------------

void Album::setTitleAtTheBottom(bool titleAtTheBottom)
{
    m_titleAtTheBottom = titleAtTheBottom;
    if (m_combinedScore) {
        m_combinedScore->setTitleAtTheBottom(titleAtTheBottom);
    }
}

//---------------------------------------------------------
//   drawFrontCover
//---------------------------------------------------------

bool Album::drawFrontCover() const
{
    return m_drawFrontCover;
}

//---------------------------------------------------------
//   setDrawFrontCover
//---------------------------------------------------------

void Album::setDrawFrontCover(bool b)
{
    m_drawFrontCover = b;
}

//---------------------------------------------------------
//   generateContents
//---------------------------------------------------------

bool Album::generateContents() const
{
    return m_generateContents;
}

//---------------------------------------------------------
//   setGenerateContents
//---------------------------------------------------------

void Album::setGenerateContents(bool enabled)
{
    m_generateContents = enabled;
}

//---------------------------------------------------------
//   addPageBreaksEnabled
//---------------------------------------------------------

bool Album::addPageBreaksEnabled() const
{
    return m_addPageBreaksEnabled;
}

//---------------------------------------------------------
//   setAddPageBreaksEnabled
//---------------------------------------------------------

void Album::setAddPageBreaksEnabled(bool enabled)
{
    m_addPageBreaksEnabled = enabled;
}

//---------------------------------------------------------
//   includeAbsolutePaths
//---------------------------------------------------------

bool Album::includeAbsolutePaths() const
{
    return m_includeAbsolutePaths;
}

//---------------------------------------------------------
//   setIncludeAbsolutePaths
//---------------------------------------------------------

void Album::setIncludeAbsolutePaths(bool enabled)
{
    m_includeAbsolutePaths = enabled;
}

//---------------------------------------------------------
//   defaultPlaybackDelay
//---------------------------------------------------------

int Album::defaultPause() const
{
    return m_defaultPause;
}

//---------------------------------------------------------
//   setDefaultPlaybackDelay
//---------------------------------------------------------

void Album::setDefaultPause(qreal s)
{
    m_defaultPause = s;
}

//---------------------------------------------------------
//   exportedScoreFolder
//---------------------------------------------------------

const QString& Album::exportedScoreFolder() const
{
    return m_exportedScoreFolder;
}

//---------------------------------------------------------
//   exporting
//---------------------------------------------------------

bool Album::exporting() const
{
    return m_exporting;
}
}     // namespace Ms
