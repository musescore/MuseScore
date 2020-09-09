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

#ifndef __PLUGIN_API_TIE_H__
#define __PLUGIN_API_TIE_H__

#include "elements.h"
#include "libmscore/tie.h"

namespace Ms {
namespace PluginAPI {
//---------------------------------------------------------
//   Tie
///  Provides access to internal Ms::Tie objects.
///  \since MuseScore 3.3
//---------------------------------------------------------

class Tie : public Element
{
    Q_OBJECT
    /// The starting note of the tie.
    /// \since MuseScore 3.3
    Q_PROPERTY(Ms::PluginAPI::Note* startNote READ startNote)
    /// The ending note of the tie.
    /// \since MuseScore 3.3
    Q_PROPERTY(Ms::PluginAPI::Note* endNote READ endNote)

    /// \cond MS_INTERNAL

public:
    Tie(Ms::Tie* tie, Ownership own = Ownership::PLUGIN)
        : Element(tie, own) {}

    Note* startNote();
    Note* endNote();

    /// \endcond
};

extern Tie* tieWrap(Ms::Tie* tie);
} // namespace PluginAPI
} // namespace Ms
#endif
