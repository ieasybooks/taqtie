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
  this->timeHelpers = new TimeHelpers();

#ifdef Q_OS_MACOS
  this->appendMacOSExecutablePaths();
#endif
}

FFmpegWrapper::~FFmpegWrapper() {
  delete fileHelpers;
  delete processHelpers;
}

void FFmpegWrapper::logPacket(const AVFormatContext* avFormatContext, const AVPacket* avPacket, const QString tag) {
  AVRational* timeBase = &avFormatContext->streams[avPacket->stream_index]->time_base;

  qDebug() << QString("%1: pts:%2 pts_time:%3 dts:%4 dts_time:%5 duration:%6 duration_time:%7 stream_index:%8")
                  .arg(tag)
                  .arg(av_ts2str(avPacket->pts), av_ts2timestr(avPacket->pts, timeBase))
                  .arg(av_ts2str(avPacket->dts))
                  .arg(av_ts2timestr(avPacket->dts, timeBase), av_ts2str(avPacket->duration))
                  .arg(av_ts2timestr(avPacket->duration, timeBase))
                  .arg(avPacket->stream_index);
}

bool FFmpegWrapper::areFilesMergeable(const QStringList& filesPaths) {
  assert(filesPaths.size() >= 2);
  return this->areFilesStreamsEqual(filesPaths, FFmpegWrapper::MERGE_SENSITIVE_PROPERTIES);
}

bool FFmpegWrapper::areFilesStreamsEqual(const QStringList& filesPaths, const QString& properties) {
  return this->areFilesStreamsEqual(filesPaths, properties.split(','));
}

