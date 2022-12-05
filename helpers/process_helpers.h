#ifndef PROCESSHELPERS_H
#define PROCESSHELPERS_H

#include <QString>

class ProcessHelpers {
 public:
  ProcessHelpers();

  const QString doBlockingProcess(const QString& executablePath, const QStringList& arguments);
};

#endif  // PROCESSHELPERS_H
