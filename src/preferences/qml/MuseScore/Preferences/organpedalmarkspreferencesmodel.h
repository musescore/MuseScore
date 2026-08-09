/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#pragma once

#include <qqmlintegration.h>

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "engraving/iengravingconfiguration.h"

namespace mu::preferences {
class OrganPedalMarksPreferencesModel : public QObject, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT
    QML_ELEMENT;

    Q_PROPERTY(QVariantList placements READ placements CONSTANT)
    Q_PROPERTY(int placement READ placement WRITE setPlacement NOTIFY placementChanged)

    Q_PROPERTY(QVariantList popupSets READ popupSets CONSTANT)
    Q_PROPERTY(int popupSet READ popupSet WRITE setPopupSet NOTIFY popupSetChanged)

    Q_PROPERTY(QVariantList navigationPlacements READ navigationPlacements CONSTANT)
    Q_PROPERTY(int navigationPlacement READ navigationPlacement WRITE setNavigationPlacement NOTIFY navigationPlacementChanged)

    Q_PROPERTY(QVariantList pedalMarks READ pedalMarks CONSTANT)
    Q_PROPERTY(int defaultPedalMark READ defaultPedalMark WRITE setDefaultPedalMark NOTIFY defaultPedalMarkChanged FINAL)

    muse::GlobalInject<engraving::IEngravingConfiguration> configuration;

public:
    explicit OrganPedalMarksPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();

    QVariantList placements() const;
    int placement() const;
    void setPlacement(int placement);

    QVariantList popupSets() const;
    int popupSet() const;
    void setPopupSet(int popupSet);

    QVariantList navigationPlacements() const;
    int navigationPlacement() const;
    void setNavigationPlacement(int navigationPlacement);

    QVariantList pedalMarks() const;
    int defaultPedalMark() const;
    void setDefaultPedalMark(int pedalMark);

signals:
    void placementChanged();
    void popupSetChanged();
    void navigationPlacementChanged();
    void defaultPedalMarkChanged();
};
}