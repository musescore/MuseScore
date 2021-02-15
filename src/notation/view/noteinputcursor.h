//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_NOTATION_NOTEINPUTCURSOR_H
#define MU_NOTATION_NOTEINPUTCURSOR_H

#include <QRect>
#include <QColor>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/inotationconfiguration.h"

class QPainter;

namespace mu::notation {
class NoteInputCursor
{
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, INotationConfiguration, configuration)

public:

    NoteInputCursor() = default;

    void paint(QPainter* painter);

private:
    INotationNoteInputPtr currentNoteInput() const;

    bool isNoteInputMode() const;
    QRectF rect() const;
    QColor cursorColor() const;
    QColor fillColor() const;
};
}

#endif // MU_NOTATION_NOTEINPUTCURSOR_H
