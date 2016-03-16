/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
  Vamp feature extraction plugin for the BeatRoot beat tracker.

  Centre for Digital Music, Queen Mary, University of London.
  This file copyright 2011 Simon Dixon, Chris Cannam and QMUL.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.  See the file
  COPYING included with this distribution for more information.
*/

#ifndef _INDUCTION_H_
#define _INDUCTION_H_

#include "Event.h"


class AgentParameters;
class AgentList;


      /** Performs tempo induction by finding clusters of similar
       *  inter-onset intervals (IOIs), ranking them according to the number
       *  of intervals and relationships between them, and returning a set
       *  of tempo hypotheses for initialising the beat tracking agents.
       */
class Induction
      {
   public:
                  /** The maximum difference in IOIs which are in the same cluster */
      static double clusterWidth;

                  /** The minimum IOI for inclusion in a cluster */
      static double minIOI;

                  /** The maximum IOI for inclusion in a cluster */
      static double maxIOI;

                  /** The minimum inter-beat interval (IBI), i.e. the maximum tempo
                   *  hypothesis that can be returned.
                   *  0.30 seconds == 200 BPM
                   *  0.25 seconds == 240 BPM
                   */
      static double minIBI;

                  /** The maximum inter-beat interval (IBI), i.e. the minimum tempo
                   *  hypothesis that can be returned.
                   *  1.00 seconds ==  60 BPM
                   *  0.75 seconds ==  80 BPM
                   *  0.60 seconds == 100 BPM
                   */
      static double maxIBI;	//  60BPM	// was 0.75 =>  80

                  /** The maximum number of tempo hypotheses to return */
      static int topN;

                  /** Performs tempo induction (see JNMR 2001 paper by Simon Dixon for details).
                   *  @param events The onsets (or other events) from which the tempo is induced
                   *  @return A list of beat tracking agents, where each is initialised with one
                   *          of the top tempo hypotheses but no beats
                   */
      static AgentList beatInduction(const AgentParameters &params,
                                     const EventList &events);

   private:
                  /** For variable cluster widths in newInduction().
                    * @param low The lowest IOI allowed in the cluster
                    * @return The highest IOI allowed in the cluster
                    */
      static int top(int low);
      };


#endif
