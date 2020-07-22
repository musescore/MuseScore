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
#ifndef MU_NOTATIONSCENE_ALIGNSELECT_H
#define MU_NOTATIONSCENE_ALIGNSELECT_H

#include "ui_align_select.h"

namespace Ms {
enum class Align : char;
}

namespace mu {
namespace scene {
namespace notation {

//---------------------------------------------------------
//   AlignSelect
//---------------------------------------------------------

class AlignSelect : public QWidget, public Ui::AlignSelect
{
    Q_OBJECT

    QButtonGroup * g1;
    QButtonGroup* g2;

    void blockAlign(bool val);

private slots:
    void _alignChanged();

signals:
    void alignChanged(Align);

public:
    AlignSelect(QWidget* parent);
    Align align() const;
    void setAlign(Align);
};
}
}
}

#endif // MU_NOTATIONSCENE_ALIGNSELECT_H
