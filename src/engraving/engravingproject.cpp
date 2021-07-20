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

#include "libmscore/score.h"
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
    return doSetupMasterScore(m_masterScore);
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
    Ms::Score::FileError err = m_masterScore->loadMscz(reader, ignoreVersionError);
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
