#include "windows/main_window.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

#include "ui_main_window.h"

const QString MainWindow::TEMPORARY_FILE_SUFFIX = "-مؤقت";
const QString MainWindow::PROCESS_TIMER_DEFAULT_VALUE = "00:00:00.000";
const qint32 MainWindow::PROCESS_UPDATE_EVERY_MILLISECONDS = 100;
const QVector<QString> MainWindow::ALLOWED_IMPORT_SECTIONS_FILE_EXTENSIONS = {"Microsoft Excel (*.xlsx)"};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      currentSection(0),
      ffmpegWrapper(new FFmpegWrapper),
      fileHelpers(new FileHelpers),
      sectionsReader(new SectionsReader),
      timeHelpers(new TimeHelpers) {
  setupUi();
}

void MainWindow::setupUi() {
  ui->setupUi(this);

  ui->sections->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  connect(ui->selectFile, &QPushButton::clicked, this, &MainWindow::selectCuttingFile);
  connect(ui->addSection, &QPushButton::clicked, this, &MainWindow::addSection);
  connect(ui->importSections, &QPushButton::clicked, this, &MainWindow::importSections);
  connect(ui->sections, &QTableWidget::itemDoubleClicked, this, &MainWindow::removeSection);
  connect(ui->clearSections, &QPushButton::clicked, this, &MainWindow::clearSections);
  connect(ui->selectIntroFile, &QPushButton::clicked, this, &MainWindow::selectIntroFile);
  connect(ui->selectOutroFile, &QPushButton::clicked, this, &MainWindow::selectOutroFile);
  connect(ui->processSections, &QPushButton::clicked, this, &MainWindow::processSections);
}

MainWindow::~MainWindow() {
  delete ui;
  delete ffmpegWrapper;
  delete fileHelpers;
  delete sectionsReader;
  delete timeHelpers;
}

void MainWindow::selectCuttingFile() { ui->cuttingFilePath->setText(fileHelpers->selectFile(this)); }

void MainWindow::addSection() {
  QString sectionTitle = ui->sectionTitle->text();

  if (sectionTitle.trimmed().isEmpty()) {
    sectionTitle = QString::number(++this->currentSection);
  }

  QTime sectionStartTime = ui->sectionStartTime->time();
  QTime sectionEndTime = ui->sectionEndTime->time();

  if (sectionStartTime >= sectionEndTime) {
    QMessageBox messageBox;
    messageBox.critical(0, "خطأ", "وقت البداية يجب أن يكون أقل من وقت النهاية.");

    return;
  }

  this->addSectionToTable(SectionInfo(sectionTitle, sectionStartTime, sectionEndTime));

  ui->sectionTitle->setText("");
  ui->sectionTitle->setFocus();
  ui->sectionStartTime->setTime(sectionEndTime);
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

void MainWindow::removeSection(QTableWidgetItem *item) { ui->sections->removeRow(item->row()); }

void MainWindow::clearSections() {
  while (ui->sections->rowCount() > 0) {
    ui->sections->removeRow(0);
  }

  this->currentSection = 0;
  ui->sectionStartTime->setTime(QTime(0, 0, 0));
  ui->sectionEndTime->setTime(QTime(0, 0, 0));
}

void MainWindow::selectIntroFile() { ui->introFilePath->setText(fileHelpers->selectFile(this)); }

void MainWindow::selectOutroFile() { ui->outroFilePath->setText(fileHelpers->selectFile(this)); }

void MainWindow::processSections() {
  this->toggleActionableElements();
  this->resetProcessTime();
  ui->processProgress->setValue(0);

  for (qint16 i = 0; i < ui->sections->rowCount(); ++i) {
    this->processSection(i);
    ui->processProgress->setValue(1.0 * (i + 1) / ui->sections->rowCount() * 100);
  }

  this->toggleActionableElements();
}

void MainWindow::addSectionToTable(const SectionInfo &sectionInfo) {
  qint16 rowsCount = ui->sections->rowCount();

  ui->sections->insertRow(rowsCount);
  ui->sections->setItem(rowsCount, SECTION_TITLE, new QTableWidgetItem(sectionInfo.getTitle()));
  ui->sections->setItem(rowsCount, SECTION_START_TIME, new QTableWidgetItem(sectionInfo.getStartTime().toString()));
  ui->sections->setItem(rowsCount, SECTION_END_TIME, new QTableWidgetItem(sectionInfo.getEndTime().toString()));
  ui->sections->setItem(rowsCount, SECTION_DURATION,
                        new QTableWidgetItem(this->timeHelpers->secondsToTimeFormat(
                            sectionInfo.getStartTime().secsTo(sectionInfo.getEndTime()))));
}

void MainWindow::processSection(const qint16 &sectionId) {
  QString cuttingFilePath = ui->cuttingFilePath->text();
  QFileInfo cuttingFilePathInfo = QFileInfo(cuttingFilePath);

  QString sectionTitle = ui->sections->item(sectionId, SECTION_TITLE)->text();
  QString sectionStartTime = ui->sections->item(sectionId, SECTION_START_TIME)->text();
  QString sectionEndTime = ui->sections->item(sectionId, SECTION_END_TIME)->text();

  QString introFilePath = ui->introFilePath->text();
  QString outroFilePath = ui->outroFilePath->text();

  QString sectionFilePath =
      cuttingFilePathInfo.absoluteDir().filePath(sectionTitle + "." + cuttingFilePathInfo.suffix());
  QString tmpSectionFilePath = cuttingFilePathInfo.absoluteDir().filePath(
      sectionTitle + MainWindow::TEMPORARY_FILE_SUFFIX + "." + cuttingFilePathInfo.suffix());

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
}

void MainWindow::resetProcessTime() { ui->processTimer->setText(MainWindow::PROCESS_TIMER_DEFAULT_VALUE); }

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