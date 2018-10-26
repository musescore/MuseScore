//=============================================================================
//  BWW to MusicXML converter
//  Part of MusE Score
//  Linux Music Score Editor
//  $Id: lexer.h 4873 2011-10-19 19:33:04Z lvinken $
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef BWW2MXML_LEXER_H
#define BWW2MXML_LEXER_H

/**
 \file
 Definition of class Lexer
 */

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include "symbols.h"

class QIODevice;

namespace Bww {

  /**
   The bww lexer.
   */

  class Lexer
  {
  public:
    Lexer(QIODevice *inDevice);
    void getSym();
    Symbol symType() const;
    QString symValue() const;
    int symLineNumber() const { return lineNumber; }
  private:
    void categorizeWord(QString word);
    QTextStream in;                     ///< Input stream
    QString line;                       ///< The current line
    int lineNumber;                     ///< The current line number (zero-based)
    QStringList list;                   ///< Unprocessed words
    Symbol type;                        ///< Last symbol type
    QString value;                      ///< Last symbol value
    QMap<QString, QString> graceMap;    ///< Map bww embellishments to separate grace notes
  };

} // namespace Bww

#endif // BWW2MXML_LEXER_H
