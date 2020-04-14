//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PLUGIN_API_SELECTION_H__
#define __PLUGIN_API_SELECTION_H__

#include "elements.h"
#include "score.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   Selection
//    Wrapper class for internal Ms::Selection
///  \since MuseScore 3.3 
//---------------------------------------------------------

class Selection : public QObject {
      Q_OBJECT
      /// Current GUI selections for the score.
      /// \since MuseScore 3.3
      Q_PROPERTY(QQmlListProperty<Ms::PluginAPI::Element> elements READ elements)

      /// \cond MS_INTERNAL

   protected:
      Ms::Selection* _select;

   public:

      Selection(Ms::Selection* select) : QObject(), _select(select) {}
      virtual ~Selection() { }

      QQmlListProperty<Element> elements()
            { return wrapContainerProperty<Element>(this, _select->elements()); }

      /// \endcond
};

extern Selection* selectionWrap(Ms::Selection* select);

} // namespace PluginAPI
} // namespace Ms
#endif
