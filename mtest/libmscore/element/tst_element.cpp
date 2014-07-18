//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>

#include "libmscore/score.h"
#include "libmscore/element.h"
#include "mtest/testutils.h"

using namespace Ms;

//---------------------------------------------------------
//   TestElement
//---------------------------------------------------------

class TestElement : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase() { initMTest(); }
      void testIds();
      };

//---------------------------------------------------------
//   testIds
//---------------------------------------------------------

void TestElement::testIds()
      {
      Element::Type ids[] = {
            Element::Type::VOLTA,
            Element::Type::OTTAVA,
            Element::Type::TEXTLINE,
            Element::Type::TRILL,
            Element::Type::PEDAL,
            Element::Type::HAIRPIN,
            Element::Type::CLEF,
            Element::Type::KEYSIG,
            Element::Type::TIMESIG,
            Element::Type::BAR_LINE,
            Element::Type::ARPEGGIO,
            Element::Type::BREATH,
            Element::Type::GLISSANDO,
            Element::Type::BRACKET,
            Element::Type::ARTICULATION,
            Element::Type::CHORDLINE,
            Element::Type::ACCIDENTAL,
            Element::Type::DYNAMIC,
            Element::Type::TEXT,
            Element::Type::INSTRUMENT_NAME,
            Element::Type::STAFF_TEXT,
            Element::Type::REHEARSAL_MARK,
            Element::Type::INSTRUMENT_CHANGE,
            Element::Type::NOTEHEAD,
            Element::Type::NOTEDOT,
            Element::Type::TREMOLO,
            Element::Type::LAYOUT_BREAK,
            Element::Type::MARKER,
            Element::Type::JUMP,
            Element::Type::REPEAT_MEASURE,
            Element::Type::ICON,
            Element::Type::NOTE,
            Element::Type::SYMBOL,
            Element::Type::FSYMBOL,
            Element::Type::CHORD,
            Element::Type::REST,
            Element::Type::SPACER,
            Element::Type::STAFF_STATE,
            Element::Type::TEMPO_TEXT,
            Element::Type::HARMONY,
            Element::Type::FRET_DIAGRAM,
            Element::Type::BEND,
            Element::Type::TREMOLOBAR,
            Element::Type::LYRICS,
            Element::Type::FIGURED_BASS,
            Element::Type::STEM,
            Element::Type::SLUR,
            Element::Type::FINGERING,
            Element::Type::HBOX,
            Element::Type::VBOX,
            Element::Type::TBOX,
            Element::Type::FBOX,
            Element::Type::MEASURE,
            Element::Type::TAB_DURATION_SYMBOL,
            Element::Type::OSSIA,
            Element::Type::INVALID
            };

      for (int i = 0; ids[i] != Element::Type::INVALID; ++i) {
            Element::Type t = ids[i];
            Element* e = Element::create(t, score);
            Element* ee = writeReadElement(e);
            QCOMPARE(e->type(), ee->type());
            delete e;
            delete ee;
            }
      }

QTEST_MAIN(TestElement)

#include "tst_element.moc"

