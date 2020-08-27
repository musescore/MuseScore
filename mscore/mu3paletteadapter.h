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
#ifndef MU3PALETTEGETACTION_H
#define MU3PALETTEGETACTION_H

#include "mu4/palette/ipaletteadapter.h"

namespace Ms {
class MU3PaletteAdapter : public mu::palette::IPaletteAdapter
{
public:
    MU3PaletteAdapter();

    QAction* getAction(const char* id) const override;
    QString actionHelp(const char* id) const override;

    void showMasterPalette(const QString&) override;

    // score
    bool isSelected() const override;
    bool applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers = {}) override;

    // qml
    Ms::PaletteWorkspace* paletteWorkspace() const override;
    mu::ValCh<bool> paletteEnabled() const override;
    void setPaletteEnabled(bool arg) override;
    void requestPaletteSearch() override;
    mu::async::Notification paletteSearchRequested() const override;
    void notifyElementDraggedToScoreView() override;
    mu::async::Notification elementDraggedToScoreView() const override;

private:

    void selectInstrument(Ms::InstrumentChange*);
    bool editMode() const;
    Ms::ScoreState mscoreState() const;
    void changeState(Ms::ViewState s);
    void cmdAddSlur(const Ms::Slur* slurTemplate = nullptr);
    void applyDrop(Ms::Score* score, Ms::Element* target, Ms::Element* e, Qt::KeyboardModifiers modifiers,
                   QPointF pt = QPointF(), bool pasteMode = false);

    void moveCursor();
    void setFocus();
    void setDropTarget(const Ms::Element*);

    mu::ValCh<bool> m_paletteEnabled;
    mu::async::Notification m_paletteSearchRequested;
    mu::async::Notification m_elementDraggedToScoreView;
};
}

#endif // MU3PALETTEGETACTION_H
