//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorFingering.h"
#include "musescore.h"
#include "libmscore/fingering.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorFingering
//---------------------------------------------------------

InspectorFingering::InspectorFingering(QWidget* parent)
   : InspectorStaffText(parent)
      {
      }

//---------------------------------------------------------
//   allowedTextStyles
//---------------------------------------------------------

const std::vector<Tid>& InspectorFingering::allowedTextStyles()
      {
      static const std::vector<Tid> _fingeringTextStyles = {
            Tid::FINGERING,
            Tid::LH_GUITAR_FINGERING,
            Tid::RH_GUITAR_FINGERING,
            Tid::STRING_NUMBER,
            Tid::USER1,
            Tid::USER2,
            Tid::USER3,
            Tid::USER4,
            Tid::USER5,
            Tid::USER6
            };

      return _fingeringTextStyles;
      }
}

