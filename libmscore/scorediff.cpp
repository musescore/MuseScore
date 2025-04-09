//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <algorithm>
#include <utility>

#include "duration.h"
#include "measure.h"
#include "score.h"
#include "scorediff.h"
#include "staff.h"
#include "xml.h"

#include "dtl/dtl.hpp"

namespace Ms {

//---------------------------------------------------------
//   MscxModeDiff
//---------------------------------------------------------

class MscxModeDiff {
      static constexpr const char* tagRegExpStr = "</?(?<name>[A-z_][A-z_0-9\\-.:]*)( [A-z_0-9\\-.:\\s=\"]*)?/?>";
      const QRegularExpression tagRegExp;

      enum TagType { START_TAG, END_TAG };
      struct Tag {
            int line;
            TagType type;
            QString name;

            Tag(int l, TagType t, const QString& n) : line(l), type(t), name(n) {}
            };

      static DiffType fromDtlDiffType(dtl::edit_t dtlType);

      void adjustSemanticsMscx(std::vector<TextDiff>&);
      int adjustSemanticsMscxOneDiff(std::vector<TextDiff>& diffs, int index);
      int nextDiffOnShiftIndex(const std::vector<TextDiff>& diffs, int index, bool down);
      static QString getOuterLines(const QString& str, int lines, bool start);
      bool assessShiftDiff(const std::vector<TextDiff>&, const std::vector<Tag>& extraTags, int index, int lines);
      int performShiftDiff(std::vector<TextDiff>&, int index, int lines);
      void readMscx(const QString& xml, std::vector<Tag>& extraTags);
      void handleTag(int line, TagType type, const QString& name, std::vector<Tag>& extraTags);

   public:
      MscxModeDiff();

      std::vector<TextDiff> lineModeDiff(const QString& s1, const QString& s2);
      std::vector<TextDiff> mscxModeDiff(const QString& s1, const QString& s2);
      };

//---------------------------------------------------------
//   TextDiffParser
//    Used by ScoreDiff class during parsing MSCX text diff
//---------------------------------------------------------

class TextDiffParser {
      const int iScore;
      const int iOtherScore;

      int tagLevel = 0;
      int currentDiffTagLevel = 0;
      std::vector<const ScoreElement*> contextsStack;
      std::vector<bool> tagIsElement;
      const ScoreElement* lastElementEnded = nullptr;
      QString markupContext;

      BaseDiff* handleToken(const QXmlStreamReader& r, const ScoreElement* newElement, bool saveDiff);
      bool insideDiffTag() const { return currentDiffTagLevel != 0; }

   public:
      TextDiffParser(int iScore);

