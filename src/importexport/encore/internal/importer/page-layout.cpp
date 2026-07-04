/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// Page and printer geometry for the Encore importer (see page-layout.h).

#include "page-layout.h"

#include <algorithm>
#include <cmath>
#include <set>

#include <QPageSize>

#include "ctx.h"
#include "../parser/elem.h"
#include "../parser/readers.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/page.h"
#include "engraving/dom/system.h"
#include "engraving/dom/rangelock.h"
#include "engraving/style/style.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::enc {
// Find the smallest standard page (in pts) whose printable area contains the WINI edges.
static bool detectPtsPageSize(qint32 rightEdge, qint32 bottomEdge,
                              double& outWidthIn, double& outHeightIn)
{
    static constexpr double kTol = 1.0;   // pts tolerance for metric rounding
    double bestArea = 1e18;
    bool found = false;
    for (int id = 0; id <= static_cast<int>(QPageSize::LastPageSize); ++id) {
        if (id == static_cast<int>(QPageSize::Custom)) {
            continue;
        }
        const QSizeF sz = QPageSize::size(static_cast<QPageSize::PageSizeId>(id),
                                          QPageSize::Inch);
        const double wPts = sz.width() * 72.0;
        const double hPts = sz.height() * 72.0;
        if (wPts + kTol < static_cast<double>(rightEdge)
            || hPts + kTol < static_cast<double>(bottomEdge)) {
            continue;
        }
        const double area = wPts * hPts;
        if (area < bestArea) {
            bestArea    = area;
            outWidthIn  = sz.width();
            outHeightIn = sz.height();
            found       = true;
        }
    }
    return found;
}

// Identify the paper size from WINI screen-pixel coordinates (pageWUnits = rightEdge + left,
// pageHUnits = bottomEdge + top) by matching the implied DPI ratio. Pass 1 tries the ISO A-series
// first: all AN sizes share the 1:sqrt(2) ratio, so a non-A format with an accidentally smaller
// delta must not win over the correct AN. Pass 2 tries every other standard size. Both keep the
// candidate with the smallest abs(dpiW - dpiH). Returns false when nothing matches (custom page).
static bool detectWiniPageSize(int pageWUnits, int pageHUnits,
                               double& outWidthIn, double& outHeightIn)
{
    static constexpr double kDpiMin   = 60.0;   // minimum plausible screen DPI
    static constexpr double kDpiMax   = 135.0;  // maximum plausible screen DPI
    static constexpr double kMaxDelta = 6.0;    // max |dpiW - dpiH|

    // ISO A-series IDs in Qt's QPageSize enum (Qt 6).
    static const QPageSize::PageSizeId kASeriesIds[] = {
        QPageSize::A0, QPageSize::A1, QPageSize::A2, QPageSize::A3,
        QPageSize::A4, QPageSize::A5, QPageSize::A6, QPageSize::A7,
        QPageSize::A8, QPageSize::A9, QPageSize::A10,
    };

    auto tryCandidate = [&](QPageSize::PageSizeId id,
                            double& bestDelta,
                            double& bestW, double& bestH) -> bool {
        const QSizeF sz = QPageSize::size(id, QPageSize::Inch);
        const double w  = sz.width();
        const double h  = sz.height();
        if (w <= 0.0 || h <= 0.0) {
            return false;
        }
        const double dpiW = pageWUnits / w;
        const double dpiH = pageHUnits / h;
        if (dpiW < kDpiMin || dpiW > kDpiMax || dpiH < kDpiMin || dpiH > kDpiMax) {
            return false;
        }
        const double delta = std::abs(dpiW - dpiH);
        if (delta < kMaxDelta && delta < bestDelta) {
            bestDelta = delta;
            bestW = w;
            bestH = h;
            return true;
        }
        return false;
    };

    // Build a set of A-series IDs for fast exclusion in pass 2.
    std::set<int> aSeriesSet;
    for (const auto id : kASeriesIds) {
        aSeriesSet.insert(static_cast<int>(id));
    }

    // Pass 1: ISO A-series.
    double bestDelta = kMaxDelta;
    bool found = false;
    for (const auto id : kASeriesIds) {
        if (tryCandidate(id, bestDelta, outWidthIn, outHeightIn)) {
            found = true;
        }
    }
    if (found) {
        return true;
    }

    // Pass 2: all other standard sizes (Letter, Legal, B-series, etc.).
    for (int id = 0; id <= static_cast<int>(QPageSize::LastPageSize); ++id) {
        if (id == static_cast<int>(QPageSize::Custom)) {
            continue;
        }
        if (aSeriesSet.count(id)) {
            continue;   // already tried in pass 1
        }
        if (tryCandidate(static_cast<QPageSize::PageSizeId>(id), bestDelta, outWidthIn, outHeightIn)) {
            found = true;
        }
    }
    return found;
}

