//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "edittools.h"
#include "musescore.h"
#include "scoreview.h"

//---------------------------------------------------------
//   editTools
//---------------------------------------------------------

EditTools* MuseScore::editTools()
      {
      if (!_editTools) {
            _editTools = new EditTools(this);
            addDockWidget(Qt::BottomDockWidgetArea, _editTools);
            }
      setFocusPolicy(Qt::NoFocus);
      return _editTools;
      }

//---------------------------------------------------------
//   EditTools
//---------------------------------------------------------

EditTools::EditTools(QWidget* parent)
  : QDockWidget(parent)
      {
      setObjectName("edit-tools");
      setWindowTitle(tr("Edit Mode Tools"));
      setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

      QToolBar* tb = new QToolBar(tr("Edit Mode Tools"));

      QToolButton* b = new QToolButton(this);
      QAction* a = getAction("hraster");
      a->setCheckable(true);
      b->setDefaultAction(a);
      b->setContextMenuPolicy(Qt::ActionsContextMenu);
      b->addAction(getAction("config-raster"));
      tb->addWidget(b);

      b = new QToolButton(this);
      a = getAction("vraster");
      a->setCheckable(true);
      b->setDefaultAction(a);
      b->setContextMenuPolicy(Qt::ActionsContextMenu);
      b->addAction(getAction("config-raster"));
      tb->addWidget(b);

      _editX  = new QDoubleSpinBox(this);
      _editX->setSuffix(tr("sp"));
      _editX->setRange(-99999, 99999);
      _editX->setSingleStep(.1);
      _editY  = new QDoubleSpinBox(this);
      _editY->setSuffix(tr("sp"));
      _editY->setRange(-99999, 99999);
      _editY->setSingleStep(.1);
      xLabel = new QLabel(tr("x:"), this);
      yLabel = new QLabel(tr("y:"), this);
      tb->addWidget(xLabel);
      tb->addWidget(_editX);
      tb->addWidget(yLabel);
      tb->addWidget(_editY);

      connect(_editX, SIGNAL(valueChanged(double)), SLOT(editXChanged(double)));
      connect(_editY, SIGNAL(valueChanged(double)), SLOT(editYChanged(double)));

      setWidget(tb);
      QWidget* w = new QWidget(this);
      setTitleBarWidget(w);
      titleBarWidget()->hide();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void EditTools::setElement(Element* e)
      {
      _element = e;
      }

//---------------------------------------------------------
//   updateTools
//---------------------------------------------------------

void EditTools::updateTools()
      {
      }

//---------------------------------------------------------
//   editXChanged
//---------------------------------------------------------

void EditTools::editXChanged(double val)
      {
      if (mscore->currentScoreView())
            mscore->currentScoreView()->setEditPos(QPointF(val, _editY->value()));
      }

//---------------------------------------------------------
//   editYChanged
//---------------------------------------------------------

void EditTools::editYChanged(double val)
      {
      if (mscore->currentScoreView())
            mscore->currentScoreView()->setEditPos(QPointF(_editX->value(), val));
      }

//---------------------------------------------------------
//   setEditX
//---------------------------------------------------------

void EditTools::setEditPos(const QPointF& pt)
      {
      _editX->blockSignals(true);
      _editY->blockSignals(true);
      _editX->setValue(pt.x());
      _editY->setValue(pt.y());
      _editX->blockSignals(false);
      _editY->blockSignals(false);
      }


