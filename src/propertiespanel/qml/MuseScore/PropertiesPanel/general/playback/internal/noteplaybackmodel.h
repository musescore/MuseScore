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

#include "propertiespanelabstractmodel.h"

namespace mu::propertiespanel {
class NotePlaybackModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(mu::propertiespanel::PropertyItem * tuning READ tuning CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * velocity READ velocity CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * playbackStartOffset READ playbackStartOffset CONSTANT)
    Q_PROPERTY(mu::propertiespanel::PropertyItem * playbackDurationOffset READ playbackDurationOffset CONSTANT)

public:
    explicit NotePlaybackModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    PropertyItem* tuning() const;
    PropertyItem* velocity() const;
    PropertyItem* playbackStartOffset() const;
    PropertyItem* playbackDurationOffset() const;

protected:
    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet& changedPropertyIdSet,
                           const mu::engraving::StyleIdSet& changedStyleIdSet) override;

private:
    // Playback start/duration offset are only ever honored on a tie chain's first note - a
    // tied-continuation note is skipped entirely during rendering (see NoteRenderer::shouldRender()
    // and the matching tieBack() skip in NotationNoteOffsetController::createOverlayForStaff()).
    // Reading/writing these two properties on the exact selected note would silently affect
    // nothing whenever that note is a tied continuation, and would disagree with what the
    // on-canvas drag-handle overlay shows for the same chain - so both directions are redirected
    // to each note's own chain head, regardless of which note in the chain is selected. A note
    // that is neither its chain's head nor its tail (a middle link in a 3+-note chain) owns no
    // handle at all in that overlay, so it's dropped from the returned list entirely rather than
    // redirected - loadPropertyItem()/setPropertyValue() then treat it as no selection at all,
    // leaving both spinboxes disabled instead of silently editing a value it has no handle for.
    QList<mu::engraving::EngravingItem*> headNoteElements() const;

    PropertyItem* m_tuning = nullptr;
    PropertyItem* m_velocity = nullptr;
    PropertyItem* m_playbackStartOffset = nullptr;
    PropertyItem* m_playbackDurationOffset = nullptr;
};
}
