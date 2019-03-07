/*
Copyright (c) 2015-2018, Jesper Helles� Hansen
jesperhh@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the <organization> nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QtCore>

#include "parser.h"

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("qml-parser");

#ifdef QML_PARSER_VERSION
  QCoreApplication::setApplicationVersion(QML_PARSER_VERSION);
#endif // QML_PARSER_VERSION

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "qml-parser generates AST document from QML files.");

  QCommandLineOption debugOption(
      "d", "Do not print reformatted sources to standard output. "
           "If a file\'s formatting is different than qmlfmt\'s, print diffs "
           "to standard output.");

  // QCommandLineOption errorOption("e", "Print all errors.");

  // QCommandLineOption listOption(
  //     "l",
  //     "Do not print reformatted sources to standard output. "
  //     "If a file\'s formatting is different from qmlfmt\'s, print its name "
  //     "to standard output.");

  // QCommandLineOption overwriteOption(
  //     "w", "Do not print reformatted sources to standard output. "
  //          "If a file\'s formatting is different from qmlfmt\'s, overwrite it
  //          " "with qmlfmt\'s version.");

  QMap<Parser::Option, QCommandLineOption> optionMap = {
      {Parser::Option::Debug, debugOption}
      // {Parser::Option::ListFileName, listOption},
      // {Parser::Option::PrintError, errorOption},
      // {Parser::Option::OverwriteFile, overwriteOption}
  };

  // set up options
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addOptions(optionMap.values());
  parser.addPositionalArgument("path",
                               "file or directory to process. If not set, "
                               "qmlfmt will process the standard input.");

  // process command line arguments
  parser.process(app);

  // validate arguments
  // if ((parser.isSet(overwriteOption) || parser.isSet(listOption)) &&
  //     parser.positionalArguments().count() == 0) {
  //   QTextStream(stderr) << "Cannot combine -" <<
  //   overwriteOption.names().first()
  //                       << " and -" << listOption.names().first()
  //                       << " with standard input\n";
  //   return 1;
  // } else if (parser.isSet(diffOption) + parser.isSet(overwriteOption) +
  //                parser.isSet(listOption) >
  //            1) {
  //   QTextStream(stderr) << "-" << diffOption.names().first() << ", -"
  //                       << overwriteOption.names().first() << " and -"
  //                       << listOption.names().first()
  //                       << " are mutually exclusive\n";
  //   return 1;
  // }

  Parser::Options options;
  for (auto kvp = optionMap.constKeyValueBegin();
       kvp != optionMap.constKeyValueEnd(); ++kvp) {
    if (parser.isSet((*kvp).second))
      options |= (*kvp).first;
  }

  Parser qmlFmt(options);
  return qmlFmt.Run(parser.positionalArguments());
}
