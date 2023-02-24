#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

#include <helpers/file_helpers.h>
#include <helpers/process_helpers.h>

class FFmpegWrapper {
 public:
  FFmpegWrapper();
  ~FFmpegWrapper();

  bool areFilesMergeable(const QStringList &filesPaths, const std::function<void()> &callback = nullptr);
  bool areFilesStreamsEqual(const QStringList &filesPaths, const QString &properties,
                            const std::function<void()> &callback = nullptr);
  bool areFilesStreamsEqual(const QStringList &filesPaths, const QStringList &properties,
                            const std::function<void()> &callback = nullptr);

  QVector<QMap<QString, QString> > getFileStreamsProperties(const QString &filePath, const QString &requiredProperties,
                                                            const std::function<void()> &callback = nullptr);
  QVector<QMap<QString, QString> > getFileStreamsProperties(const QString &filePath,
                                                            const QStringList &requiredProperties,
                                                            const std::function<void()> &callback = nullptr);

  void cutFile(const QString &inputFilePath, const QString &startTime, const QString &endTime,
               const QString &outputFilePath, const bool &isQuickCut = false,
               const std::function<void()> &callback = nullptr);

  void mergeFiles(const QStringList &filesPaths, const int &baseVideoIndex, const QString &outputFilePath,
                  const bool &isQuickMerge, const std::function<void()> &callback = nullptr);
  void mergeFilesWithConcatFilter(const QStringList &filesPaths, const int &baseVideoIndex,
                                  const QString &outputFilePath, const std::function<void()> &callback = nullptr);
  void mergeFilesWithDemuxer(const QStringList &filesPaths, const QString &outputFilePath, const bool &isQuickMerge,
                             const std::function<void()> &callback = nullptr);

 private:
  static const QStringList MERGE_SENSITIVE_PROPERTIES;

  QString ffmpegExecutablePath;
  QString ffprobeExecutablePath;

  FileHelpers *fileHelpers;
  ProcessHelpers *processHelpers;

  void appendMacOSExecutablePaths();
};

#endif  // FFMPEGWRAPPER_H
