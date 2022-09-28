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
#include "noteheadsettingsmodel.h"

#include "engraving/types/types.h"

#include "translation.h"

using namespace mu::inspector;

NoteheadSettingsModel::NoteheadSettingsModel(QObject* parent, IElementRepositoryService* repository)
    : AbstractInspectorModel(parent, repository)
{
    setTitle(qtrc("inspector", "Head"));
    setModelType(InspectorModelType::TYPE_NOTEHEAD);

    createProperties();
}

void NoteheadSettingsModel::createProperties()
{
    m_isHeadHidden = buildPropertyItem(mu::engraving::Pid::VISIBLE, [this](const mu::engraving::Pid pid, const QVariant& isHeadHidden) {
        onPropertyValueChanged(pid, !isHeadHidden.toBool());
    });

    m_isHeadSmall = buildPropertyItem(mu::engraving::Pid::SMALL);
    m_hasHeadParentheses = buildPropertyItem(mu::engraving::Pid::HEAD_HAS_PARENTHESES);
    m_headDirection = buildPropertyItem(mu::engraving::Pid::MIRROR_HEAD);
    m_headGroup = buildPropertyItem(mu::engraving::Pid::HEAD_GROUP);
    m_headType = buildPropertyItem(mu::engraving::Pid::HEAD_TYPE);
    m_headSystem = buildPropertyItem(mu::engraving::Pid::HEAD_SCHEME);
    m_dotPosition = buildPropertyItem(mu::engraving::Pid::DOT_POSITION);
    m_offset = buildPointFPropertyItem(mu::engraving::Pid::OFFSET);
}

void NoteheadSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::NOTEHEAD);
}

void NoteheadSettingsModel::loadProperties()
{
    loadPropertyItem(m_isHeadHidden, [](const QVariant& isVisible) -> QVariant {
        return !isVisible.toBool();
    });

    loadPropertyItem(m_isHeadSmall);
    loadPropertyItem(m_hasHeadParentheses);
    loadPropertyItem(m_headDirection);
    loadPropertyItem(m_headGroup);
    loadPropertyItem(m_headType);
    loadPropertyItem(m_headSystem);
    loadPropertyItem(m_dotPosition);
    loadPropertyItem(m_offset);
}

void NoteheadSettingsModel::resetProperties()
{
    m_isHeadHidden->resetToDefault();
    m_isHeadSmall->resetToDefault();
    m_hasHeadParentheses->resetToDefault();
    m_headDirection->resetToDefault();
    m_headGroup->resetToDefault();
    m_headType->resetToDefault();
    m_headSystem->resetToDefault();
    m_dotPosition->resetToDefault();
    m_offset->resetToDefault();
}

PropertyItem* NoteheadSettingsModel::isHeadHidden() const
{
    return m_isHeadHidden;
}

PropertyItem* NoteheadSettingsModel::isHeadSmall() const
{
    return m_isHeadSmall;
}

PropertyItem* NoteheadSettingsModel::hasHeadParentheses() const
{
    return m_hasHeadParentheses;
}

PropertyItem* NoteheadSettingsModel::headDirection() const
{
    return m_headDirection;
}

PropertyItem* NoteheadSettingsModel::headGroup() const
{
    return m_headGroup;
}

PropertyItem* NoteheadSettingsModel::headType() const
{
    return m_headType;
}

PropertyItem* NoteheadSettingsModel::headSystem() const
{
    return m_headSystem;
}

PropertyItem* NoteheadSettingsModel::dotPosition() const
{
    return m_dotPosition;
}

PropertyItem* NoteheadSettingsModel::offset() const
{
    return m_offset;
}

QVariantList NoteheadSettingsModel::possibleHeadSystemTypes() const
{
    QMap<mu::engraving::NoteHeadScheme, QString> types {
        { mu::engraving::NoteHeadScheme::HEAD_AUTO,                    mu::qtrc("inspector", "Auto") },
        { mu::engraving::NoteHeadScheme::HEAD_NORMAL,                  mu::qtrc("inspector", "Normal") },
        { mu::engraving::NoteHeadScheme::HEAD_PITCHNAME,               mu::qtrc("inspector", "Pitch names") },
        { mu::engraving::NoteHeadScheme::HEAD_PITCHNAME_GERMAN,        mu::qtrc("inspector", "German pitch names") },
        { mu::engraving::NoteHeadScheme::HEAD_SOLFEGE,                 mu::qtrc("inspector", "Solfège movable do") },
        { mu::engraving::NoteHeadScheme::HEAD_SOLFEGE_FIXED,           mu::qtrc("inspector", "Solfège fixed do") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_4,            mu::qtrc("inspector", "4-shape (Walker)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN,      mu::qtrc("inspector", "7-shape (Aikin)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK,       mu::qtrc("inspector", "7-shape (Funk)") },
        { mu::engraving::NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER,     mu::qtrc("inspector", "7-shape (Walker)") },
    };

    QVariantList result;

    for (mu::engraving::NoteHeadScheme type : types.keys()) {
        QVariantMap obj;

        obj["text"] = types[type];
        obj["value"] = static_cast<int>(type);

        result << obj;
    }

    return result;
}
