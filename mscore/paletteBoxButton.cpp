//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "paletteBoxButton.h"
#include "palette.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   PaletteBoxButton
//---------------------------------------------------------

PaletteBoxButton::PaletteBoxButton(Palette* p, QWidget* parent)
   : QToolButton(parent)
      {
      palette = p;
      editAction = 0;

      setCheckable(true);
      setFocusPolicy(Qt::NoFocus);
      connect(this, SIGNAL(clicked(bool)), this, SLOT(showPalette(bool)));
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      setText(qApp->translate("Palette", palette->name().toUtf8()));
      setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
      setArrowType(Qt::RightArrow);
      showPalette(false);
      setObjectName("palette");
      }

//---------------------------------------------------------
//   contextMenuEvent
//---------------------------------------------------------

void PaletteBoxButton::contextMenuEvent(QContextMenuEvent* event)
      {
      QMenu menu;

      QAction* actionProperties = menu.addAction(tr("Palette Properties..."));
      QAction* actionInsert     = menu.addAction(tr("Insert New Palette..."));
      QAction* actionUp         = menu.addAction(tr("Move Palette Up"));
      QAction* actionDown       = menu.addAction(tr("Move Palette Down"));
      QAction* actionEdit       = menu.addAction(tr("Enable Editing"));
      actionEdit->setCheckable(true);
      actionEdit->setChecked(!palette->readOnly());
      if (palette->isFilterActive())
            actionEdit->setVisible(false);

      bool _systemPalette = palette->systemPalette();
      actionProperties->setDisabled(_systemPalette);
      actionInsert->setDisabled(_systemPalette);
      actionUp->setDisabled(_systemPalette);
      actionDown->setDisabled(_systemPalette);
      actionEdit->setDisabled(_systemPalette);

      menu.addSeparator();
      QAction* actionSave = menu.addAction(tr("Save Palette..."));
      QAction* actionLoad = menu.addAction(tr("Load Palette..."));
      actionLoad->setDisabled(_systemPalette);

      menu.addSeparator();
      QAction* actionDelete = menu.addAction(tr("Delete Palette"));
      actionDelete->setDisabled(_systemPalette);

      QAction* action = menu.exec(mapToGlobal(event->pos()));
      if (action == actionProperties)
            propertiesTriggered();
      else if (action == actionInsert)
            newTriggered();
      else if (action == actionUp)
            upTriggered();
      else if (action == actionDown)
            downTriggered();
      else if (action == actionEdit)
            enableEditing(action->isChecked());
      else if (action == actionSave)
            saveTriggered();
      else if (action == actionLoad)
            loadTriggered();
      else if (action == actionDelete)
            deleteTriggered();
      }

//---------------------------------------------------------
//   enableEditing
//---------------------------------------------------------

void PaletteBoxButton::enableEditing(bool val)
      {
      palette->setReadOnly(!val);
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void PaletteBoxButton::changeEvent(QEvent* ev)
      {
      QToolButton::changeEvent(ev);
      if (ev->type() == QEvent::FontChange)
            setFixedHeight(QFontMetrics(font()).height() + 2);
      }

//---------------------------------------------------------
//   showPalette
//---------------------------------------------------------

void PaletteBoxButton::showPalette(bool visible)
      {
      if (visible && preferences.getBool(PREF_APP_USESINGLEPALETTE)) {
            // close all palettes
            emit closeAll();
            }
      palette->setVisible(visible);
      setChecked(visible);
      setArrowType(visible ? Qt::DownArrow : Qt::RightArrow );
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize PaletteBoxButton::sizeHint() const
      {
      QFontMetrics fm(font());
      return QSize(20, fm.lineSpacing() + 6);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void PaletteBoxButton::paintEvent(QPaintEvent*)
      {
      //remove automatic menu arrow
      QStylePainter p(this);

      QStyleOptionToolButton opt;
      initStyleOption(&opt);
      opt.features &= (~QStyleOptionToolButton::HasMenu);
      p.drawComplexControl(QStyle::CC_ToolButton, opt);
      }
}

