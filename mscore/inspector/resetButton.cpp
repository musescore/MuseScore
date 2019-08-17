//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "resetButton.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   ResetButton
//---------------------------------------------------------

ResetButton::ResetButton(QWidget* parent)
   : QWidget(parent)
      {
      reset = new QPushButton(this);
      reset->setToolTip(tr("Reset to style default"));
      reset->setIcon(*icons[int(Icons::reset_ICON)]);
      reset->setMinimumSize(QSize(24,24));
      reset->setMaximumSize(QSize(24,24));
      reset->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      reset->setFlat(true);

      setStyle = new QPushButton(this);
      setStyle->setText(tr("S", "set as style"));
      setStyle->setToolTip(tr("Set as style"));
      setStyle->setMinimumSize(QSize(24,24));
      setStyle->setMaximumSize(QSize(24,24));
      setStyle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      setStyle->setFlat(true);

      QHBoxLayout* l = new QHBoxLayout;
      l->setSpacing(0);
      l->setContentsMargins(0, 0, 0, 0);
      l->addWidget(reset, 0, Qt::AlignLeft);
      l->addWidget(setStyle, 0, Qt::AlignLeft);
      setLayout(l);
      setStyle->hide();
      show();

      connect(reset, SIGNAL(clicked()), SIGNAL(resetClicked()));
      connect(setStyle, SIGNAL(clicked()), SIGNAL(setStyleClicked()));
      }


//---------------------------------------------------------
//   enableSetStyle
//---------------------------------------------------------

void ResetButton::enableSetStyle(bool val)
      {
      setStyle->setVisible(val);
      }

} // namespace