      void makeDiffs(const QString& mscx, const std::vector<std::pair<const ScoreElement*, QString>>& elements, const std::vector<TextDiff>& textDiffs, std::vector<BaseDiff*>& diffs);
      };

//---------------------------------------------------------
//   MscxModeDiff::MscxModeDiff
//---------------------------------------------------------

MscxModeDiff::MscxModeDiff()
   : tagRegExp(tagRegExpStr)
      {}

//---------------------------------------------------------
//   MscxModeDiff::fromDtlDiffType
//---------------------------------------------------------

DiffType MscxModeDiff::fromDtlDiffType(dtl::edit_t dtlType)
      {
      switch(dtlType) {
            case dtl::SES_DELETE:
                  return DiffType::DELETE;
            case dtl::SES_COMMON:
                  return DiffType::EQUAL;
            case dtl::SES_ADD:
                  return DiffType::INSERT;
            }
      Q_ASSERT(false); // dtlType must have one of the values handled above.
      return DiffType::EQUAL;
      }

//---------------------------------------------------------
//   MscxModeDiff::lineModeDiff
//---------------------------------------------------------

std::vector<TextDiff> MscxModeDiff::lineModeDiff(const QString& s1, const QString& s2)
      {
      // type declarations for dtl library
      typedef QStringRef elem;
      typedef std::pair<elem, dtl::elemInfo> sesElem;
      typedef std::vector<sesElem> sesElemVec;

      // QVector does not contain range constructor used inside dtl
      // so we have to convert to std::vector.
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
      QVector<QStringRef>s1Ref = s1.splitRef('\n');
      std::vector<QStringRef> lines1 = { s1Ref.begin(), s1Ref.end() };
      QVector<QStringRef>s2Ref = s2.splitRef('\n');
      std::vector<QStringRef> lines2 = { s2Ref.begin(), s2Ref.end() };
#else
      std::vector<QStringRef> lines1 = s1.splitRef('\n').toStdVector();
      std::vector<QStringRef> lines2 = s2.splitRef('\n').toStdVector();
#endif
      dtl::Diff<QStringRef, std::vector<QStringRef>> diff(lines1, lines2);

      diff.compose();

      const sesElemVec changes = diff.getSes().getSequence();
      std::vector<TextDiff> diffs;
      int line[2][2] {{1, 1}, {1, 1}}; // for correct assigning line numbers to
                                       // DELETE and INSERT diffs we need to
                                       // count lines separately for these diff
                                       // types (EQUAL can use both counters).

      for (const sesElem& ch : changes) {
            DiffType type = fromDtlDiffType(ch.second.type);
            const int iThis = (type == DiffType::DELETE) ? 0 : 1; // for EQUAL doesn't matter

            if (diffs.empty() || diffs.back().type != type) {
                  if (!diffs.empty()) {
                        // sync line numbers
                        DiffType prevType = diffs.back().type;
                        const int prevThis = (prevType == DiffType::DELETE) ? 0 : 1;
                        const int prevOther = (prevThis == 1) ? 0 : 1;
                        if (prevType == DiffType::EQUAL)
                              std::copy_n(line[prevThis], 2, line[prevOther]);
                        else
                              line[prevThis][prevOther] = line[prevOther][prevOther];

                        if (type == DiffType::EQUAL) {
                              const int iOther = (iThis == 1) ? 0 : 1;
                              line[iThis][iOther] = line[iOther][iOther];
                              }
                        }

                  diffs.emplace_back();
                  TextDiff& d = diffs.back();
                  d.type = type;
                  std::copy_n(line[iThis], 2, d.start);
                  d.end[0] = line[iThis][0] - 1; // equal line numbers would mean an actual change in that line.
                  d.end[1] = line[iThis][1] - 1;
                  }

            TextDiff& d = diffs.back();
            d.end[iThis] = (line[iThis][iThis]++);
            d.text[iThis].append(ch.first).append('\n');
            if (type == DiffType::EQUAL) {
                  const int iOther = (iThis == 1) ? 0 : 1;
                  d.end[iOther] = (line[iThis][iOther]++);
                  d.text[iOther].append(ch.first).append('\n');
                  }
            }

      return diffs;
      }

//---------------------------------------------------------
//   MscxModeDiff::mscxModeDiff
//---------------------------------------------------------

std::vector<TextDiff> MscxModeDiff::mscxModeDiff(const QString& s1, const QString& s2)
      {
      std::vector<TextDiff> diffs = lineModeDiff(s1, s2);
      adjustSemanticsMscx(diffs);
      return diffs;
      }

//---------------------------------------------------------
//   MscxModeDiff::adjustSemanticsMscx
//---------------------------------------------------------

void MscxModeDiff::adjustSemanticsMscx(std::vector<TextDiff>& diffs)
      {
      for (unsigned i = 0; i < diffs.size(); ++i)
            i = adjustSemanticsMscxOneDiff(diffs, i);
      }

//---------------------------------------------------------
//   MscxModeDiff::adjustSemanticsMscx
//    Returns new index of the given diff
//---------------------------------------------------------

int MscxModeDiff::adjustSemanticsMscxOneDiff(std::vector<TextDiff>& diffs, int index)
      {
      TextDiff* diff = &diffs[index];
      int iScore;
      switch(diff->type) {
            case DiffType::EQUAL:
            case DiffType::REPLACE:
                  // TODO: split a REPLACE diff, though they should not be here
                  return index;
            case DiffType::INSERT:
                  iScore = 1;
                  break;
            case DiffType::DELETE:
            default:
                  iScore = 0;
                  break;
            }

      std::vector<Tag> extraTags;
      readMscx(diff->text[iScore], extraTags);

      if (extraTags.empty())
            return index; // good, nothing to adjust

      auto firstStartExtra = extraTags.begin();
      auto lastStartExtra = extraTags.begin();
      auto firstEndExtra = extraTags.begin();
      auto lastEndExtra = extraTags.begin();
      for (auto tag = extraTags.begin(); tag != extraTags.end(); ++tag) {
            TagType type = tag->type;
            switch(type) {
                  case START_TAG:
                        if (firstStartExtra->type != START_TAG)
                              firstStartExtra = tag;
                        lastStartExtra = tag;
                        break;
                  case END_TAG:
                        if (firstStartExtra->type != END_TAG)
                              firstEndExtra = tag;
                        lastEndExtra = tag;
                        break;
                  default:
                        break;
                  }
            }

      // Try to shift current diff up and down and see whether this helps
      int lines = lastEndExtra->line - firstEndExtra->line + 1;
      if (assessShiftDiff(diffs, extraTags, index, lines))
            return performShiftDiff(diffs, index, lines);
      else {
            lines = - (lastStartExtra->line - firstStartExtra->line + 1);
            if (assessShiftDiff(diffs, extraTags, index, lines))
                  return performShiftDiff(diffs, index, lines);
            }

      return index;
      }

//---------------------------------------------------------
//   MscxModeDiff::assessShiftDiff
//    Returns true if shift of the diff with the given
//    index is possible and there would be no extra tags
//    left after such a shift.
//---------------------------------------------------------

bool MscxModeDiff::assessShiftDiff(const std::vector<TextDiff>& diffs, const std::vector<Tag>& origExtraTags, int index, int lines)
      {
      if (lines == 0)
            return diffs.empty();
      const bool down = (lines > 0);
      const int nextDiffIdx = nextDiffOnShiftIndex(diffs, index, down);
      if (nextDiffIdx == index)
            return false;

      const TextDiff& diff = diffs[index];
      const TextDiff& nextDiff = diffs[nextDiffIdx];

      Q_ASSERT(diff.type == DiffType::DELETE || diff.type == DiffType::INSERT);
      const int iScore = (diff.type == DiffType::DELETE ? 0 : 1);

      const QString& diffTxt = diff.text[iScore];
      const QString& nextTxt = nextDiff.text[iScore];
      const QString diffChunk = getOuterLines(diffTxt, lines, down);
      const QString nextChunk = getOuterLines(nextTxt, lines, down);

      if (diffChunk == nextChunk) {
            std::vector<Tag> extraTags(origExtraTags);
            std::vector<Tag> diffExtraTags;
            readMscx(diffChunk, diffExtraTags);

            // merge diff chunk reading result with the result
            // for the whole chunk
            while (!diffExtraTags.empty()) {
                  auto tag = down ? diffExtraTags.begin() : (diffExtraTags.end() - 1);

                  if (extraTags.size() < 2) // would normally erase 2 tags
                        return false;

                  auto firstExtra = extraTags.begin();
                  if ((firstExtra->type == END_TAG)
                        && (firstExtra->name == tag->name))
                        extraTags.erase(firstExtra);
                  else
                        return false;
                  auto lastExtra = extraTags.end() - 1;
                  if ((lastExtra->type == START_TAG)
                        && (lastExtra->name == tag->name))
                        extraTags.erase(lastExtra);
                  else
                        return false;

                  diffExtraTags.erase(tag);
                  }

            if (extraTags.empty()) {
                  // We have succeeded
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   MscxModeDiff::performShiftDiff
//    Perform a shift of the diff with the given index and
//    return a new index of the diff.
//---------------------------------------------------------

int MscxModeDiff::performShiftDiff(std::vector<TextDiff>& diffs, int index, int lines)
      {
      if (lines == 0)
            return index;
      const bool down = (lines > 0);
      const int absLines = qAbs(lines);
      const int nextDiffIdx = nextDiffOnShiftIndex(diffs, index, down);
      if (nextDiffIdx == index)
            return index;

      TextDiff& diff = diffs[index];
      TextDiff& nextDiff = diffs[nextDiffIdx];

      Q_ASSERT(diff.type == DiffType::DELETE || diff.type == DiffType::INSERT);
      const int iScore = (diff.type == DiffType::DELETE ? 0 : 1);
      QString& diffTxt = diff.text[iScore];

      const QString diffChunk = getOuterLines(diffTxt, absLines, down);

      Q_ASSERT(nextDiff.type == DiffType::EQUAL);
      int chunkStart[2]; // line numbers of diffChunk in two scores
      int chunkEnd[2];
      if (down) {
            std::copy(diff.start, diff.start + 2, chunkStart);
            chunkEnd[0] = diff.start[0] + absLines - 1;
            chunkEnd[1] = diff.start[1] + absLines - 1;
            }
      else {
            chunkStart[0] = diff.end[0] - (absLines - 1);
            chunkStart[1] = diff.end[1] - (absLines - 1);
            std::copy(diff.end, diff.end + 2, chunkEnd);
            }

      if (down) {
            diffTxt.remove(0, diffChunk.size());
            diffTxt.append(diffChunk);
            nextDiff.text[0].remove(0, diffChunk.size());
            nextDiff.text[1].remove(0, diffChunk.size());
            }
      else {
            diffTxt.chop(diffChunk.size());
            diffTxt.prepend(diffChunk);
            nextDiff.text[0].chop(diffChunk.size());
            nextDiff.text[1].chop(diffChunk.size());
            }
      // shift line numbers: needed both for shifted diff and for diffs
      // between the shifted diff and nextDiff
      const int inc = down ? 1 : -1;
      for (int i = index; i != nextDiffIdx; i += inc) {
            TextDiff& d = diffs[i];
            Q_UNUSED(d);
            Q_ASSERT(d.type != DiffType::EQUAL); // nextDiff should be the first EQUAL diff in that direction
            diff.start[0] += lines;
            diff.end[0] += lines;
            diff.start[1] += lines;
            diff.end[1] += lines;
            }
      int* nextChunkLines = down ? nextDiff.start : nextDiff.end;
      nextChunkLines[0] += lines;
      nextChunkLines[1] += lines;

      TextDiff eqDiff;
      eqDiff.type = DiffType::EQUAL;
      eqDiff.text[0] = diffChunk;
      eqDiff.text[1] = diffChunk;
      std::copy(chunkStart, chunkStart + 2, eqDiff.start);
      std::copy(chunkEnd, chunkEnd + 2, eqDiff.end);

      const int prevDiffIdx = index + (down ? -1 : 1);
      bool merged = false;
      if (diffs[prevDiffIdx].type == DiffType::EQUAL)
            merged = diffs[prevDiffIdx].merge(eqDiff);

      if (!merged) {
            const int insertIdx = down ? index : (index + 1);
            diffs.insert(diffs.begin() + insertIdx, eqDiff);
            if (down)
                  ++index;
            }

      return index;
      }

//---------------------------------------------------------
//   MscxModeDiff::getOuterLines
//---------------------------------------------------------

QString MscxModeDiff::getOuterLines(const QString& str, int lines, bool start) {
      lines = qAbs(lines);
      const int secIdxStart = start ? 0 : -lines;
      const int secIdxEnd = start ? (lines - 1) : -1;
      constexpr auto secFlags = QString::SectionIncludeTrailingSep | QString::SectionSkipEmpty;
      return str.section('\n', secIdxStart, secIdxEnd, secFlags);
      }

//---------------------------------------------------------
//   MscxModeDiff::nextDiffOnShiftIndex
//---------------------------------------------------------

int MscxModeDiff::nextDiffOnShiftIndex(const std::vector<TextDiff>& diffs, int index, bool down)
      {
      const DiffType diffType = diffs[index].type;
      Q_ASSERT(diffType == DiffType::DELETE || diffType == DiffType::INSERT);
      int nextDiffIdx = index + (down ? 1 : -1);
      while (nextDiffIdx >= 0 && nextDiffIdx < int(diffs.size())) {
            const TextDiff& d = diffs[nextDiffIdx];
            if (d.type == diffType) {
                  // Two INSERT or DELETE diffs in a row, probably shouldn't happen
                  return index;
                  }
            if (d.type == DiffType::EQUAL)
                  return nextDiffIdx;
            // REPLACE is not here, the other type can be skipped
            Q_ASSERT(d.type != DiffType::REPLACE);
            nextDiffIdx += (down ? 1 : -1);
            }
      return index;
      }

//---------------------------------------------------------
//   MscxModeDiff::readMscx
//---------------------------------------------------------

void MscxModeDiff::readMscx(const QString& xml, std::vector<Tag>& extraTags)
      {
      int line = 0;
      int nextLineIndex = xml.indexOf('\n');
      QRegularExpressionMatchIterator m = tagRegExp.globalMatch(xml);
      while (m.hasNext()) {
            QRegularExpressionMatch match = m.next();
            QStringRef tag(match.capturedRef("name"));
            while ((match.capturedStart("name") > nextLineIndex) && (nextLineIndex > 0)) {
                  ++line;
                  nextLineIndex = xml.indexOf('\n', nextLineIndex + 1);
                  }
            if (match.capturedRef().startsWith("</")) {
                  // end element
                  handleTag(line, END_TAG, tag.toString(), extraTags);
                  if (tag == "Tuplet") {
                        // special case: <Tuplet> / <endTuplet/> pair
                        handleTag(line, START_TAG, "__tupletBound", extraTags);
                        }
                  }
            else if (match.capturedRef().endsWith("/>")) {
                  // empty element: do nothing
                  if (tag == "endTuplet") {
                        // special case: <Tuplet> / <endTuplet/> pair
                        handleTag(line, END_TAG, "__tupletBound", extraTags);
                        }
                  }
            else {
                  // start element
                  handleTag(line, START_TAG, tag.toString(), extraTags);
                  }
            }
      }

//---------------------------------------------------------
//   MscxModeDiff::handleTag
//---------------------------------------------------------

void MscxModeDiff::handleTag(int line, TagType type, const QString& name, std::vector<Tag>& extraTags)
      {
      switch(type) {
            case START_TAG:
                  extraTags.emplace_back(line, type, name);
                  break;
            case END_TAG:
                  {
                  auto lastTag = extraTags.empty() ? nullptr : &extraTags.back();
                  if (lastTag
                     && lastTag->type == START_TAG && lastTag->name == name)
                        extraTags.pop_back();
                  else
                        extraTags.emplace_back(line, type, name);
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   TextDiffParser
//---------------------------------------------------------

TextDiffParser::TextDiffParser(int iScore)
   : iScore(iScore), iOtherScore(iScore == 1 ? 0 : 1)
      {
      contextsStack.push_back(nullptr);
      }

//---------------------------------------------------------
//   TextDiffParser::makeDiffs
//---------------------------------------------------------

void TextDiffParser::makeDiffs(const QString& mscx, const std::vector<std::pair<const ScoreElement*, QString>>& elements, const std::vector<TextDiff>& textDiffs, std::vector<BaseDiff*>& diffs)
      {
      QXmlStreamReader r(mscx);
      r.readNext(); // read first tag
      auto textDiff = textDiffs.begin();
      auto textDiffEnd = textDiffs.end();
      auto nextElement = elements.begin();
      auto nextElementEnd = elements.end();
      while (!r.atEnd()) {
            // Scroll to the correct diff to be handled now
            while (textDiff != textDiffEnd
               && textDiff->end[iScore] < r.lineNumber()) {
                  // Save ContextChange when leaving equal chunk
                  if (textDiff->type == DiffType::EQUAL) {
                        ContextChange* c = new ContextChange;
                        c->type = textDiff->type;
                        c->textDiff = &(*textDiff);
                        c->ctx[iScore] = contextsStack.back();
                        c->ctx[iOtherScore] = nullptr;
                        c->before[iScore] = lastElementEnded;
                        c->before[iOtherScore] = nullptr;
                        diffs.push_back(c);
                        }

                  ++textDiff;
                  }
            if (textDiff == textDiffEnd)
                  break;

            bool saveDiff = false;
            if (!insideDiffTag()
               && (textDiff->type == DiffType::DELETE || textDiff->type == DiffType::INSERT)
               ) {
                  const int iDiffScore = (textDiff->type == DiffType::DELETE) ? 0 : 1;
                  if (iScore == iDiffScore)
                        saveDiff = true;
                  }

            const ScoreElement* newElement = nullptr;
            if (r.isStartElement()) {
                  if (nextElement != nextElementEnd
                     && nextElement->second == r.name())
                        newElement = (nextElement++)->first;

                  ++tagLevel;
                  tagIsElement.push_back(newElement);
                  }

            BaseDiff* diff = handleToken(r, newElement, saveDiff);

            if (diff) {
                  diff->type = textDiff->type;
                  diff->textDiff = &(*textDiff);
                  diff->before[iScore] = lastElementEnded;
                  diff->before[iOtherScore] = nullptr;
                  diffs.push_back(diff);

                  if (saveDiff) {
                        // It is assumed that diff items are created in handleToken
                        // only for start tags
                        Q_ASSERT(r.isStartElement());
                        ItemType type = diff->itemType();
                        if (type == ItemType::ELEMENT || type == ItemType::PROPERTY) {
                              currentDiffTagLevel = tagLevel;
                              if (type == ItemType::ELEMENT && contextsStack.back()->isSpanner()) {
                                    // Consider <Spanner> tag as the current differing tag
                                    --currentDiffTagLevel;
                                    }
                              }
                        }
                  }

            if (r.isEndElement()) {
                  if (currentDiffTagLevel == tagLevel)
                        currentDiffTagLevel = 0;
                  --tagLevel;
                  tagIsElement.pop_back();
                  }

            r.readNext();
            }
      if (r.hasError())
            qDebug("TextDiffParser::makeDiffs: error while reading MSCX output: %s", r.errorString().toLatin1().constData());
      }

//---------------------------------------------------------
//   TextDiffParser::handleToken
//    Returns nullptr or a newly allocated BaseDiff with
//    properly filled context and fields that are unique to
//    descendants of BaseDiff (e.g. el[2] for ElementDiff).
//    Type and textDiff are left unfilled.
//---------------------------------------------------------

BaseDiff* TextDiffParser::handleToken(const QXmlStreamReader& r, const ScoreElement* newElement, bool saveDiff)
      {
      if (!r.isStartElement() && !r.isEndElement())
            return nullptr;
      BaseDiff* diff = nullptr;
      const QStringRef tag(r.name());

      if (r.isStartElement()) {
            if (newElement) {
                  if (saveDiff) {
                        ElementDiff* d = new ElementDiff;
                        diff = d;
                        d->ctx[iScore] = contextsStack.back();
                        d->el[iScore] = newElement;
                        d->el[iOtherScore] = nullptr;
                        }
                  contextsStack.push_back(newElement);
                  }
            else if (tag == "Spanner")
                  markupContext = tag.toString();
            else if (saveDiff) {
                  const ScoreElement* ctx = contextsStack.back();
                  if (markupContext == "Spanner" && !(ctx && ctx->isSpanner())) {
                        // some property inside the end of a spanner
                        }
                  else if (tag == "prev" || tag == "next") {
                        // Ignore these tags
                        }
                  else if (tag == "location" || tag == "voice") {
                        if (ctx && ctx->isMeasure()) {
                              MarkupDiff* d = new MarkupDiff;
                              diff = d;
                              d->ctx[iScore] = ctx;
                              d->name = tag.toString();
                              const ScoreElement* widerCtx = *(contextsStack.end() - 2);
                              if (widerCtx && widerCtx->isStaff())
                                    d->info = toStaff(widerCtx)->idx();
                              }
                        }
                  else {
                        Pid propId = ctx ? ctx->propertyId(tag) : Pid::END;
                        if (propId != Pid::END) {
                              PropertyDiff* d = new PropertyDiff;
                              diff = d;
                              d->ctx[iScore] = contextsStack.back();
                              d->pid = propId;
                              }
                        else {
                              MarkupDiff* d = new MarkupDiff;
                              diff = d;
                              d->ctx[iScore] = contextsStack.back();
                              d->name = tag.toString();
                              if (tag == "metaTag")
                                    d->info = r.attributes().value("name").toString();
                              }
                        }
                  }
            }
      else { // end element
            if ((tagIsElement.back() && !contextsStack.back()->isSpanner())
               || (tag == "Spanner" && contextsStack.back()->isSpanner())
               ) {
                  lastElementEnded = contextsStack.back();
                  contextsStack.pop_back();
                  }
            if (!markupContext.isEmpty() && tag == markupContext)
                  markupContext.clear();
            }

      if (diff)
            diff->ctx[iOtherScore] = nullptr;
      return diff;
      }

//---------------------------------------------------------
//   lineNumberSort
//---------------------------------------------------------

static bool lineNumberSort(BaseDiff* d1, BaseDiff* d2)
      {
      const TextDiff* t1 = d1->textDiff;
      const TextDiff* t2 = d2->textDiff;
      return ((t1->end[0] < t2->end[0]) || (t1->end[1] < t2->end[1]));
      }

//---------------------------------------------------------
//   positionSort
//---------------------------------------------------------

static bool positionSort(BaseDiff* d1, BaseDiff* d2)
      {
      return (d1->afrac(0) < d2->afrac(0) || d1->afrac(1) < d2->afrac(1));
      }

//---------------------------------------------------------
//   scoreToMscx
//---------------------------------------------------------

static QString scoreToMscx(Score* s, XmlWriter& xml)
      {
      QString mscx;
      xml.setString(&mscx, QIODevice::WriteOnly);
      xml.setRecordElements(true);
      s->write(xml, /* onlySelection */ false);
      xml.flush();
      return mscx;
      }

//---------------------------------------------------------
//   ScoreDiff
//    s1, s2 are scores to compare.
//    ScoreDiff does NOT take ownership of the scores.
//---------------------------------------------------------

ScoreDiff::ScoreDiff(Score* s1, Score* s2, bool textDiffOnly)
   : _s1(s1), _s2(s2), _textDiffOnly(textDiffOnly)
      {
      update();
      }

//---------------------------------------------------------
//   makeDiffs
//---------------------------------------------------------

static void makeDiffs(const QString& mscx1, const QString& mscx2, const XmlWriter& xml1, const XmlWriter& xml2, const std::vector<TextDiff>& textDiffs, std::vector<BaseDiff*>& diffs)
      {
      TextDiffParser p1(0);
      p1.makeDiffs(mscx1, xml1.elements(), textDiffs, diffs);
      TextDiffParser p2(1);
      p2.makeDiffs(mscx2, xml2.elements(), textDiffs, diffs);

      std::stable_sort(diffs.begin(), diffs.end(), lineNumberSort);

      // Fill correct contexts for diffs
      const ScoreElement* lastCtx[2] {};
      const ScoreElement* lastBefore[2] {};
      for (BaseDiff* diff : diffs) {
            for (int i = 0; i < 2; ++i) {
                  if (diff->ctx[i])
                        lastCtx[i] = diff->ctx[i];
                  else
                        diff->ctx[i] = lastCtx[i];

                  if (diff->before[i])
                        lastBefore[i] = diff->before[i];
                  else
                        diff->before[i] = lastBefore[i];
                  }
            }

      // Filter out EQUAL diffs and dummy ContextChange items
      std::vector<BaseDiff*> filteredDiffs;
      for (BaseDiff* diff : diffs) {
            if ((diff->itemType() == ItemType::CONTEXTCHANGE)
               || (diff->type == DiffType::EQUAL))
                  delete diff;
            else
                  filteredDiffs.push_back(diff);
            }
      std::swap(diffs, filteredDiffs);
      }

//---------------------------------------------------------
//   ScoreDiff::update
//    Updates the diff according to the current state of
//    the compared scores.
//---------------------------------------------------------

void ScoreDiff::update()
      {
      if (updated())
            return;

      qDeleteAll(_diffs);
      _diffs.clear();
      _textDiffs.clear();
      qDeleteAll(_mergedTextDiffs);
      _mergedTextDiffs.clear();

      XmlWriter xml1(_s1);
      XmlWriter xml2(_s2);
      QString mscx1(scoreToMscx(_s1, xml1));
      QString mscx2(scoreToMscx(_s2, xml2));

      _textDiffs = MscxModeDiff().mscxModeDiff(mscx1, mscx2);

      if (!_textDiffOnly) {
            makeDiffs(mscx1, mscx2, xml1, xml2, _textDiffs, _diffs);

            processMarkupDiffs();
            mergeInsertDeleteDiffs();
            mergeElementDiffs();
            editPropertyDiffs();

            std::stable_sort(_diffs.begin(), _diffs.end(), positionSort);
            }

      _scoreState1 = _s1->state();
      _scoreState2 = _s2->state();
      }

//---------------------------------------------------------
//   ~ScoreDiff
//---------------------------------------------------------

ScoreDiff::~ScoreDiff()
      {
      qDeleteAll(_mergedTextDiffs);
      qDeleteAll(_diffs);
      }

//---------------------------------------------------------
//   deleteDiffs
//---------------------------------------------------------

static void deleteDiffs(std::vector<BaseDiff*>& diffsList, const std::vector<BaseDiff*>& toDelete)
      {
      for (BaseDiff* diff : toDelete) {
            for (auto it = diffsList.begin(); it != diffsList.end(); ++it) {
                  if (*it == diff) {
                        diffsList.erase(it);
                        break;
                        }
                  }
            delete diff;
            }
      }

//---------------------------------------------------------
//   measureToMscx
//---------------------------------------------------------

static QString measureToMscx(const Measure* m, XmlWriter& xml, int staff)
      {
      QString mscx;
      xml.setString(&mscx, QIODevice::WriteOnly);
      xml.setRecordElements(true);
      m->write(xml, staff, false, false);
      xml.flush();
      return mscx;
      }

//---------------------------------------------------------
//   measureInfo
//---------------------------------------------------------

struct MeasureInfo {
      const Measure* m1;
      const Measure* m2;
      int staff;

      MeasureInfo(const Measure* m1, const Measure* m2, int staff) : m1(m1), m2(m2), staff(staff) {}

      bool operator==(const MeasureInfo& other) { return m1 == other.m1 && m2 == other.m2 && staff == other.staff; }
      bool operator!=(const MeasureInfo& other) { return !(*this == other); }
      };

//---------------------------------------------------------
//   ScoreDiff::processMarkupDiffs
//    Find MarkupDiffs and convert them to the appropriate
//    diffs of other types if needed.
//---------------------------------------------------------

void ScoreDiff::processMarkupDiffs()
      {
      std::vector<BaseDiff*> abandonedDiffs;
      std::vector<MeasureInfo> measuresToProcess;
      Pid pids[] { Pid::TRACK, Pid::POSITION }; // list of pids to be accepted on measures processing
      for (BaseDiff* diff : _diffs) {
            if (diff->itemType() != ItemType::MARKUP)
                  continue;

            MarkupDiff* d = static_cast<MarkupDiff*>(diff);
            const QString& name = d->name;

            if (name == "voice" || name == "location") {
                  if (d->ctx[0]->isMeasure() && d->ctx[1]->isMeasure()) {
                        MeasureInfo m(toMeasure(d->ctx[0]), toMeasure(d->ctx[1]), d->info.toInt());
                        bool add = true;
                        for (auto& mScheduled : measuresToProcess) {
                              if (mScheduled == m) {
                                    add = false;
                                    break;
                                    }
                              }
                        if (add)
                              measuresToProcess.push_back(m);
                        abandonedDiffs.push_back(d);
                        }
                  }
            }

      std::vector<BaseDiff*> newDiffs;
      for (auto& m : measuresToProcess) {
            XmlWriter xml1(m.m1->score());
            xml1.setWriteTrack(true);
            xml1.setWritePosition(true);
            QString mscx1 = measureToMscx(m.m1, xml1, m.staff);

            XmlWriter xml2(m.m2->score());
            xml2.setWriteTrack(true);
            xml2.setWritePosition(true);
            QString mscx2 = measureToMscx(m.m2, xml2, m.staff);

            std::vector<TextDiff> textDiffs = MscxModeDiff().mscxModeDiff(mscx1, mscx2);
            makeDiffs(mscx1, mscx2, xml1, xml2, textDiffs, newDiffs);
            }

      for (BaseDiff* newDiff : newDiffs) {
            bool save = false;
            if (newDiff->itemType() == ItemType::PROPERTY) {
                  PropertyDiff* pDiff = static_cast<PropertyDiff*>(newDiff);
                  for (Pid pid : pids) {
                        if (pDiff->pid == pid) {
                              save = true;
                              break;
                              }
                        }
                  if (pDiff->pid == Pid::TRACK)
                        pDiff->pid = Pid::VOICE;

                  if (pDiff->ctx[0]->isMeasure() || pDiff->ctx[1]->isMeasure())
                        save = false;
                  }


            if (save) {
                  newDiff->textDiff = nullptr;
                  _diffs.push_back(newDiff);
                  }
            else
                  delete newDiff;
            }

      deleteDiffs(_diffs, abandonedDiffs);
      }

//---------------------------------------------------------
//   ScoreDiff::mergeInsertDeleteDiffs
//    Merge INSERT and DELETE diffs to REPLACE diffs where
//    possible.
//---------------------------------------------------------

void ScoreDiff::mergeInsertDeleteDiffs()
      {
      const ScoreElement* lastCtx1 = _s1;
      const ScoreElement* lastCtx2 = _s2;
      std::vector<BaseDiff*> diffsToMerge;
      std::vector<BaseDiff*> abandonedDiffs;
      for (BaseDiff* diff : _diffs) {
            if ((diff->ctx[0] != lastCtx1) || (diff->ctx[1] != lastCtx2)) {
                  diffsToMerge.clear();
                  lastCtx1 = diff->ctx[0];
                  lastCtx2 = diff->ctx[1];
                  }
            bool merge = false;
            for (auto diff2It = diffsToMerge.begin(); diff2It != diffsToMerge.end(); ++diff2It) {
                  BaseDiff* diff2 = (*diff2It);
                  if (diff->type == DiffType::DELETE) {
                        if ((diff2->type == DiffType::INSERT) && diff->sameItem(*diff2))
                              merge = true;
                        }
                  else if (diff->type == DiffType::INSERT) {
                        if ((diff2->type == DiffType::DELETE) && diff->sameItem(*diff2))
                              merge = true;
                        }

                  if (merge) {
                        // To preserve the correct order of diffs, we should
                        // leave diff2 in place since it appeared earlier in
                        // the _diffs list.
                        diff2->type = DiffType::REPLACE;

                        if (diff->textDiff && diff2->textDiff) {
                              TextDiff* mergedTextDiff = new TextDiff(*diff2->textDiff);
                              mergedTextDiff->merge(*diff->textDiff);
                              _mergedTextDiffs.push_back(mergedTextDiff);
                              diff2->textDiff = mergedTextDiff;
                              }
                        else if (!diff2->textDiff)
                              diff2->textDiff = diff->textDiff;

                        int idx1 = (diff->type == DiffType::DELETE) ? 0 : 1;
                        switch(diff2->itemType()) {
                              case ItemType::ELEMENT:
                                    {
                                    ElementDiff* d = static_cast<ElementDiff*>(diff);
                                    ElementDiff* d2 = static_cast<ElementDiff*>(diff2);
                                    // d2->el[idx2] is already filled
                                    d2->el[idx1] = d->el[idx1];
                                    }
                                    break;
                              case ItemType::PROPERTY:
                              case ItemType::MARKUP:
                                    // nothing to fill
                                    break;
                              case ItemType::CONTEXTCHANGE:
                                    break;
                              }

                        abandonedDiffs.push_back(diff);
                        diffsToMerge.erase(diff2It);
                        break; // loop on diffsToMerge
                        }
                  }

            if (!merge)
                  diffsToMerge.push_back(diff);
            }

      deleteDiffs(_diffs, abandonedDiffs);
      }

//---------------------------------------------------------
//   ScoreDiff::mergeElementDiffs
//    Merge element diffs with equal elements
//---------------------------------------------------------

void ScoreDiff::mergeElementDiffs()
      {
      std::map<const ScoreElement*, ElementDiff*> elementDiffs;
      std::vector<BaseDiff*> abandonedDiffs;
      for (BaseDiff* diff : _diffs) {
            if (diff->itemType() != ItemType::ELEMENT)
                  continue;

            ElementDiff* elDiff = static_cast<ElementDiff*>(diff);
            auto foundDiffIt1 = elementDiffs.find(elDiff->el[0]);
            auto foundDiffIt2 = elementDiffs.find(elDiff->el[1]);
            ElementDiff* foundDiff = nullptr;
            if (foundDiffIt1 != elementDiffs.end())
                  foundDiff = foundDiffIt1->second;
            else if (foundDiffIt2 != elementDiffs.end())
                  foundDiff = foundDiffIt2->second;

            if (foundDiff && foundDiff->type == elDiff->type
               && foundDiff->el[0] == elDiff->el[0]
               && foundDiff->el[1] == elDiff->el[1]) {
                  for (int i = 0; i < 2; ++i) {
                        const ScoreElement* se = foundDiff->el[i];
                        if (!se)
                              continue;
                        if (!se->isMeasure() && se->isElement()) {
                              const Element* m = toElement(se)->findMeasure();
                              if (m)
                                    foundDiff->ctx[i] = m;
                              else
                                    foundDiff->ctx[i] = se->score();
                              }
                        else
                              foundDiff->ctx[i] = se->score();
                        }
                  abandonedDiffs.push_back(elDiff);
                  }
            else {
                  for (int i = 0; i < 2; ++i) {
                        if (const ScoreElement* se = elDiff->el[i])
                              elementDiffs[se] = elDiff;
                        }
                  }
            }

      deleteDiffs(_diffs, abandonedDiffs);
      }

//---------------------------------------------------------
//   ScoreDiff::editPropertyDiffs
//    Merge property diffs with equal elements, edit some
//    diffs if necessary
//---------------------------------------------------------

void ScoreDiff::editPropertyDiffs()
      {
      std::multimap<const ScoreElement*, PropertyDiff*> propertyDiffs;
      std::vector<BaseDiff*> abandonedDiffs;
      for (BaseDiff* diff : _diffs) {
            if (diff->itemType() != ItemType::PROPERTY)
                  continue;

            PropertyDiff* pDiff = static_cast<PropertyDiff*>(diff);

            if (pDiff->pid == Pid::TPC1 || pDiff->pid == Pid::TPC2) {
                  // Special case: "tpc" and "pitch" properties are partially
                  // overlapped so we will mark tpc changes as pitch changes
                  pDiff->pid = Pid::PITCH;
                  }

            auto foundRange = propertyDiffs.equal_range(pDiff->ctx[0]);
            bool merged = false;
            for (auto it = foundRange.first; it != foundRange.second; ++it) {
                  PropertyDiff* foundDiff = it->second;
                  if (foundDiff && foundDiff->type == pDiff->type
                     && foundDiff->ctx[0] == pDiff->ctx[0]
                     && foundDiff->ctx[1] == pDiff->ctx[1]
                     && foundDiff->pid == pDiff->pid
                     ) {
                        abandonedDiffs.push_back(pDiff);
                        merged = true;
                        break;
                        }
                  }

            if (!merged)
                  propertyDiffs.emplace(pDiff->ctx[0], pDiff);
            }

      deleteDiffs(_diffs, abandonedDiffs);
      }

//---------------------------------------------------------
//   ScoreDiff::equal
//---------------------------------------------------------

bool ScoreDiff::equal() const
      {
      for (const TextDiff& td : _textDiffs) {
            if (td.type != DiffType::EQUAL)
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   ScoreDiff::rawDiff
//---------------------------------------------------------

QString ScoreDiff::rawDiff(bool skipEqual) const
      {
      QStringList list;
      for (const TextDiff& td : _textDiffs) {
            if (!skipEqual || td.type != DiffType::EQUAL)
                  list.push_back(td.toString(true));
            }
      return list.join('\n');
      }

//---------------------------------------------------------
//   ScoreDiff::userDiff
//---------------------------------------------------------

QString ScoreDiff::userDiff() const
      {
      QStringList list;
      for (const BaseDiff* d : _diffs)
            list.push_back(d->toString());
      return list.join('\n');
      }

//---------------------------------------------------------
//   TextDiff::merge
//---------------------------------------------------------

bool TextDiff::merge(const TextDiff& other)
      {
      if (type == other.type) {
            if (other.end[0] == (start[0] - 1) && other.end[1] == (start[1] - 1)) {
                  start[0] = other.start[0];
                  start[1] = other.start[1];
                  text[0].prepend(other.text[0]);
                  text[1].prepend(other.text[1]);
                  }
            else if ((end[0] + 1) == other.start[0] && (end[1] + 1) == other.start[1]) {
                  end[0] = other.end[0];
                  end[1] = other.end[1];
                  text[0].append(other.text[0]);
                  text[1].append(other.text[1]);
                  }
            else {
                  qDebug("TextDiff:merge: invalid argument: wrong line numbers");
                  return false;
                  }
            }
      else if ((type == DiffType::INSERT && other.type == DiffType::DELETE)
         || (type == DiffType::DELETE && other.type == DiffType::INSERT)
         ) {
            type = DiffType::REPLACE;
            const int iOther = (other.type == DiffType::DELETE) ? 0 : 1;
            start[iOther] = other.start[iOther];
            end[iOther] = other.end[iOther];
            text[iOther] = other.text[iOther];
            }
      else {
            qDebug("TextDiff:merge: invalid argument: wrong types");
            return false;
            }

      return true;
      }

//---------------------------------------------------------
//   addLinePrefix
//---------------------------------------------------------

static QString addLinePrefix(const QString& str, const QString& prefix)
      {
      if (prefix.isEmpty())
            return str;
      QVector<QStringRef> lines = str.splitRef('\n');
      if (lines.back().isEmpty())
            lines.pop_back();
      QStringList processedLines;
      for (QStringRef& line : lines)
            processedLines.push_back(QString(prefix).append(line));
      return processedLines.join('\n');
      }

//---------------------------------------------------------
//   TextDiff::toString
//    type argument allows to define which part of diff
//    should be printed. For example, it allows to print
//    only deleted chunk for REPLACE diff item.
//---------------------------------------------------------

QString TextDiff::toString(DiffType dt, bool prefixLines) const
      {
      if (dt == DiffType::REPLACE) {
            QStringList l;
            l.push_back(toString(DiffType::DELETE, prefixLines));
            l.push_back(toString(DiffType::INSERT, prefixLines));
            return l.join('\n');
            }

      int idx = (dt == DiffType::INSERT) ? 1 : 0;
      const char* prefix = (dt == DiffType::INSERT) ? ">" : "<";

      QString lines[2];
      for (int i = 0; i < 2; ++i) {
            if ((i != idx && dt != DiffType::EQUAL)
                  || end[i] <= start[i]
                  ) {
                  lines[i] = QString::number(start[i]);
                  }
            else
                  lines[i] = QString("%1--%2")
                        .arg(start[i], end[i]);
            }

      return QString("@%1;%2\n%3")
            .arg(lines[0], lines[1],
                  prefixLines ? addLinePrefix(text[idx], prefix) : text[idx]);
      }

//---------------------------------------------------------
//   BaseDiff::sameItem
//---------------------------------------------------------

bool BaseDiff::sameItem(const BaseDiff& other) const
      {
      if (itemType() != other.itemType()
         || ctx[0] != other.ctx[0] || ctx[1] != other.ctx[1])
            return false;
      if (textDiff == other.textDiff)
            return true;
      else if (!textDiff || !other.textDiff)
            return false;
      return (textDiff->start[0] == other.textDiff->start[0]
         && textDiff->start[1] == other.textDiff->start[1]
         );
      }

//---------------------------------------------------------
//   BaseDiff::afrac
//    Returns position of the changed chunk
//---------------------------------------------------------

Fraction BaseDiff::afrac(int score) const
      {
      Q_ASSERT(score == 0 || score == 1);
      if (ctx[score] && ctx[score]->isElement())
            return toElement(ctx[score])->tick();
      if (before[score] && before[score]->isElement()) {
            const Element* bef = toElement(before[score]);
            Fraction f = bef->tick();
            if (bef->isDurationElement()) {
                  const DurationElement* de = toDurationElement(bef);
                  return f + de->actualTicks();
                  }
            return f;
            }
      return Fraction(0,1);
      }

//---------------------------------------------------------
//   describeContext
//---------------------------------------------------------

static QString describeContext(const ScoreElement* ctx)
      {
      if (!ctx)
            return QString("no context");
      QString descr;
      if (ctx->isElement()) {
            const Element* e = toElement(ctx);
            if (const Measure* m = toMeasure(e->findMeasure()))
                  descr += QString("%1 %2").arg(m->userName()).arg(m->no() + 1);
            }
      if (!ctx->isMeasure()) {
            if (!descr.isEmpty())
                  descr += ": ";
            descr += ctx->userName();
            // TODO: add more info
            }
      return descr;
      }

//---------------------------------------------------------
//   ContextChange::toString
//    ContextChange elements are not intended to be shown
//    to user, this function is for debugging purposes.
//---------------------------------------------------------

QString ContextChange::toString() const
      {
      return QString("Context change: ctx1 (%1), ctx2(%2)").arg(describeContext(ctx[0]), describeContext(ctx[1]));
      }

//---------------------------------------------------------
//   ElementDiff::sameItem
//---------------------------------------------------------

bool ElementDiff::sameItem(const BaseDiff& other) const
      {
      return BaseDiff::sameItem(other);
      }

//---------------------------------------------------------
//   ElementDiff::afrac
//    Returns position of the changed chunk
//---------------------------------------------------------

Fraction ElementDiff::afrac(int score) const
      {
      Q_ASSERT(score == 0 || score == 1);
      const ScoreElement* se = el[score];
      if (se && se->isElement())
            return toElement(se)->tick();
      return BaseDiff::afrac(score);
      }

//---------------------------------------------------------
//   ElementDiff::toString
//---------------------------------------------------------

QString ElementDiff::toString() const
      {
      QString ctxDescr = describeContext(ctx[0]);
      switch(type) {
            case DiffType::DELETE:
                  return QObject::tr("%1: removed element %2", "scorediff").arg(ctxDescr, el[0]->userName());
            case DiffType::INSERT:
                  return QObject::tr("%1: inserted element %2", "scorediff").arg(ctxDescr, el[1]->userName());
            case DiffType::REPLACE:
                  return QObject::tr("%1: replaced element %2 with element %3", "scorediff").arg(ctxDescr, el[0]->userName(), el[1]->userName());
            case DiffType::EQUAL:
                  Q_ASSERT(el[0]->type() == el[1]->type());
                  return QObject::tr("%1: equal element %2", "scorediff").arg(ctxDescr, el[0]->userName());
            }
      return ctxDescr;
      }

//---------------------------------------------------------
//   PropertyDiff::sameItem
//---------------------------------------------------------

bool PropertyDiff::sameItem(const BaseDiff& otherBase) const
      {
      if (!BaseDiff::sameItem(otherBase))
            return false;
      if (itemType() != otherBase.itemType())
            return false;
      const PropertyDiff& other = static_cast<const PropertyDiff&>(otherBase);
      return (pid == other.pid);
      }

//---------------------------------------------------------
//   PropertyDiff::toString
//---------------------------------------------------------

QString PropertyDiff::toString() const
      {
      QString ctxDescr = describeContext(ctx[0]);
      QString propName = propertyUserName(pid);
      switch(propertyType(pid)) {
            case P_TYPE::BOOL:
                  {
                  bool b1 = ctx[0]->getProperty(pid).toBool();
                  Q_ASSERT(b1 != ctx[1]->getProperty(pid).toBool());
                  QString t;
                  if (b1)
                        t = QObject::tr("%1: property %2 is turned off", "scorediff");
                  else
                        t = QObject::tr("%1: property %2 is turned on", "scorediff");
                  return t.arg(ctxDescr, propName);
                  }
            default:
                  {
                  QString val1 = ctx[0]->propertyUserValue(pid);
                  QString val2 = ctx[1]->propertyUserValue(pid);
                  QString t = QObject::tr("%1: property %2 changed from %3 to %4", "scorediff");
                  return t.arg(ctxDescr, propName, val1, val2);
                  }
            }
      }

//---------------------------------------------------------
//   MarkupDiff::sameItem
//---------------------------------------------------------

bool MarkupDiff::sameItem(const BaseDiff& otherBase) const
      {
      if (!BaseDiff::sameItem(otherBase))
            return false;
      if (itemType() != otherBase.itemType())
            return false;
      const MarkupDiff& other = static_cast<const MarkupDiff&>(otherBase);
      return (name == other.name);
      }

//---------------------------------------------------------
//   MarkupDiff::toString
//---------------------------------------------------------

QString MarkupDiff::toString() const
      {
      QString ctxDescr = describeContext(ctx[0]);
      if (name == "metaTag") {
            QString tagName = info.toString();
            return QObject::tr("%1: %2 changed from %3 to %4", "scorediff")
               .arg(ctxDescr, tagName,
                    ctx[0]->score()->metaTag(tagName), ctx[1]->score()->metaTag(tagName));
            }
      return QObject::tr("%1: markup changes: %2", "scorediff")
         .arg(ctxDescr, name);
      }
}

