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
            Element::ElementType::VOLTA,
            Element::ElementType::OTTAVA,
            Element::ElementType::TEXTLINE,
            Element::ElementType::TRILL,
            Element::ElementType::PEDAL,
            Element::ElementType::HAIRPIN,
            Element::ElementType::CLEF,
            Element::ElementType::KEYSIG,
            Element::ElementType::TIMESIG,
            Element::ElementType::BAR_LINE,
            Element::ElementType::ARPEGGIO,
            Element::ElementType::BREATH,
            Element::ElementType::GLISSANDO,
            Element::ElementType::BRACKET,
            Element::ElementType::ARTICULATION,
            Element::ElementType::CHORDLINE,
            Element::ElementType::ACCIDENTAL,
            Element::ElementType::DYNAMIC,
            Element::ElementType::TEXT,
            Element::ElementType::INSTRUMENT_NAME,
            Element::ElementType::STAFF_TEXT,
            Element::ElementType::REHEARSAL_MARK,
            Element::ElementType::INSTRUMENT_CHANGE,
            Element::ElementType::NOTEHEAD,
            Element::ElementType::NOTEDOT,
            Element::ElementType::TREMOLO,
            Element::ElementType::LAYOUT_BREAK,
            Element::ElementType::MARKER,
            Element::ElementType::JUMP,
            Element::ElementType::REPEAT_MEASURE,
            Element::ElementType::ICON,
            Element::ElementType::NOTE,
            Element::ElementType::SYMBOL,
            Element::ElementType::FSYMBOL,
            Element::ElementType::CHORD,
            Element::ElementType::REST,
            Element::ElementType::SPACER,
            Element::ElementType::STAFF_STATE,
            Element::ElementType::TEMPO_TEXT,
            Element::ElementType::HARMONY,
            Element::ElementType::FRET_DIAGRAM,
            Element::ElementType::BEND,
            Element::ElementType::TREMOLOBAR,
            Element::ElementType::LYRICS,
            Element::ElementType::FIGURED_BASS,
            Element::ElementType::STEM,
            Element::ElementType::SLUR,
            Element::ElementType::ACCIDENTAL_BRACKET,
            Element::ElementType::FINGERING,
            Element::ElementType::HBOX,
            Element::ElementType::VBOX,
            Element::ElementType::TBOX,
            Element::ElementType::FBOX,
            Element::ElementType::MEASURE,
            Element::ElementType::TAB_DURATION_SYMBOL,
            Element::ElementType::OSSIA,
            Element::ElementType::INVALID
            };

      for (int i = 0; ids[i] != Element::ElementType::INVALID; ++i) {
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

