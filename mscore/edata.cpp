//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "musescore.h"
#include "libmscore/dynamic.h"
#include "libmscore/marker.h"
#include "libmscore/timesig.h"
#include "libmscore/key.h"
#include "libmscore/keysig.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/clef.h"
#include "libmscore/hook.h"
#include "libmscore/xml.h"
#include "libmscore/accidental.h"
#include "libmscore/durationtype.h"

//##  write out extended metadata about every painted symbol

namespace Ms {

//---------------------------------------------------------
//   writeData
//---------------------------------------------------------

static void writeData(Xml& xml, qreal mag, const QString& name, const Element* e)
      {
      xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(10).arg(name));
      QRectF rr(e->pageBoundingRect());
      QRectF r(rr.x() * mag, rr.y() * mag, rr.width() * mag, rr.height() * mag);
      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(r.x(),      0, 'f', 3)
         .arg(r.y(),      0, 'f', 3)
         .arg(r.width(),  0, 'f', 3)
         .arg(r.height(), 0, 'f', 3);
      xml.etag();
      }

//---------------------------------------------------------
//   writeEdata
//---------------------------------------------------------

void MuseScore::writeEdata(const QString& edataName, const QString& imageName, Score* score,
   qreal mag, const QList<const Element*>& pel)
      {
      QFile f(edataName);
      if (!f.open(QIODevice::WriteOnly)) {
            MScore::lastError = tr("Open Element metadata xml file\n%1\nfailed: %2").arg(f.fileName().arg(QString(strerror(errno))));
            return;
            }
      Xml xml(&f);
      xml.header();
      xml.stag("Annotations version=\"1.0\"");
      xml.tag("Source", "MuseScore 2.1");

      for (const Element* e : pel) {
            if (e->type() == Element::Type::PAGE) {
                  xml.stag("Page");
                  QUrl url = QUrl::fromLocalFile(imageName);
                  xml.tag("Image", url.toDisplayString());
                  QRectF r(e->pageBoundingRect());
                  xml.tag("Size", QVariant(QSize(lrint(r.width() * mag), lrint(r.height() * mag))));
                  xml.etag();
                  break;
                  }
            }
      for (const Element* e : pel) {
            switch (e->type()) {
                  case Element::Type::NOTEDOT:
                        writeData(xml, mag, "augmentationDot", e);
                        break;
                  case Element::Type::STEM:
                        writeData(xml, mag, "stem", e);
                        break;
#if 0
                  case Element::Type::HAIRPIN_SEGMENT:
                        writeData(xml, mag, e->name(), e);
                        break;
#endif
                  case Element::Type::ARTICULATION: {
                        const Articulation* a = static_cast<const Articulation*>(e);
                        switch (a->articulationType()) {
                              case ArticulationType::Staccato:
                                    writeData(xml, mag, "articStaccatoAbove", a);
                                    break;
                              case ArticulationType::Tenuto:
                                    writeData(xml, mag, "articTenutoAbove", a);
                                    break;
                              case ArticulationType::Marcato:
                                    writeData(xml, mag, "articMarcatoAbove", a);
                                    break;
                              case ArticulationType::Staccatissimo:
                                    writeData(xml, mag, "articStaccatissimoAbove", a);
                                    break;
                              case ArticulationType::Sforzatoaccent:
                                    writeData(xml, mag, "articAccentAbove", a);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;
                  case Element::Type::CLEF: {
                        const Clef* c = static_cast<const Clef*>(e);
                        switch (c->clefType()) {
                              case ClefType::G:
                                    writeData(xml, mag, c->small() ? "gClefChange" : "gClef", c);
                                    break;
                              case ClefType::G3:
                                    writeData(xml, mag, c->small() ? "gClef8vbChange" : "gClef8vb", c);
                                    break;
                              case ClefType::G1:
                                    writeData(xml, mag, c->small() ? "gClef8vaChange" : "gClef8va", c);
                                    break;
                              case ClefType::G2:
                                    writeData(xml, mag, c->small() ? "gClef15maChange" : "gClef15ma", c);
                                    break;
                              case ClefType::F:
                                    writeData(xml, mag, c->small() ? "fClefChange" : "fClef", c);
                                    break;
                              case ClefType::F8:
                                    writeData(xml, mag, c->small() ? "fClef8vbChange" : "fClef8vb", c);
                                    break;
                              case ClefType::F_8VA:
                                    writeData(xml, mag, c->small() ? "fClef8vaChange" : "fClef8va", c);
                                    break;
                              case ClefType::F15:
                                    writeData(xml, mag, c->small() ? "fClef15mbChange" : "fClef15mb", c);
                                    break;
                              case ClefType::F_15MA:
                                    writeData(xml, mag, c->small() ? "fClef15maChange" : "fClef15ma", c);
                                    break;
                              case ClefType::C1:
                              case ClefType::C2:
                              case ClefType::C3:
                              case ClefType::C4:
                                    writeData(xml, mag, c->small() ? "cClefChange" : "cClef", c);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;

                  case Element::Type::NOTE: {
                        const Note* n = static_cast<const Note*>(e);
                        const Chord* c = n->chord();
                        bool small = c->isGrace() || c->small() || n->small();
                        switch (n->chord()->durationType().headType()) {
                              case NoteHead::Type::HEAD_WHOLE:
                                    writeData(xml, mag, small ? "noteheadWholeSmall" : "noteheadWhole", n);
                                    break;
                              case NoteHead::Type::HEAD_HALF:
                                    writeData(xml, mag, small ? "noteheadHalfSmall" : "noteheadHalf", n);
                                    break;
                              case NoteHead::Type::HEAD_QUARTER:
                                    writeData(xml, mag, small ? "noteheadBlackSmall" : "noteheadBlack", n);
                                    break;
                              case NoteHead::Type::HEAD_BREVIS:
                                    writeData(xml, mag, small ? "noteheadDoubleWholeSmall" : "noteheadDoubleWhole", n);
                              default:
                                    break;
                              }
                        }
                        break;

                  case Element::Type::ACCIDENTAL: {
                        const Accidental* a = static_cast<const Accidental*>(e);
                        switch (a->accidentalType()) {
                              case AccidentalType::SHARP:
                                    writeData(xml, mag, "accidentalSharp", a);
                                    break;
                              case AccidentalType::FLAT:
                                    writeData(xml, mag, "accidentalFlat", a);
                                    break;
                              case AccidentalType::SHARP2:
                                    writeData(xml, mag, "accidentalDoubleSharp", a);
                                    break;
                              case AccidentalType::FLAT2:
                                    writeData(xml, mag, "accidentalDoubleFlat", a);
                                    break;
                              case AccidentalType::NATURAL:
                                    writeData(xml, mag, "accidentalNatural", a);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;
#if 0
                  case Element::Type::KEYSIG: {
                        const KeySig* k = static_cast<const KeySig*>(e);
                        switch (k->key()) {
                              case Key::G_B:
                                    writeData(xml, mag, "KeyFlat6", k);
                                    break;
                              case Key::D_B:
                                    writeData(xml, mag, "KeyFlat5", k);
                                    break;
                              case Key::A_B:
                                    writeData(xml, mag, "KeyFlat4", k);
                                    break;
                              case Key::E_B:
                                    writeData(xml, mag, "KeyFlat3", k);
                                    break;
                              case Key::B_B:
                                    writeData(xml, mag, "KeyFlat2", k);
                                    break;
                              case Key::F:
                                    writeData(xml, mag, "KeyFlat1", k);
                                    break;
                              case Key::G:
                                    writeData(xml, mag, "KeySharp1", k);
                                    break;
                              case Key::D:
                                    writeData(xml, mag, "KeySharp2", k);
                                    break;
                              case Key::A:
                                    writeData(xml, mag, "KeySharp3", k);
                                    break;
                              case Key::E:
                                    writeData(xml, mag, "KeySharp4", k);
                                    break;
                              case Key::B:
                                    writeData(xml, mag, "KeySharp5", k);
                                    break;
                              case Key::F_S:
                                    writeData(xml, mag, "KeySharp6", k);
                                    break;
                              case Key::C_S:
                                    writeData(xml, mag, "KeySharp7", k);
                                    break;
                              default:
                                    break;
                              }
                        };
                        break;
#endif

                  case Element::Type::HOOK: {
                        const Hook* h = static_cast<const Hook*>(e);
                        const Chord* c = h->chord();

                        if (c->isGrace()) {
                              if (h->hookType() == 1) {
                                    if (c->slash())
                                          writeData(xml, mag, "Flag1UpSmallSlash", h);
                                    else
                                          writeData(xml, mag, "Flag1UpSmall", h);
                                    }
                              break;
                              }
                        switch (h->hookType()) {
                              default:
                              case 0:
                                    break;
                              case -1:
                                    writeData(xml, mag, "flag8thUp", h);
                                    break;
                              case -2:
                                    writeData(xml, mag, "flag16thUp", h);
                                    break;
                              case -3:
                                    writeData(xml, mag, "flag32ndUp", h);
                                    break;
                              case -4:
                                    writeData(xml, mag, "flag64tjUp", h);
                                    break;
                              case -5:
                                    writeData(xml, mag, "flag128thUp", h);
                                    break;
                              case -6:
                                    writeData(xml, mag, "flag256thUp", h);
                                    break;
                              case -7:
                                    writeData(xml, mag, "flag512thUp", h);
                                    break;
                              case -8:
                                    writeData(xml, mag, "flag1024thUp", h);
                                    break;
                              case 1:
                                    writeData(xml, mag, "flag8thDown", h);
                                    break;
                              case 2:
                                    writeData(xml, mag, "flag16thDown", h);
                                    break;
                              case 3:
                                    writeData(xml, mag, "flag32ndDown", h);
                                    break;
                              case 4:
                                    writeData(xml, mag, "flag64thDown", h);
                                    break;
                              case 5:
                                    writeData(xml, mag, "flag128thDown", h);
                                    break;
                              case 6:
                                    writeData(xml, mag, "flag256thDown", h);
                                    break;
                              case 7:
                                    writeData(xml, mag, "flag512thDown", h);
                                    break;
                              case 8:
                                    writeData(xml, mag, "flag1024thDown", h);
                                    break;
                              }
                        }
                        break;

                  case Element::Type::REST: {
                        const Rest* r = static_cast<const Rest*>(e);
                        switch (r->durationType().type()) {
                              case TDuration::DurationType::V_1024TH:
                                    writeData(xml, mag, "rest1024th", r);
                                    break;
                              case TDuration::DurationType::V_512TH:
                                    writeData(xml, mag, "rest512th", r);
                                    break;
                              case TDuration::DurationType::V_256TH:
                                    writeData(xml, mag, "rest256th", r);
                                    break;
                              case TDuration::DurationType::V_128TH:
                                    writeData(xml, mag, "rest128th", r);
                                    break;
                              case TDuration::DurationType::V_64TH:
                                    writeData(xml, mag, "rest64th", r);
                                    break;
                              case TDuration::DurationType::V_32ND:
                                    writeData(xml, mag, "rest32nd", r);
                                    break;
                              case TDuration::DurationType::V_16TH:
                                    writeData(xml, mag, "rest16th", r);
                                    break;
                              case TDuration::DurationType::V_EIGHTH:
                                    writeData(xml, mag, "rest8th", r);
                                    break;
                              case TDuration::DurationType::V_QUARTER:
                                    writeData(xml, mag, "restQuarter", r);
                                    break;
                              case TDuration::DurationType::V_HALF:
                                    writeData(xml, mag, "restHalf", r);
                                    break;
                              case TDuration::DurationType::V_WHOLE:
                              case TDuration::DurationType::V_MEASURE:
                                    writeData(xml, mag, "restWhole", r);
                                    break;
                              case TDuration::DurationType::V_BREVE:
                                    writeData(xml, mag, "restDoubleWhole", r);
                                    break;
                              case TDuration::DurationType::V_LONG:
                                    writeData(xml, mag, "restLonga", r);
                                    break;
                              default:
                                    break;
                              }

                        }
                        break;

                  case Element::Type::TIMESIG: {
                        const TimeSig* ts = static_cast<const TimeSig*>(e);
                        Fraction sig      = ts->sig();
                        TimeSigType t     = ts->timeSigType();
                        switch (t) {
                              case TimeSigType::NORMAL: {
                                    QString s = QString("timeSig%1over%2").arg(sig.numerator()).arg(sig.denominator());
                                    writeData(xml, mag, s, ts);
                                    }
                                    break;
                              case TimeSigType::FOUR_FOUR:
                                    writeData(xml, mag, "timeSigCommon", ts);
                                    break;
                              case TimeSigType::ALLA_BREVE:
                                    writeData(xml, mag, "timesigCut", ts);
                                    break;
                              }
                        }
                        break;

#if 0
                  case Element::Type::MARKER: {
                        const Marker* m = static_cast<const Marker*>(e);
                        switch (m->markerType()) {
                              case Marker::Type::SEGNO:
                              case Marker::Type::VARSEGNO:
                                    writeData(xml, mag, "Segno", m);
                                    break;

                              case Marker::Type::CODA:
                              case Marker::Type::VARCODA:
                                    writeData(xml, mag, "Coda", m);
                                    break;

                              case Marker::Type::FINE:
                                    writeData(xml, mag, "Fine", m);
                                    break;

                              case Marker::Type::TOCODA:
                                    writeData(xml, mag, "ToCoda", m);
                                    break;

                              case Marker::Type::CODETTA:
                              default:
                                    break;
                              }

                        }
                        break;

                  case Element::Type::DYNAMIC: {
                        const Dynamic* d = static_cast<const Dynamic*>(e);
                        switch (d->dynamicType()) {
                              case Dynamic::Type::PPPPPP:
                              case Dynamic::Type::PPPPP:
                              case Dynamic::Type::PPPP:
                              case Dynamic::Type::PPP:
                                    break;
                              case Dynamic::Type::PP:
                                    writeData(xml, mag, "Dynamic_PP", d);
                                    break;
                              case Dynamic::Type::P:
                                    writeData(xml, mag, "Dynamic_P", d);
                                    break;
                              case Dynamic::Type::MP:
                                    writeData(xml, mag, "Dynamic_MP", d);
                                    break;
                              case Dynamic::Type::MF:
                                    writeData(xml, mag, "Dynamic_MF", d);
                                    break;
                                    break;
                              case Dynamic::Type::F:
                                    writeData(xml, mag, "Dynamic_F", d);
                                    break;
                              case Dynamic::Type::FF:
                                    writeData(xml, mag, "Dynamic_FF", d);
                                    break;
                              case Dynamic::Type::FFF:
                              case Dynamic::Type::FFFF:
                              case Dynamic::Type::FFFFF:
                              case Dynamic::Type::FFFFFF:
                                    break;
                              case Dynamic::Type::FP:
                                    writeData(xml, mag, "Dynamic_FP", d);
                                    break;
                              case Dynamic::Type::SF:
                                    writeData(xml, mag, "Dynamic_SF", d);
                                    break;
                              case Dynamic::Type::SFZ:
                                    writeData(xml, mag, "Dynamic_SFZ", d);
                                    break;
                              case Dynamic::Type::SFF:
                              case Dynamic::Type::SFFZ:
                              case Dynamic::Type::SFP:
                              case Dynamic::Type::SFPP:
                              case Dynamic::Type::RFZ:
                              case Dynamic::Type::RF:
                              case Dynamic::Type::FZ:
                              case Dynamic::Type::M:
                              case Dynamic::Type::R:
                              case Dynamic::Type::S:
                              case Dynamic::Type::Z:
                              default:
                                    break;
                              }
                        }
                        break;
#endif
                  default:
                        break;      // ignore
                  }
            }
      xml.etag();
      if (f.error() != QFile::NoError)
            MScore::lastError = tr("Write failed: %1").arg(f.errorString());
      }

}

