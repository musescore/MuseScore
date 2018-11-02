//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
      ElementType ids[] = {
            ElementType::VOLTA,
            ElementType::OTTAVA,
            ElementType::TEXTLINE,
            ElementType::TRILL,
            ElementType::PEDAL,
            ElementType::HAIRPIN,
            ElementType::CLEF,
            ElementType::KEYSIG,
            ElementType::TIMESIG,
            ElementType::BAR_LINE,
            ElementType::ARPEGGIO,
            ElementType::BREATH,
            ElementType::GLISSANDO,
//            ElementType::BRACKET,
            ElementType::ARTICULATION,
            ElementType::CHORDLINE,
            ElementType::ACCIDENTAL,
            ElementType::DYNAMIC,
            ElementType::TEXT,
            ElementType::INSTRUMENT_NAME,
            ElementType::STAFF_TEXT,
            ElementType::REHEARSAL_MARK,
            ElementType::INSTRUMENT_CHANGE,
            ElementType::NOTEHEAD,
            ElementType::NOTEDOT,
            ElementType::TREMOLO,
            ElementType::LAYOUT_BREAK,
            ElementType::MARKER,
            ElementType::JUMP,
            ElementType::REPEAT_MEASURE,
            ElementType::ICON,
            ElementType::NOTE,
            ElementType::SYMBOL,
            ElementType::FSYMBOL,
            ElementType::CHORD,
            ElementType::REST,
            ElementType::SPACER,
            ElementType::STAFF_STATE,
            ElementType::TEMPO_TEXT,
            ElementType::HARMONY,
            ElementType::FRET_DIAGRAM,
            ElementType::BEND,
            ElementType::TREMOLOBAR,
            ElementType::LYRICS,
            ElementType::FIGURED_BASS,
            ElementType::STEM,
            ElementType::SLUR,
            ElementType::FINGERING,
            ElementType::HBOX,
            ElementType::VBOX,
            ElementType::TBOX,
            ElementType::FBOX,
            ElementType::MEASURE,
            ElementType::TAB_DURATION_SYMBOL,
            ElementType::OSSIA
            };

      for (ElementType t : ids) {
            Element* e = Element::create(t, score);
            Element* ee = writeReadElement(e);
            QCOMPARE(e->type(), ee->type());
            delete e;
            delete ee;
            }
      }

QTEST_MAIN(TestElement)

#include "tst_element.moc"

