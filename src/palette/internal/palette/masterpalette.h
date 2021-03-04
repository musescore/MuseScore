//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MASTERPALETTE_H__
#define __MASTERPALETTE_H__

#include <QDialog>

#include "ui_masterpalette.h"

#include "modularity/ioc.h"
#include "framework/ui/imainwindow.h"
#include "ui/view/widgetdialog.h"

namespace Ms {
class Palette;
class TimeDialog;
class KeyEditor;

//---------------------------------------------------------
//   MasterPalette
//---------------------------------------------------------

class MasterPalette : public mu::ui::WidgetDialog, Ui::MasterPalette
{
    Q_OBJECT

    INJECT(palette, mu::ui::IMainWindow, mainWindow)

    Q_PROPERTY(QString selectedPaletteName READ selectedPaletteName WRITE setSelectedPaletteName NOTIFY selectedPaletteNameChanged)

public:
    MasterPalette(QWidget* parent = nullptr);

    static int static_metaTypeId();
    int metaTypeId() const override;

    QString selectedPaletteName() const;

public slots:
    void setSelectedPaletteName(const QString& name);

signals:
    void selectedPaletteNameChanged(QString name);

private slots:
    void currentChanged(QTreeWidgetItem*, QTreeWidgetItem*);
    void clicked(QTreeWidgetItem*, int);

    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    Palette* createPalette(int w, int h, bool grid, double mag = 1.0);
    void addPalette(Palette* sp);
    void retranslate(bool firstTime = false);

    TimeDialog* m_timeDialog = nullptr;
    KeyEditor* m_keyEditor = nullptr;
    QTreeWidgetItem* m_keyItem = nullptr;
    QTreeWidgetItem* m_timeItem = nullptr;
    QTreeWidgetItem* m_symbolItem = nullptr;

    int m_idxAllSymbols = -1;
};
} // namespace Ms

#endif
