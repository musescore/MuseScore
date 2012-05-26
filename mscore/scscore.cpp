//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scscore.cpp 5619 2012-05-11 12:52:11Z lasconic $
//
//  Copyright (C) 2009-2010 Werner Schweer and others
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

#include "musescore.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/clef.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/system.h"
#include "libmscore/page.h"
#include "libmscore/text.h"
#include "libmscore/box.h"
#include "preferences.h"
#include "libmscore/style.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/harmony.h"
#include "script.h"
#include "libmscore/score.h"
#include "libmscore/repeatlist.h"
#include "libmscore/mscore.h"

#include "scoreview.h"

Q_DECLARE_METATYPE(PageFormat*);
Q_DECLARE_METATYPE(Score*);
Q_DECLARE_METATYPE(Part*);
Q_DECLARE_METATYPE(Text*);

static const char* const function_names_score[] = {
      "title", "subtitle", "composer", "poet",
      "load", "save", "close",
      "setExpandRepeat", "appendPart", "appendMeasures",
      "pages", "measures", "parts", "part", "startUndo", "endUndo", "setStyle", "hasLyrics", "hasHarmonies",
      "staves", "keysig", "duration", "pageFormat", "metatag", "fileName", "path",
      "version", "fileVersion"
      };
static const int function_lengths_score[] = {
      1, 1, 1, 1,
      1, 6, 1,
      1, 1, 1,
      0, 0, 0, 1, 0, 0, 2, 0, 0,
      0, 1, 0, 0, 2, 0, 0,
      0, 0
      };

static const QScriptValue::PropertyFlags flags_score[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,

      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,

      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,

      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,

      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter
      };

ScriptInterface scoreInterface = {
      sizeof(function_names_score) / sizeof(*function_names_score),
      function_names_score,
      function_lengths_score,
      flags_score
      };

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

static void addText(Score* score, int subtype, const QString& s)
      {
      MeasureBase* measure = score->first();
      if (measure == 0 || measure->type() != VBOX) {
            score->insertMeasure(VBOX, measure);
            measure = score->first();
            }
      Text* text = new Text(score);
      switch(subtype) {
            case TEXT_TITLE:    text->setTextStyleType(TEXT_STYLE_TITLE);    break;
            case TEXT_SUBTITLE: text->setTextStyleType(TEXT_STYLE_SUBTITLE); break;
            case TEXT_COMPOSER: text->setTextStyleType(TEXT_STYLE_COMPOSER); break;
            case TEXT_POET:     text->setTextStyleType(TEXT_STYLE_POET);     break;
            }
      text->setParent(measure);
      text->setText(s);
      score->undoAddElement(text);
      }

//---------------------------------------------------------
//   prototype_Score_call
//---------------------------------------------------------

