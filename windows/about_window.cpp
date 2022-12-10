#include "about_window.h"

#include "ui_about_window.h"

AboutWindow::AboutWindow(QWidget *parent) : QWidget(parent), ui(new Ui::AboutWindow) { ui->setupUi(this); }

AboutWindow::~AboutWindow() { delete ui; }
