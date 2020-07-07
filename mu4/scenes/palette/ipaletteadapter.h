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
#ifndef MU_PALETTE_IPALETTEADAPTER_H
#define MU_PALETTE_IPALETTEADAPTER_H

#include "modularity/imoduleexport.h"

#include <QString>
#include <QPointF>

#include "mscore/globals.h"

class QAction;

namespace Ms {
class InstrumentChange;
class Score;
class ScriptRecorder;
enum class ViewState;
class Slur;
class Score;
class Element;
}

namespace mu {
namespace scene {
namespace palette {
//! NOTE This is an adapter for the palette,
//! that to use the palette in the MU3 and MU4 build.
class IPaletteAdapter : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPaletteAdapter)
public:
    virtual ~IPaletteAdapter() = default;

    virtual QAction* getAction(const char* id) const = 0;
    virtual QString actionHelp(const char* id) const = 0;

    virtual void showMasterPalette(const QString&) = 0;
    virtual Ms::Score* currentScore() const = 0;
    virtual Ms::ScriptRecorder* getScriptRecorder() const = 0;

    // score view
    virtual void selectInstrument(Ms::InstrumentChange*) = 0;
    virtual bool editMode() const = 0;
    virtual Ms::ScoreState mscoreState() const = 0;
    virtual void changeState(Ms::ViewState s) = 0;
    virtual void cmdAddSlur(const Ms::Slur* slurTemplate = nullptr) = 0;
    virtual void applyDrop(Ms::Score* score, Ms::Element* target, Ms::Element* e, Qt::KeyboardModifiers modifiers,
                           QPointF pt = QPointF(), bool pasteMode = false) = 0;

    virtual void moveCursor() = 0;
    virtual void setFocus() = 0;
    virtual void setDropTarget(const Ms::Element*) = 0;
};
}
}
}

#endif // MU_PALETTE_IPALETTEADAPTER_H
