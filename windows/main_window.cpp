#include "windows/main_window.h"

#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>

#include "enums/section_columns.h"
#include "ui_main_window.h"

const QString MainWindow::PROCESS_TIMER_DEFAULT_VALUE = "00:00:00.000";
const int MainWindow::PROCESS_UPDATE_EVERY_MILLISECONDS = 100;
const QStringList MainWindow::ALLOWED_IMPORT_SECTIONS_FILE_EXTENSIONS = {"Microsoft Excel (*.xlsx)"};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),

      aboutWindow(new AboutWindow),

      mainMenuBar(new QMenuBar),
      mainMenu(new QMenu),
      aboutAction(new QAction(tr("about_taqtie"))),

      currentSection(0),
      totalProcessingSteps(0),
      finishedProcessingSteps(0),

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

  ui->sectionsToMergeListLabelHint->setToolTip(tr("sections_to_merge_label_hint"));
  ui->selectIntroFileLabelHint->setToolTip(tr("select_intro_file_label_hint"));
  ui->selectOutroFileLabelHint->setToolTip(tr("select_outro_file_label_hint"));

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
    messageBox.critical(this, QObject::tr("error"), tr("start_time_less_than_end_time_error"));
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

void MainWindow::updateProcessProgress() {
  ui->processProgress->setValue(1.0 * ++finishedProcessingSteps / this->totalProcessingSteps * 100);
}

void MainWindow::updateProcessTimer() {
  QString currentTime = ui->processTimer->text();

  ui->processTimer->setText(
      timeHelpers->addMillisecondsToTime(currentTime, MainWindow::PROCESS_UPDATE_EVERY_MILLISECONDS));
}

void MainWindow::toggleActionableElements() {
  ui->sections->setEnabled(!ui->sections->isEnabled());
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

  this->finishedProcessingSteps = 0;
  this->calculateTotalProcessingSteps();
  ui->processTimer->setText(MainWindow::PROCESS_TIMER_DEFAULT_VALUE);
  ui->processProgress->setValue(0);

  bool errorHappened = false;

  if (!errorHappened) {
    errorHappened = !this->cutSections();
  }

  if (!errorHappened) {
    errorHappened = !this->mergeSections();
  }

  if (!errorHappened) {
    errorHappened = !this->mergeIntroAndOutro();
  }

  if (!errorHappened) {
    QMessageBox messageBox;
    messageBox.information(this, tr("process_finished_successfully_title"),
                           tr("process_finished_successfully_message"));
  }

  this->toggleActionableElements();
}

void MainWindow::calculateTotalProcessingSteps() {
  this->totalProcessingSteps = ui->sections->rowCount();

  if (!ui->sectionsToMergeList->text().trimmed().isEmpty()) {
    this->totalProcessingSteps += ui->sectionsToMergeList->text().split(" ").size();
  }

  if (!ui->introFilePath->text().trimmed().isEmpty() || !ui->outroFilePath->text().trimmed().isEmpty()) {
    this->totalProcessingSteps *= 2;
  }
}

bool MainWindow::cutSections() {
  for (int sectionIndex = 0; sectionIndex < ui->sections->rowCount(); ++sectionIndex) {
    QString sectionOutputFilePath = this->getSectionOutputFilePath(sectionIndex);

    if (QFile::exists(sectionOutputFilePath)) {
      QMessageBox messageBox;
      messageBox.critical(this, QObject::tr("error"),
                          tr("output_file_exists_already_error").arg(sectionOutputFilePath));
      return false;
    }

    QString sectionStartTime = ui->sections->item(sectionIndex, SectionColumns::SECTION_START_TIME)->text();
    QString sectionEndTime = ui->sections->item(sectionIndex, SectionColumns::SECTION_END_TIME)->text();

    ffmpegWrapper->cutFile(ui->cuttingFilePath->text(), sectionStartTime, sectionEndTime, sectionOutputFilePath,
                           ui->quickCut->isChecked(), std::bind(&MainWindow::updateProcessTimer, this));

    this->updateProcessProgress();
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
      messageBox.critical(this, QObject::tr("error"),
                          tr("output_file_exists_already_error").arg(sectionsToMergeOutputFilePath));
      return false;
    }

    ffmpegWrapper->mergeFiles(filesPaths, 0, sectionsToMergeOutputFilePath, ui->quickMerge->isChecked(),
                              std::bind(&MainWindow::updateProcessTimer, this));

    this->updateProcessProgress();
  }

  return true;
}

