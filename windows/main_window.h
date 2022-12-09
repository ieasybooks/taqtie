#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>

#include "helpers/file_helpers.h"
#include "helpers/time_helpers.h"
#include "readers/sections_reader.h"
#include "wrappers/ffmpeg_wrapper.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  void setupUi();
  ~MainWindow();

 private slots:
  void selectCuttingFile();
  void addSection();
  void importSections();
  void removeSection(QTableWidgetItem *item);
  void clearSections();
  void selectIntroFile();
  void selectOutroFile();
  void processSections();

 private:
  static const QString TEMPORARY_FILE_SUFFIX;
  static const QString PROCESS_TIMER_DEFAULT_VALUE;
  static const qint32 PROCESS_UPDATE_EVERY_MILLISECONDS;
  static const QVector<QString> ALLOWED_IMPORT_SECTIONS_FILE_EXTENSIONS;

  Ui::MainWindow *ui;

  qint32 currentSection;
  FFmpegWrapper *ffmpegWrapper;
  FileHelpers *fileHelpers;
  SectionsReader *sectionsReader;
  TimeHelpers *timeHelpers;

  enum columns {
    SECTION_TITLE = 0,
    SECTION_START_TIME = 1,
    SECTION_END_TIME = 2,
    SECTION_DURATION = 3,
  };

  void addSectionToTable(const SectionInfo &sectionInfo);
  bool processSection(const qint16 &sectionId);
  void resetProcessTime();
  void updateProcessTimer();
  void toggleActionableElements();
};
#endif  // MAINWINDOW_H
