#=============================================================================
#  BWW to MusicXML converter
#  Part of MusE Score
#  Linux Music Score Editor
#  $Id: bww2mxml.pro 3115 2010-05-30 14:17:03Z lvinken $
#
#  Copyright (C) 2002-2010 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

QT -= gui
TARGET = bww2mxml
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += lexer.cpp \
    main.cpp \
    mxmlwriter.cpp \
    parser.cpp \
    symbols.cpp \
    writer.cpp
HEADERS += lexer.h \
    mxmlwriter.h \
    parser.h \
    symbols.h \
    writer.h