bool FFmpegWrapper::areFilesStreamsEqual(const QStringList& filesPaths, const QStringList& properties) {
  QVector<QMap<QString, QVariant>> firstFileStreamsProperties =
      this->getFileStreamsProperties(filesPaths[0], properties);

  for (const QString& filePath : filesPaths) {
    QVector<QMap<QString, QVariant>> otherFileStreamsProperties = this->getFileStreamsProperties(filePath, properties);

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
                                                                         const QString& requiredProperties) {
  return this->getFileStreamsProperties(filePath, requiredProperties.split(','));
}

QVector<QMap<QString, QVariant>> FFmpegWrapper::getFileStreamsProperties(const QString& filePath,
                                                                         const QStringList& requiredProperties) {
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

bool FFmpegWrapper::cutFile(const QString& inputFilePath, const QString& startTime, const QString& endTime,
                            const QString& outputFilePath) {
  return this->cutFile(inputFilePath, this->timeHelpers->timeFormatToSeconds(startTime),
                       this->timeHelpers->timeFormatToSeconds(endTime), outputFilePath);
}

bool FFmpegWrapper::cutFile(const QString& inputFilePath, const long long& startSeconds, const long long& endSeconds,
                            const QString& outputFilePath) {
  int operationResult;

  AVPacket* avPacket = NULL;
  AVFormatContext* avInputFormatContext = NULL;
  AVFormatContext* avOutputFormatContext = NULL;

  avPacket = av_packet_alloc();
  if (!avPacket) {
    qCritical("Failed to allocate AVPacket.");
    return false;
  }

  try {
    operationResult = avformat_open_input(&avInputFormatContext, inputFilePath.toStdString().c_str(), 0, 0);
    if (operationResult < 0) {
      throw std::runtime_error(QString("Failed to open the input file '%1'.").arg(inputFilePath).toStdString().c_str());
    }

    operationResult = avformat_find_stream_info(avInputFormatContext, 0);
    if (operationResult < 0) {
      throw std::runtime_error(QString("Failed to retrieve the input stream information.").toStdString().c_str());
    }

    avformat_alloc_output_context2(&avOutputFormatContext, NULL, NULL, outputFilePath.toStdString().c_str());
    if (!avOutputFormatContext) {
      operationResult = AVERROR_UNKNOWN;
      throw std::runtime_error(QString("Failed to create the output context.").toStdString().c_str());
    }

    int streamIndex = 0;
    int streamMapping[avInputFormatContext->nb_streams];
    int streamRescaledStartSeconds[avInputFormatContext->nb_streams];
    int streamRescaledEndSeconds[avInputFormatContext->nb_streams];

    // Copy streams from the input file to the output file.
    for (int i = 0; i < avInputFormatContext->nb_streams; i++) {
      AVStream* outStream;
      AVStream* inStream = avInputFormatContext->streams[i];

      streamRescaledStartSeconds[i] = av_rescale_q(startSeconds * AV_TIME_BASE, AV_TIME_BASE_Q, inStream->time_base);
      streamRescaledEndSeconds[i] = av_rescale_q(endSeconds * AV_TIME_BASE, AV_TIME_BASE_Q, inStream->time_base);

      if (inStream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
          inStream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
          inStream->codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
        streamMapping[i] = -1;
        continue;
      }

      streamMapping[i] = streamIndex++;

      outStream = avformat_new_stream(avOutputFormatContext, NULL);
      if (!outStream) {
        operationResult = AVERROR_UNKNOWN;
        throw std::runtime_error(QString("Failed to allocate the output stream.").toStdString().c_str());
      }

      operationResult = avcodec_parameters_copy(outStream->codecpar, inStream->codecpar);
      if (operationResult < 0) {
        throw std::runtime_error(
            QString("Failed to copy codec parameters from input stream to output stream.").toStdString().c_str());
      }
      outStream->codecpar->codec_tag = 0;
    }

    if (!(avOutputFormatContext->oformat->flags & AVFMT_NOFILE)) {
      operationResult = avio_open(&avOutputFormatContext->pb, outputFilePath.toStdString().c_str(), AVIO_FLAG_WRITE);
      if (operationResult < 0) {
        throw std::runtime_error(
            QString("Failed to open the output file '%1'.").arg(outputFilePath).toStdString().c_str());
      }
    }

    operationResult = avformat_write_header(avOutputFormatContext, NULL);
    if (operationResult < 0) {
      throw std::runtime_error(QString("Error occurred when opening output file.").toStdString().c_str());
    }

    operationResult = avformat_seek_file(avInputFormatContext, -1, INT64_MIN, startSeconds * AV_TIME_BASE,
                                         startSeconds * AV_TIME_BASE, 0);
    if (operationResult < 0) {
      throw std::runtime_error(
          QString("Failed to seek the input file to the targeted start position.").toStdString().c_str());
    }

    while (true) {
      QCoreApplication::processEvents();

      operationResult = av_read_frame(avInputFormatContext, avPacket);
      if (operationResult < 0) break;

      // Skip packets from unknown streams and packets after the end cut position.
      if (avPacket->stream_index >= avInputFormatContext->nb_streams || streamMapping[avPacket->stream_index] < 0 ||
          avPacket->pts > streamRescaledEndSeconds[avPacket->stream_index]) {
        av_packet_unref(avPacket);
        continue;
      }

      avPacket->stream_index = streamMapping[avPacket->stream_index];
      // logPacket(avInputFormatContext, avPacket, "in");

      // Shift the packet to its new position by subtracting the rescaled start seconds.
      avPacket->pts -= streamRescaledStartSeconds[avPacket->stream_index];
      avPacket->dts -= streamRescaledStartSeconds[avPacket->stream_index];

      av_packet_rescale_ts(avPacket, avInputFormatContext->streams[avPacket->stream_index]->time_base,
                           avOutputFormatContext->streams[avPacket->stream_index]->time_base);
      avPacket->pos = -1;
      // logPacket(avOutputFormatContext, avPacket, "out");

      operationResult = av_interleaved_write_frame(avOutputFormatContext, avPacket);
      if (operationResult < 0) {
        throw std::runtime_error(QString("Failed to mux the packet.").toStdString().c_str());
      }
    }

    av_write_trailer(avOutputFormatContext);
  } catch (std::runtime_error e) {
    qCritical("%s", e.what());
  }

  av_packet_free(&avPacket);

  avformat_close_input(&avInputFormatContext);

  if (avOutputFormatContext && !(avOutputFormatContext->oformat->flags & AVFMT_NOFILE))
    avio_closep(&avOutputFormatContext->pb);
  avformat_free_context(avOutputFormatContext);

  if (operationResult < 0 && operationResult != AVERROR_EOF) {
    qCritical("%s", QString("Error occurred: %1.").arg(av_err2str(operationResult)).toStdString().c_str());
    return false;
  }

  return true;
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
