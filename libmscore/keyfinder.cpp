//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2007-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================


//=============================================================================
//  Keyfinder.cpp uses code from the "Melisma Music Analyzer"
//  project:
//
//  Copyright (C) 2000 Daniel Sleator and David Temperley
//  See http://www.link.cs.cmu.edu/music-analysis
//  for information about commercial use of this system
//=============================================================================

/* _Important comment about how pitches and keys are represented_

  Notes may be inputted in either Note format (with just a pitch) or
  TPCNote format (with a TPC). The user may also specify
  "npc_or_tpc_profile": 0 for npc, 1 for tpc. (A TPC profile is like
  an NPC profile except that a user-settable default value is used for
  TPC's outside the range of b2 to #4.) We assume that Note format
  will be used with an npc profile; if not, a fatal error is
  reported. TPCNote format could be used with either; if you use
  TPCNote input with an npc profile, the notes are simply mapped on to
  one cycle of the LOF.  (Scoring mode 0 or 3 requires an npc profile;
  otherwise a fatal error is reported.)

  Pitches (e.g. note[].tpc values) are always represented in
  line-of-fifths order, with C = 14. (This is done when the pitches
  are first read in). (If there's Note input, or TPCNote input and
  npc_or_tpc_profile = 0, all pitches are shifted into one cycle of
  the line of fifths, from 9 to 20 inclusive; again, this is done when
  the input is read.)

  Keys are also represented in line-of-fifths order. Major keys are 0
  to 27, C=14; minor keys are 28-55, C minor = 42. If
  npc_or_tpc_profile = 0, the search is nominally done on all keys,
  but only keys from 9 to 20 (inclusive) and 37 to 48 (inclusive) are
  looked at; others are given large negative values.

  Although key-profiles are read in in pitch-height order, they are
  then adjusted (in generate_tpc/npc_profiles) to line-of-fifths
  order, with the tonic as 5. */

#include <string.h>
#include <math.h>
#include "keyfinder.h"
#include "sig.h"
#include "pitchspelling.h"
#include "synthesizer/event.h"

