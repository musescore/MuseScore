//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: figuredbass.cpp 5526 2012-04-09 10:17:11Z lvinken $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "figuredbass.h"
#include "score.h"
#include "note.h"
#include "measure.h"
#include "system.h"
#include "segment.h"

#define FBIDigitNone    0

// the array of configured fonts
static QList<FiguredBassFont> g_FBFonts;

//---------------------------------------------------------
//   F I G U R E D   B A S S   I T E M
//---------------------------------------------------------

// used for indexed access to parenthesis chars
// (these is no normAccidToChar[], as accidentals may use mult. chars in normalized display):
const QChar FiguredBassItem::normParenthToChar[FBINumOfParenth] =
{ 0, '(', ')', '[', ']'};


FiguredBassItem::FiguredBassItem(Score* s, int l)
      : SimpleText(s), ord(l)
      {
      prefix      = suffix = FBIAccidNone;
      digit       = FBIDigitNone;
      parenth[0]  = parenth[1] = parenth[2] = parenth[3] = parenth[4] = FBIParenthNone;
      contLine    = false;
      setTextStyle(s->textStyle(TEXT_STYLE_FIGURED_BASS));
      }

FiguredBassItem::FiguredBassItem(const FiguredBassItem& item)
   : SimpleText(item)
      {
      ord         = item.ord;
      prefix      = item.prefix;
      digit       = item.digit;
      suffix      = item.suffix;
      parenth[0]  = item.parenth[0];
      parenth[1]  = item.parenth[1];
      parenth[2]  = item.parenth[2];
      parenth[3]  = item.parenth[3];
      parenth[4]  = item.parenth[4];
      contLine    = item.contLine;
      textWidth   = item.textWidth;
      }

FiguredBassItem::~FiguredBassItem()
      {
      }

//---------------------------------------------------------
//   FiguredBassItem parse()
//
// converts a string into a property-based representation, if possible;
// return true on success | false if the string is non-conformant
//---------------------------------------------------------

bool FiguredBassItem::parse(QString& str)
      {
      int               retVal;

      parseParenthesis(str, 0);
      retVal = parsePrefixSuffix(str, true);          // prefix
      if(retVal == -1)
            return false;
      parseParenthesis(str, 1);
      retVal = parseDigit(str);                       // digit
      if(retVal == -1)
            return false;
      parseParenthesis(str, 2);
      retVal = parsePrefixSuffix(str, false);         // suffix
      if(retVal == -1)
            return false;
      parseParenthesis(str, 3);
      // check for a possible cont. line symbol(s)
      contLine = false;                               // contLine
      while(str[0] == '-' || str[0] == '_') {
            contLine = true;
            str.remove(0, 1);
      }
      parseParenthesis(str, 4);

      // remove useless parentheses
      if(prefix == FBIAccidNone && parenth[1] == FBIParenthNone) {
            parenth[1] = parenth[0];
            parenth[0] = FBIParenthNone;
            }
      if(digit == FBIDigitNone && parenth[2] == FBIParenthNone) {
            parenth[2] = parenth[1];
            parenth[1] = FBIParenthNone;
            }
      if(!contLine && parenth[3] == FBIParenthNone) {
            parenth[3] = parenth[4];
            parenth[4] = FBIParenthNone;
            }
      if(suffix == FBIAccidNone && parenth[2] == FBIParenthNone) {
            parenth[2] = parenth[3];
            parenth[3] = FBIParenthNone;
            }

      // some checks:
      // if some extra input, str is not conformant
      if(str.size())
            return false;
      // can't have BOTH prefix and suffix
      // prefix, digit, suffix and cont.line cannot be ALL empty
      // suffix cannot combine with empty digit
      if( (prefix != FBIAccidNone && suffix != FBIAccidNone)
            || (prefix == FBIAccidNone && digit == FBIDigitNone && suffix == FBIAccidNone && !contLine)
            || ( (suffix == FBIAccidPlus || suffix == FBIAccidBackslash || suffix == FBIAccidSlash)
                  && digit == FBIDigitNone) )
            return false;
      return true;
}

//---------------------------------------------------------
//   FiguredBassItem parsePrefixSuffix()
//
//    scans str to extract prefix or suffix properties. Stops at the first char which cannot fit.
//    Fitting chars are removed from str. DOES NOT generate any display text
//
// returns the number of QChar's read from str or -1 if prefix / suffix has an illegal format
// (no prefix / suffix at all IS legal)
//---------------------------------------------------------

int FiguredBassItem::parsePrefixSuffix(QString& str, bool bPrefix)
      {
      FBIAccidental *   dest        = bPrefix ? &prefix : &suffix;
      bool              done        = false;
      int               size        = str.size();
      str = str.trimmed();

      *dest             = FBIAccidNone;

      while(str.size()) {
            switch(str.at(0).unicode())
            {
            case 'b':
                  if(*dest != FBIAccidNone) {
                        if(*dest == FBIAccidFlat)     // FLAT may double a previous FLAT
                              *dest = FBIAccidDoubleFlat;
                        else
                              return -1;              // but no other combination is acceptable
                        }
                  *dest = FBIAccidFlat;
                  break;
            case 'h':
                  if(*dest != FBIAccidNone)           // cannot combine with any other accidental
                        return -1;
                  *dest = FBIAccidNatural;
                  break;
            case '#':
                  if(*dest != FBIAccidNone) {
                        if(*dest == FBIAccidSharp)    // SHARP may double a preivous SHARP
                              *dest = FBIAccidDoubleSharp;
                        else
                              return -1;              // but no other combination is acceptable
                        }
                  *dest = FBIAccidSharp;
                  break;
            // '+', '\\' and '/' go into the suffix
            case '+':
                  if(suffix != FBIAccidNone)          // cannot combine with any other accidental
                        return -1;
                  suffix = FBIAccidPlus;
                  break;
            case '\\':
                  if(suffix != FBIAccidNone)          // cannot combine with any other accidental
                        return -1;
                  suffix = FBIAccidBackslash;
                  break;
            case '/':
                  if(suffix != FBIAccidNone)          // cannot combine with any other accidental
                        return -1;
                  suffix = FBIAccidSlash;
                  break;
            default:                                  // any other char: no longer in prefix/suffix
                  done = true;
                  break;
            }
            if(done)
                  break;
            str.remove(0,1);                         // 'eat' the char and continue
            }

      return size - str.size();                       // return how many chars we had read into prefix/suffix
      }

