//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_NOTATION_SELECTNOTEDIALOG_H
#define MU_NOTATION_SELECTNOTEDIALOG_H

#include "ui_selectnotedialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace Ms {
struct NotePattern;
class Note;
}

namespace mu::notation {
//---------------------------------------------------------
//   SelectNoteDialog
//---------------------------------------------------------

class SelectNoteDialog : public QDialog, Ui::SelectNoteDialog
{
    Q_OBJECT
    INJECT(notation, context::IGlobalContext, globalContext)

public:
    SelectNoteDialog(QWidget* parent = nullptr);
    SelectNoteDialog(const SelectNoteDialog& other);

    static int metaTypeId();

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

    const Ms::Note* m_note = nullptr;
};
}

Q_DECLARE_METATYPE(mu::notation::SelectNoteDialog)

#endif // MU_NOTATION_SELECTNOTEDIALOG_H
