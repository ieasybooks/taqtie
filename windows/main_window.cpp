#include "windows/main_window.h"

#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>

#include "enums/section_columns.h"
#include "ui_main_window.h"

const QString MainWindow::TEMPORARY_FILE_SUFFIX = "-مؤقت";
const QString MainWindow::PROCESS_TIMER_DEFAULT_VALUE = "00:00:00.000";
const int MainWindow::PROCESS_UPDATE_EVERY_MILLISECONDS = 100;
const QStringList MainWindow::ALLOWED_IMPORT_SECTIONS_FILE_EXTENSIONS = {"Microsoft Excel (*.xlsx)"};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),

      aboutWindow(new AboutWindow),

      mainMenuBar(new QMenuBar),
      mainMenu(new QMenu),
      aboutAction(new QAction("عن تقطيع")),

      currentSection(0),

      ffmpegWrapper(new FFmpegWrapper),
      fileHelpers(new FileHelpers),
      sectionsReader(new SectionsReader),
      timeHelpers(new TimeHelpers) {
  this->setupUi();

  this->aboutWindow->setWindowModality(Qt::ApplicationModal);
}

MainWindow::~MainWindow() {
  delete ui;

  delete aboutWindow;

  delete mainMenuBar;
  delete mainMenu;
  delete aboutAction;

  delete ffmpegWrapper;
  delete fileHelpers;
  delete sectionsReader;
  delete timeHelpers;
}

void MainWindow::setupUi() {
  ui->setupUi(this);

  ui->sections->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  ui->sectionsToMergeLabelHint->setToolTip("سيتم دمج هذه الأجزاء مع الإبقاء على الأجزاء المقطعة");
  ui->selectIntroFileLabelHint->setToolTip("سيتم دمج هذا الملف في بداية كل جزء من الأجزاء");
  ui->selectOutroFileLabelHint->setToolTip("سيتم دمج هذا الملف في نهاية كل جزء من الأجزاء");

  connect(ui->addSection, &QPushButton::clicked, this, &MainWindow::addSection);
  connect(ui->sections, &QTableWidget::itemDoubleClicked, this, &MainWindow::removeSection);
  connect(ui->importSections, &QPushButton::clicked, this, &MainWindow::importSections);
  connect(ui->clearSections, &QPushButton::clicked, this, &MainWindow::clearSections);
  connect(ui->processSections, &QPushButton::clicked, this, &MainWindow::processSections);

  connect(ui->selectFile, &QPushButton::clicked, this,
          [this]() -> void { ui->cuttingFilePath->setText(fileHelpers->selectFile(this)); });

  connect(ui->selectIntroFile, &QPushButton::clicked, this,
          [this]() -> void { ui->introFilePath->setText(fileHelpers->selectFile(this)); });

  connect(ui->selectOutroFile, &QPushButton::clicked, this,
          [this]() -> void { ui->outroFilePath->setText(fileHelpers->selectFile(this)); });

  this->setupMainMenu();
}

void MainWindow::setupMainMenu() {
  this->aboutAction->setMenuRole(QAction::ApplicationSpecificRole);
  this->mainMenu->addAction(this->aboutAction);
  this->mainMenuBar->addMenu(this->mainMenu);

  this->setMenuBar(this->mainMenuBar);

  connect(this->aboutAction, &QAction::triggered, this, [this]() -> void { this->aboutWindow->show(); });
}

void MainWindow::addSection() {
  QString sectionTitle = ui->sectionTitle->text();

  if (sectionTitle.trimmed().isEmpty()) {
    sectionTitle = QString::number(++this->currentSection);
  }

  QTime sectionStartTime = ui->sectionStartTime->time();
  QTime sectionEndTime = ui->sectionEndTime->time();

  if (sectionStartTime >= sectionEndTime) {
    QMessageBox messageBox;
    messageBox.critical(this, "خطأ", "وقت البداية يجب أن يكون أقل من وقت النهاية.");
    return;
  }

  this->addSectionToTable(SectionInfo(sectionTitle, sectionStartTime, sectionEndTime));

  ui->sectionTitle->setText("");
  ui->sectionTitle->setFocus();
  ui->sectionStartTime->setTime(sectionEndTime);
}

void MainWindow::removeSection(QTableWidgetItem *item) {
  int row = item->row();
  ui->sections->removeRow(row);

  for (int i = row; i < ui->sections->rowCount(); ++i) {
    ui->sections->setItem(i, SectionColumns::SECTION_INDEX, new QTableWidgetItem(QString::number(i + 1)));
  }
}