//---------------------------------------------------------
//   FiguredBassItem parseDigit()
//
//    scans str to extract digit properties. Stops at the first char which cannot belong to digit part.
//    Fitting chars are removed from str. DOES NOT generate any display text
//
// returns the number of QChar's read from str or -1 if no legal digit can be constructed
// (no digit at all IS legal)
//---------------------------------------------------------

int FiguredBassItem::parseDigit(QString& str)
      {
      int  size   = str.size();
      str         = str.trimmed();

      digit = FBIDigitNone;

      while(str.size()) {
            // any digit acceptable, if no previous digit
            if(str[0] >= '1' && str[0] <= '9') {
                  if(digit == FBIDigitNone) {
                        digit = str[0].unicode() - '0';
                        str.remove(0, 1);
                        }
                  else
                        return -1;
                  }
            // anything else: no longer in digit part
            else
                  break;
            }

      return size  - str.size();
      }

//---------------------------------------------------------
//   FiguredBassItem parseParenthesis()
//
//    scans str to extract a (possible) parenthesis, stores its code into parenth[parenthIdx]
//    and removes it from str. Only looks at first str char.
//
// returns the number of QChar's read from str (actually 0 or 1).
//---------------------------------------------------------

int FiguredBassItem::parseParenthesis(QString& str, int parenthIdx)
      {
      int c = str[0].unicode();
      FBIParenthesis code = FBIParenthNone;
      switch(c)
      {
      case '(':
            code =FBIParenthRoundOpen;
            break;
      case ')':
            code =FBIParenthRoundClosed;
            break;
      case '[':
            code =FBIParenthSquaredOpen;
            break;
      case ']':
            code =FBIParenthSquaredClosed;
            break;
      default:
            break;
            }
      parenth[parenthIdx] = code;
      if(code != FBIParenthNone) {
            str.remove(0, 1);
            return 1;
            }
      return 0;
      }

//---------------------------------------------------------
//   FiguredBassItem normalizedText()
//
// returns a string with the normalized text, i.e. the text displayed while editing;
// this is a standard textual representation of the item properties
//---------------------------------------------------------

QString FiguredBassItem::normalizedText() const
      {
      QString str = QString();
      if(parenth[0] != FBIParenthNone)
            str.append(normParenthToChar[parenth[0]]);

      if(prefix != FBIAccidNone) {
            switch(prefix)
            {
            case FBIAccidFlat:
                  str.append('b');
                  break;
            case FBIAccidNatural:
                  str.append('h');
                  break;
            case FBIAccidSharp:
                  str.append('#');
                  break;
            case FBIAccidDoubleFlat:
                  str.append("bb");
                  break;
            case FBIAccidDoubleSharp:
                  str.append("##");
                  break;
            default:
                  break;
            }
            }

      if(parenth[1] != FBIParenthNone)
            str.append(normParenthToChar[parenth[1]]);

      // digit
      if(digit != FBIDigitNone)
            str.append(QChar('0' + digit));

      if(parenth[2] != FBIParenthNone)
            str.append(normParenthToChar[parenth[2]]);

      // suffix
      if(suffix != FBIAccidNone) {
            switch(suffix)
            {
            case FBIAccidFlat:
                  str.append('b');
                  break;
            case FBIAccidNatural:
                  str.append('h');
                  break;
            case FBIAccidSharp:
                  str.append('#');
                  break;
            case FBIAccidPlus:
                  str.append('+');
                  break;
            case FBIAccidBackslash:
                  str.append('\\');
                  break;
            case FBIAccidSlash:
                  str.append('/');
                  break;
            case FBIAccidDoubleFlat:
                  str.append("bb");
                  break;
            case FBIAccidDoubleSharp:
                  str.append("##");
                  break;
            default:
                  break;
            }
            }

      if(parenth[3] != FBIParenthNone)
            str.append(normParenthToChar[parenth[3]]);
      if(contLine)
            str.append('_');
      if(parenth[4] != FBIParenthNone)
            str.append(normParenthToChar[parenth[4]]);

      return str;
      }

//---------------------------------------------------------
//   FiguredBassItem write()
//---------------------------------------------------------

