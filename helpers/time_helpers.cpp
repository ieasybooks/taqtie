#include "helpers/time_helpers.h"

#include <QTime>
#include <sstream>

TimeHelpers::TimeHelpers() {}

// This function was written by ChatGPT (with some renamings and modifications).
QString TimeHelpers::secondsToTimeFormat(const qint32 &seconds) {
  return QTime::fromMSecsSinceStartOfDay(seconds * 1000).toString("hh:mm:ss");
}

// This function was written by ChatGPT (with some renamings and modifications).
QString TimeHelpers::addMillisecondsToTime(const QString &time, const qint32 &milliseconds) {
  return QTime::fromString(time, "hh:mm:ss.zzz").addMSecs(milliseconds).toString("hh:mm:ss.zzz");
}
