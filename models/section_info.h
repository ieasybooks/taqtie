#ifndef SECTIONINFO_H
#define SECTIONINFO_H

#include <QString>
#include <QTime>

class SectionInfo {
  QString name;
  QTime startTime;
  QTime endTime;

 public:
  SectionInfo(QString name, QTime startTime, QTime endTime);
  const QString &getTitle() const;
  const QTime &getStartTime() const;
  const QTime &getEndTime() const;
};

#endif  // SECTIONINFO_H