void FiguredBassItem::write(Xml& xml) const
{
      xml.stag("FiguredBassItem");
      xml.tagE(QString("brackets b0=\"%1\" b1=\"%2\" b2=\"%3\" b3=\"%4\" b4=\"%5\"")
                    .arg(parenth[0]) .arg(parenth[1]) .arg(parenth[2]) .arg(parenth[3]) .arg(parenth[4]) );
      if(prefix != FBIAccidNone)
            xml.tag(QString("prefix"), prefix);
      if(digit != FBIDigitNone)
            xml.tag(QString("digit"), digit);
      if(suffix != FBIAccidNone)
            xml.tag(QString("suffix"), suffix);
      if(contLine)
            xml.tag("continuationLine", contLine);
      xml.etag();
}

//---------------------------------------------------------
//   FiguredBassItem read()
//---------------------------------------------------------

void FiguredBassItem::read(const QDomElement& de)
{
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            int   iVal = val.toInt();

            if(tag == "brackets") {
                  parenth[0] = (FBIParenthesis)e.attribute("b0").toInt();
                  parenth[1] = (FBIParenthesis)e.attribute("b1").toInt();
                  parenth[2] = (FBIParenthesis)e.attribute("b2").toInt();
                  parenth[3] = (FBIParenthesis)e.attribute("b3").toInt();
                  parenth[4] = (FBIParenthesis)e.attribute("b4").toInt();
                  }
            else if(tag == "prefix")
                  prefix = (FBIAccidental)iVal;
            else if(tag == "digit")
                  digit = iVal;
            else if(tag == "suffix")
                  suffix = (FBIAccidental)iVal;
            else if(tag == "continuationLine")
                  contLine = iVal;
            else if(!Element::readProperties(e))
                  domError(e);
            }
}

//---------------------------------------------------------
//   Convert MusicXML prefix/suffix to FBIAccidental
//---------------------------------------------------------

// TODO add missing non-accidental types

FiguredBassItem::FBIAccidental FiguredBassItem::MusicXML2FBIAccidental(const QString prefix) const
      {
      if (prefix == "sharp")
            return FBIAccidSharp;
      else if (prefix == "flat")
            return FBIAccidFlat;
      else if (prefix == "natural")
            return FBIAccidNatural;
      else if (prefix == "double-sharp")
            return FBIAccidDoubleSharp;
      else if (prefix == "flat-flat")
            return FBIAccidDoubleFlat;
      else if (prefix == "sharp-sharp")
            return FBIAccidDoubleSharp;
      else if (prefix == "slash")
            return FBIAccidSlash;
      else
            return FBIAccidNone;
      }

//---------------------------------------------------------
//   Convert FBIAccidental to MusicXML prefix/suffix
//---------------------------------------------------------

// TODO add missing non-accidental types

QString FiguredBassItem::FBIAccidental2MusicXML(FiguredBassItem::FBIAccidental prefix) const
      {
      switch (prefix) {
            case FBIAccidNone:        return "";
            case FBIAccidDoubleFlat:  return "flat-flat";
            case FBIAccidFlat:        return "flat";
            case FBIAccidNatural:     return "natural";
            case FBIAccidSharp:       return "sharp";
            case FBIAccidDoubleSharp: return "double-sharp";
            case FBIAccidPlus:        return ""; // TODO TBD
            case FBIAccidBackslash:   return ""; // TODO TBD
            case FBIAccidSlash:       return "slash";
            case FBINumOfAccid:       return ""; // prevent gcc "‘FBINumOfAccid’ not handled in switch" warning
            }
      return "";
      }

//---------------------------------------------------------
//   Read MusicXML
//
// Set the FiguredBassItem state based on the MusicXML <figure> node de.
// In MusicXML, parentheses is set to "yes" or "no" for the figured-bass
// node instead of for each individual <figure> node.
//---------------------------------------------------------

void FiguredBassItem::readMusicXML(const QDomElement& de, bool paren)
      {
      // read the <figure> node de
      for (QDomElement e = de.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            int   iVal = val.toInt();
            if (tag == "extend")
                  ; // TODO
            else if (tag == "figure-number") {
                  // MusicXML spec states figure-number is a number
                  // MuseScore can only handle single digit
                  if (1 <= iVal && iVal <= 9)
                        digit = iVal;
                  }
            else if (tag == "prefix")
                  prefix = MusicXML2FBIAccidental(val);
            else if (tag == "suffix")
                  suffix = MusicXML2FBIAccidental(val);
            else
                  domError(e);
            }
      // set parentheses
      if (paren) {
            // parenthesis open
            if (prefix != FBIAccidNone)
                  parenth[0] = FBIParenthRoundOpen; // before prefix
            else if (digit != FBIDigitNone)
                  parenth[1] = FBIParenthRoundOpen; // before digit
            else if (suffix != FBIAccidNone)
                  parenth[2] = FBIParenthRoundOpen; // before suffix
            // parenthesis close
            if (suffix != FBIAccidNone)
                  parenth[3] = FBIParenthRoundClosed; // after suffix
            else if (digit != FBIDigitNone)
                  parenth[2] = FBIParenthRoundClosed; // after digit
            else if (prefix != FBIAccidNone)
                  parenth[1] = FBIParenthRoundClosed; // after prefix
            }
      }

//---------------------------------------------------------
//   Write MusicXML
//---------------------------------------------------------

void FiguredBassItem::writeMusicXML(Xml& xml) const
      {
      xml.stag("figure");
      QString strPrefix = FBIAccidental2MusicXML(prefix);
      if (strPrefix != "")
            xml.tag("prefix", strPrefix);
      if (digit != FBIDigitNone)
            xml.tag("figure-number", digit);
      QString strSuffix = FBIAccidental2MusicXML(suffix);
      if (strSuffix != "")
            xml.tag("suffix", strSuffix);
      xml.etag();
      }

