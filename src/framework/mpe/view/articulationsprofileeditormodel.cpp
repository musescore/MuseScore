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

#include "articulationsprofileeditormodel.h"

using namespace muse;
using namespace muse::mpe;

static const std::string PROFILE_EXTENSION = "(*.json)";

ArticulationsProfileEditorModel::ArticulationsProfileEditorModel(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

void ArticulationsProfileEditorModel::init()
{
    loadItems();

    setProfile(profilesRepository()->createNew());
}

void ArticulationsProfileEditorModel::requestToOpenProfile()
{
    //! Make these strings translatable when we expose this tool to users
    std::vector<std::string> filter = { /*qtrc*/ std::string("MPE articulations profile") + " " + PROFILE_EXTENSION };
    io::path_t path = interactive()->selectOpeningFileSync(/*trc*/ "Open MPE articulations profile", "", filter);

    if (path.empty()) {
        return;
    }

    setCurrentPath(path.toQString());

    setProfile(profilesRepository()->loadProfile(m_profilePath));
}

bool ArticulationsProfileEditorModel::requestToCreateProfile()
{
    std::vector<std::string> filter = { /*qtrc*/ std::string("MPE articulations profile") + " " + PROFILE_EXTENSION };
    io::path_t path = interactive()->selectSavingFileSync(/*trc*/ "Save MPE articulations profile", "", filter);

    if (path.empty()) {
        return false;
    }

    setCurrentPath(path.toQString());
    return true;
}

void ArticulationsProfileEditorModel::requestToSaveProfile()
{
    if (m_profilePath.empty()) {
        if (!requestToCreateProfile()) {
            return;
        }
    }

    for (const auto& item : m_singleNoteItems) {
        if (item->isActive()) {
            m_profile->setPattern(item->type(), item->patternData());
        } else {
            m_profile->removePattern(item->type());
        }
    }

    for (const auto& item : m_multiNoteItems) {
        if (item->isActive()) {
            m_profile->setPattern(item->type(), item->patternData());
        } else {
            m_profile->removePattern(item->type());
        }
    }

    profilesRepository()->saveProfile(m_profilePath, m_profile);
}

void ArticulationsProfileEditorModel::copyPatternDataFromItem(ArticulationPatternItem* item)
{
    if (!item || !selectedItem()) {
        return;
    }

    m_selectedItem->load(item->patternData());
}

void ArticulationsProfileEditorModel::setProfile(ArticulationsProfilePtr ptr)
{
    m_profile = std::move(ptr);

    auto load = [this](const QMap<ArticulationType, ArticulationPatternItem*>& items) {
        for (auto& item : items) {
            item->setIsActive(m_profile->contains(item->type()));
            item->load(m_profile->pattern(item->type()));
        }
    };

    load(m_singleNoteItems);
    load(m_multiNoteItems);

    setSelectedItem(m_singleNoteItems.first());
}

void ArticulationsProfileEditorModel::loadItems()
{
    int firstIdx = static_cast<int>(ArticulationType::Standard);
    int lastIdx = static_cast<int>(ArticulationType::Last);

    for (int i = firstIdx; i < lastIdx; ++i) {
        ArticulationType type = static_cast<ArticulationType>(i);

        if (isSingleNoteArticulation(type)) {
            m_singleNoteItems.insert(type, buildItem(type, true /*isSingleNoteType*/));
        } else {
            m_multiNoteItems.insert(type, buildItem(type, false /*isSingleNoteType*/));
        }
    }
}

ArticulationPatternItem* ArticulationsProfileEditorModel::buildItem(const ArticulationType type, const bool isSingleNoteType)
{
    ArticulationPatternItem* item = new ArticulationPatternItem(this, type, isSingleNoteType);

    connect(item, &ArticulationPatternItem::isSelectedChanged, this, [this, item]() {
        setSelectedItem(item);
    });

    return item;
}

QString ArticulationsProfileEditorModel::currentPath() const
{
    return m_profilePath.toQString();
}

void ArticulationsProfileEditorModel::setCurrentPath(const QString& newCurrentPath)
{
    io::path_t newPath(newCurrentPath);

    if (m_profilePath == newPath) {
        return;
    }
    m_profilePath = newPath;
    emit currentPathChanged();
}

ArticulationPatternItem* ArticulationsProfileEditorModel::selectedItem() const
{
    return m_selectedItem;
}

void ArticulationsProfileEditorModel::setSelectedItem(ArticulationPatternItem* newSelectedItem)
{
    if (m_selectedItem == newSelectedItem) {
        return;
    }

    if (m_selectedItem) {
        m_selectedItem->setIsSelected(false);
    }

    newSelectedItem->setIsSelected(true);
    m_selectedItem = newSelectedItem;
    emit selectedItemChanged();
}

QList<ArticulationPatternItem*> ArticulationsProfileEditorModel::singleNoteItems() const
{
    QList<ArticulationPatternItem*> result;

    for (const auto& item : m_singleNoteItems) {
        result << item;
    }

    return result;
}

QList<ArticulationPatternItem*> ArticulationsProfileEditorModel::multiNoteItems() const
{
    QList<ArticulationPatternItem*> result;

    for (const auto& item : m_multiNoteItems) {
        result << item;
    }

    return result;
}

bool ArticulationsProfileEditorModel::isArrangementVisible() const
{
    return m_isArrangementVisible;
}

void ArticulationsProfileEditorModel::setIsArrangementVisible(bool newIsArrangementVisible)
{
    if (m_isArrangementVisible == newIsArrangementVisible) {
        return;
    }
    m_isArrangementVisible = newIsArrangementVisible;
    emit isArrangementVisibleChanged();
}

bool ArticulationsProfileEditorModel::isPitchVisible() const
{
    return m_isPitchVisible;
}

void ArticulationsProfileEditorModel::setIsPitchVisible(bool newIsPitchVisible)
{
    if (m_isPitchVisible == newIsPitchVisible) {
        return;
    }
    m_isPitchVisible = newIsPitchVisible;
    emit isPitchVisibleChanged();
}

bool ArticulationsProfileEditorModel::isExpressionVisible() const
{
    return m_isExpressionVisible;
}

void ArticulationsProfileEditorModel::setIsExpressionVisible(bool newIsExpressionVisible)
{
    if (m_isExpressionVisible == newIsExpressionVisible) {
        return;
    }
    m_isExpressionVisible = newIsExpressionVisible;
    emit isExpressionVisibleChanged();
}
