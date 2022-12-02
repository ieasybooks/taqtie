#ifndef FILEHELPERS_H
#define FILEHELPERS_H

#include <QWidget>

class FileHelpers {
 public:
  FileHelpers();
  QString selectFile(QWidget *widget, const QString &filter = "");
};

#endif  // FILEHELPERS_H
