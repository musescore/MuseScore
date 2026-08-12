/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "lyricslinesettingsmodel.h"

#include "translation.h"

using namespace mu::propertiespanel;

LyricsLineSettingsModel::LyricsLineSettingsModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx,
                                                 IElementRepositoryService* repository,
                                                 ElementType elementType)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    if (elementType == ElementType::LyricsLine) {
        setTitle(muse::qtrc("propertiespanel", "Lyrics line"));
        setElementType(mu::engraving::ElementType::LYRICSLINE);
        setModelType(PropertiesPanelModelType::TYPE_LYRICS_LINE);
    } else if (elementType == ElementType::PartialLyricsLine) {
        setTitle(muse::qtrc("propertiespanel", "Partial lyrics line"));
        setElementType(mu::engraving::ElementType::PARTIAL_LYRICSLINE);
        setModelType(PropertiesPanelModelType::TYPE_PARTIAL_LYRICS_LINE);
        m_hasVerse = true;
    }
    setIcon(muse::ui::IconCode::Code::LYRICS);

    createProperties();
}

PropertyItem* LyricsLineSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* LyricsLineSettingsModel::verse() const
{
    return m_verse;
}

bool LyricsLineSettingsModel::hasVerse() const
{
    return m_hasVerse;
}

void LyricsLineSettingsModel::createProperties()
{
    m_thickness = buildPropertyItem(mu::engraving::Pid::LINE_WIDTH);
    m_verse = buildPropertyItem(mu::engraving::Pid::VERSE);
}

void LyricsLineSettingsModel::loadProperties()
{
    loadPropertyItem(m_thickness);
    loadPropertyItem(m_verse);
}