// Map a Windows DEVMODE dmPaperSize (DMPAPER_*) to a Qt page size. Returns Custom for
// values without a standard mapping; the caller then falls back to dmPaperLength/Width or
// to the WINI geometry heuristic.
static QPageSize::PageSizeId dmPaperToQt(int dmPaper)
{
    switch (dmPaper) {
    case 1:  return QPageSize::Letter;
    case 5:  return QPageSize::Legal;
    case 7:  return QPageSize::Executive;
    case 8:  return QPageSize::A3;
    case 9:  return QPageSize::A4;
    case 11: return QPageSize::A5;
    case 12: return QPageSize::B4;     // DMPAPER_B4 (JIS)
    case 13: return QPageSize::B5;     // DMPAPER_B5 (JIS)
    default: return QPageSize::Custom;
    }
}

// Resolve the page size (inches) from the PREC (DEVMODE) block: dmPaperSize enum, falling back
// to dmPaperLength/Width (tenths of a millimetre) for custom sizes, with the landscape swap
// applied. Returns false when PREC has no usable size (caller falls back to WINI geometry).
bool precPageSizeInches(const EncPrintSetup& pr, double& wIn, double& hIn)
{
    if (!pr.hasData) {
        return false;
    }
    const QPageSize::PageSizeId id = dmPaperToQt(pr.paperSize);
    if (id != QPageSize::Custom) {
        const QSizeF sz = QPageSize::size(id, QPageSize::Inch);
        wIn = sz.width();
        hIn = sz.height();
    } else if (pr.paperLength > 0 && pr.paperWidth > 0) {
        wIn = pr.paperWidth / 254.0;
        hIn = pr.paperLength / 254.0;
    } else {
        return false;
    }
    if (pr.orientation == 2) {   // DMORIENT_LANDSCAPE
        std::swap(wIn, hIn);
    }
    return true;
}

// Apply page size, orientation and notation scale from the PREC (DEVMODE) block. Returns
// true when the page size was set (so the WINI margin pass must not override it). PREC is
// present in almost every Encore file across all formats, while WINI (margins) exists only
// in v0xC4, so this is the primary source of the page size for v0xA6/v0xC2 and for the
// many v0xC4 files without a WINI block.
static bool applyPagePrintSetup(MasterScore* score, const EncPrintSetup& pr)
{
    double wIn = 0.0, hIn = 0.0;
    if (!precPageSizeInches(pr, wIn, hIn)) {
        return false;   // unknown paper: let the WINI geometry heuristic decide
    }
    score->style().set(Sid::pageWidth,  wIn);
    score->style().set(Sid::pageHeight, hIn);
    // TODO: dmScale (Encore's notation-size percent) is parsed and logged but not applied.
    // MuseScore has no global percentage scale, and mapping it onto spatium would compound with
    // the per-staff size from applyStaffScale (Pid::MAG); the reconciliation needs investigation.
    LOGD() << "  PREC: orientation=" << pr.orientation << " paperSize=" << pr.paperSize
           << " paper=" << pr.paperWidth << "x" << pr.paperLength << "(0.1mm)"
           << " scale(zoom)=" << pr.scale << "%"
           << " -> " << QString::number(wIn, 'f', 2).toStdString()
           << "x" << QString::number(hIn, 'f', 2).toStdString() << "in";
    return true;
}

