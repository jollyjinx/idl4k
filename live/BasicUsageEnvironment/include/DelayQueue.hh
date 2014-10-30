/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
 // Copyright (c) 1996-2011, Live Networks, Inc.  All rights reserved
// Delay queue
// C++ header

#ifndef _DELAY_QUEUE_HH
#define _DELAY_QUEUE_HH

#ifndef _NET_COMMON_H
#include "NetCommon.h"
#endif

#ifdef TIME_BASE
typedef TIME_BASE time_base_seconds;
#else
typedef long time_base_seconds;
#endif

///// A "Timeval" can be either an absolute time, or a time interval /////

class Timeval {
public:
  virtual time_base_seconds seconds() const {
    return fTv.tv_sec;
  }
  virtual time_base_seconds seconds() {
    return fTv.tv_sec;
  }
  virtual time_base_seconds useconds() const {
    return fTv.tv_usec;
  }
  virtual time_base_seconds useconds() {
    return fTv.tv_usec;
  }

  int operator>=(Timeval const& arg2) const;
  int operator<=(Timeval const& arg2) const {
    return arg2 >= *this;
  }
  int operator<(Timeval const& arg2) const {
    return !(*this >= arg2);
  }
  int operator>(Timeval const& arg2) const {
    return arg2 < *this;
  }
  int operator==(Timeval const& arg2) const {
    return *this >= arg2 && arg2 >= *this;
  }
  int operator!=(Timeval const& arg2) const {
    return !(*this == arg2);
  }

  virtual void operator+=(class DelayInterval const& arg2);
  virtual void operator-=(class DelayInterval const& arg2);
  // returns ZERO iff arg2 >= arg1

protected:
  Timeval(time_base_seconds seconds, time_base_seconds useconds) {
    fTv.tv_sec = seconds; fTv.tv_usec = useconds;
  }

private:
  time_base_seconds& secs() {
    return (time_base_seconds&)fTv.tv_sec;
  }
  time_base_seconds& usecs() {
    return (time_base_seconds&)fTv.tv_usec;
  }

  struct timeval fTv;
};

#ifndef max
inline Timeval max(Timeval const& arg1, Timeval const& arg2) {
  return arg1 >= arg2 ? arg1 : arg2;
}
#endif
#ifndef min
inline Timeval min(Timeval const& arg1, Timeval const& arg2) {
  return arg1 <= arg2 ? arg1 : arg2;
}
#endif

class DelayInterval operator-(Timeval const& arg1, Timeval const& arg2);
// returns ZERO iff arg2 >= arg1

///// A "Timespec" can be either an absolute time, or a time interval /////

class Timespec : public Timeval {
public:
  virtual time_base_seconds seconds() const {
    return fTs.tv_sec;
  }
  virtual time_base_seconds seconds() {
    return fTs.tv_sec;
  }
  virtual time_base_seconds useconds() const {
    return fTs.tv_nsec/1000L;
  }
  virtual time_base_seconds useconds() {
    return fTs.tv_nsec/1000L;
  }
  virtual time_base_seconds nseconds() const {
    return fTs.tv_nsec;
  }
  virtual time_base_seconds nseconds() {
    return fTs.tv_nsec;
  }

  int operator>=(Timespec const& arg2) const;
  int operator<=(Timespec const& arg2) const {
    return arg2 >= *this;
  }
  int operator<(Timespec const& arg2) const {
    return !(*this >= arg2);
  }
  int operator>(Timespec const& arg2) const {
    return arg2 < *this;
  }
  int operator==(Timespec const& arg2) const {
    return *this >= arg2 && arg2 >= *this;
  }
  int operator!=(Timespec const& arg2) const {
    return !(*this == arg2);
  }

  virtual void operator+=(class DelayInterval const& arg2);
  virtual void operator-=(class DelayInterval const& arg2);

protected:
  Timespec(time_base_seconds seconds, time_base_seconds nseconds)
  : Timeval(seconds, nseconds/1000L) {
    fTs.tv_sec = seconds; fTs.tv_nsec = nseconds;
  }

private:
  time_base_seconds& secs() {
    return (time_base_seconds&)fTs.tv_sec;
  }
  time_base_seconds& nsecs() {
    return (time_base_seconds&)fTs.tv_nsec;
  }

  struct timespec fTs;
};

///// DelayInterval /////

class DelayInterval: public Timespec {
public:
  DelayInterval(time_base_seconds seconds, time_base_seconds useconds)
    : Timespec(seconds, useconds*1000L) {}
};

DelayInterval operator*(short arg1, DelayInterval const& arg2);

extern DelayInterval const DELAY_ZERO;
extern DelayInterval const DELAY_SECOND;

///// EventTime /////

class EventTime: public Timeval {
public:
  EventTime(unsigned secondsSinceEpoch = 0,
	    unsigned usecondsSinceEpoch = 0)
    // We use the Unix standard epoch: January 1, 1970
    : Timeval(secondsSinceEpoch, usecondsSinceEpoch) {}
};

///// EventTimeMonotonic /////

class EventTimeMonotonic : public Timespec {
public:
  EventTimeMonotonic(unsigned secondsSinceStart = 0,
	    unsigned nsecondsSinceStart = 0)
  : Timespec(secondsSinceStart, nsecondsSinceStart) {}
};

EventTimeMonotonic TimeNow();

extern EventTimeMonotonic const THE_END_OF_TIME;

///// DelayQueueEntry /////

class DelayQueueEntry {
public:
  virtual ~DelayQueueEntry();

  intptr_t token() {
    return fToken;
  }

protected: // abstract base class
  DelayQueueEntry(DelayInterval delay);

  virtual void handleTimeout();

private:
  friend class DelayQueue;
  DelayQueueEntry* fNext;
  DelayQueueEntry* fPrev;
  DelayInterval fDeltaTimeRemaining;

  intptr_t fToken;
  static intptr_t tokenCounter;
};

///// DelayQueue /////

class DelayQueue: public DelayQueueEntry {
public:
  DelayQueue();
  virtual ~DelayQueue();

  void addEntry(DelayQueueEntry* newEntry); // returns a token for the entry
  void updateEntry(DelayQueueEntry* entry, DelayInterval newDelay);
  void updateEntry(intptr_t tokenToFind, DelayInterval newDelay);
  void removeEntry(DelayQueueEntry* entry); // but doesn't delete it
  DelayQueueEntry* removeEntry(intptr_t tokenToFind); // but doesn't delete it

  DelayInterval const& timeToNextAlarm();
  void handleAlarm();

private:
  DelayQueueEntry* head() { return fNext; }
  DelayQueueEntry* findEntryByToken(intptr_t token);
  void synchronize(); // bring the 'time remaining' fields up-to-date

  EventTimeMonotonic fLastSyncTime;
};

#endif
