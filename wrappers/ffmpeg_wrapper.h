#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/parseutils.h>
#include <libavutil/timestamp.h>
}

#include <helpers/file_helpers.h>
#include <helpers/process_helpers.h>
#include <helpers/time_helpers.h>

class FFmpegWrapper {
 public:
  FFmpegWrapper();
  ~FFmpegWrapper();

  /**
   * @brief Print the information of the passed packet.
   *
   * @fn logPacket
   * @param avFormatContext AVFormatContext of the given packet.
   * @param avPacket AVPacket to log.
   * @param tag String to tag the log output.
   */
  void logPacket(const AVFormatContext *avFormatContext, const AVPacket *avPacket, const QString tag);

  bool areFilesMergeable(const QStringList &filesPaths);
  bool areFilesStreamsEqual(const QStringList &filesPaths, const QString &properties);
  bool areFilesStreamsEqual(const QStringList &filesPaths, const QStringList &properties);

  QVector<QMap<QString, QVariant> > getFileStreamsProperties(const QString &filePath,
                                                             const QString &requiredProperties);
  QVector<QMap<QString, QVariant> > getFileStreamsProperties(const QString &filePath,
                                                             const QStringList &requiredProperties);

  /**
   * @brief Cut a file in the given input file path based on the start and end seconds, and output the cutted file to
   * the given output file path.
   *
   * @fn cutFile
   * @param inputFilePath Input file path to be cutted.
   * @param startTime Cutting start time in HH:MM:SS format.
   * @param endTime Cutting end time in HH:MM:SS format.
   * @param outputFilePath Output file path to write the new cutted file.
   *
   * @details This function will take an input file path and cut it based on the given start and end seconds. The cutted
   * file will then be outputted to the given output file path.
   *
   * @return True if the cutting operation finished successfully, false otherwise.
   */
  bool cutFile(const QString &inputFilePath, const QString &startTime, const QString &endTime,
               const QString &outputFilePath);

  /**
   * @brief Cut a file in the given input file path based on the start and end seconds, and output the cutted file to
   * the given output file path.
   *
   * @fn cutFile
   * @param inputFilePath Input file path to be cutted.
   * @param startSeconds Cutting start time in seconds.
   * @param endSeconds Cutting end time in seconds.
   * @param outputFilePath Output file path to write the new cutted file.
   *
   * @details This function will take an input file path and cut it based on the given start and end seconds. The cutted
   * file will then be outputted to the given output file path.
   *
   * @return True if the cutting operation finished successfully, false otherwise.
   */
  bool cutFile(const QString &inputFilePath, const long long &startSeconds, const long long &endSeconds,
               const QString &outputFilePath);

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
  TimeHelpers *timeHelpers;

  void appendMacOSExecutablePaths();
};

#endif  // FFMPEGWRAPPER_H
