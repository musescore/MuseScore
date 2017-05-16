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
#include "libmscore/score.h"
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
#include "libmscore/sym.h"
#include "libmscore/staff.h"

//##  write out extended metadata about every painted symbol

namespace Ms {

//    TODO: spatium is not constant 20 but may depend on score

//---------------------------------------------------------
//   writeData
//---------------------------------------------------------

static void writeData(Xml& xml, qreal spatium, qreal scale, const QString& name, const Element* e)
      {
      qreal interline = 20.0;
      if (e->staff()) {
            interline *= (e->staff()->spatium() / e->score()->spatium());
            scale     *= (e->score()->spatium() / e->staff()->spatium());
            }
      if (scale == 1.0)
            xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(interline).arg(name));
      else
            xml.stag(QString("Symbol interline=\"%1\" scale=\"%2\" shape=\"%3\"").arg(interline).arg(scale).arg(name));
      QRectF rr(e->pageBoundingRect());
      QRectF r(rr.x() * spatium, rr.y() * spatium, rr.width() * spatium, rr.height() * spatium);
      xml.putLevel();
      xml << QString("<%1 x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\"/>\n").arg("Bounds")
         .arg(r.x(),      0, 'f', 3)
         .arg(r.y(),      0, 'f', 3)
         .arg(r.width(),  0, 'f', 3)
         .arg(r.height(), 0, 'f', 3);
      xml.etag();
      }

//---------------------------------------------------------
//   writeSymbol
//---------------------------------------------------------