// Display size (1-4) for an instrument: per-instrument staffSizeHint from the LINE staff entry,
// falling back to the global header.scoreSize for files without LINE data.
// See ENCORE_FORMAT.md §System block (LINE).
int staffDisplaySize(const EncRoot& enc, int instrIdx)
{
    if (!enc.lines.empty()) {
        for (const EncLineStaffData& lsd : enc.lines[0].staffData) {
            if (static_cast<int>(lsd.instrumentIndex()) == instrIdx) {
                return std::clamp(static_cast<int>(lsd.staffSizeHint) + 1, 1, 4);
            }
        }
    }
    return std::clamp(static_cast<int>(enc.header.scoreSize), 1, 4);
}

double winiUnitsPerInch(int rightEdge, int left, double pageWIn)
{
    if (pageWIn <= 0.0) {
        return 72.0;
    }
    // (rightEdge + left) / pageWidth near 72 means the WINI is in typographic points; a clearly
    // larger value (about 84) means screen pixels at the monitor DPI. Snap the near-72 case to
    // exactly 72. The pixel estimate is exact only when left/right margins are symmetric; with
    // asymmetric margins it reads about 2% low.
    const double est = static_cast<double>(rightEdge + left) / pageWIn;
    return (est <= 76.0) ? 72.0 : est;
}

static void applyPageMargins(MasterScore* score, const EncPageSetup& ps, bool pageSizeLocked)
{
    if (!ps.hasData) {
        return;
    }
    // WINI fields are nominally typographic points (1/72 inch), but some Encore versions store
    // them in screen pixels at the monitor DPI (about 84-85 PPI on older hardware); the tell is
    // rightEdge/bottomEdge exceeding the page size in pts (e.g. 672 > A4 width 595). The pts case
    // recovers the page via detectPtsPageSize, the pixel case via detectWiniPageSize (DPI ratio).
    // See ENCORE_FORMAT.md §WINI block.
    // Cap each margin to a fraction of the page so a misread WINI cannot produce an absurd margin,
    // while still allowing legitimately large margins (2"+ are common on A3/landscape).
    static constexpr double kMaxMarginFrac = 0.45;

    double pageHIn = score->style().styleD(Sid::pageHeight);
    double pageWIn = score->style().styleD(Sid::pageWidth);

    // 1 pt tolerance mirrors detectPtsPageSize: metric page heights convert to fractional pts
    // (A4 297mm = 841.89pt, stored as 842) so the integer WINI value can exceed floor(pageH*72)
    // by 1 without being screen-pixels.
    static constexpr double kPixelTol = 1.0;
    const bool screenPixelFmt = (ps.rightEdge > static_cast<qint32>(pageWIn * 72.0 + kPixelTol))
                                || (ps.bottomEdge > static_cast<qint32>(pageHIn * 72.0 + kPixelTol));
    double scaleUpi = 72.0;
    if (pageSizeLocked) {
        // The page size is known (from PREC), so derive the WINI unit directly from the printable
        // extent rather than guessing whether it is points or screen pixels.
        scaleUpi = winiUnitsPerInch(ps.rightEdge, ps.left, pageWIn);
    } else if (screenPixelFmt) {
        const int pageWUnits = ps.rightEdge + ps.left;
        const int pageHUnits = ps.bottomEdge + ps.top;
        double detectedW = 0.0, detectedH = 0.0;
        if (detectWiniPageSize(pageWUnits, pageHUnits, detectedW, detectedH)) {
            pageWIn  = detectedW;
            pageHIn  = detectedH;
            score->style().set(Sid::pageWidth,  pageWIn);
            score->style().set(Sid::pageHeight, pageHIn);
        }
        scaleUpi = static_cast<double>(pageWUnits) / pageWIn;
    } else {
        double detectedW = 0.0, detectedH = 0.0;
        if (detectPtsPageSize(ps.rightEdge, ps.bottomEdge, detectedW, detectedH)) {
            pageWIn  = detectedW;
            pageHIn  = detectedH;
            score->style().set(Sid::pageWidth,  pageWIn);
            score->style().set(Sid::pageHeight, pageHIn);
        }
        // scaleUpi stays 72.0 (pts = 1/72 inch by definition)
    }

    double topIn  = ps.top / scaleUpi;
    double leftIn = ps.left / scaleUpi;
    double printW = (ps.rightEdge - ps.left) / scaleUpi;
    double printH = (ps.bottomEdge - ps.top) / scaleUpi;

    LOGD() << "  enc margins (in): T=" << QString::number(topIn,  'f', 3).toStdString()
           << "  L=" << QString::number(leftIn, 'f', 3).toStdString()
           << "  R=" << QString::number(pageWIn - leftIn - printW, 'f', 3).toStdString()
           << "  B=" << QString::number(pageHIn - topIn - printH, 'f', 3).toStdString()
           << "  paper=" << QString::number(pageWIn * 25.4, 'f', 1).toStdString()
           << "x" << QString::number(pageHIn * 25.4, 'f', 1).toStdString() << "mm"
           << (screenPixelFmt ? "  [pixels]" : "  [pts]");

    topIn  = std::clamp(topIn,  0.0, pageHIn * kMaxMarginFrac);
    leftIn = std::clamp(leftIn, 0.0, pageWIn * kMaxMarginFrac);

    const double maxPrintW = pageWIn - leftIn;
    if (printW > maxPrintW) {
        printW = maxPrintW;
    }

    double bottomIn = std::max(0.0, pageHIn - topIn - printH);
    bottomIn = std::min(bottomIn, pageHIn * kMaxMarginFrac);

    LOGD() << "  applied (in):     T=" << QString::number(topIn,   'f', 3).toStdString()
           << "  L=" << QString::number(leftIn,   'f', 3).toStdString()
           << "  R=" << QString::number(pageWIn - leftIn - printW, 'f', 3).toStdString()
           << "  B=" << QString::number(bottomIn, 'f', 3).toStdString()
           << "  paper=" << QString::number(pageWIn * 25.4, 'f', 1).toStdString()
           << "x" << QString::number(pageHIn * 25.4, 'f', 1).toStdString() << "mm";

    score->style().set(Sid::pageOddTopMargin,     topIn);
    score->style().set(Sid::pageEvenTopMargin,    topIn);
    score->style().set(Sid::pageOddLeftMargin,    leftIn);
    score->style().set(Sid::pageEvenLeftMargin,   leftIn);
    score->style().set(Sid::pagePrintableWidth,   printW);
    score->style().set(Sid::pageOddBottomMargin,  bottomIn);
    score->style().set(Sid::pageEvenBottomMargin, bottomIn);
}

