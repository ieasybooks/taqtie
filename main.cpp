#include <QApplication>

#include "windows/main_window.h"

int main(int argc, char *argv[]) {
  QApplication qApplication(argc, argv);

  MainWindow mainWindow;
  mainWindow.show();

  return qApplication.exec();
}
