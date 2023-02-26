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
    throw std::invalid_argument("File extension should be Xlsx.");
  }
}

// This function was refactored by ChatGPT (with some modifications).
const QVector<SectionInfo> SectionsReader::fromExcel(const QString &filePath) {
  QVector<SectionInfo> sections;

  Document sectionsFileReader(filePath);

  if (!sectionsFileReader.load()) {
    return sections;
  }

  for (int row = 1; true; ++row) {
    Cell *cell = nullptr;
    QString sectionTitle;
    QTime sectionStartTime;
    QTime sectionEndTime;

    cell = sectionsFileReader.cellAt(row, 1);
    if (cell != nullptr && cell->readValue().userType() == QMetaType::QString) {
      sectionTitle = cell->readValue().toString();
    } else if (cell == nullptr) {
      sectionTitle = "";
    } else {
      break;
    }

    cell = sectionsFileReader.cellAt(row, 2);
    if (cell == nullptr || cell->readValue().userType() != QMetaType::QTime) {
      break;
    }
    sectionStartTime = cell->readValue().toTime();

    cell = sectionsFileReader.cellAt(row, 3);
    if (cell == nullptr || cell->readValue().userType() != QMetaType::QTime) {
      break;
    }
    sectionEndTime = cell->readValue().toTime();

    sections.push_back(SectionInfo(sectionTitle, sectionStartTime, sectionEndTime));
  }

  return sections;
}
