//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef OPENFILELOCATION_H
#define OPENFILELOCATION_H

namespace Ms {
/// A class with static functions to open a file location
class OpenFileLocation
{
    Q_DECLARE_TR_FUNCTIONS(Ms::OpenFileLocation);

public:
    static bool openFileLocation(const QString& path);
    static const QString platformText();
};
} // namespace Ms

#endif // OPENFILELOCATION_H
