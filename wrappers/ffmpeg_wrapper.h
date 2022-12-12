#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

#include <helpers/file_helpers.h>
#include <helpers/process_helpers.h>

class FFmpegWrapper {
 public:
  FFmpegWrapper();
  ~FFmpegWrapper();

  bool areFilesMergeable(const QVector<QString> &filesPaths, const std::function<void()> &callback = nullptr);
  bool areFilesPropertiesEqual(const QVector<QString> &filesPaths, const QString &properties,
                               const std::function<void()> &callback = nullptr);
  bool areFilesPropertiesEqual(const QVector<QString> &filesPaths, const QVector<QString> &properties,
                               const std::function<void()> &callback = nullptr);

  QVector<QMap<QString, QString> > getFileStreamsProperties(const QString &filePath, const QString &requiredProperties,
                                                            const std::function<void()> &callback = nullptr);
  QVector<QMap<QString, QString> > getFileStreamsProperties(const QString &filePath,
                                                            const QVector<QString> &requiredProperties,
                                                            const std::function<void()> &callback = nullptr);

  void cutFile(const QString &filePath, const QString &startTime, const QString &endTime, const QString &outputFilePath,
               const bool &isQuickCut = false, const std::function<void()> &callback = nullptr);

  void mergeFiles(const QVector<QString> &filesPaths, const int &baseVideoIndex, const QString &outputFilePath,
                  const bool &isQuickMerge, const std::function<void()> &callback = nullptr);
  void mergeFilesWithConcatFilter(const QVector<QString> &filesPaths, const int &baseVideoIndex,
                                  const QString &outputFilePath, const std::function<void()> &callback = nullptr);
  void mergeFilesWithDemuxer(const QVector<QString> &filesPaths, const QString &outputFilePath,
                             const bool &isQuickMerge, const std::function<void()> &callback = nullptr);

 private:
  static const QString MERGE_SENSITIVE_PROPERTIES;

  QString ffmpegExecutablePath;
  QString ffprobeExecutablePath;

  FileHelpers *fileHelpers;
  ProcessHelpers *processHelpers;

  void appendMacOSExecutablePaths();
};

#endif  // FFMPEGWRAPPER_H