//---------------------------------------------------------
//   startsWithParenthesis
//---------------------------------------------------------

bool FiguredBassItem::startsWithParenthesis() const
      {
      if (prefix != FBIAccidNone)
            return (parenth[0] != FBIParenthNone);
      if (digit != FBIDigitNone)
            return (parenth[1] != FBIParenthNone);
      if (suffix != FBIAccidNone)
            return (parenth[2] != FBIParenthNone);
      return false;
      }

//---------------------------------------------------------
//   FiguredBassItem layout()
//    creates the display text (set as element text) and computes
//    the horiz. offset needed to align the right part as well as the vert. offset
//---------------------------------------------------------

void FiguredBassItem::layout()
      {
      qreal             h, w, x, x1, x2, y;

      setTextStyle(score()->textStyle(TEXT_STYLE_FIGURED_BASS));    // needed?

      QFontMetricsF     fm(textStyle().font(spatium()));
      QString           str = QString();
      x = symbols[score()->symIdx()][quartheadSym].width(magS()) * .5;
      x1 = x2 = 0.0;

      // create display text
      int font = 0;
      int style = score()->styleI(ST_figuredBassStyle);

      if(parenth[0] != FBIParenthNone)
            str.append(g_FBFonts.at(font).displayParenthesis[parenth[0]]);

      // prefix
      if(prefix != FBIAccidNone) {
            // if no digit, the string created so far 'hangs' to the left of the note
            if(digit == FBIDigitNone)
                  x1 = fm.width(str);
            str.append(g_FBFonts.at(font).displayAccidental[prefix]);
            // if no digit, the string from here onward 'hangs' to the right of the note
            if(digit == FBIDigitNone)
                  x2 = fm.width(str);
            }

      if(parenth[1] != FBIParenthNone)
            str.append(g_FBFonts.at(font).displayParenthesis[parenth[1]]);

      // digit
      if(digit != FBIDigitNone) {
            // if some digit, the string created so far 'hangs' to the left of the note
            x1 = fm.width(str);
            // if suffix is a combining shape, combine it with digit
            // unless there is a parenthesis in between
            if( (suffix == FBIAccidPlus || suffix == FBIAccidBackslash || suffix == FBIAccidSlash)
                        && parenth[2] == FBIParenthNone)
                  str.append(g_FBFonts.at(font).displayDigit[style][digit][suffix-(FBIAccidPlus-1)]);
            else
                  str.append(g_FBFonts.at(font).displayDigit[style][digit][0]);
            // if some digit, the string from here onward 'hangs' to the right of the note
            x2 = fm.width(str);
            }

      if(parenth[2] != FBIParenthNone)
            str.append(g_FBFonts.at(font).displayParenthesis[parenth[2]]);

      // suffix
      // append only if non-combining shape or cannot combine (no digit or parenthesis in between)
      if( suffix != FBIAccidNone
                  && ( (suffix != FBIAccidPlus && suffix != FBIAccidBackslash && suffix != FBIAccidSlash)
                        || digit == FBIDigitNone
                        || parenth[2] != FBIParenthNone) )
            str.append(g_FBFonts.at(font).displayAccidental[suffix]);

      if(parenth[3] != FBIParenthNone)
            str.append(g_FBFonts.at(font).displayParenthesis[parenth[3]]);

      setText(str);                             // this text will be displayed

      // position the text so that [x1<-->x2] is centered below the note
      x = x - (x1+x2) * 0.5;
      h = fm.lineSpacing();
      h *= score()->styleD(ST_figuredBassLineHeight);
      w = fm.width(str);
      textWidth = w;
      int lineLen;
      if(contLine && (lineLen=figuredBass()->lineLength(0)) > w)
            w = lineLen;
      y = h * ord;
      setPos(x, y);
      setbbox(QRect(0, 0, w, h));
}

//---------------------------------------------------------
//   FiguredBassItem draw()
//---------------------------------------------------------

void FiguredBassItem::draw(QPainter* painter) const
      {
      int font = 0;
//      SimpleText::draw(painter);
      // set font from general style
      QFont f(g_FBFonts.at(font).family);
#ifdef USE_GLYPHS
      f.setHintingPreference(QFont::PreferVerticalHinting);
#endif
      qreal m = score()->styleD(ST_figuredBassFontSize) * MScore::DPI / PPI;
      m *= spatium() / (SPATIUM20 * MScore::DPI);     // make spatium dependent
      f.setPixelSize(lrint(m));

      painter->setFont(f);
      painter->setBrush(Qt::NoBrush);
      painter->setPen(figuredBass()->curColor());
      painter->drawText(bbox(), Qt::TextDontClip | Qt::AlignLeft | Qt::AlignTop, getText());
//      drawFrame(p);

      // continuation line
      qreal len = 0.0;
      if(contLine) {
            len = figuredBass()->lineLength(0);
            if(len > 0.0) {
                  qreal h = bbox().height() * 0.75;
//                  painter->setPen((QPen(figuredBass()->curColor(), 1)));
                  painter->drawLine(textWidth, h, len - ipos().x(), h);
                  }
            }

      // closing cont.line parenthesis
      if(parenth[4] != FBIParenthNone) {
            int x = len > 0.0 ? len : textWidth;
            painter->drawText(QRectF(x, 0, bbox().width(), bbox().height()), Qt::AlignLeft | Qt::AlignTop,
                  g_FBFonts.at(font).displayParenthesis[parenth[4]]);
            }
      }

