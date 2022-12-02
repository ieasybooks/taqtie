#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

#include <QString>

class FFmpegWrapper {
 public:
  FFmpegWrapper();
  void cutFile(const QString &filePath, const QString &startTime,
               const QString &endTime, const QString &outputFilePath,
               const bool &isQuickCut = false);
  QString getMergeSensitiveFileProperties(const QString &filePath);

 private:
  QString ffmpegExecutablePath;
  QString ffprobeExecutablePath;
};

#endif  // FFMPEGWRAPPER_H
