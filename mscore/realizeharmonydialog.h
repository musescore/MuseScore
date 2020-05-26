//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#ifndef __REALIZEHARMONYDIALOG_H__
#define __REALIZEHARMONYDIALOG_H__

#include "ui_realizeharmonydialog.h"

namespace Ms {
class Harmony;

class RealizeHarmonyDialog : public QDialog, Ui::RealizeHarmonyDialogBase
{
    Q_OBJECT

private slots:
    void toggleChordTable();
public:
    RealizeHarmonyDialog(QWidget* parent = 0);
    void setChordList(QList<Harmony*>);

    bool getLiteral() { return voicingSelect->getLiteral(); }
    int getVoicing() { return voicingSelect->getVoicing(); }
    int getDuration() { return voicingSelect->getDuration(); }
    bool optionsOverride() { return optionsBox->isChecked(); }
};
}

#endif // REALIZEHARMONYDIALOG_H
