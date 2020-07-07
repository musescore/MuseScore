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
#ifndef MU_PALETTE_MU4PALETTEADAPTER_H
#define MU_PALETTE_MU4PALETTEADAPTER_H

#include "../ipaletteadapter.h"

namespace mu {
namespace scene {
namespace palette {
class MU4PaletteAdapter : public IPaletteAdapter
{
public:
    MU4PaletteAdapter() = default;

    QAction* getAction(const char* id) const override;
    QString actionHelp(const char* id) const override;

    void showMasterPalette(const QString&) override;
    Ms::Score* currentScore() const override;
    Ms::ScriptRecorder* getScriptRecorder() const override;

    // score view
    void selectInstrument(Ms::InstrumentChange*) override;
    bool editMode() const override;
    Ms::ScoreState mscoreState() const override;
    void changeState(Ms::ViewState s) override;
    void cmdAddSlur(const Ms::Slur* slurTemplate = nullptr) override;
    void applyDrop(Ms::Score* score, Ms::Element* target, Ms::Element* e, Qt::KeyboardModifiers modifiers,
                   QPointF pt = QPointF(), bool pasteMode = false) override;

    void moveCursor() override;
    void setFocus() override;
    void setDropTarget(const Ms::Element*) override;
};
}
}
}

#endif // MU_PALETTE_MU4PALETTEADAPTER_H
