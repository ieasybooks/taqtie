#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>

#include "helpers/file_helpers.h"
#include "helpers/time_helpers.h"
#include "readers/sections_reader.h"
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
  static const QString TEMPORARY_FILE_SUFFIX;
  static const QString PROCESS_TIMER_DEFAULT_VALUE;
  static const int PROCESS_UPDATE_EVERY_MILLISECONDS;
  static const QStringList ALLOWED_IMPORT_SECTIONS_FILE_EXTENSIONS;

  Ui::MainWindow *ui;

  AboutWindow *aboutWindow;

  QMenuBar *mainMenuBar;
  QMenu *mainMenu;
  QAction *aboutAction;

  int currentSection;
  FFmpegWrapper *ffmpegWrapper;
  FileHelpers *fileHelpers;
  SectionsReader *sectionsReader;
  TimeHelpers *timeHelpers;

  void setupUi();
  void setupMainMenu();

 private slots:
  void addSection();
  void importSections();
  void clearSections();
  void processSections();

 private:
  void addSectionToTable(const SectionInfo &sectionInfo);
  bool processSection(const qint16 &sectionId);
  void resetProcessTimer();
  void updateProcessTimer();
  void toggleActionableElements();
};
#endif  // MAINWINDOW_H
