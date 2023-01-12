#include "wrappers/ffmpeg_wrapper.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/pixdesc.h>
}

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
const QStringList FFmpegWrapper::MERGE_SENSITIVE_PROPERTIES = {
    "avg_frame_rate",       "ch_layout",   "codec_id",  "codec_type",  "codecpar.sample_aspect_ratio",
    "display_aspect_ratio", "format",      "height",    "nb_channels", "r_frame_rate",
    "sample_aspect_ratio",  "sample_rate", "time_base", "width"};

FFmpegWrapper::FFmpegWrapper() {
  this->ffmpegExecutablePath = "ffmpeg";
  this->ffprobeExecutablePath = "ffprobe";

  this->fileHelpers = new FileHelpers();
  this->processHelpers = new ProcessHelpers();

#ifdef Q_OS_MACOS
  this->appendMacOSExecutablePaths();
#endif
}

FFmpegWrapper::~FFmpegWrapper() {
  delete fileHelpers;
  delete processHelpers;
}

bool FFmpegWrapper::areFilesMergeable(const QStringList& filesPaths, const std::function<void()>& callback) {
  assert(filesPaths.size() >= 2);
  return this->areFilesStreamsEqual(filesPaths, FFmpegWrapper::MERGE_SENSITIVE_PROPERTIES, callback);
}

bool FFmpegWrapper::areFilesStreamsEqual(const QStringList& filesPaths, const QString& properties,
                                         const std::function<void()>& callback) {
  return this->areFilesStreamsEqual(filesPaths, properties.split(','), callback);
}

bool FFmpegWrapper::areFilesStreamsEqual(const QStringList& filesPaths, const QStringList& properties,
                                         const std::function<void()>& callback) {
  QVector<QMap<QString, QVariant>> firstFileStreamsProperties =
      this->getFileStreamsProperties(filesPaths[0], properties, callback);

  for (const QString& filePath : filesPaths) {
    QVector<QMap<QString, QVariant>> otherFileStreamsProperties =
        this->getFileStreamsProperties(filePath, properties, callback);

    if (otherFileStreamsProperties.size() != firstFileStreamsProperties.size()) {
      return false;
    }

    for (int j = 0; j < firstFileStreamsProperties.size(); ++j) {
      QStringList streamProperties = firstFileStreamsProperties[j].keys();

      for (const QString& property : streamProperties) {
        if (!otherFileStreamsProperties[j].contains(property) ||
            QPartialOrdering::Equivalent !=
                QVariant::compare(otherFileStreamsProperties[j][property], firstFileStreamsProperties[j][property])) {
          return false;
        }
      }
    }
  }

  return true;
}

QVector<QMap<QString, QVariant>> FFmpegWrapper::getFileStreamsProperties(const QString& filePath,
                                                                         const QString& requiredProperties,
                                                                         const std::function<void()>& callback) {
  return this->getFileStreamsProperties(filePath, requiredProperties.split(','), callback);
}

QVector<QMap<QString, QVariant>> FFmpegWrapper::getFileStreamsProperties(const QString& filePath,
                                                                         const QStringList& requiredProperties,
                                                                         const std::function<void()>& callback) {
  avformat_network_init();

  AVFormatContext* formatContext = nullptr;
  if (avformat_open_input(&formatContext, filePath.toStdString().c_str(), nullptr, nullptr) != 0) {
    return {};
  }

  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    avformat_close_input(&formatContext);
    return {};
  }

  QVector<QMap<QString, QVariant>> streamsProperties;
  for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
    QMap<QString, QVariant> streamProperties;

    AVStream* stream = formatContext->streams[i];

    for (const QString& property : requiredProperties) {
      if (property == "avg_frame_rate") {
        streamProperties["avg_frame_rate"] =
            QString("%1/%2").arg(stream->avg_frame_rate.num).arg(stream->avg_frame_rate.den);
      } else if (property == "bit_rate") {
        streamProperties["bit_rate"] = stream->codecpar->bit_rate;
      } else if (property == "ch_layout") {
        char ch_layout[64];
        av_channel_layout_describe(&stream->codecpar->ch_layout, ch_layout, sizeof(ch_layout));
        streamProperties["ch_layout"] = QString(ch_layout);
      } else if (property == "codec_id") {
        streamProperties["codec_id"] = avcodec_get_name(stream->codecpar->codec_id);
      } else if (property == "codec_tag") {
        streamProperties["codec_tag"] = stream->codecpar->codec_tag;
      } else if (property == "codec_type") {
        streamProperties["codec_type"] = av_get_media_type_string(stream->codecpar->codec_type);
      } else if (property == "codecpar.sample_aspect_ratio") {
        streamProperties["codecpar.sample_aspect_ratio"] = QString("%1:%2")
                                                               .arg(stream->codecpar->sample_aspect_ratio.num)
                                                               .arg(stream->codecpar->sample_aspect_ratio.den);
      } else if (property == "display_aspect_ratio") {
        AVRational display_aspect_ratio;
        av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
                  stream->codecpar->width * stream->sample_aspect_ratio.num,
                  stream->codecpar->height * stream->sample_aspect_ratio.den, 1024 * 1024);
        streamProperties["display_aspect_ratio"] =
            QString("%1:%2").arg(display_aspect_ratio.num).arg(display_aspect_ratio.den);
      } else if (property == "duration") {
        streamProperties["duration"] = stream->duration;
      } else if (property == "format") {
        streamProperties["format"] = av_get_pix_fmt_name((AVPixelFormat)stream->codecpar->format);
      } else if (property == "frame_size") {
        streamProperties["frame_size"] = stream->codecpar->frame_size;
      } else if (property == "height") {
        streamProperties["height"] = stream->codecpar->height;
      } else if (property == "nb_channels") {
        streamProperties["nb_channels"] = stream->codecpar->ch_layout.nb_channels;
      } else if (property == "nb_frames") {
        streamProperties["nb_frames"] = stream->nb_frames;
      } else if (property == "r_frame_rate") {
        streamProperties["r_frame_rate"] = QString("%1/%2").arg(stream->r_frame_rate.num).arg(stream->r_frame_rate.den);
      } else if (property == "sample_aspect_ratio") {
        streamProperties["sample_aspect_ratio"] =
            QString("%1:%2").arg(stream->sample_aspect_ratio.num).arg(stream->sample_aspect_ratio.den);
      } else if (property == "sample_rate") {
        streamProperties["sample_rate"] = stream->codecpar->sample_rate;
      } else if (property == "start_time") {
        streamProperties["start_time"] = stream->start_time;
      } else if (property == "time_base") {
        streamProperties["time_base"] = QString("%1/%2").arg(stream->time_base.num).arg(stream->time_base.den);
      } else if (property == "width") {
        streamProperties["width"] = stream->codecpar->width;
      }
    }

    streamsProperties.push_back(streamProperties);
  }

  avformat_close_input(&formatContext);

  return streamsProperties;
}

