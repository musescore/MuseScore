/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#pragma once

#include "abstractstyledialogmodel.h"

namespace mu::notation {
class SlursAndTiesPageModel : public AbstractStyleDialogModel
{
    Q_OBJECT

    Q_PROPERTY(StyleItem * slurEndWidth READ slurEndWidth CONSTANT)
    Q_PROPERTY(StyleItem * slurMidWidth READ slurMidWidth CONSTANT)
    Q_PROPERTY(StyleItem * slurDottedWidth READ slurDottedWidth CONSTANT)
    Q_PROPERTY(StyleItem * slurMinDistance READ slurMinDistance CONSTANT)
    Q_PROPERTY(StyleItem * angleHangingSlursAwayFromStaff READ angleHangingSlursAwayFromStaff CONSTANT)
    Q_PROPERTY(StyleItem * tieEndWidth READ tieEndWidth CONSTANT)
    Q_PROPERTY(StyleItem * tieMidWidth READ tieMidWidth CONSTANT)
    Q_PROPERTY(StyleItem * tieDottedWidth READ tieDottedWidth CONSTANT)
    Q_PROPERTY(StyleItem * tieMinDistance READ tieMinDistance CONSTANT)
    Q_PROPERTY(StyleItem * minTieLength READ minTieLength CONSTANT)
    Q_PROPERTY(StyleItem * minHangingTieLength READ minHangingTieLength CONSTANT)
    Q_PROPERTY(StyleItem * tiePlacementSingleNote READ tiePlacementSingleNote CONSTANT)
    Q_PROPERTY(StyleItem * tiePlacementChord READ tiePlacementChord CONSTANT)
    Q_PROPERTY(StyleItem * tieDotsPlacement READ tieDotsPlacement CONSTANT)
    Q_PROPERTY(StyleItem * minLaissezVibLength READ minLaissezVibLength CONSTANT)
    Q_PROPERTY(StyleItem * laissezVibUseSmuflSym READ laissezVibUseSmuflSym CONSTANT)

public:
    explicit SlursAndTiesPageModel(QObject* parent = nullptr);

    StyleItem* slurEndWidth() const;
    StyleItem* slurMidWidth() const;
    StyleItem* slurDottedWidth() const;
    StyleItem* slurMinDistance() const;
    StyleItem* angleHangingSlursAwayFromStaff() const;
    StyleItem* tieEndWidth() const;
    StyleItem* tieMidWidth() const;
    StyleItem* tieDottedWidth() const;
    StyleItem* tieMinDistance() const;
    StyleItem* minTieLength() const;
    StyleItem* minHangingTieLength() const;
    StyleItem* tiePlacementSingleNote() const;
    StyleItem* tiePlacementChord() const;
    StyleItem* tieDotsPlacement() const;
    StyleItem* minLaissezVibLength() const;
    StyleItem* laissezVibUseSmuflSym() const;
};
}
