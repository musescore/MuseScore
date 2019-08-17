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

#include "offsetSelect.h"
#include "libmscore/types.h"
#include "icons.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   OffsetSelect
//---------------------------------------------------------

OffsetSelect::OffsetSelect(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);

      showRaster(false);

      QAction* a = getAction("hraster");
      a->setCheckable(true);
      hRaster->setDefaultAction(a);
      hRaster->setContextMenuPolicy(Qt::ActionsContextMenu);
      hRaster->addAction(getAction("config-raster"));

      a = getAction("vraster");
      a->setCheckable(true);
      vRaster->setDefaultAction(a);
      vRaster->setContextMenuPolicy(Qt::ActionsContextMenu);
      vRaster->addAction(getAction("config-raster"));

      connect(xVal, SIGNAL(valueChanged(double)), SLOT(_offsetChanged()));
      connect(yVal, SIGNAL(valueChanged(double)), SLOT(_offsetChanged()));
      }

//---------------------------------------------------------
//   setSuffix
//---------------------------------------------------------

void OffsetSelect::setSuffix(const QString& s)
      {
      xVal->setSuffix(s);
      yVal->setSuffix(s);
      }

//---------------------------------------------------------
//   showRaster
//---------------------------------------------------------

void OffsetSelect::showRaster(bool v)
      {
      hRaster->setVisible(v);
      vRaster->setVisible(v);
      }

//---------------------------------------------------------
//   _offsetChanged
//---------------------------------------------------------

void OffsetSelect::_offsetChanged()
      {
      emit offsetChanged(QPointF(xVal->value(), yVal->value()));
      }

//---------------------------------------------------------
//   offset
//---------------------------------------------------------

QPointF OffsetSelect::offset() const
      {
      return QPointF(xVal->value(), yVal->value());
      }

//---------------------------------------------------------
//   blockOffset
//---------------------------------------------------------

void OffsetSelect::blockOffset(bool val)
      {
      xVal->blockSignals(val);
      yVal->blockSignals(val);
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void OffsetSelect::setOffset(const QPointF& o)
      {
      blockOffset(true);
      xVal->setValue(o.x());
      yVal->setValue(o.y());
      blockOffset(false);
      }

}


