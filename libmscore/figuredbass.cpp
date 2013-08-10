//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "chord.h"
#include "rest.h"
#include "score.h"

namespace Ms {

// the array of configured fonts
static QList<FiguredBassFont> g_FBFonts;

//---------------------------------------------------------
//   F I G U R E D   B A S S   I T E M
//---------------------------------------------------------

// used for indexed access to parenthesis chars
// (these is no normAccidToChar[], as accidentals may use mult. chars in normalized display):
const QChar FiguredBassItem::normParenthToChar[NumOfParentheses] =
{ 0, '(', ')', '[', ']'};


FiguredBassItem::FiguredBassItem(Score* s, int l)
      : Element(s), ord(l)
      {
      _prefix     = _suffix = ModifierNone;
      _digit      = FBIDigitNone;
      parenth[0]  = parenth[1] = parenth[2] = parenth[3] = parenth[4] = ParenthesisNone;
      _contLine   = ContLineNone;
      }

FiguredBassItem::FiguredBassItem(const FiguredBassItem& item)
      : Element(item)
      {
      ord         = item.ord;
      _prefix     = item._prefix;
      _digit      = item._digit;
      _suffix     = item._suffix;
      parenth[0]  = item.parenth[0];
      parenth[1]  = item.parenth[1];
      parenth[2]  = item.parenth[2];
      parenth[3]  = item.parenth[3];
      parenth[4]  = item.parenth[4];
      _contLine   = item._contLine;
      textWidth   = item.textWidth;
      _displayText= item._displayText;
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
      _contLine = ContLineNone;                       // contLine
      if(str[0] == '-' || str[0] == '_') {            // 1 symbol: simple continuation
            _contLine = ContLineSimple;
            str.remove(0, 1);
      }
      while(str[0] == '-' || str[0] == '_') {         // more than 1 symbol: extended continuation
            _contLine = ContLineExtended;
            str.remove(0, 1);
      }
      parseParenthesis(str, 4);

      // remove useless parentheses, moving external parentheses toward central digit element
      if(_prefix == ModifierNone && parenth[1] == ParenthesisNone) {
            parenth[1] = parenth[0];
            parenth[0] = ParenthesisNone;
            }
      if(_digit == FBIDigitNone && parenth[2] == ParenthesisNone) {
            parenth[2] = parenth[1];
            parenth[1] = ParenthesisNone;
            }
      if(_contLine == ContLineNone && parenth[3] == ParenthesisNone) {
            parenth[3] = parenth[4];
            parenth[4] = ParenthesisNone;
            }
      if(_suffix == ModifierNone && parenth[2] == ParenthesisNone) {
            parenth[2] = parenth[3];
            parenth[3] = ParenthesisNone;
            }

      // some checks:
      // if some extra input, str is not conformant
      if(str.size())
            return false;
      // can't have BOTH prefix and suffix
      // prefix, digit, suffix and cont.line cannot be ALL empty
      // suffix cannot combine with empty digit
      if( (_prefix != ModifierNone && _suffix != ModifierNone)
            || (_prefix == ModifierNone && _digit == FBIDigitNone && _suffix == ModifierNone && _contLine == ContLineNone)
            || ( (_suffix == ModifierCross || _suffix == ModifierBackslash || _suffix == ModifierSlash)
                  && _digit == FBIDigitNone) )
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
      Modifier *  dest  = bPrefix ? &_prefix : &_suffix;
      bool        done  = false;
      int         size  = str.size();
      str = str.trimmed();

      *dest       = ModifierNone;

      while(str.size()) {
            switch(str.at(0).unicode())
            {
            case 'b':
                  if(*dest != ModifierNone) {
                        if(*dest == ModifierFlat)     // FLAT may double a previous FLAT
                              *dest = ModifierDoubleFlat;
                        else
                              return -1;              // but no other combination is acceptable
                        }
                  *dest = ModifierFlat;
                  break;
            case 'h':
                  if(*dest != ModifierNone)           // cannot combine with any other accidental
                        return -1;
                  *dest = ModifierNatural;
                  break;
            case '#':
                  if(*dest != ModifierNone) {
                        if(*dest == ModifierSharp)    // SHARP may double a preivous SHARP
                              *dest = ModifierDoubleSharp;
                        else
                              return -1;              // but no other combination is acceptable
                        }
                  *dest = ModifierSharp;
                  break;
            case '+':
                  // accept '+' as both a prefix and a suffix for harmony notation
                  if(*dest != ModifierNone)           // cannot combine with any other accidental
                        return -1;
                  *dest = ModifierCross;
                  break;
            // '\\' and '/' go into the suffix
            case '\\':
                  if(_suffix != ModifierNone)         // cannot combine with any other accidental
                        return -1;
                  _suffix = ModifierBackslash;
                  break;
            case '/':
                  if(_suffix != ModifierNone)         // cannot combine with any other accidental
                        return -1;
                  _suffix = ModifierSlash;
                  break;
            default:                                 // any other char: no longer in prefix/suffix
                  done = true;
                  break;
            }
            if(done)
                  break;
            str.remove(0,1);                         // 'eat' the char and continue
            }

      return size - str.size();                      // return how many chars we had read into prefix/suffix
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

      _digit = FBIDigitNone;

      while(str.size()) {
            // any digit acceptable, if no previous digit
            if(str[0] >= '1' && str[0] <= '9') {
                  if(_digit == FBIDigitNone) {
                        _digit = str[0].unicode() - '0';
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
      Parenthesis code = ParenthesisNone;
      switch(c)
      {
      case '(':
            code =ParenthesisRoundOpen;
            break;
      case ')':
            code =ParenthesisRoundClosed;
            break;
      case '[':
            code =ParenthesisSquaredOpen;
            break;
      case ']':
            code =ParenthesisSquaredClosed;
            break;
      default:
            break;
            }
      parenth[parenthIdx] = code;
      if(code != ParenthesisNone) {
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
      if(parenth[0] != ParenthesisNone)
            str.append(normParenthToChar[parenth[0]]);

      if(_prefix != ModifierNone) {
            switch(_prefix)
            {
            case ModifierFlat:
                  str.append('b');
                  break;
            case ModifierNatural:
                  str.append('h');
                  break;
            case ModifierSharp:
                  str.append('#');
                  break;
            case ModifierCross:
                  str.append('+');
                  break;
            case ModifierDoubleFlat:
                  str.append("bb");
                  break;
            case ModifierDoubleSharp:
                  str.append("##");
                  break;
            default:
                  break;
            }
            }

      if(parenth[1] != ParenthesisNone)
            str.append(normParenthToChar[parenth[1]]);

      // digit
      if(_digit != FBIDigitNone)
            str.append(QChar('0' + _digit));

      if(parenth[2] != ParenthesisNone)
            str.append(normParenthToChar[parenth[2]]);

      // suffix
      if(_suffix != ModifierNone) {
            switch(_suffix)
            {
            case ModifierFlat:
                  str.append('b');
                  break;
            case ModifierNatural:
                  str.append('h');
                  break;
            case ModifierSharp:
                  str.append('#');
                  break;
            case ModifierCross:
                  str.append('+');
                  break;
            case ModifierBackslash:
                  str.append('\\');
                  break;
            case ModifierSlash:
                  str.append('/');
                  break;
            case ModifierDoubleFlat:
                  str.append("bb");
                  break;
            case ModifierDoubleSharp:
                  str.append("##");
                  break;
            default:
                  break;
            }
            }

      if(parenth[3] != ParenthesisNone)
            str.append(normParenthToChar[parenth[3]]);
      if(_contLine > ContLineNone) {
            str.append('_');
            if (_contLine > ContLineSimple)
                  str.append('_');
            }
      if(parenth[4] != ParenthesisNone)
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
      if(_prefix != ModifierNone)
            xml.tag(QString("prefix"), _prefix);
      if(_digit != FBIDigitNone)
            xml.tag(QString("digit"), _digit);
      if(_suffix != ModifierNone)
            xml.tag(QString("suffix"), _suffix);
      if(_contLine)
            xml.tag("continuationLine", _contLine);
      xml.etag();
}

//---------------------------------------------------------
//   FiguredBassItem read()
//---------------------------------------------------------

void FiguredBassItem::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "brackets") {
                  parenth[0] = (Parenthesis)e.intAttribute("b0");
                  parenth[1] = (Parenthesis)e.intAttribute("b1");
                  parenth[2] = (Parenthesis)e.intAttribute("b2");
                  parenth[3] = (Parenthesis)e.intAttribute("b3");
                  parenth[4] = (Parenthesis)e.intAttribute("b4");
                  e.readNext();
                  }
            else if (tag == "prefix")
                  _prefix = (Modifier)(e.readInt());
            else if (tag == "digit")
                  _digit = e.readInt();
            else if (tag == "suffix")
                  _suffix = (Modifier)(e.readInt());
            else if(tag == "continuationLine")
                  _contLine = (ContLine)(e.readInt());
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   FiguredBassItem layout()
//    creates the display text (set as element text) and computes
//    the horiz. offset needed to align the right part as well as the vert. offset
//---------------------------------------------------------

void FiguredBassItem::layout()
      {
      qreal             h, w, x, x1, x2, y;

      // contruct font metrics
      int   fontIdx = 0;
      QFont f(g_FBFonts.at(fontIdx).family);
      // font size in points, scaled according to spatium()
      qreal m = score()->styleD(ST_figuredBassFontSize) * spatium() / ( SPATIUM20 * MScore::DPI);
      f.setPointSizeF(m);
      QFontMetrics      fm(f);

      QString           str = QString();
      x = symbols[score()->symIdx()][quartheadSym].width(magS()) * .5;
      x1 = x2 = 0.0;

      // create display text
      int font = 0;
      int style = score()->styleI(ST_figuredBassStyle);

      if(parenth[0] != ParenthesisNone)
            str.append(g_FBFonts.at(font).displayParenthesis[parenth[0]]);

      // prefix
      if(_prefix != ModifierNone) {
            // if no digit, the string created so far 'hangs' to the left of the note
            if(_digit == FBIDigitNone)
                  x1 = fm.width(str);
            str.append(g_FBFonts.at(font).displayAccidental[_prefix]);
            // if no digit, the string from here onward 'hangs' to the right of the note
            if(_digit == FBIDigitNone)
                  x2 = fm.width(str);
            }

      if(parenth[1] != ParenthesisNone)
            str.append(g_FBFonts.at(font).displayParenthesis[parenth[1]]);

      // digit
      if(_digit != FBIDigitNone) {
            // if some digit, the string created so far 'hangs' to the left of the note
            x1 = fm.width(str);
            // if suffix is a combining shape, combine it with digit
            // unless there is a parenthesis in between
            if( (_suffix == ModifierCross || _suffix == ModifierBackslash || _suffix == ModifierSlash)
                        && parenth[2] == ParenthesisNone)
                  str.append(g_FBFonts.at(font).displayDigit[style][_digit][_suffix-(ModifierCross-1)]);
            else
                  str.append(g_FBFonts.at(font).displayDigit[style][_digit][0]);
            // if some digit, the string from here onward 'hangs' to the right of the note
            x2 = fm.width(str);
            }

      if(parenth[2] != ParenthesisNone)
            str.append(g_FBFonts.at(font).displayParenthesis[parenth[2]]);

      // suffix
      // append only if non-combining shape or cannot combine (no digit or parenthesis in between)
      if( _suffix != ModifierNone
                  && ( (_suffix != ModifierCross && _suffix != ModifierBackslash && _suffix != ModifierSlash)
                        || _digit == FBIDigitNone
                        || parenth[2] != ParenthesisNone) )
            str.append(g_FBFonts.at(font).displayAccidental[_suffix]);

      if(parenth[3] != ParenthesisNone)
            str.append(g_FBFonts.at(font).displayParenthesis[parenth[3]]);

      setDisplayText(str);                // this text will be displayed

      if (str.size())                     // if some text
            x = x - (x1+x2) * 0.5;        // position the text so that [x1<-->x2] is centered below the note
      else                                // if no text (but possibly a line)
            x = 0;                        // start at note left margin
      // vertical position
      h = fm.lineSpacing();
      h *= score()->styleD(ST_figuredBassLineHeight);
      if (score()->styleI(ST_figuredBassAlignment) == 0)          // top alignment: stack down from first item
            y = h * ord;
      else                                                        // bottom alignment: stack up from last item
            y = -h * (figuredBass()->numOfItems() - ord);
      setPos(x, y);
      // determine bbox from text width
//      w = fm.width(str);
      w = fm.boundingRect(str).width();
      textWidth = w;
      // if there is a cont.line, extend width to cover the whole FB element duration line
      int lineLen;
      if(_contLine != ContLineNone && (lineLen=figuredBass()->lineLength(0)) > w)
            w = lineLen;
      bbox().setRect(0, 0, w, h);
      }

//---------------------------------------------------------
//   FiguredBassItem draw()
//---------------------------------------------------------

void FiguredBassItem::draw(QPainter* painter) const
      {
      int font = 0;
      // set font from general style
      QFont f(g_FBFonts.at(font).family);
#ifdef USE_GLYPHS
      f.setHintingPreference(QFont::PreferVerticalHinting);
#endif
      // font size in pixels, scaled according to spatium()
      qreal m = score()->styleD(ST_figuredBassFontSize) * MScore::DPI / PPI;
      m *= spatium() / (SPATIUM20 * MScore::DPI);     // make spatium dependent
      f.setPixelSize(lrint(m));

      painter->setFont(f);
      painter->setBrush(Qt::NoBrush);
      painter->setPen(figuredBass()->curColor());
      painter->drawText(bbox(), Qt::TextDontClip | Qt::AlignLeft | Qt::AlignTop, displayText());

      // continuation line
      qreal lineEndX = 0.0;
      if (_contLine > ContLineNone) {
            qreal _spatium = spatium();
            qreal lineStartX   = textWidth;                       // by default, line starts right after text
            if (lineStartX > 0.0)
                  lineStartX += _spatium * 0.1;                   // if some text, give some room after it
            lineEndX = figuredBass()->printedLineLength();        // by default, line ends
            if(lineEndX - lineStartX < 1.0)                       // if line length < 1 sp, ignore it
                  lineEndX = 0.0;

            // if extended cont.line and no closing parenthesis: look at next FB element
            if (_contLine > ContLineSimple && parenth[4] == ParenthesisNone) {
                  FiguredBass * nextFB;
                  // if there is a contiguous FB element
                  if ( (nextFB=figuredBass()->nextFiguredBass()) != 0) {
                        // retrieve the X position (in page coords) of a possible cont. line of nextFB
                        // on the same line of 'this'
                        QPointF pgPos = pagePos();
                        qreal nextContPageX = nextFB->additionalContLineX(pgPos.y());
                        // if an additional cont. line has been found, extend up to its initial X coord
                        if (nextContPageX > 0)
                              lineEndX = nextContPageX - pgPos.x() + _spatium*0.125;  // with a little bit of overlap
                        else
                              lineEndX = figuredBass()->lineLength(0);              // if none found, draw to the duration end
                        }
                  }
            // if some line, draw it
            if (lineEndX > 0.0) {
                  qreal h = bbox().height() * 0.875;
                  painter->drawLine(lineStartX, h, lineEndX - ipos().x(), h);
                  }
            }

      // closing cont.line parenthesis
      if(parenth[4] != ParenthesisNone) {
            int x = lineEndX > 0.0 ? lineEndX : textWidth;
            painter->drawText(QRectF(x, 0, bbox().width(), bbox().height()), Qt::AlignLeft | Qt::AlignTop,
                  g_FBFonts.at(font).displayParenthesis[parenth[4]]);
            }
      }

//---------------------------------------------------------
//   PROPERTY METHODS
//---------------------------------------------------------

QVariant FiguredBassItem::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_FBPREFIX:
                  return _prefix;
            case P_FBDIGIT:
                  return _digit;
            case P_FBSUFFIX:
                  return _suffix;
            case P_FBCONTINUATIONLINE:
                  return _contLine;
            case P_FBPARENTHESIS1:
                  return parenth[0];
            case P_FBPARENTHESIS2:
                  return parenth[1];
            case P_FBPARENTHESIS3:
                  return parenth[2];
            case P_FBPARENTHESIS4:
                  return parenth[3];
            case P_FBPARENTHESIS5:
                  return parenth[4];
            default:
                  return Element::getProperty(propertyId);
            }
      }

bool FiguredBassItem::setProperty(P_ID propertyId, const QVariant& v)
      {
      score()->addRefresh(canvasBoundingRect());
      int   val = v.toInt();
      switch(propertyId) {
            case P_FBPREFIX:
                  if(val < ModifierNone || val > ModifierCross)
                        return false;
                  _prefix = (Modifier)val;
                  break;
            case P_FBDIGIT:
                  if(val < 1 || val > 9)
                        return false;
                  _digit = val;
                  break;
            case P_FBSUFFIX:
                  if(val < ModifierNone || val >= NumOfModifiers)
                        return false;
                  _suffix = (Modifier)val;
                  break;
            case P_FBCONTINUATIONLINE:
                  _contLine = (ContLine)val;
                  break;
            case P_FBPARENTHESIS1:
                  if(val < ParenthesisNone || val > ParenthesisSquaredClosed)
                        return false;
                  parenth[0] = (Parenthesis)val;
                  break;
            case P_FBPARENTHESIS2:
                  if(val < ParenthesisNone || val > ParenthesisSquaredClosed)
                        return false;
                  parenth[1] = (Parenthesis)val;
                  break;
            case P_FBPARENTHESIS3:
                  if(val < ParenthesisNone || val > ParenthesisSquaredClosed)
                        return false;
                  parenth[2] = (Parenthesis)val;
                  break;
            case P_FBPARENTHESIS4:
                  if(val < ParenthesisNone || val > ParenthesisSquaredClosed)
                        return false;
                  parenth[3] = (Parenthesis)val;
                  break;
            case P_FBPARENTHESIS5:
                  if(val < ParenthesisNone || val > ParenthesisSquaredClosed)
                        return false;
                  parenth[4] = (Parenthesis)val;
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      score()->setLayoutAll(true);
      return true;
      }

QVariant FiguredBassItem::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_FBPREFIX:
            case P_FBSUFFIX:
                  return ModifierNone;
            case P_FBDIGIT:
                  return FBIDigitNone;
            case P_FBCONTINUATIONLINE:
                  return false;
            default:
                  return Element::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   UNDOABLE PROPERTY SETTERS
//---------------------------------------------------------

void FiguredBassItem::undoSetPrefix(Modifier pref)
      {
      if(pref <= ModifierCross) {
            score()->undoChangeProperty(this, P_FBPREFIX, (int)pref);
            // if setting some prefix and there is a suffix already, clear suffix
            if(pref != ModifierNone && _suffix != ModifierNone)
                  score()->undoChangeProperty(this, P_FBSUFFIX, ModifierNone);
            layout();                     // re-generate displayText
            }
      }

void FiguredBassItem::undoSetDigit(int digit)
      {
      if(digit >= 0 && digit <= 9) {
            score()->undoChangeProperty(this, P_FBDIGIT, digit);
            layout();                     // re-generate displayText
            }
      }

void FiguredBassItem::undoSetSuffix(Modifier suff)
      {
      score()->undoChangeProperty(this, P_FBSUFFIX, suff);
      // if setting some suffix and there is a prefix already, clear prefix
      if(suff != ModifierNone && _prefix != ModifierNone)
            score()->undoChangeProperty(this, P_FBPREFIX, ModifierNone);
      layout();                     // re-generate displayText
      }

void FiguredBassItem::undoSetContLine(bool val)
      {
      score()->undoChangeProperty(this, P_FBCONTINUATIONLINE, val);
      layout();                     // re-generate displayText
      }

void FiguredBassItem::undoSetParenth1(Parenthesis par)
      {
      score()->undoChangeProperty(this, P_FBPARENTHESIS1, par);
      layout();                     // re-generate displayText
      }
void FiguredBassItem::undoSetParenth2(Parenthesis par)
      {
      score()->undoChangeProperty(this, P_FBPARENTHESIS2, par);
      layout();                     // re-generate displayText
      }
void FiguredBassItem::undoSetParenth3(Parenthesis par)
      {
      score()->undoChangeProperty(this, P_FBPARENTHESIS3, par);
      layout();                     // re-generate displayText
      }
void FiguredBassItem::undoSetParenth4(Parenthesis par)
      {
      score()->undoChangeProperty(this, P_FBPARENTHESIS4, par);
      layout();                     // re-generate displayText
      }
void FiguredBassItem::undoSetParenth5(Parenthesis par)
      {
      score()->undoChangeProperty(this, P_FBPARENTHESIS5, par);
      layout();                     // re-generate displayText
      }

//---------------------------------------------------------
//
//    MusicXML I/O
//
//---------------------------------------------------------

//---------------------------------------------------------
//   Convert MusicXML prefix/suffix to Modifier
//---------------------------------------------------------

// TODO add missing non-accidental types

FiguredBassItem::Modifier FiguredBassItem::MusicXML2Modifier(const QString prefix) const
      {
      if (prefix == "sharp")
            return ModifierSharp;
      else if (prefix == "flat")
            return ModifierFlat;
      else if (prefix == "natural")
            return ModifierNatural;
      else if (prefix == "double-sharp")
            return ModifierDoubleSharp;
      else if (prefix == "flat-flat")
            return ModifierDoubleFlat;
      else if (prefix == "sharp-sharp")
            return ModifierDoubleSharp;
      else if (prefix == "slash")
            return ModifierSlash;
      else
            return ModifierNone;
      }

//---------------------------------------------------------
//   Convert Modifier to MusicXML prefix/suffix
//---------------------------------------------------------

// TODO add missing non-accidental types

QString FiguredBassItem::Modifier2MusicXML(FiguredBassItem::Modifier prefix) const
      {
      switch (prefix) {
            case ModifierNone:        return "";
            case ModifierDoubleFlat:  return "flat-flat";
            case ModifierFlat:        return "flat";
            case ModifierNatural:     return "natural";
            case ModifierSharp:       return "sharp";
            case ModifierDoubleSharp: return "double-sharp";
            case ModifierCross:       return ""; // TODO TBD
            case ModifierBackslash:   return ""; // TODO TBD
            case ModifierSlash:       return "slash";
            case NumOfModifiers:      return ""; // prevent gcc "‘FBINumOfAccid’ not handled in switch" warning
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

#if 0
void FiguredBassItem::readMusicXML(XmlReader& e, bool paren, bool& extend)
      {
      // read the <figure> node de
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "extend")
                  extend = true;
            else if (tag == "figure-number") {
                  // MusicXML spec states figure-number is a number
                  // MuseScore can only handle single digit
                  int iVal = e.readInt();
                  if (1 <= iVal && iVal <= 9)
                        _digit = iVal;
                  }
            else if (tag == "prefix")
                  _prefix = MusicXML2Modifier(e.readElementText());
            else if (tag == "suffix")
                  _suffix = MusicXML2Modifier(e.readElementText());
            else
                  e.unknown();
            }
      // set parentheses
      if (paren) {
            // parenthesis open
            if (_prefix != ModifierNone)
                  parenth[0] = ParenthesisRoundOpen; // before prefix
            else if (_digit != FBIDigitNone)
                  parenth[1] = ParenthesisRoundOpen; // before digit
            else if (_suffix != ModifierNone)
                  parenth[2] = ParenthesisRoundOpen; // before suffix
            // parenthesis close
            if (_suffix != ModifierNone)
                  parenth[3] = ParenthesisRoundClosed; // after suffix
            else if (_digit != FBIDigitNone)
                  parenth[2] = ParenthesisRoundClosed; // after digit
            else if (_prefix != ModifierNone)
                  parenth[1] = ParenthesisRoundClosed; // after prefix
            }
      }
#endif

//---------------------------------------------------------
//   Write MusicXML
//---------------------------------------------------------

void FiguredBassItem::writeMusicXML(Xml& xml, bool doFigure, bool doExtend) const
      {
      xml.stag("figure");
      if (doFigure) {
            QString strPrefix = Modifier2MusicXML(_prefix);
            if (strPrefix != "")
                  xml.tag("prefix", strPrefix);
            if (_digit != FBIDigitNone)
                  xml.tag("figure-number", _digit);
            QString strSuffix = Modifier2MusicXML(_suffix);
            if (strSuffix != "")
                  xml.tag("suffix", strSuffix);
            }
      if (doExtend) {
            xml.tagE("extend");
            }
      xml.etag();
      }

//---------------------------------------------------------
//   startsWithParenthesis
//---------------------------------------------------------

bool FiguredBassItem::startsWithParenthesis() const
      {
      if (_prefix != ModifierNone)
            return (parenth[0] != ParenthesisNone);
      if (_digit != FBIDigitNone)
            return (parenth[1] != ParenthesisNone);
      if (_suffix != ModifierNone)
            return (parenth[2] != ParenthesisNone);
      return false;
      }

//---------------------------------------------------------
//   F I G U R E D   B A S S
//---------------------------------------------------------

FiguredBass::FiguredBass(Score* s)
   : Text(s)
      {
      setOnNote(true);
      setTextStyleType(TEXT_STYLE_FIGURED_BASS);
      TextStyle st("Figured Bass", g_FBFonts[0].family, score()->styleD(ST_figuredBassFontSize),
                  false, false, false, ALIGN_LEFT | ALIGN_TOP, QPointF(0, score()->styleD(ST_figuredBassYOffset)), OFFSET_SPATIUM);
      st.setSizeIsSpatiumDependent(true);
      setTextStyle(st);
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
      // if unparseable items or non-standard style, write full text data
      if (items.size() < 1 || textStyleType() != TEXT_STYLE_FIGURED_BASS) {
            if (items.size() < 1 || textStyleType() == TEXT_STYLE_UNSTYLED)
                  Text::writeProperties(xml, true);
            else
                  // if all items parsed and not unstled, we simnply have a special style: write it
                  xml.tag("style", textStyle().name());
            }
      foreach(FiguredBassItem item, items)
            item.write(xml);
      if (textStyleType() != TEXT_STYLE_UNSTYLED)
            Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FiguredBass::read(XmlReader& e)
      {
      QString normalizedText;
      int idx = 0;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "ticks")
                  setTicks(e.readInt());
            else if (tag == "onNote")
                  setOnNote(e.readInt() != 0l);
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
//            else if (tag == "style")
//                  setTextStyleType(e.readElementText());
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      // if items could be parsed and text is styled, set normalized text
      if(items.size() > 0 && textStyleType() != TEXT_STYLE_UNSTYLED)
            setText(normalizedText);            // this is the text to show while editing
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FiguredBass::layout()
      {
//      qreal       y;

      // if 'our' style, force 'our' style data from FiguredBass parameters
      if(textStyleType() == TEXT_STYLE_FIGURED_BASS) {
            TextStyle st("Figured Bass", g_FBFonts[0].family, score()->styleD(ST_figuredBassFontSize),
                        false, false, false, ALIGN_LEFT | ALIGN_TOP, QPointF(0, score()->styleD(ST_figuredBassYOffset)),
                        OFFSET_SPATIUM);
            st.setSizeIsSpatiumDependent(true);
            setTextStyle(st);
            }
      Text::layout();                     // Text and/or SimpleText may expect some internal data to be setup

      // if style has been changed (or text not styled), do nothing else, keeping default laying out and formatting
      if(textStyleType() != TEXT_STYLE_FIGURED_BASS)
            return;

      // bounding box
      if(editMode()) {
            qreal             h, w, w1;
            QFontMetricsF     fm(textStyle().font(spatium()));
            // box width
            w = 0;
            QStringList list = text().split('\n');
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
//            setPos(0, y);
            bbox().setRect(0-2, 0-2, w+4, h+4);
            }
      else {
            // if element could be parsed into items, layout each element
            if(items.size() > 0) {
                  layoutLines();
                  bbox().setRect(0, 0, _lineLenghts.at(0), 0);
                  // layout each item and enlarge bbox to include items bboxes
                  for(int i=0; i < items.size(); i++) {
                        items[i].layout();
                        addbbox(items[i].bbox().translated(items[i].pos()));
                        }
                  }
            }

//      adjustReadPos();            // already taken into account by Text::layout()
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

      ChordRest * lastCR;                                   // the last ChordRest of this
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
            // locate the last ChordRest of this
            if (nextSegm)
                  lastCR = nextSegm->prev1()->nextChordRest(track(), true);
            }
      if (m == 0 || nextSegm == 0) {
            qDebug("FiguredBass layout: no segment found for tick %d\n", nextTick);
            goto NoLen;
            }

      // get length of printed lines from horiz. page position of lastCR
      // (enter a bit 'into' the ChordRest for clarity)
      _printedLineLength = lastCR->pageX() - pageX() + 1.5*spatium();

      // get duration indicator line(s) from page position of nextSegm
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
      // if in edit mode or with custom style, use default drawing
      if(editMode() || textStyleType() != TEXT_STYLE_FIGURED_BASS)
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
      QString txt = text();
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
      // if all items parsed and text is styled, replaced entered text with normalized text
      if(items.size() > 0 && textStyleType() != TEXT_STYLE_UNSTYLED) {
            setText(normalizedText);
            layout();
            }
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
//   nextFiguredBass
//
//    returns the next *contiguous* FiguredBass element if it exists,
//    i.e. the FiguredBass element which starts where 'this' ends
//    returns 0 if none
//---------------------------------------------------------

FiguredBass* FiguredBass::nextFiguredBass() const
      {
      if (_ticks <= 0)                                      // if _ticks unset, no clear idea of when 'this' ends
            return 0;
      Segment *   nextSegm;                                 // the Segment beyond this' segment
      int         nextTick = segment()->tick() + _ticks;    // the tick beyond this' duration

      // locate the ChordRest segment right after this' end
      nextSegm = score()->tick2segment(nextTick, true, Segment::SegChordRest);
      if (nextSegm == 0)
            return 0;

      // scan segment annotations for an existing FB element in the this' staff
      for (Element* e : nextSegm->annotations())
            if (e->type() == FIGURED_BASS && e->track() == track())
                  return static_cast<FiguredBass*>(e);

      return 0;
      }

//---------------------------------------------------------
//   additionalContLineX
//
//    if there is a continuation line, without other text elements, at pagePosY, returns its X coord (in page coords)
//    returns 0 if no cont.line there or if there are text elements before the cont.line
//
//    In practice, returns the X coord of a cont. line which can be the continuation of a previous cont. line
//
//    Note: pagePosY is the Y coord of the FiguredBassItem containing the line, not of the line itself,
//    as line position might depend on styles.
//---------------------------------------------------------

qreal FiguredBass::additionalContLineX(qreal pagePosY) const
{
      QPointF pgPos = pagePos();
      foreach (FiguredBassItem fbi, items)
            // if item has cont.line but nothing before it
            // and item Y coord near enough to pagePosY
            if(fbi.contLine()
                  && fbi.digit() == FBIDigitNone
                     && fbi.prefix() == FiguredBassItem::ModifierNone
                        && fbi.suffix() == FiguredBassItem::ModifierNone
                           && fbi.parenth4() == FiguredBassItem::ParenthesisNone
                              && qAbs(pgPos.y() + fbi.ipos().y() - pagePosY) < 0.05)
                  return pgPos.x() + fbi.ipos().x();

      return 0.0;                               // no suitable line
}

//---------------------------------------------------------
//   PROPERTY METHODS
//---------------------------------------------------------

QVariant FiguredBass::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            default:
                  return Text::getProperty(propertyId);
            }
      }

bool FiguredBass::setProperty(P_ID propertyId, const QVariant& v)
      {
      score()->addRefresh(canvasBoundingRect());
      switch(propertyId) {
            default:
                  return Text::setProperty(propertyId, v);
            }
      score()->setLayoutAll(true);
      return true;
      }

QVariant FiguredBass::propertyDefault(P_ID id) const
      {
      switch(id) {
            default:
                  return Text::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   TEMPORARY HACK!!!
//---------------------------------------------------------

FiguredBassItem * FiguredBass::addItem()
      {
      int line = items.size();
      FiguredBassItem fib(score(), line);
      items.append(fib);
      return &(items.last());
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
      FiguredBass* fb = 0;
      for (Element* e : seg->annotations()) {
            if (e->type() == FIGURED_BASS && (e->track() / VOICES) == staff) {
                  // an FB already exists in segment: re-use it
                  fb = static_cast<FiguredBass*>(e);
                  *pNew = false;
                  endTick = seg->tick() + fb->ticks();
                  break;
                  }
            }
      if (fb == 0) {                          // no FB at segment: create new
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
            for (int i = track; i < track + VOICES; i++)         // if segment has chord in staff, set onNote
                  if (seg->element(i) && seg->element(i)->type() == CHORD) {
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
                  for (Element* e : prevSegm->annotations()) {
                        if (e->type() == FIGURED_BASS && (e->track() ) == track) {
                              prevFB = static_cast<FiguredBass*>(e);   // previous FB found
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

bool FiguredBassFont::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "family")
                  family = e.readElementText();
            else if (tag == "displayName")
                  displayName = e.readElementText();
            else if (tag == "defaultPitch")
                  defPitch = e.readDouble();
            else if (tag == "defaultLineHeight")
                  defLineHeight = e.readDouble();
            else if (tag == "parenthesisRoundOpen")
                  displayParenthesis[1] = e.readElementText()[0];
            else if (tag == "parenthesisRoundClosed")
                  displayParenthesis[2] = e.readElementText()[0];
            else if (tag == "parenthesisSquareOpen")
                  displayParenthesis[3] = e.readElementText()[0];
            else if (tag == "parenthesisSquareClosed")
                  displayParenthesis[4] = e.readElementText()[0];
            else if (tag == "doubleflat")
                  displayAccidental[FiguredBassItem::ModifierDoubleFlat]= e.readElementText()[0];
            else if (tag == "flat")
                  displayAccidental[FiguredBassItem::ModifierFlat]      = e.readElementText()[0];
            else if (tag == "natural")
                  displayAccidental[FiguredBassItem::ModifierNatural]   = e.readElementText()[0];
            else if (tag == "sharp")
                  displayAccidental[FiguredBassItem::ModifierSharp]     = e.readElementText()[0];
            else if (tag == "doublesharp")
                  displayAccidental[FiguredBassItem::ModifierDoubleSharp]= e.readElementText()[0];
            else if (tag == "cross")
                  displayAccidental[FiguredBassItem::ModifierCross]     = e.readElementText()[0];
            else if (tag == "backslash")
                  displayAccidental[FiguredBassItem::ModifierBackslash] = e.readElementText()[0];
            else if (tag == "slash")
                  displayAccidental[FiguredBassItem::ModifierSlash]     = e.readElementText()[0];
            else if (tag == "digit") {
                  int digit = e.intAttribute("value");
                  if (digit < 0 || digit > 9)
                        return false;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "simple")
                              displayDigit[FiguredBassItem::StyleModern]  [digit][FiguredBassItem::CombSimple]      = e.readElementText()[0];
                        else if (tag == "crossed")
                              displayDigit[FiguredBassItem::StyleModern]  [digit][FiguredBassItem::CombCrossed]     = e.readElementText()[0];
                        else if (tag == "backslashed")
                              displayDigit[FiguredBassItem::StyleModern]  [digit][FiguredBassItem::CombBackslashed] = e.readElementText()[0];
                        else if (tag == "slashed")
                              displayDigit[FiguredBassItem::StyleModern]  [digit][FiguredBassItem::CombSlashed]     = e.readElementText()[0];
                        else if (tag == "simpleHistoric")
                              displayDigit[FiguredBassItem::StyleHistoric][digit][FiguredBassItem::CombSimple]      = e.readElementText()[0];
                        else if (tag == "crossedHistoric")
                              displayDigit[FiguredBassItem::StyleHistoric][digit][FiguredBassItem::CombCrossed]     = e.readElementText()[0];
                        else if (tag == "backslashedHistoric")
                              displayDigit[FiguredBassItem::StyleHistoric][digit][FiguredBassItem::CombBackslashed] = e.readElementText()[0];
                        else if (tag == "slashedHistoric")
                              displayDigit[FiguredBassItem::StyleHistoric][digit][FiguredBassItem::CombSlashed]     = e.readElementText()[0];
                        else {
                              e.unknown();
                              return false;
                              }
                        }
                  }
            else {
                  e.unknown();
                  return false;
                  }
            }
      displayParenthesis[0] = displayAccidental[FiguredBassItem::ModifierNone] = ' ';
      return true;
      }

//---------------------------------------------------------
//   Read Configuration File
//
//    reads a configuration and appends read data to g_FBFonts
//    resets everythings and reads the built-in config file if fileName is null or empty
//---------------------------------------------------------

bool FiguredBass::readConfigFile(const QString& fileName)
      {
      QString     path;

      if(fileName == 0 || fileName.isEmpty()) {       // defaults to built-in xml
#ifdef Q_OS_IOS
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

      QFile f(path);
      if (!f.open(QIODevice::ReadOnly)) {
            QString s = QT_TRANSLATE_NOOP("file", "cannot open figured bass description:\n%1\n%2");
            MScore::lastError = s.arg(f.fileName()).arg(f.errorString());
qDebug("FiguredBass::read failed: <%s>\n", qPrintable(path));
            return false;
            }
      XmlReader e(&f);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  // QString version = e.attribute(QString("version"));
                  // QStringList sl = version.split('.');
                  // int _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();

                  while (e.readNextStartElement()) {
                        if (e.name() == "font") {
                              FiguredBassFont f;
                              if (f.read(e))
                                    g_FBFonts.append(f);
                              else
                                    return false;
                              }
                        else
                              e.unknown();
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
//
//    MusicXML I/O
//
//---------------------------------------------------------

//---------------------------------------------------------
//   Read MusicXML
//
// Set the FiguredBass state based on the MusicXML <figured-bass> node de.
// Note that onNote and ticks must be set by the MusicXML importer,
// as the required context is not present in the items DOM tree.
// Exception: if a <duration> element is present, tick can be set.
// Return true if valid, non-empty figure(s) are found
// Set extend to true if extend elements were found
//---------------------------------------------------------

#if 0
bool FiguredBass::readMusicXML(XmlReader& e, int divisions, bool& extend)
      {
      extend = false;
      bool parentheses = e.attribute("parentheses") == "yes";
      QString normalizedText;
      int idx = 0;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "duration") {
                  QString val(e.readElementText());
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
                  bool figureExtend = false;
                  FiguredBassItem * pItem = new FiguredBassItem(score(), idx++);
                  pItem->setTrack(track());
                  pItem->setParent(this);
                  pItem->readMusicXML(e, parentheses, figureExtend);
                  if (figureExtend)
                        extend = true;
                  items.append(*pItem);
                  // add item normalized text
                  if (!normalizedText.isEmpty())
                        normalizedText.append('\n');
                  normalizedText.append(pItem->normalizedText());
                  }
            else {
                  e.unknown();
                  return false;
                  }
            }
      setText(normalizedText);                  // this is the text to show while editing
      bool res = !normalizedText.isEmpty();
      return res;
      }
#endif

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

void FiguredBass::writeMusicXML(Xml& xml, bool doFigure, bool doExtend) const
      {
      if (doFigure || doExtend) {
            QString stag = "figured-bass";
            if (hasParentheses())
                  stag += " parentheses=\"yes\"";
            xml.stag(stag);
            foreach(FiguredBassItem item, items)
                  item.writeMusicXML(xml, doFigure, doExtend);
            xml.etag();
            }
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


FiguredBass* Score::addFiguredBass()
      {
      Element* el = selection().element();
      if (el == 0 || (el->type() != Element::NOTE && el->type() != Element::FIGURED_BASS)) {
            QMessageBox::information(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("No note or figured bass selected:\n"
                  "Please select a single note or figured bass and retry.\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
            return 0;
            }

      FiguredBass * fb;
      bool bNew;
      if (el->type() == Element::NOTE) {
            ChordRest * cr = static_cast<Note*>(el)->chord();
            fb = FiguredBass::addFiguredBassToSegment(cr->segment(),
                        (cr->track() / VOICES) * VOICES, 0, &bNew);
            }
      else if (el->type() == Element::FIGURED_BASS) {
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

}

