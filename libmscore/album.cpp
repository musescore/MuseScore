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
//---------------------------------------------------------
//
//   AlbumExcerpt
//
//---------------------------------------------------------
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
//---------------------------------------------------------
//
//   AlbumItem
//
//---------------------------------------------------------
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
    if (score) {
        score->setPartOfActiveAlbum(false); // also called in ~AlbumManagerItem, FIXME
    }
    // TODO_SK: if (score not in score list, delete it)
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
    score->setEnabled(b);
    if (album.getDominant()) {
        album.getDominant()->update();
        album.getDominant()->doLayout();
    }
}

bool AlbumItem::enabled() const
{
    if (!checkReadiness()) {
        return false;
    }
    return m_enabled;
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

int AlbumItem::setScore(MasterScore* score)
{
    // if you want to change the score, create a new AlbumItem
    if (this->score != nullptr) {
        qDebug() << "The AlbumItem already has a Score, don't set a new one. Create a new AlbumItem." << endl;
//        Q_ASSERT(false);
        return -1;
    }
    // don't set an empty score
    if (score == nullptr) {
        qDebug() << "You are trying to set an empty score." << endl;
//        Q_ASSERT(false);
        return -1;
    }

    this->score = score;
    setEnabled(m_enabled);
    score->setPartOfActiveAlbum(true);
    if (!score->importedFilePath().isEmpty()) {
        fileInfo.setFile(score->importedFilePath());
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
//   addAlbumSectionBreak
//---------------------------------------------------------

void AlbumItem::addAlbumSectionBreak()
{
    if (!checkReadiness()) {
        return;
    }
    if (!score->lastMeasure()->sectionBreak()) { // add only if there isn't one
        LayoutBreak* lb = new LayoutBreak(score);
        lb->setLayoutBreakType(LayoutBreak::Type::SECTION);
        score->lastMeasure()->add(lb);
        if (m_pauseDuration >= 0) {
            lb->setPause(m_pauseDuration);
        } else {
            m_pauseDuration = lb->pause();
        }
        m_extraSectionBreak = true;
        score->update();
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
    if (m_extraSectionBreak && score->lastMeasure()) {
        m_pauseDuration = getSectionBreak()->pause();
        score->lastMeasure()->remove(getSectionBreak());
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
    if (!score->lastMeasure()->pageBreak()) { // add only if there isn't one
        LayoutBreak* lb = new LayoutBreak(score);
        lb->setLayoutBreakType(LayoutBreak::Type::PAGE);
        score->lastMeasure()->add(lb);
        m_extraPageBreak = true;
        score->update();
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
    if (m_extraPageBreak && score->lastMeasure()) {
        for (auto& e : score->lastMeasure()->el()) {
            if (e->isLayoutBreak() && toLayoutBreak(e)->isPageBreak()) {
                score->lastMeasure()->remove(e);
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
            fileInfo.setFile(reader.readElementText());
            album.setIncludeAbsolutePaths(true);
        } else if (tag == "relativePath") {
            if (!fileInfo.exists()) {
                QDir dir(album.fileInfo().dir());
                QString relativePath = reader.readElementText();
                fileInfo.setFile(dir, relativePath);
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
//   saveAlbumItem
//---------------------------------------------------------

void AlbumItem::writeAlbumItem(XmlWriter& writer)
{
    writer.stag("Score");
    writer.tag("alias", "");
    if (album.includeAbsolutePaths()) {
        writer.tag("path", fileInfo.absoluteFilePath());
    }
    if (!album.exporting()) {
        QDir dir(album.fileInfo().dir());
        QString relativePath = dir.relativeFilePath(fileInfo.absoluteFilePath());
        writer.tag("relativePath", relativePath);
    } else {
        writer.tag("relativePath", album.exportedScoreFolder() + QDir::separator() + score->title() + ".mscx");
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
    m_duration = score->duration();
    emit durationChanged();
}

//---------------------------------------------------------
//   getSectionBreak
//---------------------------------------------------------

LayoutBreak* AlbumItem::getSectionBreak() const
{
    if (!score) {
        return nullptr;
    }
    return score->lastMeasure()->sectionBreakElement();
}

bool AlbumItem::checkReadiness() const
{
    if (!score) {
        qDebug() << "You need to load a score before you use an AlbumItem." << endl;
        Q_ASSERT(false);
        return false;
    }
    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
//
//   Album
//
//---------------------------------------------------------
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
        if (x->score == score) {
            return true;
        }
    }
    if (score == activeAlbum->m_dominantScore) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

Album::Album()
{
    Album::activeAlbum = this;
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
        std::cout << "There is no score to add to album..." << std::endl;
        return nullptr;
    }

    if (checkPartCompatibility() && !checkPartCompatibility(score)) { // if you break compatibility by adding this
        QMessageBox msgBox;
        msgBox.setWindowTitle(QObject::tr("Incompatible parts"));
        msgBox.setText(QString("The parts of your new score are incompatible with the rest of the album."));
        msgBox.setDetailedText(QString("The parts of your new score are incompatible with the rest of the album. That means "
                                       "that adding this score will break the `Parts` functionality for your Album. You can"
                                       "remove this score to restore this functionality. "));
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(
            QMessageBox::Cancel | QMessageBox::Ignore
            );
        auto response = msgBox.exec();
        if (response == QMessageBox::Cancel) {
            return nullptr;
        } else {
            while (m_dominantScore->excerpts().size()) {
                m_dominantScore->removeExcerpt(m_dominantScore->excerpts().first());
            }
        }
    }

    std::cout << "Adding score to album..." << std::endl;
    AlbumItem* a = createItem(score, enabled);

    if (m_dominantScore) {
        m_dominantScore->addMovement(score);
        m_dominantScore->update();
        m_dominantScore->doLayout(); // position the movements correctly
        if (m_dominantScore->excerpts().size() > 0) {
            // add movement to excerpts
            for (auto& e : m_dominantScore->excerpts()) {
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
        if (m_albumItems.at(i)->score == score) {
            removeScore(i);
            break;
        }
    }
}

void Album::removeScore(int index)
{
    if (m_dominantScore) {
        // remove the movement from the dominantScore
        if (m_dominantScore->excerpts().size() > 0) {
            // remove movement from excerpts
            for (auto& e : m_dominantScore->excerpts()) {
                static_cast<MasterScore*>(e->partScore())->removeMovement(index + 1);
            }
        }
        m_dominantScore->removeMovement(index + 1);
    }
    m_albumItems.at(index)->score->setPartOfActiveAlbum(false);
    m_albumItems.erase(m_albumItems.begin() + index);
}

//---------------------------------------------------------
//   swap
//---------------------------------------------------------

void Album::swap(int indexA, int indexB)
{
    std::swap(m_albumItems.at(indexA), m_albumItems.at(indexB));
    if (m_dominantScore) {
        int offset = m_dominantScore->firstRealMovement();
        std::swap(m_dominantScore->movements()->at(indexA + offset), m_dominantScore->movements()->at(indexB + offset));
        m_dominantScore->doLayout(); // position the movements correctly
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
//   composers
//---------------------------------------------------------

QStringList Album::composers() const
{
    QStringList composers;

    for (auto& item : m_albumItems) {
        QString composer = item->score->composer();
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
        QString lyricist = item->score->lyricist();
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
        if (!item->score->emptyMovement()) {
            QString title = item->score->realTitle();
            title = title.isEmpty() ? item->score->title() : title;
            scoreTitles.push_back(title);
        }
    }

    return scoreTitles;
}

//---------------------------------------------------------
//   checkPartCompatibility
//---------------------------------------------------------

bool Album::checkPartCompatibility() const
{
    if (!m_albumItems.size()) {
        return true;
    }

    MasterScore* firstMovement = m_albumItems.at(0)->score;
    int partCount = firstMovement->parts().size();
    // check number of parts
    for (auto& x : m_albumItems) {
        if (partCount < x->score->parts().size()) {
            return false;
        }
    }
    // check part names
    for (int i = 0; i < partCount; i++) {
        for (auto& x : m_albumItems) {
            if (x->score->parts().at(i)->partName().compare(firstMovement->parts().at(i)->partName(),
                                                            Qt::CaseSensitivity::CaseInsensitive)) {
                return false;
            }
        }
    }
    return true;
}

bool Album::checkPartCompatibility(MasterScore* score)
{
    if (!m_albumItems.size()) {
        return true;
    }

    // check if the new score breaks compatibility
    MasterScore* firstMovement = m_albumItems.at(0)->score;
    int partCount = firstMovement->parts().size();
    // check number of parts
    if (partCount < score->parts().size()) {
        return false;
    }
    // check part names
    for (int i = 0; i < partCount; i++) {
        if (score->parts().at(i)->partName().compare(firstMovement->parts().at(i)->partName(),
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
    while (m_dominantScore->excerpts().size()) {
        m_dominantScore->removeExcerpt(m_dominantScore->excerpts().first());
    }
    for (auto& x : m_albumItems) {
        for (auto y : x->score->albumExcerpts()) {
            x->score->removeExcerpt(y, true);
        }
    }
}

//---------------------------------------------------------
//   prepareMovementExcerpt
//---------------------------------------------------------

Excerpt* Album::prepareMovementExcerpt(Excerpt* masterExcerpt, MasterScore* score)
{
    Excerpt* e = new Excerpt(score);
    for (auto part : masterExcerpt->parts()) {
        int index = masterExcerpt->oscore()->parts().indexOf(part);
        e->parts().append(score->parts().at(index));
    }
    e->tracks() = masterExcerpt->tracks();
    e->setTitle(masterExcerpt->title());
    return e;
}

//---------------------------------------------------------
//   createMovementExcerpt
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
    qDebug() << " + Add part : " << e->title();
    e->oscore()->addExcerpt(e, true);
    Excerpt::createExcerpt(e);

    // a new excerpt is created in AddExcerpt, make sure the parts are filed
    for (Excerpt* ee : e->oscore()->albumExcerpts()) {
        if (ee->partScore() == nscore && ee != e) {
            ee->parts().clear();
            ee->parts().append(e->parts());
        }
    }
    nscore->undoChangeStyleVal(MSQE_Sid::Sid::spatium, 25.016); // hack: normally it's 25 but it draws crazy stuff with that
    nscore->doLayout();
    return e;
}

//---------------------------------------------------------
//   setDominant
//---------------------------------------------------------

MasterScore* Album::createDominant()
{
    if (m_dominantScore) {
        qDebug() << "There is a dominant score already..." << endl;
        return m_dominantScore;
    }

    //
    // clone the first score and use the clone as the main/dominant score and as the front cover.
    //
    m_dominantScore = m_albumItems.at(0)->score->clone();
    m_dominantScore->setMasterScore(m_dominantScore);
    m_dominantScore->setName("Temporary Album Score");
    m_dominantScore->style().reset(m_dominantScore);
    // remove all systems/measures other than the first one (that is used for the front cover).
    while (m_dominantScore->systems().size() > 1) {
        for (auto x : m_dominantScore->systems().last()->measures()) {
            m_dominantScore->removeElement(x);
        }
        m_dominantScore->systems().removeLast();
    }
    m_dominantScore->setEmptyMovement(true); // TODO_SK: rename emptyMovement (it's not really empty)
    m_dominantScore->setPartOfActiveAlbum(true);

    if (generateContents()) {
        m_dominantScore->setfirstRealMovement(2);
    } else {
        m_dominantScore->setfirstRealMovement(1);
    }

    // add the album's scores as movements and layout the combined score
    for (auto& item : m_albumItems) {
        m_dominantScore->addMovement(item->score);
    }
    if (m_drawFrontCover) {
        updateFrontCover();
    }
    if (m_generateContents) {
        updateContents();
    }
    m_dominantScore->setLayoutAll();
    m_dominantScore->update();

    //
    // parts - excerpts
    //
    if (m_dominantScore->excerpts().isEmpty()) {
        for (auto& e : m_albumExcerpts) {
            //
            // prepare Excerpts
            //
            Excerpt* ne = new Excerpt(m_dominantScore);
            ne->setTitle(e->title);
            for (auto partIndex : e->partIndices) {
                ne->parts().append(m_dominantScore->parts().at(partIndex));
            }
            ne->setTracks(e->tracks);
            //
            // create Excerpts
            //
            MasterScore* nscore = new MasterScore(ne->oscore());
            ne->setPartScore(nscore);
            nscore->setName(ne->oscore()->title() + "_part_" + ne->oscore()->excerpts().size());
            m_dominantScore->addExcerpt(ne);
            Excerpt::createExcerpt(ne);

            // a new excerpt is created in AddExcerpt, make sure the parts are filed
            for (Excerpt* ee : ne->oscore()->excerpts()) {
                if (ee->partScore() == nscore && ee != ne) {
                    ee->parts().clear();
                    ee->parts().append(ne->parts());
                }
            }

            for (auto m : *m_dominantScore->movements()) {
                if (m == m_dominantScore) {
                    continue;
                }
                Excerpt* ee = createMovementExcerpt(prepareMovementExcerpt(ne, m));
                nscore->addMovement(static_cast<MasterScore*>(ee->partScore()));
            }

            nscore->setLayoutAll();
            nscore->setUpdateAll();
            nscore->undoChangeStyleVal(MSQE_Sid::Sid::spatium, 25.016); // hack: normally it's 25 but it draws crazy stuff with that
            nscore->update();
        }
    }

    return m_dominantScore;
}

//---------------------------------------------------------
//   getDominant
//---------------------------------------------------------

MasterScore* Album::getDominant() const
{
    return m_dominantScore;
}

//---------------------------------------------------------
//   loadFromFile
//---------------------------------------------------------

bool Album::loadFromFile(const QString& path)
{
    std::cout << "Loading album from file..." << std::endl;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open filestream to open album: " << path << endl;
        return false;
    }

    m_fileInfo.setFile(path);
    XmlReader reader(&f);
    reader.setDevice(&f);
    readAlbum(reader);
    readExcerpts(reader);
    f.close();
    return true;
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
        } else if (tag == "generateContents") {
            m_generateContents = reader.readBool();
        } else if (tag == "addPageBreaks") {
            m_addPageBreaksEnabled = reader.readBool();
        } else if (tag == "titleAtTheBottom") {
            m_titleAtTheBottom = reader.readBool();
        } else if (tag == "playbackDelay") {
            m_defaultPlaybackDelay = reader.readInt();
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
    std::cout << "Saving album to file..." << std::endl;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open filestream to save album: " << path << endl;
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
    writer.tag("generateContents", m_generateContents);
    writer.tag("addPageBreaks", m_addPageBreaksEnabled);
    writer.tag("titleAtTheBottom", m_titleAtTheBottom);
    writer.tag("playbackDelay", m_defaultPlaybackDelay);
    for (auto& aItem : m_albumItems) {
        aItem->writeAlbumItem(writer);
    }
    writer.etag();
    writer.stag("Excerpts");
    if (m_dominantScore) {
        for (auto e : m_dominantScore->excerpts()) {
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
//---------------------------------------------------------

void Album::importAlbum(const QString& compressedFilePath, QDir destinationFolder)
{
    destinationFolder.mkdir("Scores");
    MQZipReader uz(compressedFilePath, QIODevice::ReadWrite);
    uz.extractAll(destinationFolder.path());
}

//---------------------------------------------------------
//   exportAlbum
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

    for (auto x : albumItems()) {
        QString path = m_exportedScoreFolder + QDir::separator() + x->score->title() + ".mscx";
        dbuf.open(QIODevice::ReadWrite);
        x->score->Score::saveFile(&dbuf, false);
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
        x->score->setLayoutMode(lm);
        x->score->doLayout();
    }
    if (m_dominantScore) {
        m_dominantScore->doLayout();
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
//   updateFrontCover
//---------------------------------------------------------

void Album::updateFrontCover()
{
    if (!getDominant()) {
        return;
    }

    VBox* box = toVBox(getDominant()->measures()->first());
    qreal pageHeight = getDominant()->pages().at(0)->height();
    qreal scoreSpatium = getDominant()->spatium();

    box->setOffset(0, pageHeight * 0.1);
    box->setBoxHeight(Spatium(pageHeight * 0.8 / scoreSpatium));

    // make sure that we have these 3 text fields
    MeasureBase* measure = getDominant()->measures()->first();
    measure->clearElements();
    Text* s = new Text(getDominant(), Tid::TITLE);
    s->setPlainText("");
    measure->add(s);
    s = new Text(getDominant(), Tid::COMPOSER);
    s->setPlainText("");
    measure->add(s);
    s = new Text(getDominant(), Tid::POET);
    s->setPlainText("");
    measure->add(s);

    for (auto x : getDominant()->measures()->first()->el()) {
        if (x && x->isText()) {
            Text* t = toText(x);

            if (t->tid() == Tid::TITLE) {
                t->setFontStyle(FontStyle::Bold); // I should be calling t->setBold(true) (this overwrites other styles) but it crashes
                t->setSize(36);

                t->cursor()->setRow(0);
                t->setPlainText(albumTitle());
            } else if (t->tid() == Tid::COMPOSER) {
                t->setSize(16);
                t->setPlainText(composers().join("\n"));
            } else if (t->tid() == Tid::POET) {
                t->setSize(16);
                t->setPlainText(lyricists().join("\n"));
            }
        }
    }
}

//---------------------------------------------------------
//   updateContents
//---------------------------------------------------------

void Album::updateContents()
{
    if (!getDominant()) {
        return;
    }

    if (!generateContents()) {
        return;
    }

    qreal pageWidth = getDominant()->pages().at(0)->width();
    qreal scoreSpatium = getDominant()->spatium();
    int charWidth = pageWidth / scoreSpatium;

    if (!getDominant()->movements()->at(1)->emptyMovement()) {    // there is no contents page
        MasterScore* ms = m_albumItems.at(0)->score->clone();
        ms->setName("Contents");
        ms->setEmptyMovement(true);
        getDominant()->insertMovement(ms, 1);

        while (ms->systems().size() > 1) {
            for (auto x : ms->systems().last()->measures()) {
                ms->removeElement(x);
            }
            ms->systems().removeLast();
        }

        // make sure that we have these 2 text fields
        MeasureBase* measure = ms->measures()->first();
        measure->clearElements();
        Text* s = new Text(ms, Tid::TITLE);
        s->setPlainText("");
        measure->add(s);
        s = new Text(ms, Tid::SUBTITLE);
        s->setPlainText("");
        measure->add(s);
    }

    MasterScore* ms = getDominant()->movements()->at(1);
    for (auto x : ms->measures()->first()->el()) {
        if (x && x->isText()) {
            Text* t = toText(x);

            if (t->tid() == Tid::TITLE) {
                t->setFontStyle(FontStyle::Bold); // I should be calling t->setBold(true) (this overwrites other styles) but it crashes
                t->setSize(36);

                t->cursor()->setRow(0);
                t->setPlainText("Contents");
            } else if (t->tid() == Tid::SUBTITLE) {
                t->setSize(16);
                t->setAlign(Align::LEFT | Align::BASELINE);

                QString str("");

                int i = 0;
                for (auto x : scoreTitles()) {
                    QString temp(x);
                    temp.append(QString(".").repeated(charWidth - x.length()));
                    temp += QString::number(albumItems().at(i)->score->pageIndexInAlbum());
                    temp += "\n";
                    str += temp;
                    i++;
                }

                t->cursor()->setRow(0);
                t->setPlainText(str);
            } else if (t->tid() == Tid::COMPOSER) {
                t->setSize(16);
                t->setPlainText("");
            } else if (t->tid() == Tid::POET) {
                t->setSize(16);
                t->setPlainText("");
            }
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

bool Album::albumModeActive() const
{
    return m_albumModeActive;
}

//---------------------------------------------------------
//   setAlbumModeActive
//---------------------------------------------------------

void Album::setAlbumModeActive(bool b)
{
    m_albumModeActive = b;
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
    if (m_dominantScore) {
        m_dominantScore->setTitleAtTheBottom(titleAtTheBottom);
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

int Album::defaultPlaybackDelay() const
{
    return m_defaultPlaybackDelay;
}

//---------------------------------------------------------
//   setDefaultPlaybackDelay
//---------------------------------------------------------

void Album::setDefaultPlaybackDelay(int ms)
{
    m_defaultPlaybackDelay = ms;
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
