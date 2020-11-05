#pragma once

#ifndef AST_GENERATOR_BASE_H
#define AST_GENERATOR_BASE_H

#include <iostream>

#include <private/qqmlengine_p.h>
#include <private/qqmljsast_p.h>

#include "Location.h"
#include "parser.h"

using namespace std;
using namespace QQmlJS::AST;

struct lineColumn {
  int line = 1;
  int column = 0;
};

class AstGeneratorBase : protected Visitor {
public:
  AstGeneratorBase(QQmlJS::Engine *engine, int level);

protected:
  QQmlJS::Engine *engine;
  int level;

  template <typename T1, typename T2>
  void print(const T1 &str, const T2 &extra) {
    if (!Foobar::debug) {
      return;
    }

    cout << string(level * 2, '-') << "> \033[1m" << str << "\033[0m";

    if (extra != "") {
      cout << " (" << extra << ")";
    }

    cout << "\n";
  }

  template <typename T1> void print(const T1 &str) { print(str, ""); }

  string toString(const QString &str);
  string toString(const Location &loc);
  string toString(const QStringRef &str);
  string toString(const Location &first, const Location &second);
  string toString(UiQualifiedId *node);

  json getLoc(const SourceLocation &loc);
  json getLoc(const Location &loc);

  Location mergeLocs(const SourceLocation &loc);

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

  lineColumn getLineColumn(const int index);

  QChar getCharAt(const int index);
  int getNextPrintableCharIndex(const int startFromIndex);

  bool visit(UiPragma *node) override;
  bool visit(UiImport *node) override;
  bool visit(UiObjectDefinition *node) override;
  bool visit(UiObjectInitializer *node) override;
  bool visit(UiParameterList *list) override;
  bool visit(UiPublicMember *node) override;
  bool visit(UiObjectBinding *node) override;
  bool visit(UiScriptBinding *node) override;
  bool visit(UiArrayBinding *node) override;
  bool visit(UiArrayMemberList *node) override;

  // bool visit(ThisExpression *node) override;
  // bool visit(NullExpression *node) override;
  // bool visit(TrueLiteral *node) override;
  // bool visit(FalseLiteral *node) override;
  // bool visit(IdentifierExpression *node) override;
  // bool visit(StringLiteral *node) override;
  // bool visit(NumericLiteral *node) override;
  // bool visit(RegExpLiteral *node) override;
  // bool visit(ArrayLiteral *node) override;
  // bool visit(ObjectLiteral *node) override;
  // bool visit(ElementList *node) override;
  // bool visit(PropertyAssignmentList *node) override;
  // bool visit(NestedExpression *node) override;
  // bool visit(IdentifierPropertyName *node) override;
  // bool visit(StringLiteralPropertyName *node) override;
  // bool visit(NumericLiteralPropertyName *node) override;
  // bool visit(ArrayMemberExpression *node) override;
  // bool visit(FieldMemberExpression *node) override;
  // bool visit(NewMemberExpression *node) override;
  // bool visit(NewExpression *node) override;
  // bool visit(CallExpression *node) override;
  // bool visit(PostIncrementExpression *node) override;
  // bool visit(PostDecrementExpression *node) override;
  // bool visit(PreIncrementExpression *node) override;
  // bool visit(PreDecrementExpression *node) override;
  // bool visit(DeleteExpression *node) override;
  // bool visit(VoidExpression *node) override;
  // bool visit(TypeOfExpression *node) override;
  // bool visit(UnaryPlusExpression *node) override;
  // bool visit(UnaryMinusExpression *node) override;
  // bool visit(TildeExpression *node) override;
  // bool visit(NotExpression *node) override;
  // bool visit(BinaryExpression *node) override;
  // bool visit(ConditionalExpression *node) override;
  // bool visit(Block *node) override;
  // bool visit(VariableStatement *node) override;
  // bool visit(VariableDeclaration *node) override;
  // bool visit(EmptyStatement *node) override;
  // bool visit(IfStatement *node) override;
  // bool visit(DoWhileStatement *node) override;
  // bool visit(WhileStatement *node) override;
  // bool visit(ForStatement *node) override;
  // bool visit(LocalForStatement *node) override;
  // bool visit(ForEachStatement *node) override;
  // bool visit(LocalForEachStatement *node) override;
  // bool visit(ContinueStatement *node) override;
  // bool visit(BreakStatement *node) override;
  // bool visit(ReturnStatement *node) override;
  // bool visit(ThrowStatement *node) override;
  // bool visit(WithStatement *node) override;
  // bool visit(SwitchStatement *node) override;
  // bool visit(CaseBlock *node) override;
  // bool visit(CaseClause *node) override;
  // bool visit(DefaultClause *node) override;
  // bool visit(LabelledStatement *node) override;
  // bool visit(TryStatement *node) override;
  // bool visit(Catch *node) override;
  // bool visit(Finally *node) override;
  // bool visit(FunctionDeclaration *node) override;
  // bool visit(FunctionExpression *node) override;
  // bool visit(UiHeaderItemList *node) override;
  // bool visit(UiObjectMemberList *node) override;
  // bool visit(UiQualifiedId *node) override;
  // bool visit(UiQualifiedPragmaId *node) override;
  // bool visit(Elision *node) override;
  // bool visit(ArgumentList *node) override;
  // bool visit(StatementList *node) override;
  // bool visit(SourceElements *node) override;
  // bool visit(VariableDeclarationList *node) override;
  // bool visit(CaseClauses *node) override;
  // bool visit(FormalParameterList *node) override;
};

#endif // AST_GENERATOR_BASE_H