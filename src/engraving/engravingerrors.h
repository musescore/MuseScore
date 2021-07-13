/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_ENGRAVING_ENGRAVINGERRORS_H
#define MU_ENGRAVING_ENGRAVINGERRORS_H

namespace mu::engraving {
enum class Err {
    Undefined       = -1,
    NoError         = 0,
    UnknownError    = 2000,

    // file
    FileUnknownError = 2001,
    FileNotFound = 2002,
    FileOpenFailed = 2003,
    FileBadFormat = 2004,
    FileUnknownType = 2005,
    FileTooOld = 2006,
    FileTooNew = 2007,
    FileOld300Format = 2008,
    FileCorrupted = 2009,
    FileCriticallyCorrupated = 2010,
    FileUserAbort = 2011,
    FileIgnoreError = 2012
};
}

#endif // MU_ENGRAVING_ENGRAVINGERRORS_H
