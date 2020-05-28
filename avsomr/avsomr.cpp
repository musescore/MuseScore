//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "avsomr.h"

#include "avslog.h"

namespace {
static int TRASH_GLYPH_SIZE{ 10 };
static int DEFAULT_SYSTEM_GAP{ 200 };
static float SYSTEM_GAP_PERCENT{ 0.55f };
}

using namespace Ms::Avs;

AvsOmr::AvsOmr()
{
}

//---------------------------------------------------------
//   resolve - calculates some elements and attributes
//---------------------------------------------------------

void AvsOmr::resolve()
{
    auto firstMS = [](Sheet* sh) -> const MStack* {
                       if (sh->page.systems.empty()) {
                           return nullptr;
                       }

                       for (int si = 0; si < sh->page.systems.count(); ++si) {
                           if (sh->page.systems.at(si).mstacks.empty()) {
                               continue;
                           }

                           return &sh->page.systems.at(si).mstacks.first();
                       }
                       return nullptr;
                   };

    auto lastMS = [](Sheet* sh) -> const MStack* {
                      if (sh->page.systems.empty()) {
                          return nullptr;
                      }

                      for (int si = (sh->page.systems.count() - 1); si != 0; --si) {
                          if (sh->page.systems.at(si).mstacks.empty()) {
                              continue;
                          }
                          return &sh->page.systems.at(si).mstacks.last();
                      }
                      return nullptr;
                  };

    auto staffBarlineBBox = [](const Sheet* sh, const System& sys, const Staff& staff) {
                                IF_FAILED(!staff.barlines.empty()) {
                                    return QRect();
                                }

                                ID barID = staff.barlines.first();
                                Barline bar = sys.inters.barlines.value(barID);
                                const Glyph* gly = sh->glyphs.value(bar.glyphID, nullptr);
                                IF_FAILED(gly) {
                                    return QRect();
                                }

                                return gly->bbox;
                            };

    auto isTrashGlyph = [](const AvsOmr::Glyph* g) {
                            if (g->bbox.width() <= TRASH_GLYPH_SIZE && g->bbox.height() <= TRASH_GLYPH_SIZE) {
                                return true;
                            }

                            return false;
                        };

    Idx midx{ 0 };
    for (int shi = 0; shi < _sheets.count(); ++shi) {
        Sheet* sh = _sheets[shi];
        if (sh->page.systems.empty()) {
            continue;
        }

        for (System& sys : sh->page.systems) {
            // meausure idx
            for (MStack& ms : sys.mstacks) {
                ms.idx = midx;
                ++midx;
            }

            // system top and bottom
            IF_FAILED(!sys.part.staffs.empty()) {
                continue;
            }

            QRect topBarBBox = staffBarlineBBox(sh, sys, sys.part.staffs.first());
            QRect bottomBarBBox = staffBarlineBBox(sh, sys, sys.part.staffs.last());
            sys.top = topBarBBox.top();
            sys.bottom = bottomBarBBox.bottom() + 40;
        }

        // meausure ranges
        const MStack* fms = firstMS(sh);
        const MStack* lms = lastMS(sh);
        sh->mbeginIdx = fms ? fms->idx : 0;
        sh->mendIdx = lms ? lms->idx : 0;

        // glyph used
        QList<QRect> usedBBoxs;
        int gap = 5;
        for (const Glyph* g : sh->glyphs) {
            if (GlyphUsed::Used == g->used) {
                usedBBoxs.append(g->bbox.adjusted(-gap, -gap, gap, gap));
            }
        }

        auto isUsedContains = [](const QList<QRect>& usedBBoxs, const QRect& bbox) {
                                  for (const QRect& r : usedBBoxs) {
                                      if (r.contains(bbox)) {
                                          return true;
                                      }
                                  }
                                  return false;
                              };

        //! TODO optimization may be needed because complexity O^2
        for (Glyph* g : sh->glyphs) {
            if (isTrashGlyph(g)) {
                g->used = GlyphUsed::Trash;
            } else if (GlyphUsed::Free == g->used) {
                if (isUsedContains(usedBBoxs, g->bbox)) {
                    g->used = GlyphUsed::Free_Covered;
                }
            }
        }
    }
}

//---------------------------------------------------------
//   sheetNumByMeausereIdx
//---------------------------------------------------------

AvsOmr::Num AvsOmr::sheetNumByMeausereIdx(const Idx& meausureIdx) const
{
    for (const Sheet* sh : _sheets) {
        if (meausureIdx >= sh->mbeginIdx && meausureIdx <= sh->mendIdx) {
            return sh->num;
        }
    }

    return 0;
}

//---------------------------------------------------------
//   sheet
//---------------------------------------------------------

