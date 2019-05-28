#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>

#include <private/qqmljsengine_p.h>
#include <private/qqmljslexer_p.h>
#include <private/qqmljsparser_p.h>
#include <private/qv4value_p.h>

#include "AstGenerator.h"
#include "parser.h"

using namespace std;

bool Foobar::debug = true;

void Foobar::setDebug(bool debug_) { Foobar::debug = debug_; }

int Foobar::InternalRun(const QString &code) {
  QQmlJS::Engine engine;
  QQmlJS::Lexer lexer(&engine);

  lexer.setCode(code, 1, true);
  QQmlJS::Parser parser(&engine);
  bool success = parser.parse();

  if (!success) {
    const auto diagnosticMessages = parser.diagnosticMessages();

    for (const QQmlJS::DiagnosticMessage &m : diagnosticMessages) {
      qWarning("%d: %s", m.loc.startLine, qPrintable(m.message));
    }

    return 1;
  }

  const bool debug = this->m_options.testFlag(Option::Debug);
  setDebug(debug);

  AstGenerator generator(&engine, 0);
  const json ast = generator(parser.ast());

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

Foobar::Foobar(Options options) { this->m_options = options; }

int Foobar::Run(QStringList args) {
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
    const QString code = QString::fromUtf8(file.readAll());

    returnValue = this->InternalRun(code);
  } else {
    const QString code = pathOrText;
    returnValue = this->InternalRun(code);
  }

  return returnValue;
}
