#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <QFileDialog>
#include <QMessageBox>
#include "../../third_party/json/src/json.hpp"

using json = nlohmann::json;

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

  stringstream ss;
  ss << "{\"result\": {\"c0\": [\"a\"], \"c1\": [\"b\"]}}";
  json j;
  ss >> j;
  json cols = j["result"];

  int colCount = 0;
  int lineCount = 0;
  for (auto it = cols.begin(); it != cols.end(); ++it) {
    colCount++;
    for (auto j = it.value().begin(); j != it.value().end(); ++j) {
      lineCount++;
    }
  }

  ui->tableResult->setRowCount(lineCount);
  ui->tableResult->setColumnCount(colCount);

  int colNo = 0;
  for (auto it = cols.begin(); it != cols.end(); ++it) {
    string colName = it.key();

    ui->tableResult->setItem(0, colNo, new QTableWidgetItem(colName.c_str()));

    int lineNo = 1;
    for (auto j = it.value().begin(); j != it.value().end(); ++j) {
      string itemStr = *j;
      ui->tableResult->setItem(lineNo, colNo, new QTableWidgetItem(itemStr.c_str()));
      lineNo++;
    }
    colNo++;
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