void FFmpegWrapper::cutFile(const QString& filePath, const QString& startTime, const QString& endTime,
                            const QString& outputFilePath, const bool& isQuickCut,
                            const std::function<void()>& callback) {
  QStringList arguments{"-ss", startTime, "-to", endTime, "-i", filePath};

  if (isQuickCut) {
    arguments << "-c"
              << "copy";
  }

  arguments << outputFilePath;

  this->processHelpers->doBlockingProcess(this->ffmpegExecutablePath, arguments, callback);
}

void FFmpegWrapper::mergeFiles(const QStringList& filesPaths, const int& baseVideoIndex, const QString& outputFilePath,
                               const bool& isQuickMerge, const std::function<void()>& callback) {
  if (this->areFilesMergeable(filesPaths)) {
    this->mergeFilesWithDemuxer(filesPaths, outputFilePath, isQuickMerge, callback);
  } else {
    this->mergeFilesWithConcatFilter(filesPaths, baseVideoIndex, outputFilePath, callback);
  }
}

void FFmpegWrapper::mergeFilesWithConcatFilter(const QStringList& filesPaths, const int& baseVideoIndex,
                                               const QString& outputFilePath, const std::function<void()>& callback) {
  QVector<QMap<QString, QVariant>> fileStreamsProperties =
      this->getFileStreamsProperties(filesPaths[baseVideoIndex], FFmpegWrapper::MERGE_SENSITIVE_PROPERTIES);

  QStringList arguments;

  QString filterComplex = "";

  for (int i = 0; i < filesPaths.size(); ++i) {
    arguments << "-i" << filesPaths[i];
    filterComplex += "[" + QString::number(i) + ":v]";
    filterComplex += "scale=" + fileStreamsProperties[0]["width"].toString() + ":" +
                     fileStreamsProperties[0]["height"].toString() + ",";
    filterComplex += "setsar=" + fileStreamsProperties[0]["sample_aspect_ratio"].toString().replace(':', '/') + ",";
    filterComplex += "setdar=" + fileStreamsProperties[0]["display_aspect_ratio"].toString().replace(':', '/');
    filterComplex += "[vout" + QString::number(i) + "];";
  }

  for (int i = 0; i < filesPaths.size(); ++i) {
    filterComplex += "[vout" + QString::number(i) + "][" + QString::number(i) + ":a]";
  }

  filterComplex += "concat=n=" + QString::number(filesPaths.size()) + ":v=1:a=1[vout][aout]";

  if (!this->areFilesStreamsEqual(filesPaths, "sample_aspect_ratio,display_aspect_ratio")) {
    arguments << "-fps_mode"
              << "vfr";
  }

  arguments << "-filter_complex" << filterComplex << "-map"
            << "[vout]"
            << "-map"
            << "[aout]" << outputFilePath;

  this->processHelpers->doBlockingProcess(this->ffmpegExecutablePath, arguments, callback);
}

void FFmpegWrapper::mergeFilesWithDemuxer(const QStringList& filesPaths, const QString& outputFilePath,
                                          const bool& isQuickMerge, const std::function<void()>& callback) {
  QString demuxerListFileName = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);

  QStringList lines;
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

  this->processHelpers->doBlockingProcess(this->ffmpegExecutablePath, arguments, callback);

  fileHelpers->deleteFile(demuxerListFileName);
}

void FFmpegWrapper::appendMacOSExecutablePaths() {
  QString pathEnvironmentVariable = qgetenv("PATH");
  pathEnvironmentVariable.append(":/opt/homebrew/bin:/usr/local/bin:/usr/bin");
  setenv("PATH", pathEnvironmentVariable.toStdString().c_str(), true);
}