static void writeSymbol(Xml& xml, qreal mag, const QString& name, const Element* e, SymId sym, QPointF p)
      {
      xml.stag(QString("Symbol interline=\"%1\" shape=\"%2\"").arg(20).arg(name));
      QRectF bbox = e->score()->scoreFont()->bbox(sym, e->magS());
      QRectF rr   = bbox.translated(p);
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
   qreal gmag, const QList<const Element*>& pel)
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
//                  xml.tag("Image", url.toDisplayString());
                  xml.tag("Image", QString("file:%1").arg(imageName));  // no path
                  QRectF r(e->pageBoundingRect());
                  xml.tag("Size", QVariant(QSize(lrint(r.width() * gmag), lrint(r.height() * gmag))));
                  xml.etag();
                  break;
                  }
            }
      for (const Element* e : pel) {
            if (!e->visible())
                  continue;
            Staff* staff = e->staff();

            qreal smag = staff ? score->spatium() / staff->spatium() : gmag;
            qreal mag  = gmag * smag;
            switch (e->type()) {
                  case Element::Type::NOTEDOT:
                        writeData(xml, mag, e->mag(), "augmentationDot", e);
                        break;
                  case Element::Type::STEM:
                        writeData(xml, mag, e->mag() / smag, "stem", e);
                        break;
#if 0
                  case Element::Type::HAIRPIN_SEGMENT:
                        writeData(xml, mag, e->mag(), e->name(), e);
                        break;
#endif
                  case Element::Type::ARTICULATION: {
                        const Articulation* a = static_cast<const Articulation*>(e);
                        SymId symId = a->sym();
                        QString symName = Sym::id2userName(symId);
                        writeData(xml, mag, a->mag(), symName, a);
                        }
                        break;
                  case Element::Type::CLEF: {
                        const Clef* c = static_cast<const Clef*>(e);
                        SymId symId = c->sym();
                        QString symName = Sym::id2userName(symId);
                        writeData(xml, mag, c->symMag(), symName, c);   // hack symMag
                        }
                        break;

                  case Element::Type::NOTE: {
                        const Note* n = static_cast<const Note*>(e);
                        SymId symId = n->noteHead();
                        QString symName = Sym::id2userName(symId);
                        writeData(xml, mag, n->mag(), symName, n);
                        }
                        break;

                  case Element::Type::ACCIDENTAL: {
                        const Accidental* a = static_cast<const Accidental*>(e);
                        switch (a->accidentalType()) {
                              case AccidentalType::SHARP:
                                    writeData(xml, mag, a->mag(), "accidentalSharp", a);
                                    break;
                              case AccidentalType::FLAT:
                                    writeData(xml, mag, a->mag(), "accidentalFlat", a);
                                    break;
                              case AccidentalType::SHARP2:
                                    writeData(xml, mag, a->mag(), "accidentalDoubleSharp", a);
                                    break;
                              case AccidentalType::FLAT2:
                                    writeData(xml, mag, a->mag(), "accidentalDoubleFlat", a);
                                    break;
                              case AccidentalType::NATURAL:
                                    writeData(xml, mag, a->mag(), "accidentalNatural", a);
                                    break;
                              default:
                                    break;
                              }
                        }
                        break;
                  case Element::Type::KEYSIG: {
                        const KeySig* k = static_cast<const KeySig*>(e);
                        QPointF p (k->pagePos());
                        for (const KeySym& ks: k->keySigEvent().keySymbols()) {
                              QString name;
                              switch (ks.sym) {
                                    case SymId::accidentalSharp:
                                          name = "keySharp";
                                          break;
                                    case SymId::accidentalFlat:
                                          name = "keyFlat";
                                          break;
                                    case SymId::accidentalNatural:
                                          name = "keyNatural";
                                          break;
                                    default:
                                          name = "unknown";
                                          break;
                                    }
                              writeSymbol(xml, mag, name, k, ks.sym, p + ks.pos);
                              }
                        };
                        break;

                  case Element::Type::HOOK: {
                        const Hook* h = static_cast<const Hook*>(e);
                        const Chord* c = h->chord();

                        if (c->isGrace()) {
                              if (h->hookType() == 1) {
                                    if (c->slash())
                                          writeData(xml, mag, c->mag(), "Flag1UpSmallSlash", h);
                                    else
                                          writeData(xml, mag, c->mag(), "Flag1UpSmall", h);
                                    }
                              break;
                              }
                        switch (h->hookType()) {
                              default:
                              case 0:
                                    break;
                              case -1:
                                    writeData(xml, mag, h->mag(), "flag8thDown", h);
                                    break;
                              case -2:
                                    writeData(xml, mag, h->mag(), "flag16thDown", h);
                                    break;
                              case -3:
                                    writeData(xml, mag, h->mag(), "flag32ndDown", h);
                                    break;
                              case -4:
                                    writeData(xml, mag, h->mag(), "flag64thDown", h);
                                    break;
                              case -5:
                                    writeData(xml, mag, h->mag(), "flag128thDown", h);
                                    break;
                              case -6:
                                    writeData(xml, mag, h->mag(), "flag256thDown", h);
                                    break;
                              case -7:
                                    writeData(xml, mag, h->mag(), "flag512thDown", h);
                                    break;
                              case -8:
                                    writeData(xml, mag, h->mag(), "flag1024thDown", h);
                                    break;
                              case 1:
                                    writeData(xml, mag, h->mag(), "flag8thUp", h);
                                    break;
                              case 2:
                                    writeData(xml, mag, h->mag(), "flag16thUp", h);
                                    break;
                              case 3:
                                    writeData(xml, mag, h->mag(), "flag32ndUp", h);
                                    break;
                              case 4:
                                    writeData(xml, mag, h->mag(), "flag64thUp", h);
                                    break;
                              case 5:
                                    writeData(xml, mag, h->mag(), "flag128thUp", h);
                                    break;
                              case 6:
                                    writeData(xml, mag, h->mag(), "flag256thUp", h);
                                    break;
                              case 7:
                                    writeData(xml, mag, h->mag(), "flag512thUp", h);
                                    break;
                              case 8:
                                    writeData(xml, mag, h->mag(), "flag1024thUp", h);
                                    break;
                              }
                        }
                        break;

                  case Element::Type::REST: {
                        const Rest* r = static_cast<const Rest*>(e);
                        switch (r->durationType().type()) {
                              case TDuration::DurationType::V_1024TH:
                                    writeData(xml, mag, r->mag(), "rest1024th", r);
                                    break;
                              case TDuration::DurationType::V_512TH:
                                    writeData(xml, mag, r->mag(), "rest512th", r);
                                    break;
                              case TDuration::DurationType::V_256TH:
                                    writeData(xml, mag, r->mag(), "rest256th", r);
                                    break;
                              case TDuration::DurationType::V_128TH:
                                    writeData(xml, mag, r->mag(), "rest128th", r);
                                    break;
                              case TDuration::DurationType::V_64TH:
                                    writeData(xml, mag, r->mag(), "rest64th", r);
                                    break;
                              case TDuration::DurationType::V_32ND:
                                    writeData(xml, mag, r->mag(), "rest32nd", r);
                                    break;
                              case TDuration::DurationType::V_16TH:
                                    writeData(xml, mag, r->mag(), "rest16th", r);
                                    break;
                              case TDuration::DurationType::V_EIGHTH:
                                    writeData(xml, mag, r->mag(), "rest8th", r);
                                    break;
                              case TDuration::DurationType::V_QUARTER:
                                    writeData(xml, mag, r->mag(), "restQuarter", r);
                                    break;
                              case TDuration::DurationType::V_HALF:
                                    writeData(xml, mag, r->mag(), "restHalf", r);
                                    break;
                              case TDuration::DurationType::V_WHOLE:
                              case TDuration::DurationType::V_MEASURE:
                                    writeData(xml, mag, r->mag(), "restWhole", r);
                                    break;
                              case TDuration::DurationType::V_BREVE:
                                    writeData(xml, mag, r->mag(), "restDoubleWhole", r);
                                    break;
                              case TDuration::DurationType::V_LONG:
                                    writeData(xml, mag, r->mag(), "restLonga", r);
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
                                    writeData(xml, mag, ts->mag(), s, ts);
                                    }
                                    break;
                              case TimeSigType::FOUR_FOUR:
                                    writeData(xml, mag, ts->mag(), "timeSigCommon", ts);
                                    break;
                              case TimeSigType::ALLA_BREVE:
                                    writeData(xml, mag, ts->mag(), "timesigCut", ts);
                                    break;
                              }
                        }
                        break;

                  case Element::Type::MARKER: {
                        const Marker* m = static_cast<const Marker*>(e);
                        switch (m->markerType()) {
                              case Marker::Type::SEGNO:
                              case Marker::Type::VARSEGNO:
                                    writeData(xml, mag, m->mag(), "segno", m);
                                    break;

                              case Marker::Type::VARCODA:
                                    writeData(xml, mag, m->mag(), "codaSquare", m);
                                    break;

                              case Marker::Type::CODA:
                                    writeData(xml, mag, m->mag(), "coda", m);
                                    break;

                              case Marker::Type::FINE:
                                    writeData(xml, mag, m->mag(), "fine", m);
                                    break;

                              case Marker::Type::TOCODA:
                                    writeData(xml, mag, m->mag(), "toCoda", m);
                                    break;

                              case Marker::Type::CODETTA:
                                    writeData(xml, mag, m->mag(), "unknown", m);
                                    break;

                              default:
                                    break;
                              }

                        }
                        break;

                  case Element::Type::DYNAMIC: {
                        const Dynamic* d = static_cast<const Dynamic*>(e);
                        switch (d->dynamicType()) {
                              case Dynamic::Type::PPPPPP:
                                    writeData(xml, mag, d->mag(), "dynamicPPPPPP", d);
                                    break;
                              case Dynamic::Type::PPPPP:
                                    writeData(xml, mag, d->mag(), "dynamicPPPPP", d);
                                    break;
                              case Dynamic::Type::PPPP:
                                    writeData(xml, mag, d->mag(), "dynamicPPPP", d);
                                    break;
                              case Dynamic::Type::PPP:
                                    writeData(xml, mag, d->mag(), "dynamicPPP", d);
                                    break;
                              case Dynamic::Type::PP:
                                    writeData(xml, mag, d->mag(), "dynamicPP", d);
                                    break;
                              case Dynamic::Type::P:
                                    writeData(xml, mag, d->mag(), "dynamicPiano", d);
                                    break;
                              case Dynamic::Type::MP:
                                    writeData(xml, mag, d->mag(), "dynamicMP", d);
                                    break;
                              case Dynamic::Type::MF:
                                    writeData(xml, mag, d->mag(), "dynamicMF", d);
                                    break;
                              case Dynamic::Type::F:
                                    writeData(xml, mag, d->mag(), "dynamicForte", d);
                                    break;
                              case Dynamic::Type::FF:
                                    writeData(xml, mag, d->mag(), "dynamicFF", d);
                                    break;
                              case Dynamic::Type::FFF:
                                    writeData(xml, mag, d->mag(), "dynamicFFF", d);
                                    break;
                              case Dynamic::Type::FFFF:
                                    writeData(xml, mag, d->mag(), "dynamicFFFF", d);
                                    break;
                              case Dynamic::Type::FFFFF:
                                    writeData(xml, mag, d->mag(), "dynamicFFFF", d);
                                    break;
                              case Dynamic::Type::FFFFFF:
                                    writeData(xml, mag, d->mag(), "dynamicFFFFF", d);
                                    break;
                              case Dynamic::Type::FP:
                                    writeData(xml, mag, d->mag(), "dynamicFortePiano", d);
                                    break;
                              case Dynamic::Type::SF:
                                    writeData(xml, mag, d->mag(), "dynamicSforzando1", d);
                                    break;
                              case Dynamic::Type::SFZ:
                                    writeData(xml, mag, d->mag(), "dynamicSforzato", d);
                                    break;
                              case Dynamic::Type::SFF:
                                    writeData(xml, mag, d->mag(), "unknown", d);
                                    break;
                              case Dynamic::Type::SFFZ:
                                    writeData(xml, mag, d->mag(), "dynamicSforzatoFF", d);
                                    break;
                              case Dynamic::Type::SFP:
                                    writeData(xml, mag, d->mag(), "dynamicSforzatoPiano", d);
                                    break;
                              case Dynamic::Type::SFPP:
                                    writeData(xml, mag, d->mag(), "dynamicSforzatoPianissimo", d);
                                    break;
                              case Dynamic::Type::RFZ:
                                    writeData(xml, mag, d->mag(), "dynamicRinforzando2", d);
                                    break;
                              case Dynamic::Type::RF:
                                    writeData(xml, mag, d->mag(), "unknown", d);
                                    break;
                              case Dynamic::Type::FZ:
                                    writeData(xml, mag, d->mag(), "dynamicForzando", d);
                                    break;
                              case Dynamic::Type::M:
                                    writeData(xml, mag, d->mag(), "unknown", d);
                                    break;
                              case Dynamic::Type::R:
                                    writeData(xml, mag, d->mag(), "dynamicRinforzando", d);
                                    break;
                              case Dynamic::Type::S:
                                    writeData(xml, mag, d->mag(), "dynamicSforzando", d);
                                    break;
                              case Dynamic::Type::Z:
                                    writeData(xml, mag, d->mag(), "dynamicZ", d);
                                    break;
                              default:
                                    writeData(xml, mag, d->mag(), "unknown", d);
                                    break;
                              }
                        }
                        break;

                  default:
                        break;      // ignore
                  }
            }
      xml.etag();
      if (f.error() != QFile::NoError)
            MScore::lastError = tr("Write failed: %1").arg(f.errorString());
      }

}