// Inclusive [firstBlock, lastBlock] MEAS-block range covered by line[li]. Prefer the stored
// per-line measureCount; when it is absent (0, as in SCO5) fall back to the gap to the next
// line's start, or to totalBlocks for the last line. lastBlock < firstBlock when it spans nothing.
struct LineBlockSpan {
    int firstBlock;
    int lastBlock;
};

static LineBlockSpan lineSpanBlocks(const std::vector<EncLine>& lines, size_t li, int totalBlocks)
{
    const int firstBlock = static_cast<int>(lines[li].start);
    int span = static_cast<int>(lines[li].measureCount);
    if (span <= 0) {
        const int nextStart = (li + 1 < lines.size())
                              ? static_cast<int>(lines[li + 1].start)
                              : totalBlocks;
        span = nextStart - firstBlock;
    }
    return { firstBlock, firstBlock + span - 1 };
}

// SystemLocks enforce Encore's line layout as hard constraints so the engine compresses
// spacing within the system rather than redistributing measures across lines.
static void applySystemLocksFromLines(BuildCtx& ctx)
{
    const auto& lines  = ctx.enc.lines;
    const auto& enc2ms = ctx.encToMsIdx;
    const int totalMeas = static_cast<int>(ctx.measuresByIdx.size());

    for (size_t li = 0; li < lines.size(); ++li) {
        const auto [firstBlock, lastBlock] = lineSpanBlocks(lines, li, static_cast<int>(enc2ms.size()));

        if (firstBlock < 0 || lastBlock < firstBlock
            || firstBlock >= static_cast<int>(enc2ms.size())
            || lastBlock >= static_cast<int>(enc2ms.size())) {
            continue;
        }

        const int firstMsIdx = static_cast<int>(enc2ms[static_cast<size_t>(firstBlock)]);
        // Last MuseScore measure = first MS index of the last MEAS block's range plus that block's
        // span (the gap to the next block, or to the end).
        const int nextBlockMs = (lastBlock + 1 < static_cast<int>(enc2ms.size()))
                                ? static_cast<int>(enc2ms[static_cast<size_t>(lastBlock + 1)])
                                : totalMeas;
        const int lastMsIdx = nextBlockMs - 1;

        if (firstMsIdx < 0 || lastMsIdx < firstMsIdx
            || firstMsIdx >= totalMeas || lastMsIdx >= totalMeas) {
            continue;
        }

        Measure* firstM = ctx.measuresByIdx[static_cast<size_t>(firstMsIdx)];
        Measure* lastM  = ctx.measuresByIdx[static_cast<size_t>(lastMsIdx)];
        if (!firstM || !lastM) {
            continue;
        }
        ctx.score->addSystemLock(new RangeLock(firstM, lastM));
    }
}

