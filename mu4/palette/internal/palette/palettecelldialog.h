//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
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

#ifndef __PALETTECELLDIALOGS_H__
#define __PALETTECELLDIALOGS_H__

namespace Ui {
class PaletteCellProperties;
}

namespace Ms {
struct PaletteCell;

//---------------------------------------------------------
//   PaletteCellProperties
//---------------------------------------------------------

class PaletteCellPropertiesDialog : public QDialog
{
    Q_OBJECT

    Ui::PaletteCellProperties* ui;
    PaletteCell* cell;

    virtual void hideEvent(QHideEvent*);
    virtual void reject();

    void applyInitialPropertiesToThePalette();
    bool areInitialPropertiesChanged() const;
    void fillControlsWithData();
    void setInitialProperties();

    bool isDrawStaffCheckBoxChanged = false;
    bool isNameChanged = false;
    bool isXOffsetChanged = false;
    bool isYOffsetChanged = false;
    bool isScaleChanged = false;

    int drawStaffCheckboxInitialState = 0;
    QString initialName;
    QString initialTranslatedName;
    double initialXOffset = 0.f;
    double initialYOffset = 0.f;
    double initialScale = 0.f;
    bool initialCustomState = false;

public:
    PaletteCellPropertiesDialog(PaletteCell* p, QWidget* parent = 0);
    ~PaletteCellPropertiesDialog();

    void drawStaffCheckBoxChanged(int state);
    void nameChanged(const QString& text);
    void xOffsetChanged(double xOffset);
    void yOffsetChanged(double yOffset);
    void scaleChanged(double scale);

signals:
    void changed();
};
} // namespace Ms

#endif //__PALETTECELLDIALOGS_H__
