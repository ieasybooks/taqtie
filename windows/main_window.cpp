#include "windows/main_window.h"

#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>

#include "enums/section_columns.h"
#include "ui_main_window.h"

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
      validator(new MainWindowValidator(ui)),
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

  ui->sectionsToMergeListLabelHint->setToolTip(MainWindow::SECTIONS_TO_MERGE_LABEL_HIND);
  ui->selectIntroFileLabelHint->setToolTip(MainWindow::SELECT_INTRO_FILE_LABEL_HINT);
  ui->selectOutroFileLabelHint->setToolTip(MainWindow::SELECT_OUTRO_FILE_LABEL_HINT);

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
    messageBox.critical(this, "خطأ", MainWindow::START_TIME_LESS_THAN_END_TIME_ERROR);
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

void MainWindow::resetProcessTimer() { ui->processTimer->setText(MainWindow::PROCESS_TIMER_DEFAULT_VALUE); }

void MainWindow::updateProcessTimer() {
  QString currentTime = ui->processTimer->text();
  ui->processTimer->setText(
      timeHelpers->addMillisecondsToTime(currentTime, MainWindow::PROCESS_UPDATE_EVERY_MILLISECONDS));
}

void MainWindow::toggleActionableElements() {
  ui->sectionsToMergeList->setEnabled(!ui->sectionsToMergeList->isEnabled());
  QList<QPushButton *> buttons = ui->centralwidget->findChildren<QPushButton *>();
  QList<QCheckBox *> checkBoxes = ui->centralwidget->findChildren<QCheckBox *>();

  for (int i = 0; i < buttons.size(); ++i) {
    buttons[i]->setEnabled(!buttons[i]->isEnabled());
  }

  for (int i = 0; i < checkBoxes.size(); ++i) {
    checkBoxes[i]->setEnabled(!checkBoxes[i]->isEnabled());
  }
}

void MainWindow::processSections() {
  if (!this->validator->validate()) {
    return;
  }

  this->toggleActionableElements();
  this->resetProcessTimer();
  ui->processProgress->setValue(0);

  if (!this->cutSections()) {
    return;
  }

  if (!this->mergeSections()) {
    return;
  }

  if (!this->mergeIntroAndOutro()) {
    return;
  }

  QMessageBox messageBox;
  messageBox.information(this, "إنتهينا!", MainWindow::PROCESS_FINISHED_SUCCESSFULLY_MESSAGE);

  this->toggleActionableElements();
}

bool MainWindow::cutSections() {
  for (int sectionIndex = 0; sectionIndex < ui->sections->rowCount(); ++sectionIndex) {
    QString sectionOutputFilePath = this->getSectionOutputFilePath(sectionIndex);

    if (QFile::exists(sectionOutputFilePath)) {
      QMessageBox messageBox;
      messageBox.critical(this, "خطأ", MainWindow::OUTPUT_FILE_EXISTS_ALREADY_ERROR.arg(sectionOutputFilePath));
      return false;
    }

    QString sectionStartTime = ui->sections->item(sectionIndex, SectionColumns::SECTION_START_TIME)->text();
    QString sectionEndTime = ui->sections->item(sectionIndex, SectionColumns::SECTION_END_TIME)->text();

    ffmpegWrapper->cutFile(ui->cuttingFilePath->text(), sectionStartTime, sectionEndTime, sectionOutputFilePath,
                           ui->quickCut->isChecked(), std::bind(&MainWindow::updateProcessTimer, this));
  }

  return true;
}

bool MainWindow::mergeSections() {
  if (ui->sectionsToMergeList->text().trimmed().isEmpty()) {
    return true;
  }

  for (const QString &sectionsToMerge : ui->sectionsToMergeList->text().trimmed().split(" ")) {
    QStringList tokenizedSectionsToMerge = sectionsToMerge.split("+");

    QStringList filesPaths;

    for (int i = 0; i < tokenizedSectionsToMerge.size(); ++i) {
      if (tokenizedSectionsToMerge[i].split("-").size() == 1) {
        filesPaths.append(this->getSectionOutputFilePath(tokenizedSectionsToMerge[i].toInt() - 1));
      } else {
        for (int j = tokenizedSectionsToMerge[i].split("-")[0].toInt();
             j <= tokenizedSectionsToMerge[i].split("-")[1].toInt(); ++j) {
          filesPaths.append(this->getSectionOutputFilePath(j - 1));
        }
      }
    }

    QString sectionsToMergeOutputFilePath = this->getSectionsToMergeOutputFilePath(sectionsToMerge);

    if (QFile::exists(sectionsToMergeOutputFilePath)) {
      QMessageBox messageBox;
      messageBox.critical(this, "خطأ", MainWindow::OUTPUT_FILE_EXISTS_ALREADY_ERROR.arg(sectionsToMergeOutputFilePath));
      return false;
    }

    ffmpegWrapper->mergeFiles(filesPaths, 0, sectionsToMergeOutputFilePath, ui->quickMerge->isChecked(),
                              std::bind(&MainWindow::updateProcessTimer, this));
  }

  return true;
}