static QScriptValue prototype_Score_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      Score* score = qscriptvalue_cast<Score*>(context->thisObject());
      if (!score) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Score.%0(): this object is not a Score")
               .arg(function_names_score[_id]));
            }
      int argc = context->argumentCount();
      switch(_id) {
            case 0:     // "title",
                  {
                  Text* t = score->getText(TEXT_TITLE);
                  if (argc == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (argc == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        if (t)
                              t->setText(s);
                        else
                              addText(score, TEXT_TITLE, s);
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 1:     // "subtitle",
                  {
                  Text* t = score->getText(TEXT_SUBTITLE);
                  if (argc == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (argc == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        if (t)
                              t->setText(s);
                        else
                              addText(score, TEXT_SUBTITLE, s);
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 2:     // "composer",
                  {
                  Text* t = score->getText(TEXT_COMPOSER);
                  if (argc == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (argc == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        if (t)
                              t->setText(s);
                        else
                              addText(score, TEXT_COMPOSER, s);
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 3:     // "poet",
                  {
                  Text* t = score->getText(TEXT_POET);
                  if (argc == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (argc == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        if (t)
                              t->setText(s);
                        else
                              addText(score, TEXT_POET, s);
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 4:    // "load",
                  if (argc == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        return qScriptValueFromValue(context->engine(), mscore->readScore(score, s));
                        }
                  break;
            case 5:     // "save",
                  {
                  QString s, ext, sf;
                  if (argc >= 2) {
                        s = qscriptvalue_cast<QString>(context->argument(0));
                        ext = qscriptvalue_cast<QString>(context->argument(1));
                        }

                  if (argc == 2) {
                        if(ext == "time") {
                              score->updateRepeatList(true);
                              QFile file(s);
                              file.open(QIODevice::WriteOnly | QIODevice::Text);
                              QTextStream out(&file);
                              out << "<events>" << endl;
                              Measure* lastMeasure = 0;
                              foreach (const RepeatSegment* rs, *(score->repeatList())) {
                                    int startTick  = rs->tick;
                                    int endTick    = startTick + rs->len;
                                    int tickOffset = rs->utick - rs->tick;
                                    for (Measure* m = score->tick2measure(startTick); m; m = m->nextMeasure()) {
                                          int offset = 0;
                                          if (lastMeasure && m->isRepeatMeasure())
                                                offset = m->tick() - lastMeasure->tick();
                                          else
                                                lastMeasure = m;
                                          
                                          SegmentTypes st = SegGrace | SegChordRest;
                                          for (Segment* seg = lastMeasure->first(st); seg; seg = seg->next(st)) {
                                                int tick = seg->tick() + tickOffset + offset;
                                                int time = score->utick2utime(tick) * 1000;
                                                out <<  QString(" <event elid=\"%1\" position=\"%2\" />").arg(seg->tick()).arg(time) << endl;
                                                }
                                          if (m->tick() + m->ticks() >= endTick)
                                                break;
                                          }
                                    }
                              out << "</events>";
                              file.close();
                              return context->engine()->undefinedValue();
                              }
                        else
                              return qScriptValueFromValue(context->engine(), mscore->saveAs(score, true, s, ext));
                        }

                  else if (argc == 6 && ext == "png") {
                        bool screenshot  = context->argument(2).toBool();
                        bool transparent = context->argument(3).toBool();
                        double convDpi = context->argument(4).toNumber();
                        bool grayscale = context->argument(5).toBool();
                        QImage::Format f = grayscale ? QImage::Format_Indexed8 : QImage::Format_ARGB32_Premultiplied;
                        mscore->savePng(score, s, screenshot, transparent, convDpi, f);
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 6:    // "close",
                 {
                  if (argc == 0) {
                        mscore->closeScore(score);
                        }
                  return context->engine()->undefinedValue();
                  }
            case 7:    // "setExpandRepeat",
                  if (argc == 1) {
                        bool f = context->argument(0).toBool();
                        getAction("repeat")->setChecked(f);
                        preferences.midiExpandRepeats = f;
                        score->updateRepeatList(f);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 8:    // "appendPart",
                  {
                  InstrumentTemplate* t = 0;
                  static InstrumentTemplate defaultInstrument;

                  if (argc == 1)
                        t = searchTemplate(qscriptvalue_cast<QString>(context->argument(0)));
                  else if (argc != 0)
                        break;
                  if (t == 0) {
                        t = &defaultInstrument;
                        if (t->channel.isEmpty()) {
                              Channel a;
                              a.chorus       = 0;
                              a.reverb       = 0;
                              a.name         = "normal";
                              a.program      = 0;
                              a.bank         = 0;
                              a.volume       = 100;
                              a.pan         = 60;
                              t->channel.append(a);
                              }
                        }
                  Part* part = new Part(score);
                  part->initFromInstrTemplate(t);
                  int n = score->nstaves();
                  for (int i = 0; i < t->staves; ++i) {
                        Staff* staff = new Staff(score, part, i);
                        // staff->setClef(0, t->clefIdx[i]);
                        staff->setLines(t->staffLines[i]);
                        staff->setSmall(t->smallStaff[i]);
                        if (i == 0) {
                              staff->setBracket(0, t->bracket[0]);
                              staff->setBracketSpan(0, t->staves);
                              }
                        score->undoInsertStaff(staff, n + i);
                        }
                  part->staves()->front()->setBarLineSpan(part->nstaves());
                  score->cmdInsertPart(part, n);
                  score->fixTicks();
                  score->rebuildMidiMapping();
                  return context->engine()->undefinedValue();
                  }
                  break;
            case 9:    // "appendMeasures",
                  if (argc == 1) {
                        int n = context->argument(0).toInt32();
                        for (int i = 0; i < n; ++i)
                              score->insertMeasure(MEASURE, 0, false);
                        return context->engine()->undefinedValue();
                        }
                  break;

            case 10:    // "pages",
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), score->pages().size());
                  break;
            case 11:    // "measures",
                  if (argc == 0) {
                        int n = 0;
                        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure())
                              n++;
                        return qScriptValueFromValue(context->engine(), n);
                        }
                  break;
            case 12:    // "parts",
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), score->parts().size());
                  break;
            case 13:    // "part",
                  if (argc == 1) {
                        int n = context->argument(0).toInt32();
                        if(n >= 0 && n < score->parts().size()){
                            Part* part = score->parts().at(n);
                            return qScriptValueFromValue(context->engine(), part);
                            }
                        }
                  break;
            case 14:    // "startUndo",
                  if (argc == 0) {
                        score->startCmd();
                        return context->engine()->undefinedValue();
                        }

            case 15:    // "endUndo",
                  if (argc == 0) {
                        score->endCmd();
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 16:    // "setStyle",
                  if (argc == 2) {
                        QString name = qscriptvalue_cast<QString>(context->argument(0));
                        QString val  = qscriptvalue_cast<QString>(context->argument(1));
                        StyleVal sv(name, val);
                        score->style()->set(sv);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 17:    // "hasLyrics",
                  if (argc == 0) {
                        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                              for (Segment* seg = m->first(); seg; seg = seg->next()) {
                                    for (int i = 0; i < score->nstaves(); ++i) {
                                          if (seg->lyricsList(i) && seg->lyricsList(i)->size() > 0)
                                                return qScriptValueFromValue(context->engine(), true);
                                          }
                                    }
                              }
                        return qScriptValueFromValue(context->engine(), false);
                        }
                  break;
            case 18:    // "hasHarmonies"
                  if (argc == 0) {
                        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                              foreach(Element* element, *m->el()) {
                                    if (element->type() == HARMONY) {
                                          Harmony* h = static_cast<Harmony*>(element);
                                          if (h->id() != -1)
                                                return qScriptValueFromValue(context->engine(), true);
                                          }
                                    }
                              }
                        return qScriptValueFromValue(context->engine(), false);
                        }
                  break;
            case 19:    // staves
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), score->nstaves());
                  break;
            case 20:    // keysig
                  if (argc == 0){
                        int result = 0;
                        if(score->nstaves() > 0){
                            Staff* st = score->staff(0);
                            KeyList* kl = st->keymap();
                            KeySigEvent key = kl->key(0);
                            if(key.custom()) {
                                 QString s = "undefined";
                                 return qScriptValueFromValue(context->engine(), s);
                                 }
                            result = key.accidentalType();
                            int tr =  st->part()->instr()->transpose().chromatic;
                            if (!score->styleB(ST_concertPitch) && tr){
                                result = transposeKey(key.accidentalType(), tr);
                                }
                            }
                        return qScriptValueFromValue(context->engine(), result);
                        }
                   else if(argc == 1) {
                        //qDebug(":::setKeysig\n");
                        int newKey = context->argument(0).toInt32();
                        KeySigEvent ke;
                        ke.setAccidentalType(newKey);

                        for (int idx = 0; idx < score->nstaves(); idx++) {
                            int curKey = score->staff(idx)->key(0).accidentalType();
                            if (curKey != newKey) {
                                score->undoChangeKeySig(score->staff(idx), 0, ke);
                            }
                        }
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 21:   //duration
                  if (argc == 0){
                    RepeatSegment* rs = score->repeatList()->last();
                    long duration = lrint(score->utick2utime(rs->utick + rs->len));
                    return qScriptValueFromValue(context->engine(), duration);
                  }
                  break;
/* TODO            case 21:   // pageFormat
                  if (argc == 0) {
                        return qScriptValueFromValue(context->engine(), score->pageFormat());
                        }
                  break;
*/
            case 23:   //metatag
                  if (argc == 1) {
                        QString tag = qscriptvalue_cast<QString>(context->argument(0));
                        QString val = score->metaTag(tag);
                        return qScriptValueFromValue(context->engine(), val);
                        }
                  else if (argc == 2) {
                        QString tag = qscriptvalue_cast<QString>(context->argument(0));
                        QString val = qscriptvalue_cast<QString>(context->argument(1));
                        score->setMetaTag(tag, val);
                        return context->engine()->undefinedValue();
                  }
                  break;
            case 24: // fileName
                  if (argc == 0) {
                        QString fname = score->fileInfo()->fileName();
                        return qScriptValueFromValue(context->engine(), fname);
                  }
                  break;
            case 25: // path
                  if (argc == 0) {
                        QString fpath;
                        fpath = score->created() ? "" : score->fileInfo()->path();
                        return qScriptValueFromValue(context->engine(), fpath);
                  }
                  break;
            case 26:   //version
                  if (argc == 0) {
                        return qScriptValueFromValue(context->engine(), score->mscoreVersion());
                        }
                  else
                  break;
            case 27:   //fileVersion
                  if (argc == 0) {
                        return qScriptValueFromValue(context->engine(), score->mscVersion());
                        }
                  else
                  break;
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Score.%0(): bad argument count or value")
         .arg(function_names_score[_id]));
      }

//---------------------------------------------------------
//   static_Score_call
//---------------------------------------------------------

static QScriptValue static_Score_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Score(): Did you forget to construct with 'new'?"));
      Score* score = new Score(MScore::defaultStyle());
      score->setName(mscore->createDefaultName());
      mscore->setCurrentScoreView(mscore->appendScore(score));
      score->startCmd();
      return context->engine()->newVariant(context->thisObject(), qVariantFromValue(score));
      }

//---------------------------------------------------------
//   create_Score_class
//---------------------------------------------------------

QScriptValue create_Score_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &scoreInterface;

      engine->setDefaultPrototype(qMetaTypeId<Score*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((Score*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Score_call, function_lengths_score[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<Score*>(), proto);
      return engine->newFunction(static_Score_call, proto, 1);
      }


#if 0
//---------------------------------------------------------
//   setTitle
//---------------------------------------------------------

void ScScorePrototype::setTitle(const QString& text)
      {
      MeasureBaseList* ml = thisScore()->measures();
      MeasureBase* measure;
      if (!ml->first() || ml->first()->type() != VBOX) {
            measure = new VBox(thisScore());
            measure->setTick(0);
            thisScore()->undoInsertMeasure(measure);
            }
      else
            measure = ml->first();
      Text* s = new Text(thisScore());
      s->setTextStyleType(TEXT_STYLE_TITLE);
      s->setSubtype(TEXT_TITLE);
      s->setParent(measure);
      s->setText(text);
      thisScore()->undoAddElement(s);
      }
#endif

