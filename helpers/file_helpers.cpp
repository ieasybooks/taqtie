#include "helpers/file_helpers.h"

#include <QFileDialog>

FileHelpers::FileHelpers() {}

QString FileHelpers::selectFile(QWidget *widget, const QString &filter) {
  return QFileDialog::getOpenFileName(widget, "قم باختيار الملف",
                                      QDir::currentPath(), filter);
}