void MainWindow::importSections() {
  QString filePath = fileHelpers->selectFile(this, MainWindow::ALLOWED_IMPORT_SECTIONS_FILE_EXTENSIONS.join(";;"));

  if (!filePath.trimmed().isEmpty()) {
    QVector<SectionInfo> sections = this->sectionsReader->read(filePath.trimmed());

    for (SectionInfo &sectionInfo : sections) {
      if (sectionInfo.getTitle().isEmpty()) {
        this->addSectionToTable(
            SectionInfo(QString::number(++this->currentSection), sectionInfo.getStartTime(), sectionInfo.getEndTime()));
      } else {
        this->addSectionToTable(sectionInfo);
      }
    }
  }
}

void MainWindow::clearSections() {
  while (ui->sections->rowCount() > 0) {
    ui->sections->removeRow(0);
  }

  this->currentSection = 0;
  ui->sectionStartTime->setTime(QTime(0, 0, 0));
  ui->sectionEndTime->setTime(QTime(0, 0, 0));
}

void MainWindow::processSections() {
  if (!this->areInputsValid()) {
    return;
  }

  this->toggleActionableElements();
  this->resetProcessTimer();
  ui->processProgress->setValue(0);

  bool errorsHappened = false;

  for (int i = 0; i < ui->sections->rowCount(); ++i) {
    if (!this->processSection(i)) {
      errorsHappened = true;
      break;
    }
    ui->processProgress->setValue(1.0 * (i + 1) / ui->sections->rowCount() * 100);
  }

  if (!errorsHappened) {
    QMessageBox messageBox;
    messageBox.information(this, "إنتهينا!", "تم الإنتهاء من العملية التي بدأتها! ستجد المخرجات في مجلد الملف الأصلي.");
  }

  this->toggleActionableElements();
}

void MainWindow::addSectionToTable(const SectionInfo &sectionInfo) {
  int rowsCount = ui->sections->rowCount();

  ui->sections->insertRow(rowsCount);
  ui->sections->setItem(rowsCount, SectionColumns::SECTION_INDEX, new QTableWidgetItem(QString::number(rowsCount + 1)));
  ui->sections->setItem(rowsCount, SectionColumns::SECTION_TITLE, new QTableWidgetItem(sectionInfo.getTitle()));
  ui->sections->setItem(rowsCount, SectionColumns::SECTION_START_TIME,
                        new QTableWidgetItem(sectionInfo.getStartTime().toString()));
  ui->sections->setItem(rowsCount, SectionColumns::SECTION_END_TIME,
                        new QTableWidgetItem(sectionInfo.getEndTime().toString()));
  ui->sections->setItem(rowsCount, SectionColumns::SECTION_DURATION,
                        new QTableWidgetItem(this->timeHelpers->secondsToTimeFormat(
                            sectionInfo.getStartTime().secsTo(sectionInfo.getEndTime()))));
}

bool MainWindow::processSection(const int &sectionId) {
  QString cuttingFilePath = ui->cuttingFilePath->text();
  QFileInfo cuttingFilePathInfo = QFileInfo(cuttingFilePath);

  QString sectionTitle = ui->sections->item(sectionId, SectionColumns::SECTION_TITLE)->text();
  QString sectionStartTime = ui->sections->item(sectionId, SectionColumns::SECTION_START_TIME)->text();
  QString sectionEndTime = ui->sections->item(sectionId, SectionColumns::SECTION_END_TIME)->text();

  QString introFilePath = ui->introFilePath->text();
  QString outroFilePath = ui->outroFilePath->text();

  QString sectionFilePath =
      cuttingFilePathInfo.absoluteDir().filePath(sectionTitle + "." + cuttingFilePathInfo.suffix());
  QString tmpSectionFilePath = cuttingFilePathInfo.absoluteDir().filePath(
      sectionTitle + MainWindow::TEMPORARY_FILE_SUFFIX + "." + cuttingFilePathInfo.suffix());

  if (QFile::exists(sectionFilePath)) {
    QMessageBox messageBox;
    messageBox.critical(this, "خطأ",
                        "يوجد ملف باسم \"" + sectionTitle +
                            "\" مسبقًا، يُرجى اختيار اسم آخر.\n"
                            "سيتم إيقاف العملية عند هذا الملف، يُرجى متابعة العملية بعد حل المشكلة.");
    return false;
  }

  if (introFilePath.isEmpty() && outroFilePath.isEmpty()) {
    ffmpegWrapper->cutFile(ui->cuttingFilePath->text(), sectionStartTime, sectionEndTime, sectionFilePath,
                           ui->quickCut->isChecked(), std::bind(&MainWindow::updateProcessTimer, this));
  } else {
    ffmpegWrapper->cutFile(ui->cuttingFilePath->text(), sectionStartTime, sectionEndTime, tmpSectionFilePath,
                           ui->quickCut->isChecked(), std::bind(&MainWindow::updateProcessTimer, this));

    QVector<QString> filesToMerge;
    int baseVideoIndex = 0;

    if (!introFilePath.isEmpty()) {
      filesToMerge.append(introFilePath);
      baseVideoIndex = 1;
    }

    filesToMerge.append(tmpSectionFilePath);

    if (!outroFilePath.isEmpty()) {
      filesToMerge.append(outroFilePath);
    }

    ffmpegWrapper->mergeFiles(filesToMerge, baseVideoIndex, sectionFilePath, ui->quickMerge->isChecked(),
                              std::bind(&MainWindow::updateProcessTimer, this));

    fileHelpers->deleteFile(tmpSectionFilePath);
  }

  return true;
}

