#include "models/section_info.h"

SectionInfo::SectionInfo(QString name, QTime startTime, QTime endTime)
    : name(name), startTime(startTime), endTime(endTime) {}

const QString &SectionInfo::getTitle() const { return name; }

const QTime &SectionInfo::getStartTime() const { return startTime; }

const QTime &SectionInfo::getEndTime() const { return endTime; }