namespace Ms {

#if 0
//---------------------------------------------------------
//   SBeat
//---------------------------------------------------------

struct SBeat {
      int time;
      };

//---------------------------------------------------------
//   MidiSegment
//---------------------------------------------------------

struct MidiSegment {
      int start;
      int end;
      QList<Event> snote;
      int numnotes;           // number of notes in the segment
      qreal average_dur;     // average input vector value (needed for K-S algorithm)
      };

#define CHANGE_PENALTY        12

static float change_penalty   = CHANGE_PENALTY;
static int npc_or_tpc_profile = 1;
static int scoring_mode       = 1;
static int verbosity          = 1;

static qreal major_profile[12] = {5.0, 2.0, 3.5, 2.0, 4.5, 4.0, 2.0, 4.5, 2.0, 3.5, 1.5, 4.0};
static qreal minor_profile[12] = {5.0, 2.0, 3.5, 4.5, 2.0, 4.0, 2.0, 4.5, 3.5, 2.0, 1.5, 4.0};
static qreal default_profile_value=1.5;

/*

CBMS profiles:

major_profile = 5.0 2.0 3.5 2.0 4.5 4.0 2.0 4.5 2.0 3.5 1.5 4.0
minor_profile = 5.0 2.0 3.5 4.5 2.0 4.0 2.0 4.5 3.5 2.0 1.5 4.0

Bayesian profiles (based on frequencies in Kostka-Payne corpus):

major_profile =  0.748  0.060  0.488  0.082  0.670  0.460  0.096  0.715  0.104  0.366  0.057  0.400
minor_profile =  0.712  0.084  0.474  0.618  0.049  0.460  0.105  0.747  0.404  0.067  0.133  0.330

Krumhansl's profiles:

major_profile = 6.35 2.23 3.48 2.33 4.38 4.09 2.52 5.19 2.39 3.66 2.29 2.88
minor_profile = 6.33 2.68 3.52 5.38 2.60 3.53 2.54 4.75 3.98 2.69 3.34 3.17

Krumhansl's minor, normalized: 5.94 2.51 3.30 5.05 2.44 3.31 2.38 4.46 3.73 2.52 3.13 2.97

*/

static int firstbeat;
static QList<Event> note;
static QList<MidiSegment> segment;  // An array storing the notes in each segment
static int segtotal;                // total number of segments - 1

static qreal seglength;

static QList<int> seg_prof[28];
static QList<qreal> keyscore()[56];
static QList<SBeat> sbeat;
static QList<qreal> analysis[56][56];

static QList<int> best[56];
static QList<int> final;

// static int numnotes, numchords, num_sbeats;
static int num_sbeats;

static QList<int> pc_tally;
static QList<qreal> finalscore();

static qreal key_profile[56][28];
static int final_timepoint;

//---------------------------------------------------------
//   print_keyname
//---------------------------------------------------------

static void print_keyname(int f)
      {
      static const char letter[] = "CGDAEBF";

      int mf = f % 27;
      mf -= 14;

      qDebug("(%d,%d)%c", f, mf, letter[f % 7]);
      if (f < 6 || (f >= 28 && f < 34))
            qDebug("-");
      if ((f >= 6 && f < 13) || (f >= 34 && f < 41))
            qDebug("b");
      if ((f >= 20 && f < 27) || (f >=48 && f < 55))
            qDebug("#");
      if (f == 27 || f == 55)
            qDebug("x");
      if (f >= 28)
            qDebug("m");
      if (f < 28)
            qDebug(" ");
      qDebug(" ");
      }

//---------------------------------------------------------
//   create_segments
//    Each segment starts at a sbeat and ends at the
//    following sbeat
//---------------------------------------------------------

static void create_segments()
      {
      segment.append(MidiSegment());

      segment[0].start = firstbeat; // Always start a segment at the very beginning of the piece (the first beat)
      for (int b = 0; b < num_sbeats; b++) {
            if (b == 0 && (sbeat[0].time - firstbeat) < ((sbeat[1].time-firstbeat) - (sbeat[0].time-firstbeat))/2)
                  continue;
            /* If it's the first beat of the piece, and the upbeat is
               less than half of the first beat interval, don't start a segment
            */
            else {
                  MidiSegment seg;
                  seg.start          = sbeat[b].time;
                  segment.back().end = sbeat[b].time;
                  segment.append(seg);
                  }
            }
      int s = segment.size() - 1;

    /* If final segment starts at or after final timepoint of piece, ignore it,
       decrementing number of segments by 1; if not, set that segment's ending
       to final timepoint of piece
       */
      if (segment[s].start >= final_timepoint) {
            s--;
            }
      else {
            segment[s].end = final_timepoint;
            /* qDebug("Final segment ends at %d", segment[s].end); */
            }
      segtotal = s; // index of final segment
      }

//---------------------------------------------------------
//   fill_segments
//---------------------------------------------------------

static void fill_segments()
      {
      for (int s = 0; s < segment.size(); ++s) {
            foreach (const Event& n, note) {
                  int ontime  = n.ontime();
                  int offtime = n.offtime();
                  int start   = segment[s].start;
                  int end     = segment[s].end;

                  if (ontime >= start && ontime < end && offtime <= end) {
                        // note begins and ends in segment
                        Event sn(ME_NOTE);
                        sn.setDataA(n.dataA());
                        sn.setTpc(n.tpc());
                        sn.setDuration(n.duration());
                        segment[s].snote.append(sn);
                        }
                  if (ontime >= start && ontime < end && offtime > end) {
                        // note begins, doesn't end in segment
                        Event sn(ME_NOTE);
                        sn.setDataA(n.dataA());
                        sn.setTpc(n.tpc());
                        sn.setDuration(end - ontime);
                        segment[s].snote.append(sn);
                        }
                  if (ontime < start && offtime > start && offtime <= end) {
                        // note ends, doesn't begin in segment
                        Event sn(ME_NOTE);
                        sn.setDataA(n.dataA());
                        sn.setTpc(n.tpc());
                        sn.setDuration(offtime - start);
                        segment[s].snote.append(sn);
                        }
                  if (ontime < start && offtime > end) {
                        // note doesn't begin or end in segment
                        Event sn(ME_NOTE);
                        sn.setDataA(n.dataA());
                        sn.setTpc(n.tpc());
                        sn.setDuration(end - start);
                        segment[s].snote.append(sn);
                        }
                  }
            segment[s].numnotes = segment[s].snote.size();
            // qDebug("fillSegments %d: %d-%d  %d", s, segment[s].start,
            //   segment[s].end, segment[s].numnotes);
            }
      }

//---------------------------------------------------------
//   count_segment_notes
//    In each segment, tally up the notes of each TPC
//---------------------------------------------------------

static void count_segment_notes()
      {
      for (int s = 0; s <= segtotal; ++s) {
            pc_tally[s] = 0;

            for (int y = 0; y < 28; ++y)  // cycle through the pc's, make sure all the seg_prof values are zero
                  seg_prof[y].append(0);

            qreal total_dur = 0;

            for (int n = 0; n < segment[s].numnotes; ++n) {
                  if (scoring_mode == 0)
                        total_dur += segment[s].snote[n].duration();
                  for (int y=0; y<28; ++y) {
                        if (segment[s].snote[n].tpc() == y) {
                              if(seg_prof[y][s]==0)
                                    pc_tally[s]++;
                              /* This keeps track of how many different pc's the segment contains. This counts TPCs, not NPCs! */
                              /* If scoring_mode is > 1, set array value to 1. If 0, add the note's duration to the
                                 array value (as in the K-S algorithm) */
                              if (scoring_mode > 0)
                                    seg_prof[y][s] = 1;
                              else {
                                    seg_prof[y][s] += segment[s].snote[n].duration();
                                    }
                              }
                        }
                  }

            if(scoring_mode == 0) {
                  if(pc_tally[s]==0)
                        segment[s].average_dur = 0.0;
                  segment[s].average_dur = total_dur / 12.0;
                  /* qDebug("Segment %d total dur = %6.3f, average dur = %6.3f", s, total_dur, segment[s].average_dur); */
                  }

            if (verbosity>=2) {
                  qDebug("Segment %d: ", s);
                  for (int y=0; y<28; ++y) {
                        if(npc_or_tpc_profile == 0 && (y<9 || y>20))
                              continue;
                        qDebug("%d ", seg_prof[y][s]);
                        }
                  }
            /* qDebug("pc_tally = %d", pc_tally[s]); */
            }
      }

//---------------------------------------------------------
//   prepare_profiles
//    We're only here if scoring_mode is 0 (the K-S algorithm).
//    Sum all the profile values, take the mean,
//    and subtract that from each value
//---------------------------------------------------------

static void prepare_profiles()
      {
      qreal total = 0.0;
      for (int i = 0; i < 12; i++) {
            total += major_profile[i];
            }
      qreal average = total / 12.0;
      for (int i = 0; i < 12; i++)
            major_profile[i]=major_profile[i] - average;

      total = 0;
      for (int i = 0; i < 12; i++) {
            total += minor_profile[i];
            }
      average = total / 12.0;
      for (int i = 0; i < 12; i++)
            minor_profile[i] = minor_profile[i] - average;

      if (verbosity > 2) {
            qDebug("Adjusted major profile: ");
            for(int i = 0; i < 12; i++)
                  qDebug("%6.3f ", major_profile[i]);
            qDebug("Adjusted minor profile: ");
            for (int i = 0; i < 12; i++)
                  qDebug("%6.3f ", minor_profile[i]);
            }
      }

/* Here we generate the key profiles. (This is assuming tpc input.) Key_profile[key] numbers correspond to
   the line of fifths, with C = 14.  Major keys are 0-27, minor keys are 28-55. PCs are also numbered
   according to the line of fifths. The major_step_profile has the tonic in step 5. For a given key, the
   profile value for a given tpc is equal to the line of fifths difference between the tpc and the key, plus
   5. */

//---------------------------------------------------------
//   generate_tpc_profiles
//---------------------------------------------------------

static void generate_tpc_profiles()
      {
      int key, shift, tpc, i;
      float majp[12];
      float minp[12];

      /* First we rearrange the key profile values (inputted in pitch height order) into lof order, C = 5 */
      for(i=0; i<12; i++) {
            majp[((((i * 7) % 12) + 5) % 12)] = major_profile[i];
            minp[((((i * 7) % 12) + 5) % 12)] = minor_profile[i];
            }

      for (key=0, shift=0; key<28; ++key, ++shift) {
            for (tpc=0; tpc<28; ++tpc) {
                  if (tpc-shift >= -5 && tpc-shift <= 6) {
                        key_profile[key][tpc] = majp[5 + (tpc-shift)];    /* For example: for key 14 (C major) and tpc 17 (A),
                                                                             use profile step 5 + (17-14) = 8 */
                        }
                  if (tpc-shift < -5 || tpc-shift > 6) {
                        key_profile[key][tpc] = default_profile_value;
                        }
                  }
            }
      for (key=28, shift=0; key<56; ++key, ++shift) {
            for (tpc=0; tpc<28; ++tpc) {
                  if (tpc-shift >= -5 && tpc-shift <= 6 ) {
                        key_profile[key][tpc] = minp[5 + (tpc-shift)];
                        }
                  if (tpc-shift < -5 || tpc-shift > 6) {
                        key_profile[key][tpc] = default_profile_value;
                        }
                  }
            }
/*
      This routine just prints out the key profiles
      for(key=0; key<56; ++key) {
            for(tpc=0; tpc<28; ++tpc) {
                  qDebug("%1.2f ", key_profile[key][tpc]);
                  }
            }
*/
      }

//-----------------------------------------------------------------------------
//   generate_npc_profiles
//    This is an alternative function for generating profiles given NPC
//    profile. It's similar to the TPC version, except that only keys on
//    a certain range of the line are assigned non-zero values. (This is
//    really an unnecessary step, as steps outside the range will be
//    disqualified in "match_profiles" in any case.) Also, for those keys
//    considered, only key profile slots within the 9-to-20 range are
//    assigned non-zero values. (This is necessary, since the input profile
//    is within this range as well.) We begin by assigning default values
//    of ZERO to everything.
//-----------------------------------------------------------------------------

static void generate_npc_profiles()
      {
      int key, shift, tpc, tpc_to_use, i;
      float majp[12];
      float minp[12];

      /* First we rearrange the key profile values (inputted in pitch height order) into lof order, C = 5 */
      for(i=0; i<12; i++) {
            majp[((((i * 7) % 12) + 5) % 12)] = major_profile[i];
            minp[((((i * 7) % 12) + 5) % 12)] = minor_profile[i];
            }
      for (key=0; key<56; ++key) {
            for(tpc=0; tpc<28; ++tpc) {
                  key_profile[key][tpc]=0;
                  }
            }

      for (key=9, shift=9; key<21; ++key, ++shift) {
            for (tpc=0; tpc<28; ++tpc) {
                  /* tpc_to_use is the profile slot to use for a given value of the key-profile. In this way we keep
                     all profile slots within the 9-to-20 range
                  */
                  if(tpc<9)
                        tpc_to_use=tpc+12;
                  else if(tpc>20)
                        tpc_to_use=tpc-12;
                  else
                        tpc_to_use = tpc;
                  if (tpc-shift >= -5 && tpc-shift <= 6) {
                        /* For example: for key 14 (C major) and tpc 17 (A),
                        read from profile step 5 + (17-14) = 8. For degree
                        6 of B major (key 19), degree 6 (22) is outside the
                        9-to-20 range, so tpc_to_use is 22-12=10; still
                        read from profile step 5 + (22-19) = 8. */

                        key_profile[key][tpc_to_use] = majp[5 + (tpc-shift)];
                        }
                  }
            }
      for (key = 37, shift = 9; key < 49; ++key, ++shift) {
            for (tpc = 0; tpc < 28; ++tpc) {
                  if (tpc<9)
                        tpc_to_use = tpc + 12;
                  else if (tpc > 20)
                        tpc_to_use = tpc - 12;
                  else
                        tpc_to_use = tpc;
                  if (tpc - shift >= -5 && tpc - shift <= 6 ) {
                        key_profile[key][tpc_to_use] = minp[5 + (tpc-shift)];
                        }
                  }
            }

/*
      for(key = 0; key < 56; ++key) {
            for(tpc=0; tpc<28; ++tpc) {
                  qDebug("%1.2f ", key_profile[key][tpc]);
                  }
            }
*/
      }

//---------------------------------------------------------
//   match_profiles
//    Here we generate the "key scores" - the local score for each key
//
//    Notice that we generate profiles for all 56 keys and all 28
//    tpc's, even in the case where npc profiles are being used. In
//    this case, though, all keys outside the allowable NPC range are
//    given large negative values.  And within both the key profiles
//    and the segment profiles, only TPC's within the range 9 to 20
//    have been given nonzero values.
//---------------------------------------------------------

static void match_profiles()
      {
      int key, tpc, s, best_key, i;
      qreal major_sumsq, minor_sumsq, input_sumsq;
      qreal kprob[56];

      for (key = 0; key < 56; ++key) {
            for (s = 0; s <= segtotal; ++s)
                  keyscore()[key].append(0.0);
            }

      if (scoring_mode==0) {
            major_sumsq = 0.0;
            minor_sumsq = 0.0;
            for(i=0; i<12; i++)
                  major_sumsq += major_profile[i]*major_profile[i];
            for(i=0; i<12; i++)
                  minor_sumsq += minor_profile[i]*minor_profile[i];
            if (verbosity==3)
                  qDebug("major_sumsq = %6.3f, minor_sumsq = %6.3f", major_sumsq, minor_sumsq);
            }

      qreal total_prob[segtotal + 1];

      for (s = 0; s <= segtotal; ++s) {
            if (scoring_mode==0) {
                  input_sumsq = 0.0;
                  for (i = 9; i <= 20; i++) {
                        input_sumsq += pow((seg_prof[i][s]-segment[s].average_dur), 2.0);
                        /* qDebug("%d X %6.3f squared is %6.3f", seg_prof[i][s], segment[s].average_dur, pow((seg_prof[i][s]-segment[s].average_dur), 2.0)); */
                        }
                  if (verbosity==3)
                        qDebug("For segment %d: average_dur = %6.3f; input_sumsq = %6.3f", s, segment[s].average_dur, input_sumsq);
                  }
            best_key=0;

            for (key=0; key<56; ++key) {
                  kprob[key] = 0.0;
                  keyscore()[key][s] = -DBL_MAX;
                  if (npc_or_tpc_profile==0 && (key<9 || (key>20 && key<37) || key>48))
                        continue;
                  kprob[key] = 1.0;
                  keyscore()[key][s] = 0.0;
                  for (tpc=0; tpc<28; ++tpc) {

          /*
             If scoring mode is 0, this is the K-S algorithm (this works for npc mode only). Segment
             profile values represent total duration of each pc (in all other cases, they're just 1
             for present pc's and 0 for absent ones). Key-profiles have been normalized linearly
             around the average key-profile value. We normalize the input values similarly by taking
             (seg_prof[tpc][s]-segment[s].average_dur). Then we multiply each normalized KP value by
             the normalized input value, and sum these products; this gives us the numerator of the
             correlation expression (as commented below). We've summed the squares of the normalized
             key-profile value (major_sumsq and minor_sumsq above) and the normalized input values
             (input_sumsq above), so this allows us to calculate the denominator also.

             If scoring_mode is 1, the key score is the sum of key-profile values for all pc's present
             (this is the algorithm used in CBMS)

             If scoring_mode is 2, calculate key scores as above, but divide each one by the number
             of pc's in the segment

             If scoring_mode is 3: for each key, add the log of the key-profile value for all present pc's;
             subtract values for all absent pc's. (This is the Bayesian approach; assume key-profiles
             represent pc distribution's in a corpus, i.e. the number of segments containing each scale
             degree)
          */

                        if(scoring_mode == 0) {
                              if(tpc<9 || tpc>20)
                                    continue;
                              /* calculate numerator */
                              keyscore()[key][s] += key_profile[key][tpc] * (seg_prof[tpc][s]-segment[s].average_dur);
                              /* qDebug("x-X=%6.3f, y-Y=%6.3f, product=%6.3f, new total=%6.3f", key_profile[key][tpc], seg_prof[tpc][s]-segment[s].average_dur, key_profile[key][tpc] * (seg_prof[tpc][s]-segment[s].average_dur), keyscore()[key][s]); */
                              }

                        if(scoring_mode==1 || scoring_mode==2)
                              keyscore()[key][s] += (key_profile[key][tpc] * seg_prof[tpc][s]);

                        if(scoring_mode == 3) {
                              /* if(tpc>11) continue; */
                              /* if(tpc<9 || tpc>20) continue; */

                              if(seg_prof[tpc][s]==0) {
                                    keyscore()[key][s] += log(1.000 - key_profile[key][tpc]);
                              /* qDebug("kp value = %6.3f: log(1-p) = %6.3f: score = %6.3f", key_profile[key][tpc], log(1.000 - key_profile[key][tpc]), keyscore()[key][s]); */
                                    if(tpc>=9 && tpc<=20)
                                          kprob[key] *= (1.000 - key_profile[key][tpc]);
                                    }
                              else {
                                    keyscore()[key][s] += log(key_profile[key][tpc]);
                                    if(tpc>=9 && tpc<=20)
                                          kprob[key] *= key_profile[key][tpc];
                                    }

                              /* qDebug("kp value = %6.3f: log(p) = %6.3f: score = %6.3f", key_profile[key][tpc], log(key_profile[key][tpc]), keyscore()[key][s]); */
                              }
                        }

                  if(scoring_mode == 0) {
                        /* qDebug("sqrt(major_sumsq * input_sumsq) = %6.3f", sqrt(major_sumsq * input_sumsq)); */
                        /* calculate denominator */
                        if(key<28)
                              keyscore()[key][s] = keyscore()[key][s] / sqrt(major_sumsq * input_sumsq);
                        else
                              keyscore()[key][s] = keyscore()[key][s] / sqrt(minor_sumsq * input_sumsq);
                        }
                  if(scoring_mode == 2) {
                        if(pc_tally[s] == 0)
                              keyscore()[key][s] = 0;
                        else
                              keyscore()[key][s] = keyscore()[key][s] / pc_tally[s];
                        }

                  /* if(s==0) qDebug("local score for key %d on segment %d: %6.3f", key, s, keyscore()[key][s]); */
                  if (keyscore()[key][s] > keyscore()[best_key][s])
                        best_key = key;
                  }

            if(verbosity>=2) {
                  qDebug("The best local key for segment %d at time %d is ", s, segment[s].start);
                  print_keyname(best_key);
                  qDebug("with score %6.3f", keyscore()[best_key][s]);
                  }

            if(scoring_mode==3) {
                  total_prob[s]=0.0;
                  for(key=0; key<56; key++) {
                        total_prob[s] += kprob[key] / 24.0;
                        /* qDebug("  Prob of segment %d given key %d: %6.8f", s, key, kprob[key]);  */
                        }

                  /* Now total_prob[s] is the total probability of the segment occurring: its probability given
                     a key, summed over all keys. But suppose we want to know the probability of ANY major triad
                     occurring? Then we have to multiply this by 12. (But not for something like a diminished
                     seventh which is symmetrical! */

                  if (verbosity>=3) {
                        qDebug("Best key for segment %d = %d, score = %6.8f", s, best_key, kprob[best_key]);
                        qDebug("Total (local) probability of segment %d: %6.8f", s, total_prob[s]);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   choose_best_i
//---------------------------------------------------------

static void choose_best_i(int seg)
      {
      for (int j = 0; j < 56; ++j) {
            int k = 0;
            for (int i = 0; i < 56; ++i) {
                  if (analysis[i][j][seg] > analysis[k][j][seg])
                        k=i;
                  }
            /* For a given segment seg, and the key j at that segment,
               the best previous key is k
            */
            int size = best[j].size();
            for (int i = size; i < seg+1; ++i)
                  best[j].append(0);
            best[j][seg] = k;
            /* qDebug("For segment-%d-key %d, best segment-%d-key is %d, with score %d", seg, j, seg-1, k, analysis[k][j][seg]); */
            }
      }

//---------------------------------------------------------
//   make_first_table
//---------------------------------------------------------

static void make_first_table(int seg)
      {
      int i, j, s;
      qreal seg_factor, mod_factor, nomod_factor;

      if (scoring_mode==3) {
            mod_factor = log(change_penalty);
            nomod_factor = log(1.0 - change_penalty);
            seg_factor = 1.0;
            }
      else {
            mod_factor = -change_penalty;
            nomod_factor = 0.0;
            seg_factor = seglength;
            }

      for(s = 0; s <= segtotal; s++) {
            for(i = 0; i < 56; ++i) {
                  for(j = 0; j < 56; ++j)
                        analysis[i][j].append(-1000.0);
                  }
            }

      for(i = 0; i < 56; ++i) {
            for(j = 0; j < 56; ++j) {
                  if (j != i)
                        analysis[i][j][1] = ((keyscore()[i][0] + keyscore()[j][1]) * seg_factor) + mod_factor;
                  else
                        analysis[i][j][1] = ((keyscore()[i][0] + keyscore()[j][1]) * seg_factor) + nomod_factor;
                  }
            }
      choose_best_i(seg);
      }

//---------------------------------------------------------
//   make_tables
//---------------------------------------------------------

static void make_tables()
      {
      qreal seg_factor, mod_factor, nomod_factor;

      /* When scoring_mode = 3, the change_penalty represents the probability of changing key. So raising
         the penalty actually _increases_ the likelihood of modulations. */

      if (scoring_mode == 3) {
            mod_factor = log(change_penalty / 23.0);
            nomod_factor = log(1.0 - change_penalty);
            seg_factor = 1.0;
            }
      else {
            mod_factor = -change_penalty;
            nomod_factor = 0.0;
            seg_factor = seglength;
            }

      for (int seg = 2; seg <= segtotal; ++seg) {
            /* qDebug("mod_factor = %6.6f; ; nomod_factor = %6.6f", mod_factor, nomod_factor);  */
            for(int j = 0; j < 56; ++j) {
                  for(int i = 0; i < 56; ++i) {
                        int n = best[i][seg-1];
                        if (j != i)
                              analysis[i][j][seg] = analysis[n][i][seg-1] + (keyscore()[j][seg] * seg_factor) + mod_factor;
                        else
                              analysis[i][j][seg] = analysis[n][i][seg-1] + (keyscore()[j][seg] * seg_factor) + nomod_factor;
                        }
                  }
            choose_best_i(seg);
            }
      }

//---------------------------------------------------------
//   best_key_analysis
//---------------------------------------------------------

static void best_key_analysis()
      {
      int n, m, f, tie1=-1, tie2=-1;
      int s = segtotal;
      int k = 0;
      for(int j = 0; j < 56; ++j) {
            n = best[j][s];
            m = best[k][s];

            if (analysis[n][j][s] < analysis[m][k][s] + .001 && analysis[n][j][s] > analysis[m][k][s] - .001 && j!=k) {
                  tie1=j;
                  tie2=k;
                  }

            if (verbosity>1 && !(npc_or_tpc_profile == 0 && (j<9 || (j>20 && j<37) || j>48))) {
                  qDebug("Final score for ");
                  print_keyname(j);
                  /* qDebug("is %6.3f", analysis[n][j][s] * 1000 / (segment[segtotal].end - segment[0].start));   */
                  qDebug("is %6.3f", analysis[n][j][s]);
                  }
            if (analysis[n][j][s] > analysis[m][k][s] + .000001) {
                  /* The .000001 is to fix a strange bug: sometimes it thinks the conditional is satisfied in the case of ties */
                  k = j;   /* compute best key of final segment */
                  }
            }

      // To force a key choice at the final segment, insert key number here

      final[s] = k;
      if (verbosity > 1)
            if (k==tie1 || k==tie2)
                  qDebug("Tie at the end between %d and %d", tie1, tie2);

      // Here's where we take the best key choices and put them into final[s]

      for(s = segtotal; s >= 1; --s) {
            final[s-1] = best[k][s];
            k = final[s-1];
            }

      if (verbosity >= 2) {
            qDebug("Segment 0: key choice %d; total score %6.3f; segment score %6.3f", final[0], keyscore()[final[0]][0],
               keyscore()[final[0]][0]);
            qDebug("Segment 1: key choice %d; total score %6.3f; segment score %6.3f", final[1], analysis[final[0]][final[1]][1], analysis[final[0]][final[1]][1] - keyscore()[final[0]][0]);
            for(s = 2; s <= segtotal; s++) {
                  qDebug("Segment %d: key choice %d; total score %6.3f; segment score %6.3f", s, final[s], analysis[final[s-1]][final[s]][s], analysis[final[s-1]][final[s]][s] - analysis[final[s-2]][final[s-1]][s-1]);
                  }
            }

      if (verbosity > 1)
            qDebug("'Key-fit' scores for preferred analysis:");

      /* This routine calculates the key-fit scores for the final analysis. These are really per-second scores.
         Key-profile scores are not multiplied by seglength (as they would be in actually computing the analyses);
         change penalties are divided by seglength. */

      for (s = 0; s <= segtotal; ++s) {
            f = final[s];
            if (s > 0 && final[s] != final[s-1])
                  finalscore()[s] = (keyscore()[f][s]) - (change_penalty / seglength);
            else
                  finalscore()[s]=keyscore()[f][s];
            if (verbosity > 1) {
                  qDebug(" segment %d: %6.3f", s, finalscore()[s]);
                  }
            }
      }

//---------------------------------------------------------
//   findKey
//---------------------------------------------------------

int findKey(MidiTrack* mt, TimeSigMap* sigmap)
      {
      int tpc_found, npc_found;

      if ((scoring_mode == 0 || scoring_mode == 3) && npc_or_tpc_profile == 1) {
            qDebug("Error: scoring mode %d requires an npc profile", scoring_mode);
            exit(1);
            }

      final_timepoint=0;

      tpc_found = 0;
      npc_found = 0;

      int lastTick = 0;
      const EventList el = mt->events();

      foreach (Event e, el) {
            if (e.type() != ME_NOTE)
                  continue;
            if (e.offtime() > lastTick)
                  lastTick = e.offtime();
            // For note input, generate TPC labels within the 9-to-20 range
            e.setTpc((((((e.pitch() % 12) * 7) % 12) + 5) % 12) + 9);
            note.append(e);
            }
      spell(note, 0);
      npc_found = 1;

      // create one segment for every measure
      for (int i = 0;; ++i) {
            int tick = sigmap->bar2tick(i, 0, 0);
            SBeat b;
            b.time = tick;
            sbeat.append(b);
            if (tick > lastTick)
                  break;
            }

      firstbeat       = sbeat.first().time;
      final_timepoint = sbeat.last().time;
      num_sbeats      = sbeat.size();

      numnotes   = note.size();
      numchords  = 0;

      if (note.empty()) {
            qDebug("Error: No notes in input.");
            return 0;
            }

      seglength  = (sbeat[1].time - sbeat[0].time) / 1000.0; /* define segment length as the length of the first segment */
      if (verbosity > 1)
            qDebug("seglength = %3.3f", seglength);

      create_segments();
      for (int i = 0; i < segtotal+1; ++i) {
            final.append(0);
            pc_tally.append(0);
            finalscore().append(0.0);
            }

      fill_segments();
      count_segment_notes();
      if (scoring_mode==0)
            prepare_profiles();
      if (npc_or_tpc_profile == 1)
            generate_tpc_profiles();
      else
            generate_npc_profiles();
      match_profiles();
      if (segtotal > 0) {
            make_first_table(1);
            make_tables();
            best_key_analysis();
            }

      QList<int> keys;
      for (int i = 0; i < 27; ++i)
            keys.append(0);
      for (int i = 0; i <= segtotal; ++i) {
            keys[final[i] % 27]++;        // fold major/minor
//            qDebug("key %d: %d  %d", i, final[i], (final[i] % 27) - 14);
            }
      int xkey   = 0;
      int xcount = 0;
      for (int i = 0; i < 27; ++i) {
            if (keys[i] > xcount) {
                  xcount = keys[i];
                  xkey   = i;
                  }
            }

      xkey -= 14;
      if (xkey < -7 || xkey > 7) {
            qDebug("findKey(): illegal key %d found", xkey);
            xkey = 0;
            }

      spell(note, xkey);      // spell again with found key

      //
      // clear all arrays
      //
      note.clear();
      segment.clear();
      sbeat.clear();
      for (int i = 0; i < 28; ++i)
            seg_prof[i].clear();
      for (int i = 0; i < 56; ++i) {
            keyscore()[i].clear();
            best[i].clear();
            for (int k = 0; k < 56; ++k)
                  analysis[i][k].clear();
            }
      final.clear();
      pc_tally.clear();
      finalscore().clear();

      return xkey;
      }
#endif

}

