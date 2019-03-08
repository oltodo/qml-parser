#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>

#include <qmljs/qmljsmodelmanagerinterface.h>

#include "AstGenerator.h"
#include "parser.h"

using namespace QmlJS;
using namespace QmlJS::AST;
using namespace std;

bool Parser::debug = true;

void Parser::setDebug(bool debug_) { Parser::debug = debug_; }

int Parser::InternalRun(const QString &source) {
  QmlJS::Document::MutablePtr document =
      QmlJS::Document::create("", QmlJS::Dialect::Qml);
  document->setSource(source);
  document->parse();

  const bool debug = this->m_options.testFlag(Option::Debug);

  setDebug(debug);

  AstGenerator generator(document, 0);
  const json ast = generator(document->ast());

  if (debug) {
    ofstream myfile;
    myfile.open("sandbox/test.json");
    myfile << ast.dump(2);
    myfile.close();
  } else {
    cout << ast.dump(2);
  }

  return 0;
}

Parser::Parser(Options options) {
  this->m_options = options;
  new QmlJS::ModelManagerInterface();
}

int Parser::Run(QStringList args) {
  if (args.count() == 0) {
    QTextStream(stderr) << "Please provide a path or a QML text\n";
    return 1;
  }

  if (args.count() > 1) {
    QTextStream(stderr) << "Please provide only one path or one QML text\n";
    return 1;
  }

  int returnValue = 0;
  const QString pathOrText = args[0];

  QFileInfo fileInfo(pathOrText);

  if (fileInfo.isDir()) {
    QTextStream(stderr) << "qml-parser doesn't hangle path to directory\n";
    return 1;
  }

  if (fileInfo.isFile()) {
    QFile file(pathOrText);
    file.open(QFile::ReadOnly | QFile::Text);
    const QString source = QString::fromUtf8(file.readAll());

    returnValue = this->InternalRun(source);
  } else {
    const QString source = pathOrText;
    returnValue = this->InternalRun(source);
  }

  return returnValue;
}
