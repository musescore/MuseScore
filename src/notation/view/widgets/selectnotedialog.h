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
#ifndef MU_NOTATION_SELECTNOTEDIALOG_H
#define MU_NOTATION_SELECTNOTEDIALOG_H

#include "ui_selectnotedialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::engraving {
struct NotePattern;
class Note;
}

namespace mu::notation {
class SelectNoteDialog : public QDialog, Ui::SelectNoteDialog, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<context::IGlobalContext> globalContext = { this };

public:
    SelectNoteDialog(QWidget* parent = nullptr);

    bool doReplace() const;
    bool doAdd() const;
    bool doSubtract() const;
    bool doFromSelection() const;
    bool isInSelection() const;
    void setSameStringVisible(bool v);

private slots:
    void buttonClicked(QAbstractButton* button);

private:
    virtual void hideEvent(QHideEvent*);

    INotationPtr currentNotation() const;
    INotationInteractionPtr currentNotationInteraction() const;
    INotationElementsPtr currentNotationElements() const;

    void apply() const;
    FilterNotesOptions noteOptions() const;

    const mu::engraving::Note* m_note = nullptr;
};
}

#endif // MU_NOTATION_SELECTNOTEDIALOG_H
