//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#ifndef __PALETTEDIALOGS_H__
#define __PALETTEDIALOGS_H__

namespace Ui {
class PaletteProperties;
}

namespace Ms {
class PalettePanel;

//---------------------------------------------------------
//   PalettePropertiesDialog
//---------------------------------------------------------

class PalettePropertiesDialog : public QDialog
{
    Q_OBJECT

    Ui::PaletteProperties* ui;
    PalettePanel* palette;

    void fillControlsWithData();

    virtual void hideEvent(QHideEvent*);
    virtual void reject();

    void setInitialProperties();
    void applyInitialPropertiesToThePalette();
    bool areInitialPropertiesChanged() const;

    bool isGridCheckBoxChanged = false;
    bool isNameChanged = false;
    bool isHeightChanged = false;
    bool isWidthChanged = false;
    bool isScaleChanged = false;
    bool isOffsetChanged = false;

    int gridCheckboxInitialState = 0;
    QString initialName;
    QString initialTranslatedName;
    int initialWidth = 0;
    int initialHeight = 0;
    double initialOffset = 0.f;
    double initialScale = 0.f;

public:
    PalettePropertiesDialog(PalettePanel*, QWidget* parent = nullptr);
    ~PalettePropertiesDialog();

    /*
    /  checkBoxChanged(int state)
    /      The @state values are Qt::Unchecked and Qt::Checked
    */
    void gridCheckBoxChanged(int state);

    void nameChanged(const QString& text);
    void heightChanged(int height);
    void widthChanged(int width);
    void offsetChanged(double offset);
    void scaleChanged(double scale);

signals:
    void changed();
};
} // namespace Ms

#endif
