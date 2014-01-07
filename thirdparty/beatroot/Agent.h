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

#ifndef _AGENT_H_
#define _AGENT_H_

#include "Event.h"


class AgentList;
class Event;

class AgentParameters
      {
   public:
      static const double DEFAULT_POST_MARGIN_FACTOR;
      static const double DEFAULT_PRE_MARGIN_FACTOR;
      static const double DEFAULT_MAX_CHANGE;
      static const double DEFAULT_EXPIRY_TIME;

      AgentParameters()
            : postMarginFactor(DEFAULT_POST_MARGIN_FACTOR)
            , preMarginFactor(DEFAULT_PRE_MARGIN_FACTOR)
            , maxChange(DEFAULT_MAX_CHANGE)
            , expiryTime(DEFAULT_EXPIRY_TIME)
            {}

                  /** The maximum amount by which a beat can be later than the
                   *  predicted beat time, expressed as a fraction of the beat
                   *  period.
                   */
      double postMarginFactor;

                  /** The maximum amount by which a beat can be earlier than the
                   *  predicted beat time, expressed as a fraction of the beat
                   *  period.
                   */
      double preMarginFactor;

                  /** The maximum allowed deviation from the initial tempo,
                   * expressed as a fraction of the initial beat period.
                   */
      double maxChange;

                  /** The default value of expiryTime, which is the time (in
                   *  seconds) after which an Agent that has no Event matching its
                   *  beat predictions will be destroyed.
                   */
      double expiryTime;
      };


      /** Agent is the central class for beat tracking.
       *  Each Agent object has a tempo hypothesis, a history of tracked beats, and
       *  a score evaluating the continuity, regularity and salience of its beat track.
       */
class Agent
      {
   public:
                  /** The Agent's unique identity number. */
      int idNumber;

                  /** Sum of salience values of the Events which have been
                   *  interpreted as beats by this Agent, weighted by their nearness
                   *  to the predicted beat times.
                   */
      double phaseScore;

                  /** The number of beats found by this Agent, including
                   *  interpolated beats.
                   */
      int beatCount;

                  /** The current tempo hypothesis of the Agent, expressed as the
                   *  beat period in seconds.
                   */
      double beatInterval;

                  /** The initial tempo hypothesis of the Agent, expressed as the
                   *  beat period in seconds.
                   */
      double initialBeatInterval;

                  /** The time of the most recent beat accepted by this Agent. */
      double beatTime;

                  /** The maximum allowed deviation from the initial tempo,
                   *  expressed as a fraction of the initial beat period.
                   */
      double maxChange;

                  /** The list of Events (onsets) accepted by this Agent as beats,
                   *  plus interpolated beats. */
      EventList events;

                  /** Constructor: the work is performed by init()
                   *  @param ibi The beat period (inter-beat interval)
                   *  of the Agent's tempo hypothesis.
                   */
      Agent(const AgentParameters &params, double ibi);

      Agent *clone() const;

                  /** Accept a new Event as a beat time, and update the state of the Agent accordingly.
                   *  @param e The Event which is accepted as being on the beat.
                   *  @param err The difference between the predicted and actual beat times.
                   *  @param beats The number of beats since the last beat that matched an Event.
                   */
      void accept(const Event &e, double err, int beats);

                  /** The given Event is tested for a possible beat time.
                   *  The following situations can occur:
                   *  1) The Agent has no beats yet; the Event is accepted as the first beat.
                   *  2) The Event is beyond expiryTime seconds after the Agent's last
                   *     'confirming' beat; the Agent is terminated.
                   *  3) The Event is within the innerMargin of the beat prediction;
                   *     it is accepted as a beat.
                   *  4) The Event is within the postMargin's of the beat prediction;
                   *     it is accepted as a beat by this Agent,
                   *     and a new Agent is created which doesn't accept it as a beat.
                   *  5) The Event is ignored because it is outside the windows around
                   *     the Agent's predicted beat time.
                   * @param e The Event to be tested
                   * @param a The list of all agents, which is updated if a new agent is created.
                   * @return Indicate whether the given Event was accepted as a beat by this Agent.
                   */
      bool considerAsBeat(const Event &e, AgentList &a);

                  /** Interpolates missing beats in the Agent's beat track,
                   *  starting from the beginning of the piece.
                   */
      void fillBeats()
            {
            fillBeats(-1.0);
            }
                  /** Interpolates missing beats in the Agent's beat track.
                   *  @param start Ignore beats earlier than this start time
                   */
      void fillBeats(double start);

   private:
                  /** The identity number of the next created Agent */
      static int idCounter;

                  /** The default value of innerMargin, which is the maximum time
                   *  (in seconds) that a beat can deviate from the predicted beat
                   *  time without a fork occurring.
                   */
      static const double INNER_MARGIN;

                  /** The slope of the penalty function for onsets which do not
                   *  coincide precisely with predicted beat times.
                   */
      static const double CONF_FACTOR;

                  /** The reactiveness/inertia balance, i.e. degree of change in the
                   *  tempo, is controlled by the correctionFactor variable.  This
                   *  constant defines its default value, which currently is not
                   *  subsequently changed. The beat period is updated by the
                   *  reciprocal of the correctionFactor multiplied by the
                   *  difference between the predicted beat time and matching
                   *  onset.
                   */
      static const double DEFAULT_CORRECTION_FACTOR;

                  /** The size of the outer half-window before the predicted beat time. */
      double preMargin;

                  /** The size of the outer half-window after the predicted beat time. */
      double postMargin;

                  /** The maximum time (in seconds) that a beat can deviate from the
                   *  predicted beat time without a fork occurring (i.e. a 2nd Agent
                   *  being created).
                   */
      double innerMargin;

                  /** Controls the reactiveness/inertia balance, i.e. degree of
                   *  change in the tempo.  The beat period is updated by the
                   *  reciprocal of the correctionFactor multiplied by the
                   *  difference between the predicted beat time and matching
                   *  onset.
                   */
      double correctionFactor;

                  /** The time (in seconds) after which an Agent that has no Event
                   *  matching its beat predictions will be destroyed.
                   */
      double expiryTime;

      };

#endif
