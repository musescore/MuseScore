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
      Element::ElementType ids[] = {
            Element::VOLTA,
            Element::OTTAVA,
            Element::TEXTLINE,
            Element::TRILL,
            Element::PEDAL,
            Element::HAIRPIN,
            Element::CLEF,
            Element::KEYSIG,
            Element::TIMESIG,
            Element::BAR_LINE,
            Element::ARPEGGIO,
            Element::BREATH,
            Element::GLISSANDO,
            Element::BRACKET,
            Element::ARTICULATION,
            Element::CHORDLINE,
            Element::ACCIDENTAL,
            Element::DYNAMIC,
            Element::TEXT,
            Element::INSTRUMENT_NAME,
            Element::STAFF_TEXT,
            Element::REHEARSAL_MARK,
            Element::INSTRUMENT_CHANGE,
            Element::NOTEHEAD,
            Element::NOTEDOT,
            Element::TREMOLO,
            Element::LAYOUT_BREAK,
            Element::MARKER,
            Element::JUMP,
            Element::REPEAT_MEASURE,
            Element::ICON,
            Element::NOTE,
            Element::SYMBOL,
            Element::FSYMBOL,
            Element::CHORD,
            Element::REST,
            Element::SPACER,
            Element::STAFF_STATE,
            Element::TEMPO_TEXT,
            Element::HARMONY,
            Element::FRET_DIAGRAM,
            Element::BEND,
            Element::TREMOLOBAR,
            Element::LYRICS,
            Element::FIGURED_BASS,
            Element::STEM,
            Element::SLUR,
            Element::ACCIDENTAL_BRACKET,
            Element::FINGERING,
            Element::HBOX,
            Element::VBOX,
            Element::TBOX,
            Element::FBOX,
            Element::MEASURE,
            Element::TAB_DURATION_SYMBOL,
            Element::OSSIA,
            Element::INVALID
            };

      for (int i = 0; ids[i] != Element::INVALID; ++i) {
            Element::ElementType t = ids[i];
            Element* e = Element::create(t, score);
            Element* ee = writeReadElement(e);
            QCOMPARE(e->type(), ee->type());
            delete e;
            delete ee;
            }
      }

QTEST_MAIN(TestElement)

#include "tst_element.moc"