//---------------------------------------------------------
//   F I G U R E D   B A S S
//---------------------------------------------------------

#include "chord.h"
#include "rest.h"

FiguredBass::FiguredBass(Score* s)
   : Text(s)
      {
      setOnNote(true);
      setTextStyle(s->textStyle(TEXT_STYLE_FIGURED_BASS));
      setTicks(0);
      items.clear();
      }

FiguredBass::FiguredBass(const FiguredBass& fb)
   : Text(fb)
      {
      setOnNote(fb.onNote());
      setTicks(fb.ticks());
      items = fb.items;
      }

FiguredBass::~FiguredBass()
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FiguredBass::write(Xml& xml) const
      {
      xml.stag("FiguredBass");
      if(!onNote())
            xml.tag("onNote", onNote());
      if (ticks() > 0)
            xml.tag("ticks", ticks());
      foreach(FiguredBassItem item, items)
            item.write(xml);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FiguredBass::read(const QDomElement& de)
      {
      QString normalizedText = QString();
      int idx = 0;
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if(tag == "ticks")
                  setTicks(val.toInt());
            else if(tag == "onNote")
                  setOnNote(val.toInt() != 0l);
            else if (tag == "FiguredBassItem") {
                  FiguredBassItem * pItem = new FiguredBassItem(score(), idx++);
                  pItem->setTrack(track());
                  pItem->setParent(this);
                  pItem->read(e);
                  items.append(*pItem);
                  // add item normalized text
                  if(!normalizedText.isEmpty())
                        normalizedText.append('\n');
                  normalizedText.append(pItem->normalizedText());
                  }
            else if(!Element::readProperties(e))
                  domError(e);
            }
      setText(normalizedText);                  // this is the text to show while editing
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

// uncomment to use stadard, built-in Text layout during editing
//#define _USE_EDIT_TEXT_LAYOUT_

void FiguredBass::layout()
      {
      if (!styled())
            setTextStyle(score()->textStyle(TEXT_STYLE_FIGURED_BASS));
      qreal       y;

      layoutLines();

#ifdef _USE_EDIT_TEXT_LAYOUT_
      if (_editMode) {
            Text::layout();
            return;
            }
#endif

      // vertical position
      y = 0;                                          // default vert. pos.
      // if a staff exists for this F.B., use its y position
      if(parent() && track() >= 0) {
            System* sys = ((Segment*)parent())->measure()->system();
            if (sys == 0)
                  qDebug("FiguredBass layout: no system!");
            else {
                  SysStaff* staff = sys->staff(staffIdx());
                  y = staff->y();
                  }
            }
      y += point(score()->styleS(ST_figuredBassYOffset));

      // bounding box
#ifndef _USE_EDIT_TEXT_LAYOUT_
      if(editMode()) {
            qreal             h, w, w1;
            QFontMetricsF     fm(textStyle().font(spatium()));
            // box width
            w = 0;
            QStringList list = getText().split('\n');
            foreach(QString str, list) {
                  w1 = fm.width(str);
                  if(w1 > w)
                        w = w1;
                  }
            // bbox height
            h = fm.lineSpacing();
            h *= score()->styleD(ST_figuredBassLineHeight);
            h *= (list.size() > 1 ? list.size() : 1);      // at least 1 line
            // ready to set position and bbox
            setPos(0, y);
            setbbox(QRectF(0-2, 0-2, w+4, h+4));
            }
      else
#endif
            {
            setPos(0, y);
            setbbox(QRectF(0, 0, _lineLenghts.at(0), 0));
            // if element could be parsed into items, layout each element
            if(items.size() > 0) {
                  // layout each item and enlarge bbox to include items bboxes
                  for(int i=0; i < items.size(); i++) {
                        items[i].layout();
                        addbbox(items[i].bbox().translated(items[i].pos()));
                        }
                  }
            else
                  // if not, fall back to standard Text layout
                  Text::layout();
            }

      adjustReadPos();
      }

//---------------------------------------------------------
//   layoutLines
//
//    lays out the duration indicator line(s), filling the _lineLengths array
//---------------------------------------------------------

void FiguredBass::layoutLines()
      {
      if(_ticks <= 0) {
NoLen:
            _lineLenghts.resize(1);
            _lineLenghts[0] = 0;
            return;
            }
// Adapted from System::layoutLyrics(Lyrics* l, Segment* s, int staffIdx) (system.cpp, line 829 and foll)
      Segment *   nextSegm;                                 // the Segment beyond this' segment
      int         nextTick = segment()->tick() + _ticks;    // the tick beyond this' duration

      // locate the measure containing the last tick of this; it is either:
      // the same measure containing nextTick, if nextTick is not the first tick of a measure
      //    (and line should stop right before it)
      // or the previous measure, if nextTick is the first tick of a measure
      //    (and line should stop before any measure terminal segment (bar, clef, ...) )
      Measure* m = score()->tick2measure(nextTick-1);
      if (m != 0) {
            // locate the first segment (of ANY type) right after this' last tick
            for (nextSegm = m->first(Segment::SegAll); nextSegm; ) {
                  if(nextSegm->tick() >= nextTick)
                        break;
                  nextSegm = nextSegm->next();
                  }
            }
      if (m == 0 || nextSegm == 0) {
            qDebug("FiguredBass layout: no segment found for tick %d\n", nextTick);
            goto NoLen;
            }

      QList<System*>* systems = score()->systems();
      System* s1  = segment()->measure()->system();
      System* s2  = nextSegm->measure()->system();
      int sysIdx1 = systems->indexOf(s1);
      int sysIdx2 = systems->indexOf(s2);

      int i, len ,segIdx;
      for (i = sysIdx1, segIdx = 0; i <= sysIdx2; ++i, ++segIdx) {
            len = 0;
            if (sysIdx1 == sysIdx2 || i == sysIdx1) {
                  // single line
                  len = nextSegm->pageX() - pageX() - 4;         // stop 4 raster units before next segm
                  }
            else if (i == sysIdx1) {
                  // initial line
                  qreal w   = s1->staff(staffIdx())->right();
                  qreal x   = s1->pageX() + w;
                  len = x - pageX();
                  }
            else if (i > 0 && i != sysIdx2) {
                  // middle line
qDebug("FiguredBass: duration indicator middle line not implemented");
                  }
            else if (i == sysIdx2) {
                  // end line
qDebug("FiguredBass: duration indicator end line not implemented");
                  }
            // store length item, reusing array items if already present
            if (_lineLenghts.size() <= segIdx)
                  _lineLenghts.append(len);
            else
                  _lineLenghts[segIdx] = len;
            }
      // if more array items than needed, truncate array
      if(_lineLenghts.size() > segIdx)
            _lineLenghts.resize(segIdx);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FiguredBass::draw(QPainter* painter) const
      {
      // if not printing, draw duration line(s)
      if( !score()->printing() ) {
            foreach(qreal len, _lineLenghts)
                  if(len > 0) {
                        painter->setPen(QPen(Qt::lightGray, 1));
                        painter->drawLine(0.0, -2, len, -2);      // -2: 2 rast. un. above digits
                        }
            }
      if(editMode())
            Text::draw(painter);
      else {
            if(items.size() < 1)
                  Text::draw(painter);
            else
                  foreach(FiguredBassItem item, items) {
                        painter->translate(item.pos());
                        item.draw(painter);
                        painter->translate(-item.pos());
                        }
            }
/* DEBUG
      QString str = QString();
      str.setNum(_ticks);
      painter->drawText(0, (_onNote ? 40 : 30), str);
*/
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void FiguredBass::endEdit()
      {
      int         idx;

      Text::endEdit();
      QString txt = getText();
      if(txt.isEmpty()) {                       // if no text, nothing to do
            return;
            }

      // split text into lines and create an item for each line
      QStringList list = txt.split('\n', QString::SkipEmptyParts);
      items.clear();
      QString normalizedText = QString();
      idx = 0;
      foreach(QString str, list) {
            FiguredBassItem* pItem = new FiguredBassItem(score(), idx++);
            if(!pItem->parse(str)) {            // if any item fails parsing
                  items.clear();                // clear item list
                  Text::layout();               // keeping text as entered by user
                  return;
                  }
            pItem->setTrack(track());
            pItem->setParent(this);
            items.append(*pItem);

            // add item normalized text
            if(!normalizedText.isEmpty())
                  normalizedText.append('\n');
            normalizedText.append(pItem->normalizedText());
            }
      setText(normalizedText);                  // if all items parsed, replaced entered text with normal. text
      layout();
      }

//---------------------------------------------------------
//   setSelected /setVisible
//
//    forward flags to items
//---------------------------------------------------------

void FiguredBass::setSelected(bool flag)
      {
      Element::setSelected(flag);
      for(int i=0; i < items.size(); i++) {
            items[i].setSelected(flag);
            }
      }

void FiguredBass::setVisible(bool flag)
      {
      Element::setVisible(flag);
      for(int i=0; i < items.size(); i++) {
            items[i].setVisible(flag);
            }
      }

//---------------------------------------------------------
//   STATIC FUNCTION
//    adding a new FiguredBass to a Segment;
//    the main purpose of this function is to ensure that ONLY ONE F.b. element exists for each Segment/staff;
//    it either re-uses an existing FiguredBass or creates a new one if none if found;
//    returns the FiguredBass and sets pNew to true if it has been newly created.
//
//    Sets an initial duration of the element up to the next ChordRest of the same staff.
//
//    As the F.b. very concept requires the base chord to have ONLY ONE note,
//    FiguredBass elements are created and looked for only in the first track of the staff.
//---------------------------------------------------------

FiguredBass * FiguredBass::addFiguredBassToSegment(Segment * seg, int track, int extTicks, bool * pNew)
      {
      int         endTick;                      // where this FB is initially assumed to end
      int         staff = track / VOICES;       // convert track to staff
      track = staff * VOICES;                   // first track for this staff

      // scan segment annotations for an existing FB element in the same staff
      const QList<Element*>& annot = seg->annotations();
      int i;
      int count = annot.size();
      FiguredBass* fb;
      for(i = 0; i < count; i++) {
            if(annot.at(i)->type() == FIGURED_BASS && (annot.at(i)->track() / VOICES) == staff) {
                  // an FB already exists in segment: re-use it
                  fb = static_cast<FiguredBass*>(annot.at(i));
                  *pNew = false;
                  endTick = seg->tick() + fb->ticks();
                  break;
                  }
            }
      if(i >= count) {                          // no FB at segment: create new
            fb = new FiguredBass(seg->score());
            fb->setTrack(track);
            fb->setParent(seg);

            // locate next SegChordRest in the same staff to estimate presumed duration of element
            endTick = INT_MAX;
            Segment *   nextSegm;
            for (int iVoice = 0; iVoice < VOICES; iVoice++) {
                  nextSegm = seg->nextCR(track + iVoice);
                  if(nextSegm && nextSegm->tick() < endTick)
                        endTick = nextSegm->tick();
                  }
            if(endTick == INT_MAX) {            // no next segment: set up to score end
                  Measure * meas = seg->score()->lastMeasure();
                  endTick = meas->tick() + meas->ticks();
                  }
            fb->setTicks(endTick - seg->tick());

            // set onNote status
            fb->setOnNote(false);               // assume not onNote
            for(i = track; i < track + VOICES; i++)         // if segment has chord in staff, set onNote
                  if(seg->element(i) && seg->element(i)->type() == CHORD) {
                        fb->setOnNote(true);
                        break;
                  }
            *pNew = true;
            }

      // if we are extending a previous FB
      if(extTicks > 0) {
            // locate previous FB for same staff
            Segment *         prevSegm;
            FiguredBass*      prevFB = 0;
            for(prevSegm = seg->prev1(Segment::SegChordRest); prevSegm; prevSegm = prevSegm->prev1(Segment::SegChordRest)) {
                  const QList<Element*>& annot = prevSegm->annotations();
                  count = annot.size();
                  for(i = 0; i < count; i++) {
                        if(annot.at(i)->type() == FIGURED_BASS && (annot.at(i)->track() ) == track) {
                              prevFB = static_cast<FiguredBass*>(annot[i]);   // previous FB found
                              break;
                              }
                        }
                  if(prevFB) {
                        // if previous FB did not stop more than extTicks before this FB...
                        int delta = seg->tick() - prevFB->segment()->tick();
                        if(prevFB->ticks() + extTicks >= delta)
                              prevFB->setTicks(delta);      // update prev FB ticks to last up to this FB
                        break;
                        }
                  }
            }
      return fb;
      }

//---------------------------------------------------------
//   STATIC FUNCTIONS FOR FONT CONFIGURATION MANAGEMENT
//---------------------------------------------------------

bool FiguredBassFont::read(const QDomElement &de)
{
      for (QDomElement e = de.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if(val.size() < 1)
                  return false;

            if (tag == "family")
                  family = val;
            else if(tag == "displayName")
                  displayName = val;
            else if(tag == "defaultPitch")
                  defPitch = val.toDouble();
            else if(tag == "defaultLineHeight")
                  defLineHeight = val.toDouble();
            else if(tag == "parenthesisRoundOpen")
                  displayParenthesis[1] = val[0];
            else if(tag == "parenthesisRoundClosed")
                  displayParenthesis[2] = val[0];
            else if(tag == "parenthesisSquareOpen")
                  displayParenthesis[3] = val[0];
            else if(tag == "parenthesisSquareClosed")
                  displayParenthesis[4] = val[0];
            else if(tag == "doubleflat")
                  displayAccidental[1] = val[0];
            else if(tag == "flat")
                  displayAccidental[2] = val[0];
            else if(tag == "natural")
                  displayAccidental[3] = val[0];
            else if(tag == "sharp")
                  displayAccidental[4] = val[0];
            else if(tag == "doublesharp")
                  displayAccidental[5] = val[0];
            else if(tag == "digit") {
                  int digit = e.attribute("value").toInt();
                  if(digit < 1 || digit > 9)
                        return false;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        const QString& val(ee.text());
                        if(val.size() < 1)
                              return false;
                        if (tag == "simple")
                              displayDigit[0][digit][0] = val[0];
                        else if (tag == "crossed")
                              displayDigit[0][digit][1] = val[0];
                        else if (tag == "backslashed")
                              displayDigit[0][digit][2] = val[0];
                        else if (tag == "slashed")
                              displayDigit[0][digit][3] = val[0];
                        else if (tag == "simpleHistoric")
                              displayDigit[1][digit][0] = val[0];
                        else if (tag == "crossedHistoric")
                              displayDigit[1][digit][1] = val[0];
                        else if (tag == "backslashedHistoric")
                              displayDigit[1][digit][2] = val[0];
                        else if (tag == "slashedHistoric")
                              displayDigit[1][digit][3] = val[0];
                        else {
                              domError(ee);
                              return false;
                              }
                        }
                  }
            else {
                  domError(e);
                  return false;
                  }
            }
      return true;
}

//---------------------------------------------------------
//   Read Configuration File
//
//    reads a confoiguration and appends read data to g_FBFonts
//    resets everythings and reads the built-in config file if fileName is null or empty
//---------------------------------------------------------

bool FiguredBass::readConfigFile(const QString& fileName)
{
      QString     path;

      if(fileName == 0 || fileName.isEmpty()) {       // defaults to built-in xml
#ifdef Q_WS_IOS
            {
            extern QString resourcePath();
            QString rpath = resourcePath();
            path = rpath + QString("/fonts_figuredbass.xml");
            }
#else
            path = ":/fonts/fonts_figuredbass.xml";
#endif
            g_FBFonts.clear();
            }
      else
            path = fileName;

      QFileInfo fi(path);
      QFile f(path);

      if (!fi.exists() || !f.open(QIODevice::ReadOnly)) {
            QString s = QT_TRANSLATE_NOOP("file", "cannot open figured bass description:\n%1\n%2");
            MScore::lastError = s.arg(f.fileName()).arg(f.errorString());
qDebug("FiguredBass::read failed: <%s>\n", qPrintable(path));
            return false;
            }
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString s = QT_TRANSLATE_NOOP("file", "error reading figured bass font description %1 at line %2 column %3: %4\n");
            MScore::lastError = s.arg(f.fileName()).arg(line).arg(column).arg(err);
            return false;
            }
      docName = f.fileName();

      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  // QString version = e.attribute(QString("version"));
                  // QStringList sl = version.split('.');
                  // int _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();

                  for (QDomElement de = e.firstChildElement(); !de.isNull();  de = de.nextSiblingElement()) {
                        const QString& tag(de.tagName());
                        if (tag == "font") {
                              FiguredBassFont f;
                              if(f.read(de))
                                    g_FBFonts.append(f);
                              else
                                    return false;
                              }
                        else
                              domError(de);
                        }
                  return true;
                  }
            }
      return false;
}

