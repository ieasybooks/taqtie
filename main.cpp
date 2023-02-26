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

  QTranslator qtTranslator;
  if (qtTranslator.load(":/qt_ar.qm")) {
    application.installTranslator(&qtTranslator);
  }

  QTranslator customTranslator;
  if (customTranslator.load(":/i18n/i18n_ar.qm")) {
    application.installTranslator(&customTranslator);
  }

  MainWindow mainWindow;
  mainWindow.show();

  return application.exec();
}
