#include "wrappers/ffmpeg_wrapper.h"

#include <QCoreApplication>
#include <QProcess>

FFmpegWrapper::FFmpegWrapper() {
  this->ffmpegExecutablePath = "ffmpeg";
  this->ffprobeExecutablePath = "ffprobe";

#ifdef Q_OS_MACOS
  this->ffmpegExecutablePath = "/opt/homebrew/bin/ffmpeg";
  this->ffprobeExecutablePath = "/opt/homebrew/bin/ffprobe";
#endif
}

void FFmpegWrapper::cutFile(const QString &filePath, const QString &startTime,
                            const QString &endTime,
                            const QString &outputFilePath,
                            const bool &isQuickCut) {
  QProcess process;

  QStringList arguments;

  arguments << "-ss" << startTime;
  arguments << "-to" << endTime;
  arguments << "-i" << filePath;

  if (isQuickCut) {
    arguments << "-c"
              << "copy";
  }

  arguments << outputFilePath;

  process.start(this->ffmpegExecutablePath, arguments);

  process.waitForStarted();
  do {
    QCoreApplication::processEvents();
  } while (!process.waitForFinished(10));

  process.close();
}

bool FFmpegWrapper::areTwoFilesMergeable(const QString &firstFilePath,
                                          const QString &secondFilePath) {
  return this->getMergeSensitiveFileProperties(firstFilePath) ==
         this->getMergeSensitiveFileProperties(secondFilePath);
}

QString FFmpegWrapper::getMergeSensitiveFileProperties(
    const QString &filePath) {
  QProcess process;

  QStringList arguments;

  arguments << "-v"
            << "error";
  arguments << "-hide_banner";
  arguments << "-show_entries";
  arguments << "stream=codec_name,width,height,pix_fmt,r_frame_rate,avg_frame_"
               "rate,time_base,"
               "sample_rate,channels,channel_layout";
  arguments << filePath;

  process.start(this->ffprobeExecutablePath, arguments);

  process.waitForStarted();
  do {
    QCoreApplication::processEvents();
  } while (!process.waitForFinished(10));

  return process.readAllStandardOutput();
}
