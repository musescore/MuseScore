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
#ifndef MU_NOTATION_NOTATION_H
#define MU_NOTATION_NOTATION_H

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "iengravingconfiguration.h"

#include "../inotation.h"
#include "igetscore.h"
#include "../inotationconfiguration.h"

namespace Ms {
class MScore;
class Score;
}

namespace mu::notation {
class NotationInteraction;
class NotationPlayback;
class Notation : virtual public INotation, public IGetScore, public async::Asyncable
{
    INJECT_STATIC(notation, INotationConfiguration, configuration)
    INJECT(notation, engraving::IEngravingConfiguration, engravingConfiguration)

public:
    explicit Notation(Ms::Score* score = nullptr);
    ~Notation() override;

    static void init();

    QString name() const override;
    QString projectName() const override;
    QString projectNameAndPartName() const override;

    QString workTitle() const override;
    QString projectWorkTitle() const override;
    QString projectWorkTitleAndPartName() const override;

    bool isOpen() const override;
    void setIsOpen(bool open) override;
    async::Notification openChanged() const override;

    void setViewMode(const ViewMode& viewMode) override;
    ViewMode viewMode() const override;

    INotationPaintingPtr painting() const override;
    INotationInteractionPtr interaction() const override;
    INotationMidiInputPtr midiInput() const override;
    INotationUndoStackPtr undoStack() const override;
    INotationElementsPtr elements() const override;
    INotationStylePtr style() const override;
    INotationAccessibilityPtr accessibility() const override;
    INotationPartsPtr parts() const override;

    async::Notification notationChanged() const override;

protected:
    Ms::Score* score() const override;
    void setScore(Ms::Score* score);
    void notifyAboutNotationChanged();

    INotationPartsPtr m_parts = nullptr;
    INotationUndoStackPtr m_undoStack = nullptr;
    async::Notification m_notationChanged;

private:
    friend class NotationInteraction;
    friend class NotationPainting;

    Ms::Score* m_score = nullptr;
    async::Notification m_openChanged;

    INotationPaintingPtr m_painting = nullptr;
    INotationInteractionPtr m_interaction = nullptr;
    INotationStylePtr m_style = nullptr;
    INotationMidiInputPtr m_midiInput = nullptr;
    INotationAccessibilityPtr m_accessibility = nullptr;
    INotationElementsPtr m_elements = nullptr;
};
}

#endif // MU_NOTATION_NOTATION_H
