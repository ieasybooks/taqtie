#include "helpers/time_helpers.h"

#include <QTime>

TimeHelpers::TimeHelpers() {}

QString TimeHelpers::secondsToTimeFormat(const qint64 &seconds) {
  QTime secondsTime = QTime(0, 0).addSecs(seconds);

  return QString("%1:%2:%3")
      .arg(secondsTime.hour(), 2, 10, QChar('0'))
      .arg(secondsTime.minute(), 2, 10, QChar('0'))
      .arg(secondsTime.second(), 2, 10, QChar('0'));
}
