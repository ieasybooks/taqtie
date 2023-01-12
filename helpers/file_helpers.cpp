#include "helpers/file_helpers.h"

#include <QFileDialog>

FileHelpers::FileHelpers() {}

QString FileHelpers::selectFile(QWidget *widget, const QString &filter) {
  return QFileDialog::getOpenFileName(widget, "قم باختيار الملف", QDir::currentPath(), filter);
}

void FileHelpers::writeLinesToFile(const QString &filePath, const QStringList &lines) {
  QFile file(filePath);

  if (file.open(QIODevice::ReadWrite)) {
    QTextStream stream(&file);
    for (int i = 0; i < lines.size(); ++i) {
      stream << lines[i] << Qt::endl;
    }
  }

  file.close();
}

void FileHelpers::deleteFile(const QString &filePath) {
  QFile file(filePath);
  file.remove();
}
