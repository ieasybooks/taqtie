#include "main_window_validator.h"

#include <QFile>
#include <QMessageBox>

MainWindowValidator::MainWindowValidator(Ui::MainWindow *ui) : ui(ui) {}

MainWindowValidator::~MainWindowValidator() { delete ui; }

bool MainWindowValidator::validate() {
  return this->validateCuttingFilePath() && this->validateSections() && this->validateSectionsToMergeList() &&
         this->validateIntroAndOutroFilesPaths();
}

bool MainWindowValidator::validateCuttingFilePath() {
  QMessageBox messageBox;

  if (ui->cuttingFilePath->text().trimmed().isEmpty()) {
    messageBox.critical(nullptr, QObject::tr("error"), QObject::tr("empty_cutting_file_path_error"));
    return false;
  }

  if (!QFile::exists(ui->cuttingFilePath->text().trimmed())) {
    messageBox.critical(nullptr, QObject::tr("error"), QObject::tr("not_existing_cutting_file_error"));
    return false;
  }

  return true;
}

bool MainWindowValidator::validateSections() {
  QMessageBox messageBox;

  if (ui->sections->rowCount() == 0) {
    messageBox.critical(nullptr, QObject::tr("error"), QObject::tr("no_sections_added_error"));
    return false;
  }

  return true;
}

bool MainWindowValidator::validateSectionsToMergeList() {
  QMessageBox messageBox;

  if (ui->sectionsToMergeList->text().trimmed().isEmpty()) {
    return true;
  }

  for (const QString &sectionsToMerge : ui->sectionsToMergeList->text().trimmed().split(" ")) {
    if (!this->validateSectionsToMergeFormat(sectionsToMerge)) {
      messageBox.critical(nullptr, QObject::tr("error"), QObject::tr("sections_to_merge_error"));
      return false;
    }

    QStringList tokenizedSectionToMerge = this->tokenizeSectionsToMerge(sectionsToMerge);

    if (!this->validateTokenizedSectionsToMerge(tokenizedSectionToMerge)) {
      messageBox.critical(nullptr, QObject::tr("error"), QObject::tr("sections_to_merge_error"));
      return false;
    }
  }

  return true;
}

bool MainWindowValidator::validateIntroAndOutroFilesPaths() {
  QMessageBox messageBox;

  if (!ui->introFilePath->text().trimmed().isEmpty() && !QFile::exists(ui->introFilePath->text().trimmed())) {
    messageBox.critical(nullptr, QObject::tr("error"), QObject::tr("not_existing_intro_file_path_error"));
    return false;
  }

  if (!ui->outroFilePath->text().trimmed().isEmpty() && !QFile::exists(ui->outroFilePath->text().trimmed())) {
    messageBox.critical(nullptr, QObject::tr("error"), QObject::tr("not_existing_outro_file_path_error"));
    return false;
  }

  return true;
}

bool MainWindowValidator::validateSectionsToMergeFormat(const QString &sectionsToMerge) {
  bool isValidFormat = true;

  for (int i = 0; i < sectionsToMerge.length(); ++i) {
    if (sectionsToMerge[i] == '-' || sectionsToMerge[i] == '+') {
      isValidFormat &= (i > 0 && sectionsToMerge[i - 1].isDigit() && i < sectionsToMerge.length() - 1 &&
                        sectionsToMerge[i + 1].isDigit());
    } else if (!sectionsToMerge[i].isDigit()) {
      return false;
    }
  }

  return isValidFormat;
}

QStringList MainWindowValidator::tokenizeSectionsToMerge(const QString &sectionsToMerge) {
  QStringList tokenizedSectionToMerge;
  bool numberExpected = true;

  for (int i = 0; i < sectionsToMerge.length();) {
    if (numberExpected) {
      QString number = "";

      while (i < sectionsToMerge.length() && sectionsToMerge[i].isDigit()) {
        number += sectionsToMerge[i++];
      }

      tokenizedSectionToMerge.append(number);
    } else {
      tokenizedSectionToMerge.append(sectionsToMerge[i++]);
    }

    numberExpected = !numberExpected;
  }

  return tokenizedSectionToMerge;
}

bool MainWindowValidator::validateTokenizedSectionsToMerge(const QStringList &tokenizedSectionsToMerge) {
  for (int i = 0; i < tokenizedSectionsToMerge.size(); ++i) {
    if (tokenizedSectionsToMerge[i] != '-' && tokenizedSectionsToMerge[i] != '+' &&
        tokenizedSectionsToMerge[i].toInt() > ui->sections->rowCount()) {
      return false;
    }

    if (tokenizedSectionsToMerge[i] == '-' &&
        tokenizedSectionsToMerge[i - 1].toInt() >= tokenizedSectionsToMerge[i + 1].toInt()) {
      return false;
    }
  }

  return true;
}
