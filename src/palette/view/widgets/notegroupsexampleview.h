/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_PALETTE_NOTEGROUPSEXAMPLEVIEW_H
#define MU_PALETTE_NOTEGROUPSEXAMPLEVIEW_H

#include "notation/view/widgets/exampleview.h"

#include "modularity/ioc.h"
#include "engraving/rendering/isinglerenderer.h"

namespace mu::engraving {
class Note;
class Chord;
class ActionIcon;
}

namespace mu::palette {
class NoteGroupsExampleView : public notation::ExampleView
{
    Q_OBJECT

    INJECT_STATIC(engraving::rendering::ISingleRenderer, engravingRender)

public:
    NoteGroupsExampleView(QWidget* parent = 0);

signals:
    void noteClicked(engraving::Note*);
    void beamPropertyDropped(engraving::Chord*, engraving::ActionIcon*);

private:
    void setDropTarget(engraving::EngravingItem* el) override;

    void dragEnterEvent(QDragEnterEvent*) override;
    void dragLeaveEvent(QDragLeaveEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dropEvent(QDropEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

    engraving::EngravingItem* m_dragElement = 0;
    const engraving::EngravingItem* m_dropTarget = 0; ///< current drop target during dragMove
    QLineF m_dropAnchor;                   ///< line to current anchor point during dragMove
};
}

#endif // MU_PALETTE_NOTEGROUPSEXAMPLEVIEW_H
