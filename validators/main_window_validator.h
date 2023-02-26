#ifndef MAIN_WINDOW_VALIDATOR_H
#define MAIN_WINDOW_VALIDATOR_H

#include <QString>

#include "ui_main_window.h"

class MainWindowValidator {
 public:
  MainWindowValidator(Ui::MainWindow *ui);
  ~MainWindowValidator();

  bool validate();

 private:
  Ui::MainWindow *ui;

  bool validateCuttingFilePath();
  bool validateSections();
  bool validateSectionsToMergeList();
  bool validateIntroAndOutroFilesPaths();
  bool validateSectionsToMergeFormat(const QString &sectionsToMerge);
  QStringList tokenizeSectionsToMerge(const QString &sectionsToMerge);
  bool validateTokenizedSectionsToMerge(const QStringList &tokenizedSectionsToMerge);
};

#endif  // MAIN_WINDOW_VALIDATOR_H