bool MainWindow::mergeIntroAndOutro() {
  if (ui->introFilePath->text().trimmed().isEmpty() && ui->introFilePath->text().trimmed().isEmpty()) {
    return true;
  }

  for (int sectionIndex = 0; sectionIndex < ui->sections->rowCount(); ++sectionIndex) {
    QString sectionOutputFilePath = this->getSectionOutputFilePath(sectionIndex);
    QString finalSectionOutputFilePath = this->getSectionOutputFilePath(sectionIndex).replace(tr("-temporary"), "");

    if (!this->finalizeMergeIntroAndOutro(sectionOutputFilePath, finalSectionOutputFilePath)) {
      return false;
    }
  }

  for (const QString &sectionsToMerge : ui->sectionsToMergeList->text().split(" ")) {
    if (sectionsToMerge.trimmed().isEmpty()) {
      continue;
    }

    QString sectionsToMergeOutputFilePath = this->getSectionsToMergeOutputFilePath(sectionsToMerge);
    QString finalSectionsToMergeOutputFilePath =
        this->getSectionsToMergeOutputFilePath(sectionsToMerge).replace(tr("-temporary"), "");

    if (!this->finalizeMergeIntroAndOutro(sectionsToMergeOutputFilePath, finalSectionsToMergeOutputFilePath)) {
      return false;
    }
  }

  return true;
}

bool MainWindow::finalizeMergeIntroAndOutro(const QString &inputFilePath, const QString &outputFilePath) {
  if (QFile::exists(outputFilePath)) {
    QMessageBox messageBox;
    messageBox.critical(this, QObject::tr("error"), tr("output_file_exists_already_error").arg(outputFilePath));
    return false;
  }

  QString introFilePath = ui->introFilePath->text();
  QString outroFilePath = ui->outroFilePath->text();

  QStringList filesToMerge;
  int baseVideoIndex = 0;

  if (!introFilePath.isEmpty()) {
    filesToMerge.append(introFilePath);
    baseVideoIndex = 1;
  }

  filesToMerge.append(inputFilePath);

  if (!outroFilePath.isEmpty()) {
    filesToMerge.append(outroFilePath);
  }

  ffmpegWrapper->mergeFiles(filesToMerge, baseVideoIndex, outputFilePath, ui->quickMerge->isChecked(),
                            std::bind(&MainWindow::updateProcessTimer, this));
  fileHelpers->deleteFile(inputFilePath);

  this->updateProcessProgress();

  return true;
}

QString MainWindow::getSectionOutputFilePath(const int &sectionIndex) {
  QFileInfo cuttingFilePathInfo = QFileInfo(ui->cuttingFilePath->text());

  QString sectionTitle = ui->sections->item(sectionIndex, SectionColumns::SECTION_TITLE)->text();

  if (ui->introFilePath->text().trimmed().isEmpty() && ui->outroFilePath->text().trimmed().isEmpty()) {
    return cuttingFilePathInfo.absoluteDir().filePath(sectionTitle + "." + cuttingFilePathInfo.suffix());
  } else {
    return cuttingFilePathInfo.absoluteDir().filePath(sectionTitle + tr("-temporary") + "." +
                                                      cuttingFilePathInfo.suffix());
  }
}

QString MainWindow::getSectionsToMergeOutputFilePath(const QString &sectionToMerge) {
  QFileInfo cuttingFilePathInfo = QFileInfo(ui->cuttingFilePath->text());

  if (ui->introFilePath->text().trimmed().isEmpty() && ui->outroFilePath->text().trimmed().isEmpty()) {
    return cuttingFilePathInfo.absoluteDir().filePath(sectionToMerge + "." + cuttingFilePathInfo.suffix());
  } else {
    return cuttingFilePathInfo.absoluteDir().filePath(sectionToMerge + tr("-temporary") + "." +
                                                      cuttingFilePathInfo.suffix());
  }
}