//---------------------------------------------------------
//   Get Font Names
//
//    returns a list of display names for the fonts  configured to work with Figured Bass;
//    the index of a name in the list can be used to retrieve the font data with fontData()
//---------------------------------------------------------

QList<QString> FiguredBass::fontNames()
      {
      QList<QString> names;
      foreach(const FiguredBassFont& f, g_FBFonts)
            names.append(f.displayName);
      return names;
      }

//---------------------------------------------------------
//   Get Font Data
//
//    retrieves data about a Figured Bass font.
//    returns: true if idx is valid | false if it is not
// any of the pointer parameter can be null, if that datum is not needed
//---------------------------------------------------------

bool FiguredBass::fontData(int nIdx, QString * pFamily, QString * pDisplayName,
            qreal * pSize, qreal * pLineHeight)
{
      if(nIdx >= 0 && nIdx < g_FBFonts.size()) {
            FiguredBassFont f = g_FBFonts.at(nIdx);
            if(pFamily)       *pFamily          = f.family;
            if(pDisplayName)  *pDisplayName     = f.displayName;
            if(pSize)         *pSize            = f.defPitch;
            if(pLineHeight)   *pLineHeight      = f.defLineHeight;
            return true;
      }
      return false;
}

//---------------------------------------------------------
//   Read MusicXML
//
// Set the FiguredBass state based on the MusicXML <figured-bass> node de.
// Note that onNote and ticks must be set by the MusicXML importer,
// as the required context is not present in the items DOM tree.
// Exception: if a <duration> element is present, tick can be set.
//---------------------------------------------------------

