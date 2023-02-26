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

  static const QString EMPTY_CUTTING_FILE_ERROR;
  static const QString NOT_EXISTING_CUTTING_FILE_ERROR;
  static const QString NO_SECTIONS_ADDED_ERROR;
  static const QString SECTIONS_TO_MERGE_ERROR;
  static const QString NOT_EXISTING_INTRO_FILE_ERROR;
  static const QString NOT_EXISTING_OUTRO_FILE_ERROR;

  bool validateCuttingFilePath();
  bool validateSections();
  bool validateSectionsToMergeList();
  bool validateIntroAndOutroFilesPaths();
  bool validateSectionsToMergeFormat(const QString &sectionsToMerge);
  QStringList tokenizeSectionsToMerge(const QString &sectionsToMerge);
  bool validateTokenizedSectionsToMerge(const QStringList &tokenizedSectionsToMerge);
};

#endif  // MAIN_WINDOW_VALIDATOR_H
