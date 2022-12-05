#include "process_helpers.h"

#include <QCoreApplication>
#include <QProcess>

ProcessHelpers::ProcessHelpers() {}

const QString ProcessHelpers::doBlockingProcess(const QString& executablePath, const QStringList& arguments) {
  QProcess process;

  process.start(executablePath, arguments);

  process.waitForStarted();
  do {
    QCoreApplication::processEvents();
  } while (!process.waitForFinished(100));

  QString processOutputs = process.readAllStandardOutput();

  process.close();

  return processOutputs;
}