void FiguredBass::readMusicXML(const QDomElement& de, int divisions)
      {
      bool parentheses = (de.attribute("parentheses") == "yes");
      QString normalizedText;
      int idx = 0;
      for (QDomElement e = de.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "duration") {
                  bool ok = true;
                  int duration = val.toInt(&ok);
                  if (ok) {
                        duration *= MScore::division;
                        duration /= divisions;
                        setTicks(duration);
                        }
                  else
                        qDebug("MusicXml-Import: bad duration value: <%s>",
                               qPrintable(val));
                  }
            else if (tag == "figure") {
                  FiguredBassItem * pItem = new FiguredBassItem(score(), idx++);
                  pItem->setTrack(track());
                  pItem->setParent(this);
                  pItem->readMusicXML(e, parentheses);
                  items.append(*pItem);
                  // add item normalized text
                  if(!normalizedText.isEmpty())
                        normalizedText.append('\n');
                  normalizedText.append(pItem->normalizedText());
                  }
            else
                  domError(e);
            }
      setText(normalizedText);                  // this is the text to show while editing
      }

//---------------------------------------------------------
//   hasParentheses
//
//   return true if any FiguredBassItem starts with a parenthesis
//---------------------------------------------------------

bool FiguredBass::hasParentheses() const
      {
      foreach(FiguredBassItem item, items)
            if (item.startsWithParenthesis())
                  return true;
      return false;
      }

