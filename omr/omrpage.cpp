//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010-2011 Werner Schweer
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

#include "omrpage.h"
#include "image.h"
#include "utils.h"
#include "omr.h"
#ifdef OCR
#include "ocr.h"
#endif
#include "libmscore/score.h"
#include "libmscore/text.h"
#include "libmscore/measurebase.h"
#include "libmscore/box.h"
#include "libmscore/sym.h"
#include "libmscore/note.h"
#include "pattern.h"

namespace Ms {
    //static const double noteTH = 1.0;
    static const double timesigTH = 0.7;
    //static const double clefTH = 0.7;
    static const double keysigTH = 0.8;

    struct Hv {
        int x;
        int val;
        Hv(int a, int b) : x(a), val(b) {}
        bool operator< (const Hv& a) const { return a.val < val; }
    };

struct Peak {
      int x;
      double val;
      int sym;

      bool operator<(const Peak& p) const { return p.val < val; }
      Peak(int _x, double _val) : x(_x), val(_val) {}
      Peak(int _x, double _val, int s) : x(_x), val(_val), sym(s) {}
      };

//---------------------------------------------------------
//   Lv
//    line + value
//---------------------------------------------------------

struct Lv {
      int line;
      double val;
      Lv(int a, double b) : line(a), val(b) {}
      bool operator< (const Lv& a) const { return a.val < val; }
      };

//---------------------------------------------------------
//   OmrPage
//---------------------------------------------------------

OmrPage::OmrPage(Omr* parent)
      {
      _omr = parent;
      cropL = cropR = cropT = cropB = 0;
      }

//---------------------------------------------------------
//   dot
//---------------------------------------------------------

bool OmrPage::dot(int x, int y) const
      {
      const uint* p = scanLine(y) + (x / 32);
      return (*p) & (0x1 << (x % 32));
      }

//---------------------------------------------------------
//   isBlack
//---------------------------------------------------------

bool OmrPage::isBlack(int x, int y) const
      {
      QRgb c = _image.pixel(x,y);
      return (qGray(c) < 100);
      }


//---------------------------------------------------------
//   read
//---------------------------------------------------------

void OmrPage::read()
      {
      //removeBorder();
      crop();
      slice();
      deSkew();
      crop();
      slice();
      getStaffLines();
      getRatio();
      }

struct SysState {
      int status;// 0 for start_staff, 1 for end_staff
      int index;//staff index
      };


struct BAR_STATE {
      int x;
      int status;//0 represents white space, 1 represents bar
      };

//---------------------------------------------------------
//   searchBarLines
//---------------------------------------------------------
float OmrPage::searchBarLines(int start_staff, int end_staff)
      {
      OmrStaff& r1 = staves[start_staff];
      OmrStaff& r2 = staves[end_staff];

      int x1 = r1.x();
      int x2 = x1 + r1.width();
      int y1 = r1.y();
      int y2 = r2.y() + r2.height();

      int th = 0;
      for (int i = start_staff; i <= end_staff; ++i) {
            th += staves[i].height() / 2;
            }

      int vpw = x2 - x1;
#if (!defined (_MSCVER) && !defined (_MSC_VER))
      float vp[vpw];
      memset(vp, 0, sizeof(float) * vpw);

      //using note constraints
      //searchNotes();

      int note_constraints[x2 - x1];
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<float> vp(vpw);      // Default-initialized, doesn't need to be cleared
      std::vector<int> note_constraints(x2 - x1);
#endif
      for (OmrNote* n : r1.notes()) {
            for(int x = n->x(); x <= n->x() + n->width(); ++x)
                  note_constraints[x - x1] = 1;
            }
      for (OmrNote* n : r2.notes()) {
            for(int x = n->x(); x <= n->x() + n->width(); ++x)
                  note_constraints[x - x1] = 1;
            }

      //
      // compute vertical projections
      //
      for (int x = x1; x < x2; ++x) {
            int dots = 0;
            for (int y = y1; y < y2; ++y) {
                  if (this->dot(x, y))
                        ++dots;
                  }
            if (!note_constraints[x - x1])
                  vp[x - x1] = dots - th;
            else
                  vp[x - x1] = -HUGE_VAL;
            }

#if (!defined (_MSCVER) && !defined (_MSC_VER))
      float scores[x2 - x1 + 1][2];
      BAR_STATE pred[x2 - x1 + 1][2];
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<float[2]> scores(x2 - x1 + 1);
      std::vector<BAR_STATE[2]> pred(x2 - x1 + 1);
#endif
      BAR_STATE bs;

      //initialization
      bs.x = -1; bs.status = -1;
      for (int x = x1; x <= x2; ++x) {
            int i = x - x1;
            for (int status = 0; status <= 1; ++status) {
                  scores[i][status] = -HUGE_VAL;
                  pred[i][status] = bs;
                  }
            }
      scores[0][0] = 0;

      //forward pass
      for (int x = x1; x <= x2; ++x) {
            int i = x - x1;
            for (int cur = 0; cur <= 1; ++cur) {
                  //current state
                  if (scores[i][cur] < -1000)
                        continue;

                  if (cur) {
                        scores[i][cur] += vp[i];
                        int next = 0;
                        int step = 3 * _spatium;//constraints between adjacent barlines
                        if (step + i <= x2 - x1) {
                              if (scores[step + i][next] < scores[i][cur]) {
                                    scores[step + i][next] = scores[i][cur];
                                    bs.x = x; bs.status = cur;
                                    pred[step + i][next] = bs;
                                    }
                              }
                        }
                  else {
                        for (int next = 0; next <= 1; ++next) {
                              int step = 1;
                              if (step + i <= x2 - x1) {
                                    if (scores[step + i][next] < scores[i][cur]) {
                                          scores[step + i][next] = scores[i][cur];
                                          bs.x = x; bs.status = cur;
                                          pred[step + i][next] = bs;
                                          }
                                    }
                              }
                        }
                  }
            }
      return(scores[x2][0]);
      }

//---------------------------------------------------------
//   identifySystems
//---------------------------------------------------------
void OmrPage::identifySystems()
      {
      int numStaves = staves.size();
      if(numStaves == 0) return;

      //
      //memory allocation
      //
      float **temp_scores = new float*[numStaves];
      for (int i = 0; i < numStaves; i++)
            temp_scores[i] = new float[numStaves];

      int **hashed = new int*[numStaves];
      for (int i = 0; i < numStaves; i++)
            hashed[i] = new int[numStaves];

      float **scores = new float*[numStaves];
      for (int i = 0; i < numStaves; i++)
            scores[i] = new float[2];
      SysState **pred = new SysState*[numStaves];
      for (int i = 0; i < numStaves; i++)
            pred[i] = new SysState[2];

      //
      //initialization
      //
      for (int i = 0; i < numStaves; i++) {
            for (int j = 0; j < numStaves; j++) {
                  hashed[i][j] = 0;
                  }
            }

      SysState ss;
      ss.index = -1; ss.status = -1;

      for (int i = 0; i < numStaves; ++i) {
            for (int j = 0; j < 2; j++) {
                  scores[i][j] = -HUGE_VAL;
                  pred[i][j] = ss;
                  }
            }
      scores[0][0] = 0;

      //
      //identify solid note heads
      //
      int **note_labels = new int*[numStaves];
      for (int i = 0; i < numStaves; i++)
            note_labels[i] = new int[width()];
      for (int i = 0; i < numStaves; i++){
            for (int j = 0; j < width(); j++){
                  note_labels[i][j] = 0;
                  }
            }

      //
      // search notes for each system
      //
//      int note_ran = spatium()/4.0;
//      for (int i = 0; i < numStaves; i++){
//            OmrSystem omrSystem(this);
//            omrSystem.staves().append(staves[i]);
//            omrSystem.searchNotes(note_labels[i], note_ran);
//            }

      //
      // System Identification
      //
      int status;
      float cur_score;
      int cur_staff,next_staff;
      for (cur_staff = 0; cur_staff < numStaves; ++cur_staff) {
            status = 0;//start_staff
            for (next_staff = cur_staff; next_staff < numStaves; ++next_staff) { //connects to end_staff
                  if (hashed[cur_staff][next_staff]) {
                        cur_score = temp_scores[cur_staff][next_staff];
                        }
                  else {
                        //evaluate staff segment [c,n], dp here
                        OmrSystem omrSystem(this);
                        for (int i = cur_staff; i <= next_staff; ++i) {
                              omrSystem.staves().append(staves[i]);
                              }

                        cur_score = omrSystem.searchBarLinesvar(next_staff - cur_staff + 1, note_labels + cur_staff);
                        temp_scores[cur_staff][next_staff] = cur_score;
                        hashed[cur_staff][next_staff] = 1;
                        }

                  //forward pass
                  if (scores[cur_staff][status] + cur_score > scores[next_staff][1 - status]) {
                        scores[next_staff][1 - status] = scores[cur_staff][status] + cur_score;
                        ss.status = status; ss.index = cur_staff;
                        pred[next_staff][1 - status] = ss;
                        }
                  }

            status = 1;//end_staff
            next_staff = cur_staff + 1;
            if (next_staff < numStaves) {
                  //forward pass
                  if (scores[cur_staff][status] > scores[next_staff][1 - status]) {
                        scores[next_staff][1 - status] = scores[cur_staff][status];
                        ss.status = status; ss.index = cur_staff;
                        pred[next_staff][1 - status] = ss;
                        }
                  }
            }

      //backtrack
      status = 1;//end_staff
      cur_staff = numStaves - 1;//last staff index
      while (cur_staff > 0 || status != 0) {
            ss = pred[cur_staff][status];

            if (!ss.status) {
                  //add system here
                  OmrSystem omrSystem(this);
                  for (int i = ss.index; i <= cur_staff; ++i) {
                        omrSystem.staves().append(staves[i]);
                        }
                  omrSystem.searchBarLinesvar(cur_staff - ss.index + 1, note_labels + ss.index);
                  _systems.append(omrSystem);
                  }
            cur_staff = ss.index;
            status = ss.status;
            }

      int systems = _systems.size();
      for (int i = 0; i < systems; ++i) {
            OmrSystem* system = &_systems[i];
            int n = system->barLines.size();
            for (int k = 0; k < n - 1; ++k) {
                  const QLine& l1 = system->barLines[k];
                  const QLine& l2 = system->barLines[k + 1];
                  OmrMeasure m(l1.x1(), l2.x1());
                  system->measures().append(m);
                  }
            }

      //delete allocated space
      for (int i = 0; i < numStaves; i++) {
            delete[] scores[i];
            delete[] pred[i];
            delete[] temp_scores[i];
            delete[] hashed[i];
            delete[] note_labels[i];
            }

      delete[] note_labels;
      delete[] scores;
      delete[] pred;
      delete[] temp_scores;
      delete[] hashed;
      }


//---------------------------------------------------------
//   readBarLines
//---------------------------------------------------------

void OmrPage::readBarLines()
      {
      //QFuture<void> bl = QtConcurrent::run(_systems, &OmrSystem::searchSysBarLines());
      //bl.waitForFinished();

      //int numStaves    = staves.size();
      int systems = _systems.size();

      for (int i = 0; i < systems; ++i) {
            OmrSystem* system = &_systems[i];
            int n = system->barLines.size();
            for (int k = 0; k < n - 1; ++k) {
                  const QLine& l1 = system->barLines[k];
                  const QLine& l2 = system->barLines[k + 1];
                  OmrMeasure m(l1.x1(), l2.x1());
                  for (int ss = 0; ss < system->staves().size(); ++ss) {
                        OmrStaff& staff = system->staves()[ss];
                        QList<OmrChord> chords;
                        int nx = 0;
                        SymId nsym = SymId::noSym;
                        OmrChord chord;
                        for(OmrNote* n1 : staff.notes()) {
                              int x = n1->x();
                              if (x >= m.x2())
                                    break;
                              if (x >= m.x1() && x < m.x2()) {
                                    if (qAbs(x - nx) > int(_spatium / 2) || (nsym != n1->sym)) {
                                          if (!chord.notes.isEmpty()) {
                                                SymId sym = chord.notes.front()->sym;
                                                if (sym == SymId::noteheadBlack)
                                                      chord.duration.setType(TDuration::DurationType::V_QUARTER);
                                                else if (sym == SymId::noteheadHalf)
                                                      chord.duration.setType(TDuration::DurationType::V_HALF);
                                                chords.append(chord);
                                                chord.notes.clear();
                                                }
                                          }
                                    nx = x;
                                    nsym = n1->sym;
                                    chord.notes.append(n1);
                                    }
                              }
                        if (!chord.notes.isEmpty()) {
                              SymId sym = chord.notes.front()->sym;
                              if (sym == SymId::noteheadBlack)
                                    chord.duration.setType(TDuration::DurationType::V_QUARTER);
                              else if (sym == SymId::noteheadHalf)
                                    chord.duration.setType(TDuration::DurationType::V_HALF);
                              chords.append(chord);
                              }
                        m.chords().append(chords);
                        }
                  system->measures().append(m);
                  }
            }

      }

//---------------------------------------------------------
//   searchClef
//---------------------------------------------------------

OmrClef OmrPage::searchClef(OmrSystem* system, OmrStaff* staff)
      {
      std::vector<Pattern*> pl = {
            Omr::trebleclefPattern,
            Omr::bassclefPattern
            };
      const OmrMeasure& m = system->measures().front();
      printf("search clef %d   %d-%d\n", staff->y(), m.x1(), m.x2());

      int x1 = m.x1() + 2;
      int x2 = x1 + (m.x2() - x1) / 2;
      OmrPattern p = searchPattern(pl, staff->y(), x1, x2);

      OmrClef clef(p);
      if (p.sym == SymId::gClef)
            clef.type = ClefType::G;
      else if (p.sym == SymId::fClef)
            clef.type = ClefType::F;
      else
            clef.type = ClefType::G;
      return clef;
      }

//---------------------------------------------------------
//   searchPattern
//---------------------------------------------------------

OmrPattern OmrPage::searchPattern(const std::vector<Pattern*>& pl, int y, int x1, int x2)
      {
      OmrPattern p;
      p.sym = SymId::noSym;
      p.prob = 0.0;
      for (Pattern* pattern : pl) {
            double val = 0.0;
            int xx = 0;
            int hw = pattern->w();

            for (int x = x1; x < (x2 - hw); ++x) {
                  double val1 = pattern->match(&image(), x - pattern->base().x(), y - pattern->base().y());
                  if (val1 > val) {
                        val = val1;
                        xx = x;
                        }
                  }

            if (val > p.prob) {
                  p.setRect(xx, y, pattern->w(), pattern->h());
                  p.sym = pattern->id();
                  p.prob = val;
                  }
            printf("Pattern found %d %f %d\n", int(pattern->id()), val, xx);
            }
      return p;
      }

//---------------------------------------------------------
//   searchTimeSig
//---------------------------------------------------------

OmrTimesig* OmrPage::searchTimeSig(OmrSystem* system)
      {
      int z = -1;
      int n = -1;
      double zval = 0;
      double nval = 0;
      QRect rz, rn;

      int y         = system->staves().front().y();
      OmrMeasure* m = &system->measures().front();
      int x1        = m->x1();

      for (int i = 0; i < 10; ++i) {
            Pattern* pattern = Omr::timesigPattern[i];
            double val = 0.0;
            int hh = pattern->h();
            int hw = pattern->w();
            int x2 = m->x2() - hw;
            QRect r;

            for (int x = x1; x < x2; ++x) {
                  double val1 = pattern->match(&image(), x, y);
                  if (val1 > val) {
                        val = val1;
                        r = QRect(x, y, hw, hh);
                        }
                  }

            if (val > timesigTH && val > zval) {
                  z = i;
                  zval = val;
                  rz = r;
                  }
            //            printf("   found %d %f\n", i, val);
            }

      if (z < 0)
            return 0;
      y = system->staves().front().y() + lrint(_spatium * 2);

      x1 = rz.x();
      int x2 = x1 + 1;
      OmrTimesig* ts = 0;
      for (int i = 0; i < 10; ++i) {
            Pattern* pattern = Omr::timesigPattern[i];
            double val = 0.0;
            int hh = pattern->h();
            int hw = pattern->w();
            QRect r;

            for (int x = x1; x < x2; ++x) {
                  double val1 = pattern->match(&image(), x, y);
                  if (val1 > val) {
                        val = val1;
                        r = QRect(x, y, hw, hh);
                        }
                  }

            if (val > timesigTH && val > nval) {
                  n = i;
                  nval = val;
                  rn = r;
                  }
            //            printf("   found %d %f\n", i, val);
            }
      if (n > 0) {
            ts = new OmrTimesig(rz | rn);
            ts->timesig = Fraction(z, n);
            printf("timesig  %d/%d\n", z, n);
            }
      return ts;
      }

//---------------------------------------------------------
//   searchKeySig
//---------------------------------------------------------

void OmrPage::searchKeySig(OmrSystem* system, OmrStaff* staff)
      {
      Pattern* pl[2];
      pl[0] = Omr::sharpPattern;
      pl[1] = Omr::flatPattern;

      double zval = 0;

      int y = system->staves().front().y();
      OmrMeasure* m = &system->measures().front();
      int x1 = m->x1();

      for (int i = 0; i < 2; ++i) {
            Pattern* pattern = pl[i];
            double val = 0.0;
            int hh = pattern->h();
            int hw = pattern->w();
            int x2 = m->x2() - hw;
            QRect r;

            for (int x = x1; x < x2; ++x) {
                  double val1 = pattern->match(&image(), x, y - hh / 2);
                  if (val1 > val) {
                        val = val1;
                        r = QRect(x, y, hw, hh);
                        }
                  }

            if (val > keysigTH && val > zval) {
                  zval = val;
                  OmrKeySig key(r);
                  key.type = i == 0 ? 1 : -1;
                  staff->setKeySig(key);
                  }
            printf(" key found %d %f\n", i, val);
            }
      }

//---------------------------------------------------------
//   maxP
//---------------------------------------------------------

int maxP(int* projection, int x1, int x2)
      {
      int xx = x1;
      int max = 0;
      for (int x = x1; x < x2; ++x) {
            if (projection[x] > max) {
                  max = projection[x];
                  xx = x;
                  }
            }
      return xx;
      }

//---------------------------------------------------------
//   BSTATE
//---------------------------------------------------------
struct BSTATE {
      int x;
      int status;//0 represents white space, 1 represents bar
      };


//---------------------------------------------------------
//   searchSysBarLines
//---------------------------------------------------------

void OmrSystem::searchSysBarLines()
      {
      OmrStaff& r1 = _staves[0];
      OmrStaff& r2 = _staves[_staves.size() - 1];//[1];

      int x1 = r1.x();
      int x2 = x1 + r1.width();
      int y1 = r1.y();
      int y2 = r2.y() + r2.height();
      int h = y2 - y1 + 1;
      int th = /*r1.height() + r2.height() - 5;*/ h / 2;     // threshold, data score for null model

      int vpw = x2 - x1;
#if (!defined (_MSCVER) && !defined (_MSC_VER))
      float vp[vpw];
      memset(vp, 0, sizeof(float) * vpw);

      //using note constraints
      searchNotes();

      int note_constraints[x2 - x1];
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<float> vp(vpw);      // Default-initialized, doesn't need to be cleared
      
      //using note constraints
      searchNotes();

      std::vector<int> note_constraints(x2 - x1);
#endif
      for (int i = 0; i < _staves.size(); i++) {
            OmrStaff& r = _staves[i];
            for (OmrNote* n : r.notes()) {
                  for (int x = n->x(); x <= n->x() + n->width(); ++x)
                        note_constraints[x - x1] = 1;
                  }
            }

      //
      // compute vertical projections
      //
      for (int x = x1; x < x2; ++x) {
            int dots = 0;
            for (int y = y1; y < y2; ++y) {
                  if (_page->dot(x, y))
                        ++dots;
                  }
            if (!note_constraints[x - x1])
                  vp[x - x1] = dots - th;
            else
                  vp[x - x1] = -HUGE_VAL;
            }

#if (!defined (_MSCVER) && !defined (_MSC_VER))
      float scores[x2 - x1 + 1][2];
      BSTATE pred[x2 - x1 + 1][2];
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<float[2]> scores(x2 - x1 + 1);
      std::vector<BSTATE[2]> pred(x2 - x1 + 1);
#endif
      BSTATE bs;

      //initialization
      bs.x = -1; bs.status = -1;
      for (int x = x1; x <= x2; ++x) {
            int i = x - x1;
            for (int status = 0; status <= 1; ++status) {
                  scores[i][status] = -HUGE_VAL;
                  pred[i][status] = bs;
                  }
            }
      scores[0][0] = 0;

      //forward pass
      for (int x = x1; x < x2; ++x) {
            int i = x - x1;
            for (int cur = 0; cur <= 1; ++cur) {
                  //current state
                  if (scores[i][cur] < -1000) continue;

                  if (cur) {
                        scores[i][cur] += vp[i];
                        int next = 0;
                        int step = 3 * _page->spatium();//constraints between adjacent barlines
                        if (step + i <= x2 - x1) {
                              if (scores[step + i][next] < scores[i][cur]) {
                                    scores[step + i][next] = scores[i][cur];
                                    bs.x = x; bs.status = cur;
                                    pred[step + i][next] = bs;
                                    }
                              }
                        }
                  else {
                        for (int next = 0; next <= 1; ++next) {
                              int step = 1;
                              if (step + i <= x2 - x1) {
                                    if(scores[step + i][next] < scores[i][cur]) {
                                          scores[step + i][next] = scores[i][cur];
                                          bs.x = x; bs.status = cur;
                                          pred[step + i][next] = bs;
                                          }
                                    }
                              }
                        }
                  }
            }

      //trace back
      int state = 0;
      for (int x = x2; x > x1; ) {
            int i = x - x1;
            bs = pred[i][state];
            state = bs.status;
            x = bs.x;

            if(state)
                  barLines.append(QLine(x, y1, x, y2));
            }
      }

//-------------------------------------------------------------------
//   searchBarLinesvar: dynamic programming for system identification
//-------------------------------------------------------------------
float OmrSystem::searchBarLinesvar(int n_staff, int **note_labels)
      {
      OmrStaff& r1 = _staves[0];
      OmrStaff& r2 = _staves[n_staff - 1];

      int x1 = r1.left();
      int x2 = r1.right();
      int y1 = r1.top();
      int y2 = r2.bottom();

      int th = (y2 - y1) / 2;//threshold

      //
      //compute note constraints
      //
      int *note_constraints = new int[x2 - x1 + 1];
      memset(note_constraints, 0, sizeof(int) * (x2 - x1 + 1));
      for (int i = 0; i < n_staff; ++i) {
            for (int j = x1; j <= x2; ++j) {
                  if (note_labels[i][j] == 1)
                        note_constraints[j - x1] = 1;
                  }
            }

      //
      // compute vertical projections
      //

      int vpw = x2 - x1 + 1;
#if (!defined (_MSCVER) && !defined (_MSC_VER))
      float vp[vpw];

      for(int i = 0; i < vpw; i++) vp[i] = -HUGE_VAL;
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<float> vp(vpw, -HUGE_VAL);       // auto-initialized to -HUGE_VAL, doesn't need the loop
#endif


      int bar_max_width = 3;
      for (int x = x1 + bar_max_width; x < x2 - bar_max_width; ++x) {
            int dots = 0;
            for (int y = y1; y < y2; ++y) {
                  for (int w = -bar_max_width; w <= bar_max_width; w++) {
                        if (_page->dot(x + w, y)) {
                              ++dots;
                              break;
                              }
                        }
                  }

            if (!note_constraints[x - x1])
                  vp[x - x1] = dots - th;
            else
                  vp[x - x1] = -HUGE_VAL;
            }

      float **scores = new float *[x2 - x1 + 1];
      BSTATE **pred = new BSTATE *[x2 - x1 + 1];
      for (int i = 0; i< x2 - x1 + 1; i++) {
            scores[i] = new float[2];
            pred[i] = new BSTATE[2];
            }

      BSTATE bs;

      //initialization
      bs.x = -1; bs.status = -1;
      for (int x = x1; x <= x2; ++x) {
            int i = x - x1;
            for (int status = 0; status <= 1; ++status) {
                  scores[i][status] = -HUGE_VAL;
                  pred[i][status] = bs;
                  }
            }
      scores[0][0] = 0;

      //forward pass

      for (int x = x1; x <= x2; ++x) {
            int i = x - x1;

            for(int cur = 0; cur <= 1; ++cur) {
                  //current state
                  if (scores[i][cur] < -1000)
                        continue;

                  if (cur) {
                        scores[i][cur] += vp[i];
                        int next = 0;
                        int step = 8 * _page->spatium();//minimum distance between adjacent barlines
                        if (step + i <= x2 - x1) {
                              if (scores[step + i][next] < scores[i][cur]) {
                                    scores[step + i][next] = scores[i][cur];
                                    bs.x = x; bs.status = cur;
                                    pred[step + i][next] = bs;
                                    }
                              }
                        else {
                              if (scores[x2 - x1][next] < scores[i][cur]) {
                                    scores[x2 - x1][next] = scores[i][cur];
                                    bs.x = x; bs.status = cur;
                                    pred[x2 - x1][next] = bs;
                                    }
                              }

                        }
                  else {
                        for (int next = 0; next <= 1; ++next) {
                              int step = 1;
                              if (step + i <= x2 - x1) {
                                    if (scores[step + i][next] < scores[i][cur]) {
                                          scores[step + i][next] = scores[i][cur];
                                          bs.x = x; bs.status = cur;
                                          pred[step + i][next] = bs;
                                          }
                                    }
                              }
                        }
                  }
            }

      //trace back
      int state = 0;

      for (int x = x2; x > x1; ) {
            int i = x - x1;
            bs = pred[i][state];
            state = bs.status;
            x = bs.x;

            if(state) {
                  barLines.append(QLine(x, y1, x, y2));
                  }
            }

      float best_score = scores[x2 - x1][0];

      delete []note_constraints;
      for (int i = 0; i< x2 - x1 + 1; i++) {
            delete []scores[i];
            delete []pred[i];
            }
      delete []scores;
      delete []pred;


      return(best_score);
      }

//---------------------------------------------------------
//   noteCompare
//---------------------------------------------------------

static bool noteCompare(OmrNote* n1, OmrNote* n2)
      {
      return n1->x() < n2->x();
      }

static bool intersectFuzz(const QRect& a, const QRect& b, int fuzz)
      {
      int xfuzz = fuzz;
      int yfuzz = fuzz;
      if (a.x() > b.x())
            xfuzz = -xfuzz;
      if (a.y() > b.y())
            yfuzz = -yfuzz;

      return (a.intersects(b.translated(xfuzz, yfuzz)));
      }

//---------------------------------------------------------
//   searchNotes
//---------------------------------------------------------

void OmrSystem::searchNotes()
      {
      for (int i = 0; i < _staves.size(); ++i) {
            OmrStaff* r = &_staves[i];
            int x1 = r->x();
            int x2 = x1 + r->width();

            //
            // search notes on a range of vertical line position
            //
            for (int line = 0; line < 8; ++line)
                  searchNotes(&r->notes(), x1, x2, r->y(), line);

            //
            // detect collisions
            //
            int fuzz = int(_page->spatium()) / 2;
            for(OmrNote* n : r->notes()) {
                  for(OmrNote* m : r->notes()) {
                        if (m == n)
                              continue;
                        if (intersectFuzz(*m, *n, fuzz)) {
                              if (m->prob > n->prob)
                                    r->notes().removeOne(n);
                              else
                                    r->notes().removeOne(m);
                              }
                        }
                  }
            qSort(r->notes().begin(), r->notes().end(), noteCompare);
            }
      }

//---------------------------------------------------------
//   searchNotes
//---------------------------------------------------------

void OmrSystem::searchNotes(int *note_labels, int ran)
      {
      for (int i = 0; i < _staves.size(); ++i) {
            OmrStaff* r = &_staves[i];
            int x1 = r->x();
            int x2 = x1 + r->width();

            //
            // search notes on a range of vertical line position
            //
            for (int line = -5; line < 14; ++line)
                  searchNotes(&r->notes(), x1, x2, r->y(), line);

            //
            // save detected note horizontal positions into note_labels
            //
            for(OmrNote* n : r->notes()) {
                  QPoint p = n->center();
                  int h_cent = p.x();
                  for(int h = h_cent - ran; h <= h_cent + ran; h++){
                        if(h >= x1 && h < x2) note_labels[h] = 1;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

#ifdef OCR
static void addText(Score* score, int subtype, const QString& s)
{
#if 0 //TODO-1
      MeasureBase* measure = score->first();
      if (measure == 0 || measure->type() != Element::VBOX) {
            measure = new VBox(score);
            measure->setNext(score->first());
            measure->setTick(0);
            score->add(measure);
            }
      Text* text = new Text(score);
      switch(subtype) {
            case TEXT_TITLE:    text->setTextStyle(TEXT_STYLE_TITLE);    break;
            case TEXT_SUBTITLE: text->setTextStyle(TEXT_STYLE_SUBTITLE); break;
            case TEXT_COMPOSER: text->setTextStyle(TEXT_STYLE_COMPOSER); break;
            case TEXT_POET:     text->setTextStyle(TEXT_STYLE_POET);     break;
            }
      text->setSubtype(subtype);
      text->setText(s);
      measure->add(text);
#endif
      }
#endif

//---------------------------------------------------------
//   readHeader
//---------------------------------------------------------

void OmrPage::readHeader(Score*)
      {
      if (_slices.isEmpty())
            return;
      double maxHeight = _spatium * 4 * 2;

      int slice = 0;
      double maxH = 0.0;
      //      int maxIdx;
      for (; slice < _slices.size(); ++slice) {
            double h = _slices[slice].height();

            if (h > maxHeight)
                  break;
            if (h > maxH) {
                  maxH = h;
                  //                  maxIdx = slice;
                  }
            }
#ifdef OCR
      //
      // assume that highest slice contains header text
      //
      OcrImage img = OcrImage(_image.bits(), _slices[maxIdx], (_image.width() + 31) / 32);
      QString s = _omr->ocr()->readLine(img).trimmed();
      if (!s.isEmpty())
            score->addText("title", s);

      QString subTitle;
      for (int i = maxIdx + 1; i < slice; ++i) {
            OcrImage img = OcrImage(_image.bits(), _slices[i], (_image.width() + 31) / 32);
            QString s = _omr->ocr()->readLine(img).trimmed();
            if (!s.isEmpty()) {
                  if (!subTitle.isEmpty())
                        subTitle += "\n";
                  subTitle += s;
                  }
            }
      if (!subTitle.isEmpty())
            score->addText("subtitle", subTitle);
#endif
#if 0
      OcrImage img = OcrImage(_image.bits(), _slices[0], (_image.width() + 31) / 32);
      QString s = _omr->ocr()->readLine(img).trimmed();
      if (!s.isEmpty())
            addText(score, TEXT_TITLE, s);

      img = OcrImage(_image.bits(), _slices[1], (_image.width() + 31) / 32);
      s = _omr->ocr()->readLine(img).trimmed();
      if (!s.isEmpty())
            addText(score, TEXT_SUBTITLE, s);

      img = OcrImage(_image.bits(), _slices[2], (_image.width() + 31) / 32);
      s = _omr->ocr()->readLine(img).trimmed();
      if (!s.isEmpty())
            addText(score, TEXT_COMPOSER, s);
#endif
      }

//---------------------------------------------------------
//   removeBorder
//---------------------------------------------------------

void OmrPage::removeBorder()
      {
#if 0
      cropT = cropB = cropL = cropR = 0;
      for (int y = 0; y < height(); ++y) {
            for (int x = 0; x < width(); ++x) {
                  QRgb c = _image.pixel(x,y);
                  if(qGray(c) > 0){
                        cropT = y;
                        break;
                        }
                  }
            if (cropT)
                  break;
            }
      for (int y = height() - 1; y >= cropT; --y) {
            for (int x = 0; x < width(); ++x) {
                  QRgb c = _image.pixel(x,y);
                  if(qGray(c) > 0){
                        cropB = y;
                        break;
                        }
                  }
            if (cropB)
                  break;
            }
      int y1 = cropT;
      int y2 = cropB;
      for (int x = 0; x < width(); ++x) {
            for (int y = y1; y < y2; ++y) {
                  QRgb c = _image.pixel(x,y);
                  if(qGray(c) > 0){
                        cropL = x;
                        break;
                        }
                  }
            if (cropL)
                  break;
            }
      for (int x = width() - 1; x >= cropL; --x) {
            for (int y = y1; y < y2; ++y) {
                  QRgb c = _image.pixel(x,y);
                  if(qGray(c) > 0){
                        cropR = x;
                        break;
                        }
                  }
            if (cropR)
                  break;
            }

      _image = _image.copy(cropL, cropT, cropR - cropL + 1, cropB - cropT + 1);
#endif
      }

//---------------------------------------------------------
//   crop
//---------------------------------------------------------

void OmrPage::crop()
      {
      int wl = wordsPerLine();
      cropT = cropB = cropL = cropR = 0;
      for (int y = 0; y < height(); ++y) {
            const uint* p = scanLine(y);
            for (int k = 0; k < wl; ++k) {
                  if (*p++) {
                        cropT = y;
                        break;
                        }
                  }
            if (cropT)
                  break;
            }
      for (int y = height() - 1; y >= cropT; --y) {
            const uint* p = scanLine(y);
            for (int k = 0; k < wl; ++k) {
                  if (*p++) {
                        cropB = height() - y - 1;
                        break;
                        }
                  }
            if (cropB)
                  break;
            }
      int y1 = cropT;
      int y2 = height() - cropT - cropB;
      for (int x = 0; x < wl; ++x) {
            for (int y = y1; y < y2; ++y) {
                  if (*(scanLine(y) + x)) {
                        cropL = x;
                        break;
                        }
                  }
            if (cropL)
                  break;
            }
      for (int x = wl - 1; x >= cropL; --x) {
            for (int y = y1; y < y2; ++y) {
                  if (*(scanLine(y) + x)) {
                        cropR = wl - x - 1;
                        break;
                        }
                  }
            if (cropR)
                  break;
            }
      //      printf("*** crop: T%d B%d L%d R:%d\n", cropT, cropB, cropL, cropR);
      }

//---------------------------------------------------------
//   slice
//---------------------------------------------------------

void OmrPage::slice()
      {
      _slices.clear();
      int y1 = cropT;
      int y2 = height() - cropB;
      int x1 = cropL;
      int x2 = wordsPerLine() - cropR;
      int xbits = (x2 - x1) * 32;

      //      _slices.append(QRect(cropL*32, y1, xbits, y2-y1));
#if 1
      for (int y = y1; y < y2;) {
            //
            // skip contents
            //
            int ys = y;
            for (; y < y2; ++y) {
                  const uint* p = scanLine(y) + cropL;
                  bool bits = false;
                  for (int x = cropL; x < x2; ++x) {
                        if (*p) {
                              bits = true;
                              break;
                              }
                        ++p;
                        }
                  if (!bits)
                        break;
                  }
            if (y - ys)
                  _slices.append(QRect(cropL * 32, ys, xbits, y - ys));
            //
            // skip space
            //
            for (; y < y2; ++y) {
                  const uint* p = scanLine(y) + cropL;
                  bool bits = false;
                  for (int x = cropL; x < x2; ++x) {
                        if (*p) {
                              bits = true;
                              break;
                              }
                        ++p;
                        }
                  if (bits)
                        break;
                  }
            }
#endif
      }

//---------------------------------------------------------
//    deSkew
//---------------------------------------------------------

void OmrPage::deSkew()
      {
      int wl = wordsPerLine();
      int h = height();
      uint* db = new uint[wl * h];
      memset(db, 0, wl * h * sizeof(uint));

      for(const QRect& r : _slices) {
            double rot = skew(r);
            if (qAbs(rot) < 0.1) {
                  memcpy(db + wl * r.y(), scanLine(r.y()), wl * r.height() * sizeof(uint));
                  continue;
                  }

            QTransform t;
            t.rotate(rot);
            QTransform tt = QImage::trueMatrix(t, width(), r.height());

            double m11 = tt.m11();
            double m12 = tt.m12();
            double m21 = tt.m21();
            double m22 = tt.m22();
            double dx = tt.m31();
            double dy = tt.m32();

            double m21y = r.y() * m21;
            double m22y = r.y() * m22;
            int y2 = r.y() + r.height();

            for (int y = r.y(); y < y2; ++y) {
                  const uint* s = scanLine(y);

                  m21y += m21;
                  m22y += m22;
                  for (int x = 0; x < wl; ++x) {
                        uint c = *s++;
                        if (c == 0)
                              continue;
                        uint mask = 1;
                        for (int xx = 0; xx < 32; ++xx) {
                              if (c & mask) {
                                    int xs = x * 32 + xx;
                                    int xd = lrint(m11 * xs + m21y + dx);
                                    int yd = lrint(m22y + m12 * xs + dy);

                                    int wxd = xd / 32;
                                    if ((xd >= 0) && (wxd < wl) && (yd >= 0) && (yd < h)) {
                                          uint* d = db + wl * yd + wxd;
                                          if( d < db + wl * h) //check that we are in the bounds.
                                                *d |= (0x1 << (xd % 32));
                                          }
                                    }
                              mask <<= 1;
                              }
                        }
                  }
            }
      memcpy(_image.bits(), db, wl * h * sizeof(uint));
      delete[] db;
      }

struct ScanLine {
      int run;
      int x1, x2;
      ScanLine() { run = 0; x1 = 100000; x2 = 0; }
      };

struct H {
      int y;
      int bits;
      H(int a, int b) : y(a), bits(b) {}
      };

//---------------------------------------------------------
//   xproject
//---------------------------------------------------------

int OmrPage::xproject(const uint* p, int wl)
      {
      int run = 0;
      int w = wl - cropL - cropR;
      int x1 = cropL + w / 4;         // only look at part of page
      int x2 = x1 + w / 2;
      for (int x = cropL; x < x2; ++x) {
            uint v = *p++;
            run += Omr::bitsSetTable[v & 0xff]
               + Omr::bitsSetTable[(v >> 8) & 0xff]
               + Omr::bitsSetTable[(v >> 16) & 0xff]
               + Omr::bitsSetTable[v >> 24];
            }
      return run;
      }

//---------------------------------------------------------
//   xproject2
//---------------------------------------------------------

double OmrPage::xproject2(int y1)
      {
      int wl = wordsPerLine();
      const uint* db = bits();
      double val = 0.0;

      int w = wl - cropL - cropR;
      int x1 = (cropL + w / 4) * 32;       // only look at part of page
      int x2 = x1 + (w / 2 * 32);

      int ddx = x2 - x1;
      for (int dy = -12; dy < 12; ++dy) {
            int onRun = 0;
            int offRun = 0;
            int on = 0;
            int off = 0;
            bool onFlag = false;
            int incy = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
            int ddy = dy < 0 ? -dy : dy;
            int y = y1;
            if (y < 1)
                  y = 0;
            int err = ddx / 2;
            for (int x = x1; x < x2;) {
                  const uint* d = db + wl * y + (x / 32);
                  if (d < db + wl)
                        break;
                  if (d >= db + (wl - 1) * height()) //check that we are in the bounds.
                        break;
                  bool bit = ((*d) & (0x1 << (x % 32)));
                  bit = bit || ((*(d + wl)) & (0x1 << (x % 32)));
                  bit = bit || ((*(d - wl)) & (0x1 << (x % 32)));
                  if (bit != onFlag) {
                        if (!onFlag) {
                              //
                              // end of offrun:
                              //
                              if (offRun > 20) {
                                    off += offRun * offRun;
                                    on += onRun * onRun;
                                    onRun = 0;
                                    offRun = 0;
                                    }
                              else {
                                    onRun += offRun;
                                    offRun = 0;
                                    }
                              }
                        onFlag = bit;
                        }
                  (bit ? onRun : offRun)++;
                  if (offRun > 100) {
                        offRun = 0;
                        off = 1;
                        on = 0;
                        onRun = 0;
                        break;
                        }
                  err -= ddy;
                  if (err < 0) {
                        err += ddx;
                        y += incy;
                        if (y < 1)
                              y = 1;
                        else if (y >= height())
                              y = height() - 1;
                        }
                  ++x;
                  }
            if (offRun > 20)
                  off += offRun * offRun;
            else
                  onRun += offRun;
            on += onRun * onRun;
            if (off == 0)
                  off = 1;
            double nval = double(on) / double(off);
            if (nval > val)
                  val = nval;
            }
      return val;
      }

static bool sortLvStaves(const Lv& a, const Lv& b)
      {
      return a.line < b.line;
      }

//---------------------------------------------------------
//   getStaffLines
//---------------------------------------------------------

void OmrPage::getStaffLines()
      {
      staves.clear();
      int h = height();
      int wl = wordsPerLine();
      // printf("getStaffLines %d %d  crop %d %d\n", h, wl, cropT, cropB);
      if (h < 1)
            return;

      int y1 = cropT;
      int y2 = h - cropB;
      if (y2 >= h)
            --y2;

#if (!defined (_MSCVER) && !defined (_MSC_VER))
      double projection[h];
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<double> projection(h);
#endif
      for (int y = 0; y < y1; ++y)
            projection[y] = 0;
      for (int y = y2; y < h; ++y)
            projection[y] = 0;
      for (int y = y1; y < y2; ++y)
            projection[y] = xproject2(y);
      int autoTableSize = (wl * 32) / 20;       // 1/20 page width
      if (autoTableSize > y2 - y1)
            autoTableSize = y2 - y1;
#if (!defined (_MSCVER) && !defined (_MSC_VER))
      double autoTable[autoTableSize];
      memset(autoTable, 0, sizeof(autoTable));
      for (int i = 0; i < autoTableSize; ++i) {
            autoTable[i] = covariance(projection + y1, projection + i + y1, y2 - y1 - i);
            }
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<double> autoTable(autoTableSize);      // Default-initialized, doesn't need to be cleared
      for(int i = 0; i < autoTableSize; ++i)
         {
         autoTable[i] = covariance(&(projection[y1]), &(projection[i + y1]), y2 - y1 - i);
         }
#endif

      //
      // search for first maximum in covariance starting at 10 to skip
      // line width. Staff line distance (spatium) must be at least 10 dots
      //
      double maxCorrelation = 0;
      _spatium = 0;
      for (int i = 5; i < autoTableSize; ++i) {
            if (autoTable[i] > maxCorrelation) {
                  maxCorrelation = autoTable[i];
                  _spatium = i;
                  }
            }
      if (_spatium == 0) {
            printf("*** no staff lines found\n");
            return;
            }

      //---------------------------------------------------
      //    look for staves
      //---------------------------------------------------

      QList<Lv> lv;
      int ly = 0;
      int lval = -1000.0;
      for (int y = y1; y < (y2 - _spatium * 4); ++y) {
            double val = 0.0;
            for (int i = 0; i < 5; ++i)
                  val += projection[y + i * int(_spatium)];
            if (val < lval) {
                  lv.append(Lv(ly, lval));
                  }
            lval = val;
            ly = y;
            }
      qSort(lv);

      QList<Lv> staveTop;
      int staffHeight = _spatium * 6;
      for (Lv a : lv) {
            if (a.val < 500)   // MAGIC to avoid false positives
                  continue;
            int line = a.line;
            bool ok = true;
            for (Lv b : staveTop) {
                  if ((line > (b.line - staffHeight)) && (line < (b.line + staffHeight))) {
                        ok = false;
                        break;
                        }
                  }
            if (ok)
                  staveTop.append(a);
            }
      qSort(staveTop.begin(), staveTop.end(), sortLvStaves);
      for (Lv a : staveTop) {
            staves.append(OmrStaff(cropL * 32, a.line, (wordsPerLine() - cropL - cropR) * 32, _spatium * 4));
            }
      }

//---------------------------------------------------------
//   getRatio
//---------------------------------------------------------

void OmrPage::getRatio()
      {
      double num_black;
      double num_white;
      _ratio = 0.0;

      num_black = 1;
      num_white = 1;
      for (int x = 0; x < width(); ++x) {
            for (int y = 0; y < height(); ++y) {
                  if(isBlack(x,y)) num_black++;
                  else num_white++;
                  }
            }
      _ratio = num_black / (num_black + num_white);
      }

//---------------------------------------------------------
//   searchNotes
//---------------------------------------------------------

void OmrSystem::searchNotes(QList<OmrNote*>* noteList, int x1, int x2, int y, int line)
      {
      //a simple and cheap note detector (heuristic approach)
      double _spatium = _page->spatium();
      y += line * _spatium * .5;

      QList<Peak> notePeaks;
      Pattern* pattern = Omr::quartheadPattern;
      int hh = pattern->h();
      int hw = pattern->w();
      double val;
      int step_size = 2;
      int note_thresh = 50;

      for (int x = x1; x < (x2 - hw); x += step_size) {
            val = pattern->match(&_page->image(), x, y - hh / 2, _page->ratio());
            if (val > note_thresh) {
                  notePeaks.append(Peak(x, val, 0));
                  }
            }

      int n = notePeaks.size();
      for (int i = 0; i < n; ++i) {
            OmrNote* note = new OmrNote;
            int hh1 = pattern->h();
            int hw1 = pattern->w();
            note->setRect(notePeaks[i].x, y - hh1 / 2, hw1, hh1);
            note->line = line;
            note->sym = pattern->id();
            note->prob = notePeaks[i].val;
            noteList->append(note);
            }
      return;
      }

//---------------------------------------------------------
//   staffDistance
//---------------------------------------------------------

double OmrPage::staffDistance() const
      {
      if (staves.size() < 2)
            return 5;
      return ((staves[1].y() - staves[0].y()) / _spatium) - 4.0;
      }

//---------------------------------------------------------
//   systemDistance
//---------------------------------------------------------

double OmrPage::systemDistance() const
      {
      if (staves.size() < 3)
            return 6.0;
      return ((staves[2].y() - staves[1].y()) / _spatium) - 4.0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void OmrPage::write(XmlWriter& xml) const
      {
      xml.stag("OmrPage");
      xml.tag("cropL", cropL);
      xml.tag("cropR", cropR);
      xml.tag("cropT", cropT);
      xml.tag("cropB", cropB);
      for(const QRect& r : staves)
            xml.tag("staff", QRectF(r));
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void OmrPage::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "cropL")
                  cropL = e.readInt();
            else if (tag == "cropR")
                  cropR = e.readInt();
            else if (tag == "cropT")
                  cropT = e.readInt();
            else if (tag == "cropB")
                  cropB = e.readInt();
            else if (tag == "staff") {
                  OmrStaff r(e.readRect().toRect());
                  staves.append(r);
                  }
            else
                  e.unknown();
            }
      }
}