void MainWindow::resetProcessTimer() { ui->processTimer->setText(MainWindow::PROCESS_TIMER_DEFAULT_VALUE); }

void MainWindow::updateProcessTimer() {
  QString currentTime = ui->processTimer->text();
  ui->processTimer->setText(
      timeHelpers->addMillisecondsToTime(currentTime, MainWindow::PROCESS_UPDATE_EVERY_MILLISECONDS));
}

void MainWindow::toggleActionableElements() {
  QList<QPushButton *> buttons = ui->centralwidget->findChildren<QPushButton *>();
  QList<QCheckBox *> checkBoxes = ui->centralwidget->findChildren<QCheckBox *>();

  for (int i = 0; i < buttons.size(); ++i) {
    buttons[i]->setEnabled(!buttons[i]->isEnabled());
  }

  for (int i = 0; i < checkBoxes.size(); ++i) {
    checkBoxes[i]->setEnabled(!checkBoxes[i]->isEnabled());
  }
}

bool MainWindow::areInputsValid() {
  QMessageBox messageBox;

  if (ui->cuttingFilePath->text().trimmed().isEmpty()) {
    messageBox.critical(this, "خطأ", "يجب اختيار ملف قبل البدأ في العملية.");
    return false;
  }

  if (!QFile::exists(ui->cuttingFilePath->text().trimmed())) {
    messageBox.critical(this, "خطأ", "الملف المراد تقطيعه غير موجود.");
    return false;
  }

  if (ui->sections->rowCount() == 0) {
    messageBox.critical(this, "خطأ", "يجب إضافة جزء واحد على الأقل.");
    return false;
  }

  if (!ui->sectionsToMerge->text().trimmed().isEmpty()) {
    QStringList tokenizedSectionsToMerge = ui->sectionsToMerge->text().trimmed().split(" ");

    for (const QString &sectionToMerge : tokenizedSectionsToMerge) {
      QStringList tokenizedSectionToMerge;
      bool numberExpected = true;

      for (int i = 0; i < sectionToMerge.length();) {
        if (numberExpected) {
          QString number = "";

          while (i < sectionToMerge.length() && sectionToMerge[i].isDigit()) {
            number += sectionToMerge[i];
            ++i;
          }

          if (number.isEmpty()) {
            messageBox.critical(this, "خطأ", "يوجد خطأ في مدخلات الأجزاء المُراد دمجها بعد التقطيع.");
            return false;
          }

          tokenizedSectionToMerge.append(number);
        } else {
          if (sectionToMerge[i] != '-' && sectionToMerge[i] != '+') {
            messageBox.critical(this, "خطأ", "يوجد خطأ في مدخلات الأجزاء المُراد دمجها بعد التقطيع.");
            return false;
          }

          tokenizedSectionToMerge.append(sectionToMerge[i++]);
        }

        numberExpected = !numberExpected;
      }

      if (numberExpected || tokenizedSectionToMerge.size() < 3) {
        messageBox.critical(this, "خطأ", "يوجد خطأ في مدخلات الأجزاء المُراد دمجها بعد التقطيع.");
        return false;
      }

      for (int i = 0; i < tokenizedSectionToMerge.size(); ++i) {
        if (tokenizedSectionToMerge[i] != '-' && tokenizedSectionToMerge[i] != '+' &&
            tokenizedSectionToMerge[i].toInt() > ui->sections->rowCount()) {
          messageBox.critical(this, "خطأ", "يوجد خطأ في مدخلات الأجزاء المُراد دمجها بعد التقطيع.");
          return false;
        }

        if (tokenizedSectionToMerge[i] == '-' &&
            tokenizedSectionToMerge[i - 1].toInt() >= tokenizedSectionToMerge[i + 1].toInt()) {
          messageBox.critical(this, "خطأ", "يوجد خطأ في مدخلات الأجزاء المُراد دمجها بعد التقطيع.");
          return false;
        }
      }
    }
  }

  if (!ui->introFilePath->text().trimmed().isEmpty() && !QFile::exists(ui->introFilePath->text().trimmed())) {
    messageBox.critical(this, "خطأ", "ملف البادئة غير موجود.");
    return false;
  }

  if (!ui->outroFilePath->text().trimmed().isEmpty() && !QFile::exists(ui->outroFilePath->text().trimmed())) {
    messageBox.critical(this, "خطأ", "ملف الخاتمة غير موجود.");
    return false;
  }

  return true;
}
