/*
  Copyright (c) 2015-2018, Jesper Hellesø Hansen
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

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>

#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/parser/qmljsastvisitor_p.h>
#include <qmljs/parser/qmljsengine_p.h>
#include <qmljs/qmljsdocument.h>
#include <qmljs/qmljsmodelmanagerinterface.h>
#include <qmljs/qmljsreformatter.h>

#include <nlohmann/json.hpp>

#include <tsl/ordered_map.h>

// #include <ast_generator.h>
#include "qmlfmt.h"

using namespace QmlJS;
using namespace QmlJS::AST;
using namespace std;

template <class Key, class T, class Ignore, class Allocator,
          class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
          class AllocatorPair = typename std::allocator_traits<
              Allocator>::template rebind_alloc<std::pair<Key, T>>,
          class ValueTypeContainer =
              std::vector<std::pair<Key, T>, AllocatorPair>>
using ordered_map =
    tsl::ordered_map<Key, T, Hash, KeyEqual, AllocatorPair, ValueTypeContainer>;

using json = nlohmann::basic_json<ordered_map>;

class Location {
public:
  int startColumn = 0;
  int startLine = 0;
  int startOffset = 0;
  int endColumn = 0;
  int endLine = 0;
  int endOffset = 0;
  int length = 0;

  Location() {}

  Location(int column, int line, int offset, int length) {
    set(column, line, offset, length);
  }

  Location(int startColumn, int startLine, int startOffset, int endColumn,
           int endLine, int endOffset)
      : startColumn(startColumn), startLine(startLine),
        startOffset(startOffset), endColumn(endColumn), endLine(endLine),
        endOffset(endOffset), length(endOffset - startOffset) {}

  Location(SourceLocation const &loc) {
    set(loc.startColumn, loc.startLine, loc.offset, loc.length);
  }

  void set(int column, int line, int offset, int length_) {
    startColumn = column;
    startLine = line;
    startOffset = offset;
    endColumn = column;
    endLine = line;
    endOffset = offset + length_;
    length = length_;
  }

  bool isValid() const { return length > 0; }

  operator json() { return toJson(); }

  Location operator+(int size) {
    return Location(startColumn, startLine, startOffset, endColumn + size,
                    endLine, endOffset + size);
  }

  Location mergeWith(const Location &loc) const {
    return Location(
        min(startColumn, loc.startColumn), min(startLine, loc.startLine),
        min(startOffset, loc.startOffset), max(endColumn, loc.endColumn),
        max(endLine, loc.endLine), max(endOffset, loc.endOffset));
  }

  json toJson() const {
    return json{
        {"start",
         {{"offset", startOffset},
          {"line", startLine},
          {"column", startColumn}}},
        {"end",
         {{"offset", endOffset}, {"line", endLine}, {"column", endColumn}}}};
  }
};

class AstGeneratorBase : protected Visitor {
public:
  explicit AstGeneratorBase(Document::Ptr doc, int level)
      : doc(doc), level(level) {}

protected:
  Document::Ptr doc;
  int level;

  template <typename T1, typename T2>
  void print(const T1 &str, const T2 &extra) {
    cout << string(level * 2, '-') << "> \033[1m" << str << "\033[0m";

    if (extra != "") {
      cout << " (" << extra << ")";
    }

    cout << "\n";
  }

  template <typename T1> void print(const T1 &str) { print(str, ""); }

  string toString(const QString &str) { return str.toUtf8().constData(); }

  string toString(const Location &loc) {
    return toString(doc->source().mid(loc.startOffset, loc.length));
  }

  string toString(const QStringRef &str) { return toString(str.toString()); }

  string toString(const Location &first, const Location &second) {
    const Location loc = mergeLocs(first, second);

    return toString(loc);
  }

  string toString(UiQualifiedId *node) {
    stringstream id;

    for (UiQualifiedId *it = node; it; it = it->next) {
      id << toString(it->identifierToken);
      if (it->next)
        id << ".";
    }

    return id.str();
  }

  json getLoc(const SourceLocation &loc) { return static_cast<Location>(loc); }
  json getLoc(const Location &loc) { return loc.toJson(); }

  template <typename T1, typename T2>
  json getLoc(const T1 &first, const T2 &second) {
    const Location loc = mergeLocs(first, second);

    return loc.toJson();
  }

  template <typename T, typename... Args>
  json getLoc(const T &first, Args &... args) {
    const Location loc = mergeLocs(first, args...);

    return loc.toJson();
  }

  Location mergeLocs(const SourceLocation &loc) { return Location(loc); }

  template <typename T1, typename T2>
  Location mergeLocs(const T1 &t1, const T2 &t2) {
    const Location first = Location(t1);
    const Location second = Location(t2);

    if (!first.isValid())
      return second;
    if (!second.isValid())
      return first;

    return first.mergeWith(second);
  }

  template <typename T1, typename T2, typename... Args>
  Location mergeLocs(const T1 &first, const T2 &second, Args... args) {
    if (!first.isValid() && !second.isValid())
      return mergeLocs(args...);
    if (!first.isValid())
      return mergeLocs(second, args...);
    if (!second.isValid())
      return mergeLocs(first, args...);

    const Location loc = mergeLocs(first, second);

    return mergeLocs(loc, args...);
  }

  struct lineColumn {
    int line = 1;
    int column = 1;
  };

  lineColumn getLineColumn(const int index) {
    const QString str = doc->source();

    lineColumn result;

    for (int i = 0; i < str.count(); ++i) {
      if (i == index) {
        break;
      }

      if (str.at(i) == '\n') {
        result.line++;
        result.column = 1;
        continue;
      }

      result.column++;
    }

    return result;
  }
};

class AstGeneratorJavascriptBlock : protected AstGeneratorBase {
  using AstGeneratorBase::AstGeneratorBase;

  json ast;
  Location loc;

public:
  json operator()(Node *node) {
    accept(node);

    ast["kind"] = "JavascriptBlock";
    ast["loc"] = getLoc(loc);
    ast["value"] = toString(loc);

    return ast;
  }

protected:
  void accept(Node *node) { Node::accept(node, this); }

  void postVisit(Node *node) override {
    loc =
        mergeLocs(loc, node->firstSourceLocation(), node->lastSourceLocation());
  }
};

class AstGenerator : protected AstGeneratorBase {
  using AstGeneratorBase::AstGeneratorBase;

  json ast;

public:
  json operator()(Node *node) {
    accept(node);
    return ast;
  }

protected:
  void accept(Node *node) { Node::accept(node, this); }

  void appendItems(const json &items) {
    if (!items.is_array())
      return;

    for (auto &item : items) {
      if (ast.is_object()) {
        if (!ast["children"].is_array())
          ast["children"] = json::array();

        ast["children"].push_back(item);
        continue;
      }

      if (ast.is_array()) {
        ast.push_back(item);
        continue;
      }

      ast = json::array();
      ast.push_back(item);
    }
  }

  bool visit(UiPragma *node) override {
    print("UiPragma", "not implemented");
    return false;
  }

  bool visit(UiImport *node) override {
    print("UiImport");

    ast = json::object();

    ast["kind"] = "Import";
    ast["loc"] =
        getLoc(node->importToken, node->fileNameToken, node->versionToken,
               node->asToken, node->importIdToken, node->semicolonToken);

    if (!node->fileName.isNull())
      ast["path"] = toString(node->fileName);
    else
      ast["identifier"] = toString(node->importUri);

    if (node->versionToken.isValid())
      ast["version"] = toString(node->versionToken);

    if (!node->importId.isNull())
      ast["as"] = toString(node->importIdToken);

    return false;
  }

  bool visit(UiObjectDefinition *node) override {
    print("UiObjectDefinition", toString(node->qualifiedTypeNameId));

    ast["kind"] = "ObjectDefinition";
    ast["identifier"] = toString(node->qualifiedTypeNameId);
    ast["children"] = json::array();

    if (node->initializer) {
      AstGenerator gen(doc, level + 1);
      const json item = gen(node->initializer);
      ast["children"] = item;
    }

    return false;
  }

  bool visit(UiObjectInitializer *node) override {
    print("UiObjectInitializer");

    AstGenerator gen(doc, level + 1);
    appendItems(gen(node->members));

    // json items;

    // for (UiObjectMemberList *it = node->members; it; it = it->next) {
    //   AstGenerator gen(doc, level + 1);
    //   const json item = gen(it);
    //   items.push_back(item);
    // }

    // appendItems(items);

    return false;
  }

  bool visit(UiParameterList *list) override {
    print("UiParameterList");

    json items;

    for (UiParameterList *it = list; it; it = it->next) {
      json item;

      item["type"] = toString(it->propertyTypeToken);
      item["identifier"] = toString(it->identifierToken);

      items.push_back(item);
    }

    ast = items;

    return false;
  }

  bool visit(UiPublicMember *node) override {
    print("UiPublicMember", toString(node->identifierToken));

    json item;

    if (node->type == UiPublicMember::Property) {
      item["kind"] = "Property";

      item["loc"] =
          getLoc(node->defaultToken, node->readonlyToken, node->propertyToken,
                 node->typeModifierToken, node->typeToken,
                 node->identifierToken, node->colonToken, node->semicolonToken);

      if (node->isDefaultMember)
        item["default"] = true;
      else if (node->isReadonlyMember)
        item["readonly"] = true;

      if (!node->typeModifier.isNull())
        item["typeModifier"] = toString(node->typeModifierToken);

      item["type"] = toString(node->typeToken);

      if (node->statement) {
        item["identifier"] = toString(node->identifierToken);

        AstGeneratorJavascriptBlock gen(doc, level + 1);
        item["value"] = gen(node->statement);
      } else if (node->binding) {
        item["identifier"] = toString(node->identifierToken);

        AstGenerator gen(doc, level + 1);
        json value = gen(node->binding);
        item["value"].push_back(value);
      } else
        item["identifier"] = toString(node->identifierToken);
    } else {
      item["kind"] = "Signal";
      item["identifier"] = toString(node->identifierToken);

      Location loc;

      if (node->parameters) {
        AstGenerator gen(doc, level + 1);
        item["parameters"] = gen(node->parameters);

        // Where is the closing parenthesis?!
        const Location lastLoc =
            static_cast<Location>(node->parameters->lastSourceLocation());

        const int closingParenthesisOffset =
            doc->source().indexOf(')', lastLoc.endOffset);

        lineColumn position = getLineColumn(closingParenthesisOffset);

        const SourceLocation parenthesisLocation = SourceLocation(
            closingParenthesisOffset, 1, position.line, position.column);

        loc = mergeLocs(node->firstSourceLocation(), parenthesisLocation);
      } else {
        loc = mergeLocs(node->firstSourceLocation(), node->identifierToken);
      }

      item["loc"] = getLoc(loc);
      item["body"] = toString(loc);
    }

    ast = item;

    return false;
  }

  bool visit(UiObjectBinding *node) override {
    print("UiObjectBinding");

    json item;

    if (node->hasOnToken) {
      cout << "ok";
      item["kind"] = "ObjectDefinition";
      item["identifier"] = toString(node->qualifiedTypeNameId);
      item["on"] = toString(node->qualifiedId);
      item["loc"] =
          getLoc(node->firstSourceLocation(), node->lastSourceLocation());
      // item["on"] = toString(node->qualifiedTypeNameId);
    } else {
      // item["identifier"] = toString(node->qualifiedId);
    }

    AstGenerator gen(doc, level + 1);
    item["children"] = gen(node->initializer);

    // if (node->hasOnToken) {
    //   accept(node->qualifiedTypeNameId);
    //   out(" on ");
    //   accept(node->qualifiedId);
    // } else {
    //   accept(node->qualifiedId);
    //   out(": ", node->colonToken);
    //   accept(node->qualifiedTypeNameId);
    // }
    // out(" ");
    // accept(node->initializer);

    ast = item;

    return false;
  }

  bool visit(UiScriptBinding *node) override {
    print("UiScriptBinding");

    json item;
    item["kind"] = "Property";
    item["identifier"] = toString(node->qualifiedId);

    AstGeneratorJavascriptBlock gen(doc, level + 1);
    item["value"] = gen(node->statement);

    ast = item;

    return false;
  }

  bool visit(UiArrayBinding *node) override {
    print("UiArrayBinding");

    json item;
    item["kind"] = "ArrayBinding";
    item["identifier"] = toString(node->qualifiedId);

    AstGenerator gen(doc, level + 1);
    item["children"] = gen(node->members);

    ast = item;

    return false;
  }
  bool visit(UiArrayMemberList *node) override {
    print("UiArrayMemberList");

    json items;

    for (UiArrayMemberList *it = node; it; it = it->next) {
      AstGenerator gen(doc, level + 1);
      const json item = gen(it->member);
      items.push_back(item);
    }

    ast = items;

    return false;
  }
  bool visit(ThisExpression *node) override {
    print("ThisExpression", "not implemented");
    return false;
  }
  bool visit(NullExpression *node) override {
    print("NullExpression", "not implemented");
    return false;
  }
  bool visit(TrueLiteral *node) override {
    print("TrueLiteral", "not implemented");
    return false;
  }
  bool visit(FalseLiteral *node) override {
    print("FalseLiteral", "not implemented");
    return false;
  }
  bool visit(IdentifierExpression *node) override {
    print("IdentifierExpression", "not implemented");
    return false;
  }
  bool visit(StringLiteral *node) override {
    print("StringLiteral", "not implemented");
    return false;
  }
  bool visit(NumericLiteral *node) override {
    print("NumericLiteral", "not implemented");
    return true;
  }
  bool visit(RegExpLiteral *node) override {
    print("RegExpLiteral", "not implemented");
    return false;
  }
  bool visit(ArrayLiteral *node) override {
    print("ArrayLiteral", "not implemented");
    return false;
  }
  bool visit(ObjectLiteral *node) override {
    print("ObjectLiteral", "not implemented");
    return false;
  }
  bool visit(ElementList *node) override {
    print("ElementList", "not implemented");
    return false;
  }
  bool visit(PropertyAssignmentList *node) override {
    print("PropertyAssignmentList", "not implemented");
    return false;
  }
  bool visit(NestedExpression *node) override {
    print("NestedExpression", "not implemented");
    return false;
  }
  bool visit(IdentifierPropertyName *node) override {
    print("IdentifierPropertyName", "not implemented");
    return false;
  }
  bool visit(StringLiteralPropertyName *node) override {
    print("StringLiteralPropertyName", "not implemented");
    return false;
  }
  bool visit(NumericLiteralPropertyName *node) override {
    print("NumericLiteralPropertyName", "not implemented");
    return false;
  }
  bool visit(ArrayMemberExpression *node) override {
    print("ArrayMemberExpression", "not implemented");
    return false;
  }
  bool visit(FieldMemberExpression *node) override {
    print("FieldMemberExpression", "not implemented");
    return false;
  }
  bool visit(NewMemberExpression *node) override {
    print("NewMemberExpression", "not implemented");
    return false;
  }
  bool visit(NewExpression *node) override {
    print("NewExpression", "not implemented");
    return false;
  }
  bool visit(CallExpression *node) override {
    print("CallExpression", "not implemented");
    return false;
  }
  bool visit(PostIncrementExpression *node) override {
    print("PostIncrementExpression", "not implemented");
    return false;
  }
  bool visit(PostDecrementExpression *node) override {
    print("PostDecrementExpression", "not implemented");
    return false;
  }
  bool visit(PreIncrementExpression *node) override {
    print("PreIncrementExpression", "not implemented");
    return false;
  }
  bool visit(PreDecrementExpression *node) override {
    print("PreDecrementExpression", "not implemented");
    return false;
  }
  bool visit(DeleteExpression *node) override {
    print("DeleteExpression", "not implemented");
    return false;
  }
  bool visit(VoidExpression *node) override {
    print("VoidExpression", "not implemented");
    return false;
  }
  bool visit(TypeOfExpression *node) override {
    print("TypeOfExpression", "not implemented");
    return false;
  }
  bool visit(UnaryPlusExpression *node) override {
    print("UnaryPlusExpression", "not implemented");
    return false;
  }
  bool visit(UnaryMinusExpression *node) override {
    print("UnaryMinusExpression", "not implemented");
    return false;
  }
  bool visit(TildeExpression *node) override {
    print("TildeExpression", "not implemented");
    return false;
  }
  bool visit(NotExpression *node) override {
    print("NotExpression", "not implemented");
    return false;
  }
  bool visit(BinaryExpression *node) override {
    print("BinaryExpression", "not implemented");
    return false;
  }
  bool visit(ConditionalExpression *node) override {
    print("ConditionalExpression", "not implemented");
    return false;
  }
  bool visit(Block *node) override {
    print("Block", "not implemented");
    return false;
  }
  bool visit(VariableStatement *node) override {
    print("VariableStatement", "not implemented");
    return false;
  }
  bool visit(VariableDeclaration *node) override {
    print("VariableDeclaration", "not implemented");
    return false;
  }
  bool visit(EmptyStatement *node) override {
    print("EmptyStatement", "not implemented");
    return false;
  }
  bool visit(IfStatement *node) override {
    print("IfStatement", "not implemented");
    return false;
  }
  bool visit(DoWhileStatement *node) override {
    print("DoWhileStatement", "not implemented");
    return false;
  }
  bool visit(WhileStatement *node) override {
    print("WhileStatement", "not implemented");
    return false;
  }
  bool visit(ForStatement *node) override {
    print("ForStatement", "not implemented");
    return false;
  }
  bool visit(LocalForStatement *node) override {
    print("LocalForStatement", "not implemented");
    return false;
  }
  bool visit(ForEachStatement *node) override {
    print("ForEachStatement", "not implemented");
    return false;
  }
  bool visit(LocalForEachStatement *node) override {
    print("LocalForEachStatement", "not implemented");
    return false;
  }
  bool visit(ContinueStatement *node) override {
    print("ContinueStatement", "not implemented");
    return false;
  }
  bool visit(BreakStatement *node) override {
    print("BreakStatement", "not implemented");
    return false;
  }
  bool visit(ReturnStatement *node) override {
    print("ReturnStatement", "not implemented");
    return false;
  }
  bool visit(ThrowStatement *node) override {
    print("ThrowStatement", "not implemented");
    return false;
  }
  bool visit(WithStatement *node) override {
    print("WithStatement", "not implemented");
    return false;
  }
  bool visit(SwitchStatement *node) override {
    print("SwitchStatement", "not implemented");
    return false;
  }
  bool visit(CaseBlock *node) override {
    print("CaseBlock", "not implemented");
    return false;
  }
  bool visit(CaseClause *node) override {
    print("CaseClause", "not implemented");
    return false;
  }
  bool visit(DefaultClause *node) override {
    print("DefaultClause", "not implemented");
    return false;
  }
  bool visit(LabelledStatement *node) override {
    print("LabelledStatement", "not implemented");
    return false;
  }
  bool visit(TryStatement *node) override {
    print("TryStatement", "not implemented");
    return false;
  }
  bool visit(Catch *node) override {
    print("Catch", "not implemented");
    return false;
  }

  bool visit(Finally *node) override {
    print("Finally", "not implemented");
    return false;
  }

  bool visit(FunctionDeclaration *node) override {
    print("FunctionDeclaration");

    return visit(static_cast<FunctionExpression *>(node));
  }

  bool visit(FunctionExpression *node) override {
    print("FunctionExpression");

    json item;
    item["kind"] = "Function";
    item["identifier"] = toString(node->identifierToken);

    Location loc = mergeLocs(node->functionToken, node->rbraceToken);
    item["loc"] = getLoc(loc);
    item["body"] = toString(loc);

    // AstGenerator gen1(doc, level + 1);
    // item["arguments"] = gen1(node->formals);

    // AstGeneratorJavascriptBlock gen2(doc, level + 1);
    // item["body"] = gen2(node->body);

    ast = item;

    return false;
  }

  bool visit(UiHeaderItemList *node) override {
    print("UiHeaderItemList");

    ast = json::object();

    ast["kind"] = "Program";
    ast["loc"] = "undefined";
    ast["children"] = json::array();

    for (UiHeaderItemList *it = node; it; it = it->next) {
      AstGenerator gen(doc, level + 1);
      const json item = gen(it->headerItem);
      ast["children"].push_back(item);
    }

    return false;
  }

  bool visit(UiObjectMemberList *node) override {
    print("UiObjectMemberList");

    json items;

    for (UiObjectMemberList *it = node; it; it = it->next) {
      AstGenerator gen(doc, level + 1);
      const json item = gen(it->member);
      items.push_back(item);
    }

    appendItems(items);

    return false;
  }
  bool visit(UiQualifiedId *node) override {
    print("UiQualifiedId", "not implemented");
    return false;
  }
  bool visit(UiQualifiedPragmaId *node) override {
    print("UiQualifiedPragmaId", "not implemented");
    return false;
  }
  bool visit(Elision *node) override {
    print("Elision", "not implemented");
    return false;
  }
  bool visit(ArgumentList *node) override {
    print("ArgumentList", "not implemented");
    return false;
  }
  bool visit(StatementList *node) override {
    print("StatementList", "not implemented");
    return false;
  }
  bool visit(SourceElements *node) override {
    print("SourceElements", "not implemented");
    return false;
  }
  bool visit(VariableDeclarationList *node) override {
    print("VariableDeclarationList", "not implemented");
    return false;
  }
  bool visit(CaseClauses *node) override {
    print("CaseClauses", "not implemented");
    return false;
  }
  bool visit(FormalParameterList *node) override {
    print("FormalParameterList", "not implemented");
    return false;
  }
};

int QmlFmt::InternalRun(QIODevice &input, const QString &path) {
  QTextStream qstdout(stdout);
  QTextStream qstderr(stderr);
  const QString source = QString::fromUtf8(input.readAll());
  const QmlJS::Dialect dialect =
      QmlJS::ModelManagerInterface::guessLanguageOfFile(path);

  QmlJS::Document::MutablePtr document = QmlJS::Document::create(path, dialect);
  document->setSource(source);
  document->parse();

  AstGenerator generator(document, 0);
  const json ast = generator(document->ast());

  ofstream myfile;
  myfile.open("sandbox/test.json");
  myfile << ast.dump(2);
  myfile.close();
  // cout << ast.dump(2);

  // if (!document->diagnosticMessages().isEmpty())
  // {
  //     if (this->m_options.testFlag(Option::PrintError))
  //     {
  //         for (const QmlJS::DiagnosticMessage &msg :
  //         document->diagnosticMessages())
  //         {
  //             qstderr << (msg.isError() ? "Error:" : "Warning:");

  //             qstderr << msg.loc.startLine << ':' << msg.loc.startColumn <<
  //             ':';

  //             qstderr << ' ' << msg.message << "\n";
  //         }
  //     }
  //     return 1;
  // }

  // const QString reformatted = QmlJS::reformat(document, 2, 2);
  // if (source == reformatted)
  //     return 0;

  // if (this->m_options.testFlag(Option::ListFileName))
  // {
  //     // List filename
  //     qstdout << path << "\n";
  // }
  // else if (this->m_options.testFlag(Option::PrintDiff))
  // {
  //     // Create and print diff
  //     diff_match_patch differ;
  //     const QList<Patch> patches = differ.patch_make(source, reformatted);
  //     qstdout << differ.patch_toText(patches);
  // }
  // else
  // {
  //     // Print reformatted file to stdout/original file
  //     QFile outFile;
  //     if (this->m_options.testFlag(Option::OverwriteFile))
  //     {
  //         outFile.setFileName(path);
  //         outFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
  //     }
  //     else
  //     {
  //         outFile.open(stdout, QFile::WriteOnly | QFile::Text);
  //     }

  //     const QByteArray bytes = reformatted.toUtf8();
  //     outFile.write(bytes);
  // }

  return 0;
}

QmlFmt::QmlFmt(Options options) {
  this->m_options = options;
  new QmlJS::ModelManagerInterface();
}

int QmlFmt::Run() {
  QFile file;
  file.open(stdin, QFile::ReadOnly | QFile::Text);
  return this->InternalRun(file, "stdin.qml");
}

int QmlFmt::Run(QStringList paths) {
  if (paths.count() == 0) {
    return Run();
  }

  int returnValue = 0;
  for (const QString &fileOrDir : paths) {
    QFileInfo fileInfo(fileOrDir);
    if (fileInfo.isFile()) {
      QFile file(fileOrDir);
      file.open(QFile::ReadOnly | QFile::Text);
      returnValue |= this->InternalRun(file, fileOrDir);
    } else if (fileInfo.isDir()) {
      QDirIterator iter(fileOrDir, QStringList{"*.qml"}, QDir::Filter::Files,
                        QDirIterator::IteratorFlag::Subdirectories);

      while (iter.hasNext()) {
        QFile file(iter.next());
        file.open(QFile::ReadOnly | QFile::Text);
        returnValue |= this->InternalRun(file, file.fileName());
      }
    } else {
      QTextStream(stderr) << "Path is not valid file or directory: "
                          << fileOrDir << "\n";
      returnValue |= 1;
    }
  }

  return returnValue;
}
