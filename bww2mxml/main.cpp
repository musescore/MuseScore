//=============================================================================
//  BWW to MusicXML converter
//  Part of MusE Score
//  Linux Music Score Editor
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

// #include <iostream>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QStringList>

#include "lexer.h"
#include "parser.h"
#include "mxmlwriter.h"

void usage()
{
  //WS std::cerr << "usage bww2mxml [infile [outfile]]" << std::endl;
}

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  QStringList args = QCoreApplication::arguments();

  QFile inFile;
  QFile outFile;

  if (args.size() > 3)
  {
    usage();
    return 1;
  }

  if (args.size() <= 1)
    inFile.open(stdin, QFile::ReadOnly);
  else
  {
    inFile.setFileName(args.at(1));
    if (!inFile.open(QIODevice::ReadOnly))
    {
//WS      std::cerr << "Cannot open file '" << qPrintable(args.at(1)) << "' for reading: "
//          << qPrintable(inFile.errorString()) << std::endl;
      return 2;
    }
  }

  if (args.size() < 3)
    outFile.open(stdout, QFile::WriteOnly);
  else
  {
    outFile.setFileName(args.at(2));
    if (!outFile.open(QIODevice::WriteOnly))
    {
//      std::cerr << "Cannot open file '" << qPrintable(args.at(2)) << "' for writing: "
//          << qPrintable(outFile.errorString()) << std::endl;
      return 2;
    }
  }

  Bww::Lexer lex(&inFile);
  Bww::MxmlWriter wrt;
  wrt.setOutDevice(&outFile);
  Bww::Parser p(lex, wrt);
  p.parse();

  return 0;
}
