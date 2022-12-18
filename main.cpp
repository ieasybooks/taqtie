#include <QApplication>
#include <QTranslator>

#include "windows/main_window.h"

int main(int argc, char *argv[]) {
  QApplication qApplication(argc, argv);

  QTranslator qtTranslator;
  if (qtTranslator.load(":/qt_ar.qm")) {
    qApplication.installTranslator(&qtTranslator);
  }

  MainWindow mainWindow;
  mainWindow.show();

  return qApplication.exec();
}
