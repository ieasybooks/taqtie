#include "wrappers/ffmpeg_wrapper.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QUuid>

/*
 * The source of this properties list came from:
 *  - https://stackoverflow.com/questions/47050033/ffmpeg-join-two-movies-with-different-timebase
 *  - https://stackoverflow.com/questions/52830468/ffmpeg-concat-videos-error-do-not-match-the-corresponding-output-link
 */
const QString FFmpegWrapper::MERGE_SENSITIVE_PROPERTIES =
    "codec_name,width,height,sample_aspect_ratio,display_aspect_ratio,pix_fmt,r_frame_rate,avg_frame_rate,time_base,"
    "sample_rate,channels,channel_layout";

FFmpegWrapper::FFmpegWrapper() {
  this->ffmpegExecutablePath = "ffmpeg";
  this->ffprobeExecutablePath = "ffprobe";

  this->fileHelpers = new FileHelpers();
  this->processHelpers = new ProcessHelpers();

#ifdef Q_OS_MACOS
  this->ffmpegExecutablePath = "/opt/homebrew/bin/ffmpeg";
  this->ffprobeExecutablePath = "/opt/homebrew/bin/ffprobe";
#endif
}

FFmpegWrapper::~FFmpegWrapper() {
  delete fileHelpers;
  delete processHelpers;
}

void FFmpegWrapper::cutFile(const QString& filePath, const QString& startTime, const QString& endTime,
                            const QString& outputFilePath, const bool& isQuickCut) {
  QStringList arguments{"-ss", startTime, "-to", endTime, "-i", filePath};

  if (isQuickCut) {
    arguments << "-c"
              << "copy";
  }

  arguments << outputFilePath;

  this->processHelpers->doBlockingProcess(this->ffmpegExecutablePath, arguments);
}

void FFmpegWrapper::mergeFiles(const QVector<QString>& filesPaths, const int& baseVideoIndex,
                               const QString& outputFilePath, const bool& isQuickMerge) {
  if (this->areFilesMergeable(filesPaths)) {
    this->mergeFilesWithDemuxer(filesPaths, outputFilePath, isQuickMerge);
  } else {
    this->mergeFilesWithConcatFilter(filesPaths, baseVideoIndex, outputFilePath);
  }
}

void FFmpegWrapper::mergeFilesWithConcatFilter(const QVector<QString>& filesPaths, const int& baseVideoIndex,
                                               const QString& outputFilePath) {
  QVector<QMap<QString, QString>> fileStreamsProperties =
      this->getFileStreamsProperties(filesPaths[baseVideoIndex], FFmpegWrapper::MERGE_SENSITIVE_PROPERTIES);

  QStringList arguments;

  QString filterComplex = "";

  for (int i = 0; i < filesPaths.size(); ++i) {
    arguments << "-i" << filesPaths[i];
    filterComplex += "[" + QString::number(i) + ":v]";
    filterComplex += "scale=" + fileStreamsProperties[0]["width"] + ":" + fileStreamsProperties[0]["height"] + ",";
    filterComplex += "setsar=" + fileStreamsProperties[0]["sample_aspect_ratio"].replace(':', '/') + ",";
    filterComplex += "setdar=" + fileStreamsProperties[0]["display_aspect_ratio"].replace(':', '/');
    filterComplex += "[vout" + QString::number(i) + "];";
  }

  for (int i = 0; i < filesPaths.size(); ++i) {
    filterComplex += "[vout" + QString::number(i) + "][" + QString::number(i) + ":a]";
  }

  filterComplex += "concat=n=" + QString::number(filesPaths.size()) + ":v=1:a=1[vout][aout]";

  arguments << "-fps_mode"
            << "vfr"
            << "-filter_complex" << filterComplex << "-map"
            << "[vout]"
            << "-map"
            << "[aout]" << outputFilePath;

  this->processHelpers->doBlockingProcess(this->ffmpegExecutablePath, arguments);
}

void FFmpegWrapper::mergeFilesWithDemuxer(const QVector<QString>& filesPaths, const QString& outputFilePath,
                                          const bool& isQuickMerge) {
  QString demuxerListFileName = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);

  QVector<QString> lines;
  for (int i = 0; i < filesPaths.size(); ++i) {
    lines.append("file '" + filesPaths[i] + "'");
  }

  fileHelpers->writeLinesToFile(demuxerListFileName, lines);

  QStringList arguments{"-f", "concat", "-safe", "0", "-i", demuxerListFileName};

  if (isQuickMerge) {
    arguments << "-c"
              << "copy";
  }

  arguments << outputFilePath;

  this->processHelpers->doBlockingProcess(this->ffmpegExecutablePath, arguments);

  fileHelpers->deleteFile(demuxerListFileName);
}

bool FFmpegWrapper::areFilesMergeable(const QVector<QString>& filesPaths) {
  assert(filesPaths.size() >= 2);

  QVector<QMap<QString, QString>> firstFileMergeSensitiveProperties =
      this->getFileStreamsProperties(filesPaths[0], FFmpegWrapper::MERGE_SENSITIVE_PROPERTIES);

  for (int i = 1; i < filesPaths.size(); ++i) {
    if (firstFileMergeSensitiveProperties !=
        this->getFileStreamsProperties(filesPaths[i], FFmpegWrapper::MERGE_SENSITIVE_PROPERTIES)) {
      return false;
    }
  }

  return true;
}

QVector<QMap<QString, QString>> FFmpegWrapper::getFileStreamsProperties(const QString& filePath,
                                                                        const QString& requiredProperties) {
  return this->getFileStreamsProperties(filePath, requiredProperties.split(','));
}

QVector<QMap<QString, QString>> FFmpegWrapper::getFileStreamsProperties(const QString& filePath,
                                                                        const QVector<QString>& requiredProperties) {
  QString processOutput = this->processHelpers->doBlockingProcess(
      this->ffprobeExecutablePath, QVector<QString>{"-v", "error", "-hide_banner", "-show_entries",
                                                    "stream=" + requiredProperties.join(','), filePath});

  QVector<QString> streamsRawProperties = processOutput.split('\n');

  QVector<QMap<QString, QString>> streamsProperties;

  for (int i = 1; i < streamsRawProperties.size(); i += 2) {
    QMap<QString, QString> streamProperties;

    while (streamsRawProperties[i] != "[/STREAM]") {
      QVector<QString> property = streamsRawProperties[i].split('=');
      streamProperties[property[0]] = property[1];
      ++i;
    }

    streamsProperties.append(streamProperties);
  }

  return streamsProperties;
}
