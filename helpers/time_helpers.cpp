#include "helpers/time_helpers.h"

#include <QTime>
#include <sstream>

TimeHelpers::TimeHelpers() {}

// This function was written by ChatGPT (with some renamings).
QString TimeHelpers::secondsToTimeFormat(const qint32 &seconds) {
  QTime t = QTime::fromMSecsSinceStartOfDay(seconds * 1000);
  return t.toString("hh:mm:ss");
}

// This function was written by ChatGPT (with some renamings).
QString TimeHelpers::addMillisecondsToTime(const QString &time, const qint32 &milliseconds) {
  QTime t = QTime::fromString(time, "hh:mm:ss.zzz");

  t = t.addMSecs(milliseconds);

  return t.toString("hh:mm:ss.zzz");
}
