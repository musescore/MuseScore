//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __TIMEMAP_H__
#define __TIMEMAP_H__

#include "timeposition.h"

namespace Ms {

//---------------------------------------------------------
//   AbstractTimeMap
///   Template for maps from any time position to any
///   other type. Can have only one value assigned to any
///   particular time moment.
//---------------------------------------------------------

template<typename Time, typename Val>
class AbstractTimeMap : private std::map<Time, Val> {
      typedef std::map<Time, Val> M;

   public:
      using typename M::iterator;
      using typename M::const_iterator;
      using typename M::value_type;
      using M::begin;
      using M::cbegin;
      using M::end;
      using M::cend;
      using M::size;
      using M::empty;

      const Val& value(const Time& t, const Val& defaultVal) const
            {
            if (empty())
                  return defaultVal;
            auto i = M::upper_bound(t);
            if (i == M::begin())
                  return defaultVal;
            return (--i)->second;
            }

      Val& editableValue(const Time& t)
            {
            Q_ASSERT(!empty());
            auto i = M::upper_bound(t);
            Q_ASSERT(i != M::begin());
            return (--i)->second;
            }

      const Val& nextValue(const Time& t, const Val& defaultVal, bool nextOrEqual = false) const
            {
            auto i = nextOrEqual ? M::lower_bound(t) : M::upper_bound(t);
            if (i == M::end())
                  return defaultVal;
            return i->second;
            }

      void insert(const Time& time, Val value)
            {
            auto i = M::find(time);
            if (i == M::end())
                  M::emplace(time, std::move(value));
            else
                  i->second = std::move(value);
            }

      template<class InputIt>
      void insert(InputIt first, InputIt last) { M::insert(first, last); }

      void clear() { M::clear(); }

      void clearRange(const Time& t1, const Time& t2)
            {
            Q_ASSERT(!(t2 < t1));
            const auto first = M::lower_bound(t1);
            const auto last  = M::lower_bound(t2);
            if (first == last)
                  return;
            M::erase(first, last);
            }

      void erase(const Time& t) { M::erase(t); }

      const Time& nextValueTime(const Time& t, const Time& defaultTime) const
            {
            if (empty())
                  return defaultTime;
            const auto i = M::upper_bound(t);
            if (i == M::end())
                  return defaultTime;
            return i->first;
            }

      const Time& currentValueTime(const Time& t, const Time& defaultTime) const
            {
            if (empty())
                  return defaultTime;
            auto i = M::upper_bound(t);
            if (i == M::begin())
                  return defaultTime;
            return (--i)->first;
            }

      bool hasChangeAt(const Time& t) const { return M::find(t) != M::end(); }

   protected:
      iterator erase(iterator pos) { return M::erase(pos); }
      iterator erase(const_iterator first, const_iterator last) { return M::erase(first, last); }
      using M::insert;
      using M::find;
      using M::lower_bound;
      using M::upper_bound;
      };

//---------------------------------------------------------
//   TimeMap
///   Specialization of AbstractTimeMap for TimePosition
///   used as a time type, extended with functions useful
///   for Fraction-based time position tracking.
//---------------------------------------------------------

template<typename Val>
class TimeMap : public AbstractTimeMap<TimePosition, Val> {
      typedef AbstractTimeMap<TimePosition, Val> M;

   public:
      Fraction nextValueTime(const TimePosition& t) const
            {
            static constexpr TimePosition invalidTime(-1_Fr);
            return M::nextValueTime(t, invalidTime).tick();
            }

      Fraction currentValueTime(const TimePosition& t) const
            {
            static constexpr TimePosition minTime(0_Fr);
            return M::currentValueTime(t, minTime).tick();
            }

      void insertTime(Fraction tick, Fraction len, bool includeStartTick = true, std::vector<std::pair<TimePosition, Val>>* removed = nullptr)
            {
            if (len == 0_Fr)
                  return;
            auto shiftStart = includeStartTick ? M::lower_bound(tick) : M::upper_bound(tick);
            if (shiftStart == M::end())
                  return;

            if (len < 0_Fr) {
                  const auto rmEnd = M::lower_bound(tick - len);
                  if (rmEnd != shiftStart) {
                        // If removed is not null, record the removed elements there
                        if (removed)
                              removed->insert(removed->end(), std::make_move_iterator(shiftStart), std::make_move_iterator(rmEnd));
                        shiftStart = M::erase(shiftStart, rmEnd);
                        if (shiftStart == M::end())
                              return;
                        }
                  }

            // shift time for the items from shiftStart to M::end()
            std::vector<std::pair<TimePosition, Val>> shifted;
            for (auto i = shiftStart; i != M::end(); ++i)
                  shifted.emplace_back(TimePosition(i->first.tick() + len), std::move(i->second));
            M::erase(shiftStart, M::end());
            M::insert(std::make_move_iterator(shifted.begin()), std::make_move_iterator(shifted.end()));
            }
      };
} // namespace Ms
#endif
