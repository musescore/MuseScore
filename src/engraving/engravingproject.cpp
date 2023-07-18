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

#include "global/allocator.h"

#include "style/defaultstyle.h"
#include "rw/mscloader.h"
#include "rw/mscsaver.h"
#include "libmscore/masterscore.h"
#include "libmscore/part.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

std::shared_ptr<EngravingProject> EngravingProject::create()
{
    if (engravingElementsProvider()) {
        engravingElementsProvider()->clearStatistic();
    }

    std::shared_ptr<EngravingProject> p = std::shared_ptr<EngravingProject>(new EngravingProject());
    p->init(DefaultStyle::defaultStyle());
    return p;
}

std::shared_ptr<EngravingProject> EngravingProject::create(const MStyle& style)
{
    if (engravingElementsProvider()) {
        engravingElementsProvider()->clearStatistic();
    }

    std::shared_ptr<EngravingProject> p = std::shared_ptr<EngravingProject>(new EngravingProject());
    p->init(style);
    return p;
}

EngravingProject::EngravingProject()
{
    ObjectAllocator::used();
}

EngravingProject::~EngravingProject()
{
    delete m_masterScore;

    ObjectAllocator::unused();

    AllocatorsRegister::instance()->printStatistic("=== Destroy engraving project ===");
    //! NOTE At the moment, the allocator is working as leak detector. No need to do cleanup, at the moment it can lead to crashes
    // AllocatorsRegister::instance()->cleanupAll("engraving");
}

void EngravingProject::init(const MStyle& style)
{
    m_masterScore = new MasterScore(style, weak_from_this());
}

IFileInfoProviderPtr EngravingProject::fileInfoProvider() const
{
    return m_masterScore->fileInfo();
}

void EngravingProject::setFileInfoProvider(IFileInfoProviderPtr fileInfoProvider)
{
    m_masterScore->setFileInfoProvider(fileInfoProvider);
}

String EngravingProject::appVersion() const
{
    return m_masterScore->mscoreVersion();
}

int EngravingProject::mscVersion() const
{
    return m_masterScore->mscVersion();
}

bool EngravingProject::readOnly() const
{
    return m_masterScore->readOnly();
}

Ret EngravingProject::setupMasterScore(bool forceMode)
{
    return doSetupMasterScore(forceMode);
}

Ret EngravingProject::doSetupMasterScore(bool forceMode)
{
    TRACEFUNC;

    m_masterScore->createPaddingTable();
    m_masterScore->connectTies();

    for (Part* p : m_masterScore->parts()) {
        p->updateHarmonyChannels(false);
    }

    m_masterScore->rebuildMidiMapping();

    for (Score* s : m_masterScore->scoreList()) {
        s->setPlaylistDirty();
        s->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
        s->setLayoutAll();
    }

    m_masterScore->updateChannel();
    m_masterScore->update();

    Ret ret = checkCorrupted();
    m_isCorruptedUponLoading = !ret;

    return forceMode ? make_ok() : ret;
}

MasterScore* EngravingProject::masterScore() const
{
    return m_masterScore;
}

Ret EngravingProject::loadMscz(const MscReader& msc, SettingsCompat& settingsCompat, bool ignoreVersionError)
{
    TRACEFUNC;
    MScore::setError(MsError::MS_NO_ERROR);
    MscLoader loader;
    return loader.loadMscz(m_masterScore, msc, settingsCompat, ignoreVersionError);
}

bool EngravingProject::writeMscz(MscWriter& writer, bool onlySelection, bool createThumbnail)
{
    TRACEFUNC;
    MscSaver saver;
    bool ok = saver.writeMscz(m_masterScore, writer, onlySelection, createThumbnail);
    if (ok && !onlySelection) {
        m_masterScore->update();
    }

    return ok;
}

bool EngravingProject::isCorruptedUponLoading() const
{
    return m_isCorruptedUponLoading;
}

Ret EngravingProject::checkCorrupted() const
{
    TRACEFUNC;

    return m_masterScore->sanityCheck();
}
