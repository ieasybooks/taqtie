#ifndef TIMEHELPERS_H
#define TIMEHELPERS_H

#include <QString>

class TimeHelpers {
 public:
  TimeHelpers();

  int timeFormatToSeconds(const QString &timeFormat);
  QString secondsToTimeFormat(const int &seconds);
  QString addMillisecondsToTime(const QString &time, const int &milliseconds);
};

#endif  // TIMEHELPERS_H
