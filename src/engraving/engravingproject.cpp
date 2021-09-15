/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "engravingproject.h"

#include <QFileInfo>

#include "style/defaultstyle.h"

#include "libmscore/masterscore.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"

#include "log.h"

using namespace mu::engraving;

std::shared_ptr<EngravingProject> EngravingProject::create()
{
    std::shared_ptr<EngravingProject> p = std::shared_ptr<EngravingProject>(new EngravingProject());
    p->init(DefaultStyle::defaultStyle());
    return p;
}

std::shared_ptr<EngravingProject> EngravingProject::create(const Ms::MStyle& style)
{
    std::shared_ptr<EngravingProject> p = std::shared_ptr<EngravingProject>(new EngravingProject());
    p->init(style);
    return p;
}

EngravingProject::~EngravingProject()
{
    delete m_masterScore;
}

void EngravingProject::init(const Ms::MStyle& style)
{
    m_masterScore = new Ms::MasterScore(style, shared_from_this());
}

void EngravingProject::setPath(const QString& path)
{
    QFileInfo fi(path);
    m_masterScore->setName(fi.completeBaseName());
    m_masterScore->setImportedFilePath(fi.filePath());
    m_masterScore->setMetaTag("originalFormat", fi.suffix().toLower());

    m_path = path;
}

QString EngravingProject::path() const
{
    return m_path;
}

bool EngravingProject::readOnly() const
{
    return m_masterScore->readOnly();
}

Err EngravingProject::setupMasterScore()
{
    TRACEFUNC;

    engravingElementsProvider()->clearStatistic();
    Err err = doSetupMasterScore(m_masterScore);
    engravingElementsProvider()->printStatistic("=== Update and Layout ===");

    return err;
}

Err EngravingProject::doSetupMasterScore(Ms::MasterScore* score)
{
    score->connectTies();

    for (Ms::Part* p : score->parts()) {
        p->updateHarmonyChannels(false);
    }
    score->rebuildMidiMapping();
    score->setSoloMute();
    for (Ms::Score* s : score->scoreList()) {
        s->setPlaylistDirty();
        s->addLayoutFlags(Ms::LayoutFlag::FIX_PITCH_VELO);
        s->setLayoutAll();
    }
    score->updateChannel();
    //score->updateExpressive(MuseScore::synthesizer("Fluid"));
    score->setSaved(true);
    score->setCreated(false);
    score->update();

    if (!score->sanityCheck(QString())) {
        return Err::FileCorrupted;
    }

    return Err::NoError;
}

Ms::MasterScore* EngravingProject::masterScore() const
{
    return m_masterScore;
}

Err EngravingProject::loadMscz(const MscReader& reader, bool ignoreVersionError)
{
    TRACEFUNC;

    engravingElementsProvider()->clearStatistic();
    Ms::Score::FileError err = m_masterScore->loadMscz(reader, ignoreVersionError);
    engravingElementsProvider()->printStatistic("=== Load ===");

    return scoreFileErrorToErr(err);
}

bool EngravingProject::writeMscz(mu::engraving::MscWriter& writer, bool onlySelection, bool createThumbnail)
{
    bool ok = m_masterScore->writeMscz(writer, onlySelection, createThumbnail);
    if (ok && !onlySelection) {
        m_masterScore->undoStack()->setClean();
        m_masterScore->setSaved(true);
        m_masterScore->update();
    }

    return ok;
}

void EngravingProject::checkTree()
{
    LOGI() << "\n\n\n";
    LOGI() << "========================";
    checkTree(m_masterScore->rootItem());

    LOGI() << "========================";
//    LOGI() << "dumpTree:";
//    int level = 0;
//    dumpTree(m_masterScore->rootItem(), level);

//    LOGI() << "========================";
//    LOGI() << "dumpTreeTree:";
//    level = 0;
//    dumpTreeTree(m_masterScore, level);

//    LOGI() << "========================";
}

void EngravingProject::dumpTree(const Ms::EngravingItem* item, int& level)
{
    if (item->isDummy()) {
        return;
    }

    ++level;
    QString gap;
    gap.fill(' ', level);
    LOGI() << gap << item->name();
    for (const Ms::EngravingObject* ch : item->children()) {
        if (!ch->isEngravingItem()) {
            LOGI() << "[" << item->name() << ": not item ch: " << ch->name();
            continue;
        }
        dumpTree(static_cast<const Ms::EngravingItem*>(ch), level);
    }
    --level;
}

void EngravingProject::dumpTreeTree(const Ms::EngravingObject* obj, int& level)
{
    ++level;
    QString gap;
    gap.fill(' ', level);
    LOGI() << gap << obj->name();
    for (int i = 0; i < obj->treeChildCount(); ++i) {
        const Ms::EngravingObject* ch = obj->treeChild(i);
        dumpTreeTree(ch, level);
    }
    --level;
}

void EngravingProject::checkTree(const Ms::EngravingObject* obj)
{
    if (obj->isDummy()) {
        return;
    }

    Ms::EngravingObject* p1 = obj->parent(true);
    Ms::EngravingObject* p2 = obj->treeParent();
    if (p1 && p2 && p1 != p2) {
        LOGI() << "obj: " << obj->name();
        LOGE() << "parents is differens, p1: " << p1->name() << ", p2: " << p2->name();
    }

    int ch1 = obj->children().size();
    int ch2 = obj->treeChildCount();
    if (ch1 != ch2) {
        LOGI() << "obj: " << obj->name();
        LOGE() << "chcount is differens, ch1: " << ch1 << ", ch2: " << ch2;

        LOGI() << "children1:";
        for (size_t i = 0; i < obj->children().size(); ++i) {
            LOGI() << i << ": " << obj->children().at(i)->name();
        }

        LOGI() << "children2:";
        for (int i = 0; i < obj->treeChildCount(); ++i) {
            LOGI() << i << ": " << obj->treeChild(i)->name();
        }
    }

    for (const Ms::EngravingObject* ch : obj->children()) {
        checkTree(ch);
    }
}
