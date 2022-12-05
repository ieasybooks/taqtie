#include "readers/sections_reader.h"

#include <QFileInfo>
#include <QMessageBox>

#include "xlsxdocument.h"

using namespace QXlsx;

SectionsReader::SectionsReader() {}

const QVector<SectionInfo> SectionsReader::read(const QString &filePath) {
  QFileInfo filePathInfo = QFileInfo(filePath);

  if (filePathInfo.suffix() == "xlsx") {
    return this->fromExcel(filePath);
  } else {
    throw std::invalid_argument("يجب أن يكون امتداد الملف xlsx.");
  }
}

const QVector<SectionInfo> SectionsReader::fromExcel(const QString &filePath) {
  QVector<SectionInfo> sections;

  Document sectionsFileReader(filePath);
  if (sectionsFileReader.load()) {
    for (qint16 row = 1; true; ++row) {
      Cell *cell = nullptr;
      QVariant sectionTitle;
      QVariant sectionStartTime;
      QVariant sectionEndTime;

      cell = sectionsFileReader.cellAt(row, 1);
      if (cell != NULL && cell->readValue().userType() == QMetaType::QString) {
        sectionTitle = cell->readValue();
      } else if (cell == NULL) {
        sectionTitle = QVariant("");
      } else {
        break;
      }

      cell = sectionsFileReader.cellAt(row, 2);
      if (cell != NULL && cell->readValue().userType() == QMetaType::QTime) {
        sectionStartTime = cell->readValue();
      } else {
        break;
      }

      cell = sectionsFileReader.cellAt(row, 3);
      if (cell != NULL && cell->readValue().userType() == QMetaType::QTime) {
        sectionEndTime = cell->readValue();
      } else {
        break;
      }

      sections.push_back(
          SectionInfo(sectionTitle.value<QString>(), sectionStartTime.value<QTime>(), sectionEndTime.value<QTime>()));
    }
  }

  return sections;
}
