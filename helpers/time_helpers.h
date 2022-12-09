#ifndef TIMEHELPERS_H
#define TIMEHELPERS_H

#include <QString>

class TimeHelpers {
 public:
  TimeHelpers();

  QString secondsToTimeFormat(const qint32 &seconds);
  QString addMillisecondsToTime(const QString &time, const qint32 &milliseconds);
};

#endif  // TIMEHELPERS_H
