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

#include "AgentList.h"
#include "Agent.h"

#include <algorithm>
#include <cmath>


bool AgentList::useAverageSalience = false;
const double AgentList::DEFAULT_BI = 0.02;
const double AgentList::DEFAULT_BT = 0.04;


void AgentList::add(Agent *newAgent, bool sort)
      {
      push_back(newAgent);
      if (sort)
            this->sort();
      }

void AgentList::sort()
      {
      struct {
            bool operator()(const Agent *a, const Agent *b)
                  {
                  if (a->beatInterval == b->beatInterval)
                        return a->idNumber < b->idNumber;         // ensure stable ordering
                  return a->beatInterval < b->beatInterval;
                  }
            } agentComparator;

      std::sort(list.begin(), list.end(), agentComparator);
      }

void AgentList::remove(const AgentList::iterator &itr)
      {
      list.erase(itr);
      }

void AgentList::removeDuplicates()
      {
      sort();
      for (iterator itr = begin(); itr != end(); ++itr) {
            if ((*itr)->phaseScore < 0.0)       // already flagged for deletion
                  continue;
            iterator itr2 = itr;
            for (++itr2; itr2 != end(); ++itr2) {
                  if ((*itr2)->beatInterval - (*itr)->beatInterval > DEFAULT_BI)
                        break;
                  if (std::fabs((*itr)->beatTime - (*itr2)->beatTime) > DEFAULT_BT)
                        continue;
                  if ((*itr)->phaseScore < (*itr2)->phaseScore) {
                        (*itr)->phaseScore = -1.0;	// flag for deletion
                        break;
                        }
                  else {
                        (*itr2)->phaseScore = -1.0;	// flag for deletion
                        }
                  }
            }

      auto last = std::remove_if(begin(), end(), [](const Agent* agent){
                  if (agent->phaseScore < 0.0) {
                        delete agent;
                        return true;
                        }
                  return false;
            });
      list.erase(last, list.end());
      }


void AgentList::beatTrack(const EventList &el,
                          const AgentParameters &params,
                          double stop)
      {
      EventList::const_iterator ei = el.begin();
      bool phaseGiven = !empty() && ((*begin())->beatTime >= 0); // if given for one, assume given for others
      while (ei != el.end()) {
            Event ev = *ei;
            ++ei;
            if ((stop > 0) && (ev.time > stop))
                  break;
            bool created = phaseGiven;
            double prevBeatInterval = -1.0;
                        // cc: Duplicate our list of agents, and scan through the
                        // copy.  This means we can safely add agents to our own
                        // list while scanning without disrupting our scan.  Each
                        // agent needs to be re-added to our own list explicitly
                        // (since it is modified by e.g. considerAsBeat)
            Container currentAgents = list;
            list.clear();
            for (Container::iterator ai = currentAgents.begin();
                        ai != currentAgents.end(); ++ai) {
                  Agent *currentAgent = *ai;
                  if (currentAgent->beatInterval != prevBeatInterval) {
                        if ((prevBeatInterval >= 0) && !created && (ev.time < 5.0)) {
                                          // Create new agent with different phase
                              Agent *newAgent = new Agent(params, prevBeatInterval);
                                          // This may add another agent to our list as well
                              newAgent->considerAsBeat(ev, *this);
                              add(newAgent);
                              }
                        prevBeatInterval = currentAgent->beatInterval;
                        created = phaseGiven;
                        }
                  if (currentAgent->considerAsBeat(ev, *this))
                        created = true;
                  add(currentAgent);
                  }           // loop for each agent
            removeDuplicates();
            }           // loop for each event
      }

Agent *AgentList::bestAgent()
      {
      double best = -1.0;
      Agent *bestAg = 0;
      for (iterator itr = begin(); itr != end(); ++itr) {
            if ((*itr)->events.empty())
                  continue;
            double conf = (*itr)->phaseScore
                        / (useAverageSalience ? (double)(*itr)->beatCount : 1.0);
            if (conf > best) {
                  bestAg = *itr;
                  best = conf;
                  }
            }
      return bestAg;
      }

