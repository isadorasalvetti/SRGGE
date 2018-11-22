// Author: Marc Comino 2018
#include "./main_window.h"

#include <QFileDialog>
#include <QMessageBox>
#include "./ui_main_window.h"

namespace gui {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::show() { QMainWindow::show(); }

void MainWindow::on_actionQuit_triggered() { close(); }

void MainWindow::on_actionLoad_triggered() {

  QString filename = "../Models/PLY/bunny.ply";
  QString filename2 = "../Models/PLY/Armadillo.ply";

    if (!ui->glwidget->LoadModel(filename, filename2))
      QMessageBox::warning(this, tr("Error"),
                           tr("The file could not be opened"));

  /*
  QString filename;

  filename = QFileDialog::getOpenFileName(this, tr("Load model"), "./",
                                          tr("PLY Files ( *.ply )"));
  if (!filename.isNull()) {
    if (!ui->glwidget->LoadModel(filename, filename))
      QMessageBox::warning(this, tr("Error"),
                           tr("The file could not be opened"));
  }
  */
}

}  //  namespace gui
