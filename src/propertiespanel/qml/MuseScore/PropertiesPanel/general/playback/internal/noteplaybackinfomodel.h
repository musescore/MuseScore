/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <vector>

#include <QVariantList>

#include "propertiespanelabstractmodel.h"

#include "engraving/playback/noteplaybackinfo.h"

namespace mu::propertiespanel {
//! Read-only summary of what playback will do with a single selected note
class NotePlaybackInfoModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(QVariantList passes READ passes NOTIFY infoChanged)
    Q_PROPERTY(bool isMuted READ isMuted NOTIFY infoChanged)
    Q_PROPERTY(QString muteReason READ muteReason NOTIFY infoChanged)
    Q_PROPERTY(int velocityOverride READ velocityOverride NOTIFY infoChanged)
    Q_PROPERTY(QVariantList tempoMarkings READ tempoMarkings NOTIFY infoChanged)
    Q_PROPERTY(QVariantList dynamicMarkings READ dynamicMarkings NOTIFY infoChanged)
    Q_PROPERTY(QVariantList otherMarkings READ otherMarkings NOTIFY infoChanged)
    Q_PROPERTY(int pinnedIndex READ pinnedIndex NOTIFY focusChanged)

public:
    explicit NotePlaybackInfoModel(
        QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    QVariantList passes() const { return m_passes; }
    bool isMuted() const { return m_isMuted; }
    QString muteReason() const { return m_muteReason; }
    int velocityOverride() const { return m_velocityOverride; }
    //! Ties, pauses, staccato/tenuto and slurs: they change how long the note lasts
    QVariantList tempoMarkings() const { return m_tempoMarkings; }
    //! Marcato and accents: they make the note louder than its dynamic
    QVariantList dynamicMarkings() const { return m_dynamicMarkings; }
    //! Every other marking that shapes how the note plays
    QVariantList otherMarkings() const { return m_otherMarkings; }
    int pinnedIndex() const { return m_pinnedIndex; }

    //! Hovering a listed item highlights only that item, centering the view on it if it's out of view; leaving
    //! goes back to the pinned item, or to where the view was if nothing is pinned. None of this changes the selection.
    Q_INVOKABLE void hoverItem(int itemIndex);
    Q_INVOKABLE void unhoverItem(int itemIndex);

    //! Pins the item: it stays centered and highlighted after the mouse leaves, until another item is clicked
    Q_INVOKABLE void clickItem(int itemIndex);

    //! Selects the item in the score; the panel then describes that item instead of the note
    Q_INVOKABLE void doubleClickItem(int itemIndex);

    bool isEmpty() const override
    {
        // Single note only
        return m_elementList.size() != 1;
    }

    bool shouldUpdateOnEmptyPropertyAndStyleIdSets() const override
    {
        // A dynamic/tempo change anywhere before the note can change what it plays at
        return true;
    }

    void createProperties() override {}
    void requestElements() override;
    void loadProperties() override;
    void onCurrentNotationChanged() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&) override;

signals:
    void infoChanged();
    void focusChanged();

private:
    const mu::engraving::Note* selectedNote() const;
    void reload();
    void updateMuteState(const mu::engraving::Note* note);

    //! The copy of a (main score) item that is shown in the current notation, which may be a part
    const mu::engraving::EngravingItem* itemInCurrentScore(const mu::engraving::EngravingItem* item) const;

    void applyFocus();
    void clearFocus();

    QVariantList m_passes;
    bool m_isMuted = false;
    QString m_muteReason;
    int m_velocityOverride = 0;

    // Every item the panel lists (sources and markings); QML refers to them by index
    std::vector<const mu::engraving::EngravingItem*> m_items;
    QVariantList m_tempoMarkings;
    QVariantList m_dynamicMarkings;
    QVariantList m_otherMarkings;

    int m_hoveredIndex = -1;
    int m_pinnedIndex = -1;
};
} // namespace mu::propertiespanel
