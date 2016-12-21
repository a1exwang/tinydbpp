#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <QFileDialog>
#include <QMessageBox>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow) {
  ui->setupUi(this);

  connect(ui->btnExec, SIGNAL(released()), this, SLOT(btnExecClicked()));
  connect(ui->btnChangeDB, SIGNAL(released()), this, SLOT(btnChangeDB()));
}

void MainWindow::btnExecClicked() {
  string txt = ui->textSQL->toPlainText().toStdString();
  // TODO exec sql txt
  // get output as json
  int rowCount = 10;
  int colCount = 5;

  ui->tableResult->setRowCount(rowCount);
  ui->tableResult->setColumnCount(colCount);
  for (int line = 0; line < rowCount; ++line) {
    for (int col = 0; col < colCount; ++col) {
      string itemStr = "data";
      ui->tableResult->setItem(line, col, new QTableWidgetItem(itemStr.c_str()));
    }
  }

}

void MainWindow::btnChangeDB() {
  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::FileMode::DirectoryOnly);
  fileDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
  fileDialog.setOption(QFileDialog::Option::ShowDirsOnly);
  fileDialog.exec();
  auto files = fileDialog.selectedFiles();
  auto filePath = files.first().toStdString();

  // TODO execute change DB
}

MainWindow::~MainWindow() {
  delete ui;
}