bool MainWindow::mergeIntroAndOutro() {
  if (ui->introFilePath->text().trimmed().isEmpty() && ui->introFilePath->text().trimmed().isEmpty()) {
    return true;
  }

  QString introFilePath = ui->introFilePath->text();
  QString outroFilePath = ui->outroFilePath->text();

  for (int sectionIndex = 0; sectionIndex < ui->sections->rowCount(); ++sectionIndex) {
    QString sectionOutputFilePath = this->getSectionOutputFilePath(sectionIndex);
    QString finalSectionOutputFilePath =
        this->getSectionOutputFilePath(sectionIndex).replace(MainWindow::TEMPORARY_FILE_SUFFIX, "");

    if (QFile::exists(finalSectionOutputFilePath)) {
      QMessageBox messageBox;
      messageBox.critical(this, "خطأ", MainWindow::OUTPUT_FILE_EXISTS_ALREADY_ERROR.arg(finalSectionOutputFilePath));
      return false;
    }

    QStringList filesToMerge;
    int baseVideoIndex = 0;

    if (!introFilePath.isEmpty()) {
      filesToMerge.append(introFilePath);
      baseVideoIndex = 1;
    }

    filesToMerge.append(sectionOutputFilePath);

    if (!outroFilePath.isEmpty()) {
      filesToMerge.append(outroFilePath);
    }

    ffmpegWrapper->mergeFiles(filesToMerge, baseVideoIndex, finalSectionOutputFilePath, ui->quickMerge->isChecked(),
                              std::bind(&MainWindow::updateProcessTimer, this));
    fileHelpers->deleteFile(sectionOutputFilePath);
  }

  for (const QString &sectionsToMerge : ui->sectionsToMergeList->text().split(" ")) {
    if (sectionsToMerge.trimmed().isEmpty()) {
      continue;
    }

    QString sectionsToMergeOutputFilePath = this->getSectionsToMergeOutputFilePath(sectionsToMerge);
    QString finalSectionsToMergeOutputFilePath =
        this->getSectionsToMergeOutputFilePath(sectionsToMerge).replace(MainWindow::TEMPORARY_FILE_SUFFIX, "");

    if (QFile::exists(finalSectionsToMergeOutputFilePath)) {
      QMessageBox messageBox;
      messageBox.critical(this, "خطأ",
                          MainWindow::OUTPUT_FILE_EXISTS_ALREADY_ERROR.arg(finalSectionsToMergeOutputFilePath));
      return false;
    }

    QStringList filesToMerge;
    int baseVideoIndex = 0;

    if (!introFilePath.isEmpty()) {
      filesToMerge.append(introFilePath);
      baseVideoIndex = 1;
    }

    filesToMerge.append(sectionsToMergeOutputFilePath);

    if (!outroFilePath.isEmpty()) {
      filesToMerge.append(outroFilePath);
    }

    ffmpegWrapper->mergeFiles(filesToMerge, baseVideoIndex, finalSectionsToMergeOutputFilePath,
                              ui->quickMerge->isChecked(), std::bind(&MainWindow::updateProcessTimer, this));
    fileHelpers->deleteFile(sectionsToMergeOutputFilePath);
  }

  return true;
}

QString MainWindow::getSectionOutputFilePath(const int &sectionIndex) {
  QFileInfo cuttingFilePathInfo = QFileInfo(ui->cuttingFilePath->text());

  QString sectionTitle = ui->sections->item(sectionIndex, SectionColumns::SECTION_TITLE)->text();

  if (ui->introFilePath->text().trimmed().isEmpty() && ui->outroFilePath->text().trimmed().isEmpty()) {
    return cuttingFilePathInfo.absoluteDir().filePath(sectionTitle + "." + cuttingFilePathInfo.suffix());
  } else {
    return cuttingFilePathInfo.absoluteDir().filePath(sectionTitle + MainWindow::TEMPORARY_FILE_SUFFIX + "." +
                                                      cuttingFilePathInfo.suffix());
  }
}

QString MainWindow::getSectionsToMergeOutputFilePath(const QString &sectionToMerge) {
  QFileInfo cuttingFilePathInfo = QFileInfo(ui->cuttingFilePath->text());

  if (ui->introFilePath->text().trimmed().isEmpty() && ui->outroFilePath->text().trimmed().isEmpty()) {
    return cuttingFilePathInfo.absoluteDir().filePath(sectionToMerge + "." + cuttingFilePathInfo.suffix());
  } else {
    return cuttingFilePathInfo.absoluteDir().filePath(sectionToMerge + MainWindow::TEMPORARY_FILE_SUFFIX + "." +
                                                      cuttingFilePathInfo.suffix());
  }
}

const QString MainWindow::TEMPORARY_FILE_SUFFIX = "-مؤقت";
const QString MainWindow::PROCESS_TIMER_DEFAULT_VALUE = "00:00:00.000";
const int MainWindow::PROCESS_UPDATE_EVERY_MILLISECONDS = 100;
const QStringList MainWindow::ALLOWED_IMPORT_SECTIONS_FILE_EXTENSIONS = {"Microsoft Excel (*.xlsx)"};

const QString MainWindow::SECTIONS_TO_MERGE_LABEL_HIND = "سيتم دمج هذه الأجزاء مع الإبقاء على الأجزاء المقطعة";
const QString MainWindow::SELECT_INTRO_FILE_LABEL_HINT = "سيتم دمج هذا الملف في بداية كل جزء من الأجزاء";
const QString MainWindow::SELECT_OUTRO_FILE_LABEL_HINT = "سيتم دمج هذا الملف في نهاية كل جزء من الأجزاء";
const QString MainWindow::START_TIME_LESS_THAN_END_TIME_ERROR = "وقت البداية يجب أن يكون أقل من وقت النهاية.";
const QString MainWindow::PROCESS_FINISHED_SUCCESSFULLY_MESSAGE =
    "تم الإنتهاء من العملية التي بدأتها! ستجد المخرجات في مجلد الملف الأصلي.";
const QString MainWindow::OUTPUT_FILE_EXISTS_ALREADY_ERROR =
    "يوجد ملف باسم %1 مسبقًا، يُرجى اختيار اسم آخر.\nسيتم إيقاف العملية عند هذا الملف، يُرجى متابعة العملية بعد حل المش"
    "كلة.";