// pageIdx in EncLineStaffData is the row-on-page counter (0-based, resets each page).
// A page break is placed at the end of line[i] whenever line[i+1].pageIdx <= line[i].pageIdx
// (the counter did not increment, meaning a new page started).
static void applyPageBreaksFromLines(BuildCtx& ctx)
{
    const auto& lines  = ctx.enc.lines;
    const auto& enc2ms = ctx.encToMsIdx;
    const int totalMeas = static_cast<int>(ctx.measuresByIdx.size());

    for (size_t li = 1; li < lines.size(); ++li) {
        const EncLine& prev = lines[li - 1];
        const EncLine& curr = lines[li];

        if (prev.staffData.empty() || curr.staffData.empty()) {
            continue;
        }
        if (curr.staffData[0].pageIdx > prev.staffData[0].pageIdx) {
            continue;   // same page: row counter incremented normally
        }

        // Page break: add LayoutBreak to the last measure of line[li-1].
        const auto [firstBlock, lastBlock] = lineSpanBlocks(lines, li - 1, static_cast<int>(enc2ms.size()));
        if (firstBlock < 0 || lastBlock < firstBlock
            || lastBlock >= static_cast<int>(enc2ms.size())) {
            continue;
        }
        const int nextBlockMs = (lastBlock + 1 < static_cast<int>(enc2ms.size()))
                                ? static_cast<int>(enc2ms[static_cast<size_t>(lastBlock + 1)])
                                : totalMeas;
        const int lastMsIdx = nextBlockMs - 1;
        if (lastMsIdx < 0 || lastMsIdx >= totalMeas) {
            continue;
        }
        Measure* lastM = ctx.measuresByIdx[static_cast<size_t>(lastMsIdx)];
        if (!lastM) {
            continue;
        }
        bool alreadyHasPageBreak = false;
        for (EngravingItem* e : lastM->el()) {
            if (e && e->isLayoutBreak() && toLayoutBreak(e)->isPageBreak()) {
                alreadyHasPageBreak = true;
                break;
            }
        }
        if (!alreadyHasPageBreak) {
            LayoutBreak* lb = Factory::createLayoutBreak(lastM);
            lb->setLayoutBreakType(LayoutBreakType::PAGE);
            lb->setTrack(0);
            lastM->add(lb);
        }
    }
}

