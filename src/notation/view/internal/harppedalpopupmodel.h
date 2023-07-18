/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_NOTATION_HARPPEDALPOPUPMODEL_H
#define MU_NOTATION_HARPPEDALPOPUPMODEL_H

#include "abstractelementpopupmodel.h"
#include "context/iglobalcontext.h"
#include "engraving/libmscore/harppedaldiagram.h"
#include "engraving/libmscore/undo.h"
#include <QObject>

namespace mu::notation {
class HarpPedalPopupModel : public AbstractElementPopupModel
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)

    Q_PROPERTY(bool isDiagram READ isDiagram WRITE setIsDiagram NOTIFY isDiagramChanged)
    Q_PROPERTY(
        QVector<mu::notation::HarpPedalPopupModel::Position> pedalState READ pedalState WRITE setDiagramPedalState NOTIFY pedalStateChanged)
    Q_PROPERTY(QRectF staffPos READ staffPos CONSTANT)
    Q_PROPERTY(bool belowStave READ belowStave CONSTANT)

public:
    enum class Position {
        FLAT,
        NATURAL,
        SHARP,

        UNSET
    };
    Q_ENUM(Position)

    explicit HarpPedalPopupModel(QObject* parent = nullptr);

    bool isDiagram() const;

    QRectF staffPos() const;

    bool belowStave() const;

    QVector<Position> pedalState() const;

    Q_INVOKABLE void init() override;

public slots:
    void setIsDiagram(bool isDiagram);
    void setDiagramPedalState(QVector<mu::notation::HarpPedalPopupModel::Position> pedalState);

signals:
    void isDiagramChanged(bool isDiagram);
    void pedalStateChanged(QVector<mu::notation::HarpPedalPopupModel::Position> pedalState);

private:
    void load();

    // Convert between mu::engraving::PedalPosition and internal qml safe Position enums
    void setPopupPedalState(std::array<mu::engraving::PedalPosition, mu::engraving::HARP_STRING_NO> pos);

    void setPopupPedalState(std::array<HarpPedalPopupModel::Position, mu::engraving::HARP_STRING_NO> pos);

    std::array<mu::engraving::PedalPosition, mu::engraving::HARP_STRING_NO> getPopupPedalState();

    bool m_isDiagram = false;

    std::array<Position, mu::engraving::HARP_STRING_NO> m_pedalState;
};
} //namespace mu::notation

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::notation::HarpPedalPopupModel::Position)
#endif

#endif // MU_NOTATION_HARPPEDALPOPUPMODEL_H
