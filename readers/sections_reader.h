#ifndef SECTIONSREADER_H
#define SECTIONSREADER_H

#include <QVector>

#include "models/section_info.h"

class SectionsReader {
 public:
  SectionsReader();
  const QVector<SectionInfo> read(const QString &filePath);

 private:
  const QVector<SectionInfo> fromExcel(const QString &filePath);
};

#endif  // SECTIONSREADER_H
