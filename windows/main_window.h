#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>

#include "helpers/file_helpers.h"
#include "helpers/time_helpers.h"
#include "readers/sections_reader.h"
#include "validators/main_window_validator.h"
#include "windows/about_window.h"
#include "wrappers/ffmpeg_wrapper.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private:
  static const QString PROCESS_TIMER_DEFAULT_VALUE;
  static const int PROCESS_UPDATE_EVERY_MILLISECONDS;
  static const QStringList ALLOWED_IMPORT_SECTIONS_FILE_EXTENSIONS;

  Ui::MainWindow *ui;

  AboutWindow *aboutWindow;

  QMenuBar *mainMenuBar;
  QMenu *mainMenu;
  QAction *aboutAction;

  int currentSection;
  int totalProcessingSteps;
  int finishedProcessingSteps;

  FFmpegWrapper *ffmpegWrapper;
  FileHelpers *fileHelpers;
  SectionsReader *sectionsReader;
  MainWindowValidator *validator;
  TimeHelpers *timeHelpers;

  void setupUi();
  void setupMainMenu();

 private slots:
  void addSection();
  void removeSection(QTableWidgetItem *item);
  void importSections();
  void clearSections();

  void processSections();

 private:
  void addSectionToTable(const SectionInfo &sectionInfo);
  void updateProcessProgress();
  void updateProcessTimer();
  void toggleActionableElements();

  void calculateTotalProcessingSteps();
  bool cutSections();
  bool mergeSections();
  bool mergeIntroAndOutro();
  bool finalizeMergeIntroAndOutro(const QString &inputFilePath, const QString &outputFilePath);

  QString getSectionOutputFilePath(const int &sectionIndex);
  QString getSectionsToMergeOutputFilePath(const QString &sectionToMerge);
};
#endif  // MAINWINDOW_H
