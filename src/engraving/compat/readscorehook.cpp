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
#include "readscorehook.h"
#include "readstyle.h"

#include "libmscore/masterscore.h"
#include "libmscore/scorefont.h"

#include "log.h"

using namespace mu::engraving::compat;
using namespace Ms;

void ReadScoreHook::installReadStyleHook(Ms::MasterScore* score, const QByteArray& scoreData, const QString& completeBaseName)
{
    m_readStyle = std::make_shared<ReadStyleHook>(score, scoreData, completeBaseName);
}

void ReadScoreHook::installReadStyleHook(Ms::Score* score)
{
    IF_ASSERT_FAILED(!score->isMaster()) {
        return;
    }
    m_readStyle = std::make_shared<ReadStyleHook>(score, QByteArray(), QString());
}

void ReadScoreHook::setupDefaultStyle()
{
    if (!m_readStyle) {
        return;
    }
    m_readStyle->setupDefaultStyle();
}

void ReadScoreHook::onReadStyleTag302(Ms::Score* score, Ms::XmlReader& xml)
{
    if (!m_readStyle) {
        return;
    }

    qreal sp = score->style().value(Sid::spatium).toDouble();

    compat::ReadStyleHook::readStyleTag(score, xml);

    // if (_layoutMode == LayoutMode::FLOAT || _layoutMode == LayoutMode::SYSTEM) {
    if (score->_layoutMode == LayoutMode::FLOAT) {
        // style should not change spatium in
        // float mode
        score->style().set(Sid::spatium, sp);
    }
    score->_scoreFont = ScoreFont::fontByName(score->style().value(Sid::MusicalSymbolFont).toString());
}

void ReadScoreHook::onReadExcerptTag302(Ms::Score* score, Ms::XmlReader& xml)
{
    if (MScore::noExcerpts) {
        xml.skipCurrentElement();
    } else {
        if (score->isMaster()) {
            MasterScore* mScore = static_cast<MasterScore*>(this);
            Excerpt* ex = new Excerpt(mScore);
            ex->read(e);
            mScore->excerpts().append(ex);
        } else {
            LOGE() << "part cannot have parts";
            xml.skipCurrentElement();
        }
    }
}
