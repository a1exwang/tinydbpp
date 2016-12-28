#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <QFileDialog>
#include <QMessageBox>
#include "../../third_party/json/src/json.hpp"
#include <Parser/Lexer.h>
#include <Parser/AST/Nodes.h>
#include <Parser/Parser.tab.hpp>
#include <Parser/ParsingError.h>
#include <boost/assert.hpp>
#include <iostream>
#include <sstream>
#include <RecordManage/TableManager.h>


using json = nlohmann::json;
using namespace std;
using namespace tinydbpp;

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
  stringstream ssin, ssout;
  Lexer lexer(ssin, ssout);
  shared_ptr<ast::Node> node;
  Parser parser(lexer, node);
  ssin << txt << endl;
  parser.parse();
  auto stmts = dynamic_pointer_cast<ast::Statements>(node);
  auto ss = stmts->get();
  for (auto &s: ss) {
    json j = s->exec();
    json cols = j["result"];
    if (cols.is_null()) {

    }
    else if (s->getType() == tinydbpp::ast::Statement::Type::DesribeTable) {
      int colCount = 0;
      int lineCount = 0;
      for (auto it = cols.begin(); it != cols.end(); ++it) {
        colCount++;
        for (auto innerIt = it.value().begin(); innerIt != it.value().end(); ++innerIt) {
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
        for (auto innerIt = it.value().begin(); innerIt != it.value().end(); ++innerIt) {
          string itemStr = (*innerIt).dump();
          ui->tableResult->setItem(lineNo, colNo, new QTableWidgetItem(itemStr.c_str()));
          lineNo++;
        }
        colNo++;
      }
    }
    else if (cols.is_string()) {
      ui->tableResult->setRowCount(1);
      ui->tableResult->setColumnCount(1);

      ui->tableResult->setItem(0, 0, new QTableWidgetItem(cols.dump().c_str()));
    }
    else if (cols.is_object()) {
      int colCount = 0;
      int lineCount = 0;
      for (auto it = cols.begin(); it != cols.end(); ++it) {
        colCount++;
        auto v = it.value();
        for (auto innerIt = v.begin(); innerIt != v.end(); ++innerIt) {
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
        for (auto innerIt = it.value().begin(); innerIt != it.value().end(); ++innerIt) {
          if (innerIt->is_string()) {
            string itemStr = *innerIt;
            ui->tableResult->setItem(lineNo, colNo, new QTableWidgetItem(itemStr.c_str()));
          }
          else if (innerIt->is_number()) {
            int itemInt = *innerIt;
            stringstream ss;
            ss << itemInt;
            ui->tableResult->setItem(lineNo, colNo, new QTableWidgetItem(ss.str().c_str()));

          }
          lineNo++;
        }
        colNo++;
      }
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
