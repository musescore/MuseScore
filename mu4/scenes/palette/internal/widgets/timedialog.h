//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __TIMEDIALOG_H__
#define __TIMEDIALOG_H__

#include "ui_timedialog.h"
#include "libmscore/fraction.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace Ms {
class Palette;
class PaletteScrollArea;
class TimeSig;
class Score;
class Chord;

//---------------------------------------------------------
//   TimeDialog
//---------------------------------------------------------

class TimeDialog : public QWidget, Ui::TimeDialogBase
{
    Q_OBJECT

    INJECT(palette, mu::framework::IGlobalConfiguration, globalConfiguration)

    PaletteScrollArea* _timePalette;
    Palette* sp;
    bool _dirty;

    int denominator() const;
    int denominator2Idx(int) const;

private slots:
    void addClicked();
    void zChanged(int);
    void nChanged(int);
    void paletteChanged(int idx);
    void textChanged();
    void setDirty() { _dirty = true; }

signals:
    void timeSigAdded(const TimeSig*);

public:
    TimeDialog(QWidget* parent = 0);
    bool dirty() const { return _dirty; }
    void showTimePalette(bool val);
    void save();
};
}

#endif
