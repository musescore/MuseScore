//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================
#ifndef MU_NOTATIONSCENE_FONTSTYLESELECT_H
#define MU_NOTATIONSCENE_FONTSTYLESELECT_H

#include "ui_font_style_select.h"

namespace Ms {
enum class Align : char;
enum class FontStyle : char;

//---------------------------------------------------------
//   FontStyleSelect
//---------------------------------------------------------

class FontStyleSelect : public QWidget, public Ui::FontStyleSelect
{
    Q_OBJECT

private slots:
    void _fontStyleChanged();

signals:
    void fontStyleChanged(FontStyle);

public:
    FontStyleSelect(QWidget* parent);
    FontStyle fontStyle() const;
    void setFontStyle(FontStyle);
};
}

#endif // MU_NOTATIONSCENE_FONTSTYLESELECT_H