const AvsOmr::Sheet* AvsOmr::sheet(const Num& sheetNum) const
{
    for (const Sheet* sh : _sheets) {
        if (sheetNum == sh->num) {
            return sh;
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   isGlyphUsed
//---------------------------------------------------------

bool AvsOmr::Sheet::isGlyphUsed(const ID& glypthID) const
{
    for (const System& sys : page.systems) {
        if (sys.inters.usedglyphs.contains(glypthID)) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   isGlyphFree
//---------------------------------------------------------

bool AvsOmr::Sheet::isGlyphFree(const ID& glypthID) const
{
    for (const System& sys : page.systems) {
        if (sys.freeglyphs.contains(glypthID)) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   stackByIdx
//---------------------------------------------------------

const AvsOmr::MStack& AvsOmr::System::stackByIdx(Idx idx, Idx* idxInSys) const
{
    for (int i = 0; i < mstacks.count(); ++i) {
        const MStack& m = mstacks.at(i);
        if (m.idx == idx) {
            if (idxInSys) {
                *idxInSys = i;
            }

            return m;
        }
    }

    static MStack dummy;
    return dummy;
}

//---------------------------------------------------------
//   mmetrics
//---------------------------------------------------------

AvsOmr::MMetrics AvsOmr::mmetrics(const Num& sheetNum, const Idx& meausureIdx) const
{
    const Sheet* sh = sheet(sheetNum);
    IF_ASSERT(sh) {
        return MMetrics();
    }

    MMetrics mm;
    int sysCount = sh->page.systems.count();
    for (int si = 0; si < sysCount; ++si) {
        const System& sys = sh->page.systems.at(si);

        Idx idxInSys{ 0 };
        const AvsOmr::MStack& m = sys.stackByIdx(meausureIdx, &idxInSys);
        if (!m.isValid()) {
            continue;
        }

        // bbox
        mm.bbox.setLeft(m.left);
        mm.bbox.setRight(m.right);
        mm.bbox.setTop(sys.top);
        mm.bbox.setBottom(sys.bottom);

        // bbox header
        if (0 == idxInSys) {
            if (!sys.part.staffs.empty()) {
                const Staff& topStaff = sys.part.staffs.first();
                mm.hbbox.setLeft(topStaff.header.start);
                mm.hbbox.setRight(topStaff.header.stop);
                mm.hbbox.setTop(mm.bbox.top());
                mm.hbbox.setBottom(mm.bbox.bottom());
            }
        }

        // bbox elements
        mm.ebbox = mm.bbox;

        bool isFirstSys = (0 == si);
        bool isLastSys = ((sysCount - 1) == si);
        int halfH = mm.bbox.height() / 2;

        if (1 == sysCount) {
            mm.ebbox.setTop(mm.ebbox.top() - halfH);
            mm.ebbox.setBottom(mm.ebbox.bottom() + halfH);
        } else {
            auto gapToNextSys = [sh](const System& sys, size_t si) {
                                    const System& nextSys = sh->page.systems.at(si + 1);
                                    int gapSys = nextSys.top - sys.bottom;
                                    IF_ASSERT(gapSys > 0) {
                                        gapSys = DEFAULT_SYSTEM_GAP;
                                    }
                                    return gapSys;
                                };

            auto gapToPrevSys = [sh](const System& sys, size_t si) {
                                    const System& prevSys = sh->page.systems.at(si - 1);
                                    int gapSys = sys.top - prevSys.bottom;
                                    IF_ASSERT(gapSys > 0) {
                                        gapSys = DEFAULT_SYSTEM_GAP;
                                    }
                                    return gapSys;
                                };

            if (isFirstSys) {
                mm.ebbox.setTop(mm.ebbox.top() - halfH);
                int gapSys = gapToNextSys(sys, si);
                mm.ebbox.setBottom(mm.ebbox.bottom() + (gapSys * SYSTEM_GAP_PERCENT));
            } else if (isLastSys) {
                int gapSys = gapToPrevSys(sys, si);
                mm.ebbox.setTop(mm.ebbox.top() - (gapSys * SYSTEM_GAP_PERCENT));
                mm.ebbox.setBottom(mm.ebbox.bottom() + halfH);
            } else {
                int gapPrevSys = gapToPrevSys(sys, si);
                mm.ebbox.setTop(mm.ebbox.top() - (gapPrevSys * SYSTEM_GAP_PERCENT));

                int gapNextSys = gapToNextSys(sys, si);
                mm.ebbox.setBottom(mm.ebbox.bottom() + (gapNextSys * SYSTEM_GAP_PERCENT));
            }
        }

        //! RETURN
        return mm;
    }

    return mm;
}

//---------------------------------------------------------
//   glyphsByBBox
//---------------------------------------------------------

QList<const AvsOmr::Glyph*> AvsOmr::glyphsByBBox(const Num& sheetNum, const QRect& bbox,
                                                 QList<GlyphUsed>& accepted) const
{
    const Sheet* sh = sheet(sheetNum);
    IF_ASSERT(sh) {
        return QList<const AvsOmr::Glyph*>();
    }

    QList<const Glyph*> list;
    for (auto g : sh->glyphs) {
        if (!accepted.contains(g->used)) {
            continue;
        }

        if (bbox.contains(g->bbox)) {
            list.append(g);
        }
    }

    return list;
}

//---------------------------------------------------------
//   config
//---------------------------------------------------------

AvsOmr::Config& AvsOmr::config()
{
    return _config;
}

//---------------------------------------------------------
//   config const
//---------------------------------------------------------

const AvsOmr::Config& AvsOmr::config() const
{
    return _config;
}

//---------------------------------------------------------
//   setMsmrFile
//---------------------------------------------------------

void AvsOmr::setMsmrFile(std::shared_ptr<MsmrFile> file)
{
    _msmrFile = file;
}

//---------------------------------------------------------
//   msmrFile
//---------------------------------------------------------

std::shared_ptr<MsmrFile> AvsOmr::msmrFile() const
{
    return _msmrFile;
}

//---------------------------------------------------------
//   info
//---------------------------------------------------------

const AvsOmr::Info& AvsOmr::info() const
{
    return _info;
}