void applyPageSetup(BuildCtx& ctx)
{
    MasterScore* score = ctx.score;
    const EncRoot& enc = ctx.enc;

    LOGD() << "  importPageLayout=" << (ctx.opts.importPageLayout ? "true" : "false");
    if (ctx.opts.importPageLayout) {
        const bool sizeFromPrec = applyPagePrintSetup(score, enc.printSetup);
        applyPageMargins(score, enc.pageSetup, sizeFromPrec);
        // SCO5 (macOS Encore 5) does not store document margins in any importable block:
        // WINI holds only window state, the PREC plist holds only printer rects, and some
        // files have no PREC at all. Apply a clean, symmetric 0.25" margin: forcing 0 looks
        // cramped (edge to edge), and MuseScore's default margins are tuned for A4 so they
        // come out asymmetric on Letter. A small uniform margin is the better default.
        if (enc.fmt && enc.fmt->usesUniformPageMargins()) {
            constexpr double kMacMarginIn = 0.25;
            const double pageWIn = score->style().styleD(Sid::pageWidth);
            score->style().set(Sid::pageOddTopMargin,     kMacMarginIn);
            score->style().set(Sid::pageEvenTopMargin,    kMacMarginIn);
            score->style().set(Sid::pageOddLeftMargin,    kMacMarginIn);
            score->style().set(Sid::pageEvenLeftMargin,   kMacMarginIn);
            score->style().set(Sid::pageOddBottomMargin,  kMacMarginIn);
            score->style().set(Sid::pageEvenBottomMargin, kMacMarginIn);
            score->style().set(Sid::pagePrintableWidth,   pageWIn - 2.0 * kMacMarginIn);
        } else if (sizeFromPrec && !enc.pageSetup.hasData) {
            // No WINI margins, but PREC set the page size (e.g. A4 landscape). MuseScore's default
            // printable width is sized for the portrait page, so the extra landscape width becomes a
            // lopsided right margin (~4" on A4 landscape). Keep the default margins but recompute the
            // printable width so the right margin equals the left. On a portrait page whose size
            // matches the default this is a no-op (printable already = width - 2*leftMargin).
            const double pageWIn = score->style().styleD(Sid::pageWidth);
            const double leftIn  = score->style().styleD(Sid::pageOddLeftMargin);
            score->style().set(Sid::pagePrintableWidth, pageWIn - 2.0 * leftIn);
        }
    }

    if (ctx.opts.importSystemLocks) {
        applySystemLocksFromLines(ctx);
    }
    if (ctx.opts.importPageBreaks) {
        applyPageBreaksFromLines(ctx);
    }
}

// Page index (0-based) that the first imported PAGE break's measure is laid out on, or -1 when
// there is no such break or its page cannot be resolved (caller then leaves the staff space
// untouched).
static int firstPageBreakPageIndex(MasterScore* score)
{
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        bool hasPageBreak = false;
        for (EngravingItem* e : mb->el()) {
            if (e && e->isLayoutBreak() && toLayoutBreak(e)->isPageBreak()) {
                hasPageBreak = true;
                break;
            }
        }
        if (!hasPageBreak) {
            continue;
        }
        const System* sys = toMeasure(mb)->system();
        if (!sys || !sys->page()) {
            return -1;
        }
        const std::vector<Page*>& pages = score->pages();
        for (size_t i = 0; i < pages.size(); ++i) {
            if (pages[i] == sys->page()) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
    return -1;
}

void fitFirstPageStaffSpace(BuildCtx& ctx)
{
    if (!ctx.opts.importPageBreaks) {
        return;
    }
    MasterScore* score = ctx.score;
    if (firstPageBreakPageIndex(score) <= 0) {
        // -1: no importable page break. 0: the first page already holds all its systems.
        return;
    }

    const double sp0 = score->style().styleD(Sid::spatium);
    constexpr double kStepInches = 0.002;   // reduction granularity
    constexpr int kMaxSteps      = 11;      // up to 0.022 inch total

    // Bubble up from the smallest reduction; the first that pulls the spilled system back onto
    // the first page is the ideal (least change from Encore's staff size).
    for (int k = 1; k <= kMaxSteps; ++k) {
        score->style().set(Sid::spatium, sp0 - kStepInches * k * DPI);
        score->doLayout();
        if (firstPageBreakPageIndex(score) == 0) {
            return;
        }
    }

    // Even a 0.022 inch reduction was not enough: restore Encore's original staff size.
    score->style().set(Sid::spatium, sp0);
    score->doLayout();
}
} // namespace mu::iex::enc