//---------------------------------------------------------
//   Write MusicXML
//---------------------------------------------------------

void FiguredBass::writeMusicXML(Xml& xml) const
      {
      QString stag = "figured-bass";
      if (hasParentheses())
            stag += " parentheses=\"yes\"";
      xml.stag(stag);
      foreach(FiguredBassItem item, items)
            item.writeMusicXML(xml);
      xml.etag();
      }

//---------------------------------------------------------
//
// METHODS BELONGING TO OTHER CLASSES
//
//    Work In Progress: kept here until the FiguredBass framwork is reasonably set up;
//    To be finally moved to their respective class implementation files.
//
//---------------------------------------------------------

//---------------------------------------------------------
//   Score::addFiguredBass
//    called from Keyboard Accelerator & menus
//---------------------------------------------------------

#include "score.h"

FiguredBass* Score::addFiguredBass()
      {
      Element* el = selection().element();
      if (el == 0 || (el->type() != NOTE && el->type() != FIGURED_BASS)) {
            QMessageBox::information(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("No note or figured bass selected:\n"
                  "Please select a single note or figured bass and retry.\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
            return 0;
            }

      FiguredBass * fb;
      bool bNew;
      if (el->type() == NOTE) {
            ChordRest * cr = static_cast<Note*>(el)->chord();
            fb = FiguredBass::addFiguredBassToSegment(cr->segment(),
                        (cr->track() / VOICES) * VOICES, 0, &bNew);
            }
      else if (el->type() == FIGURED_BASS) {
            fb = static_cast<FiguredBass*>(el);
            bNew = false;
            }
      else
            return 0;

      if(fb == 0)
            return 0;

      if(bNew)
            undoAddElement(fb);
      select(fb, SELECT_SINGLE, 0);
      return fb;
      }

