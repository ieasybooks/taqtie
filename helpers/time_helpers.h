#ifndef TIMEHELPERS_H
#define TIMEHELPERS_H

#include <QString>

class TimeHelpers {
 public:
  TimeHelpers();
  QString secondsToTimeFormat(const qint64 &seconds);
};

#endif  // TIMEHELPERS_H
