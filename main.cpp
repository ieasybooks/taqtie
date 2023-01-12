#include <QApplication>
#include <QTranslator>

#include "windows/main_window.h"

int main(int argc, char *argv[]) {
  QApplication application(argc, argv);

  application.setOrganizationName("الكتب المُيسّرة");
  application.setOrganizationDomain("ieasybooks.com");
  application.setApplicationDisplayName("تقطيع");
  application.setApplicationName("taqtie");
  application.setApplicationVersion("0.1");

  QIcon icon(":/resources/taqtie.png");
  application.setWindowIcon(icon);

  QTranslator translator;
  if (translator.load(":/qt_ar.qm")) {
    application.installTranslator(&translator);
  }

  MainWindow mainWindow;
  mainWindow.show();

  return application.exec();
}
