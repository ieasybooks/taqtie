#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

#include <helpers/file_helpers.h>
#include <helpers/process_helpers.h>

class FFmpegWrapper {
 public:
  FFmpegWrapper();
  ~FFmpegWrapper();

  void cutFile(const QString &filePath, const QString &startTime, const QString &endTime, const QString &outputFilePath,
               const bool &isQuickCut = false);

  void mergeFiles(const QVector<QString> &filesPaths, const int &baseVideoIndex, const QString &outputFilePath,
                  const bool &isQuickMerge);
  void mergeFilesWithConcatFilter(const QVector<QString> &filesPaths, const int &baseVideoIndex,
                                  const QString &outputFilePath);
  void mergeFilesWithDemuxer(const QVector<QString> &filesPaths, const QString &outputFilePath,
                             const bool &isQuickMerge);

  bool areFilesMergeable(const QVector<QString> &filesPaths);

  QVector<QMap<QString, QString> > getFileStreamsProperties(const QString &filePath,
                                                            const QVector<QString> &requiredProperties);
  QVector<QMap<QString, QString> > getFileStreamsProperties(const QString &filePath, const QString &requiredProperties);

 private:
  static const QString MERGE_SENSITIVE_PROPERTIES;

  QString ffmpegExecutablePath;
  QString ffprobeExecutablePath;

  FileHelpers *fileHelpers;
  ProcessHelpers *processHelpers;
};

#endif  // FFMPEGWRAPPER_H
