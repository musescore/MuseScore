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
      reset    = new QToolButton(this);
      reset->setToolTip(tr("Reset to style default"));
      reset->setIcon(*icons[int(Icons::reset_ICON)]);
      reset->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

      setStyle = new QToolButton(this);
      setStyle->setText(tr("S"));
      setStyle->setToolTip(tr("Set as style"));
      setStyle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

      QHBoxLayout* l = new QHBoxLayout;
      l->setSpacing(0);
      l->setContentsMargins(0, 0, 0, 0);
      l->addWidget(reset);
      l->addWidget(setStyle);
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

