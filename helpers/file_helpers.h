#ifndef FILEHELPERS_H
#define FILEHELPERS_H

#include <QWidget>

class FileHelpers {
 public:
  FileHelpers();

  QString selectFile(QWidget *widget, const QString &filter = "");

  void writeLinesToFile(const QString &filePath, const QVector<QString> &lines);

  void deleteFile(const QString &filePath);
};

#endif  // FILEHELPERS_H
