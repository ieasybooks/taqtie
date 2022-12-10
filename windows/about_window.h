#ifndef ABOUT_WINDOW_H
#define ABOUT_WINDOW_H

#include <QWidget>

namespace Ui {
class AboutWindow;
}

class AboutWindow : public QWidget {
  Q_OBJECT

 public:
  explicit AboutWindow(QWidget *parent = nullptr);
  ~AboutWindow();

 private:
  Ui::AboutWindow *ui;
};

#endif  // ABOUT_WINDOW_H
