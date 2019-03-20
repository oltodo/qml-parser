#include <iostream>
#include <sstream>

#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/parser/qmljsastvisitor_p.h>
#include <qmljs/qmljsdocument.h>

#include "AstGeneratorBase.h"
#include "Location.h"
#include "parser.h"

using namespace std;
using namespace QmlJS;
using namespace QmlJS::AST;

AstGeneratorBase::AstGeneratorBase(Document::Ptr doc, int level)
    : doc(doc), level(level) {}

string AstGeneratorBase::toString(const QString &str) {
  return str.toUtf8().constData();
}

string AstGeneratorBase::toString(const Location &loc) {
  return toString(doc->source().mid(loc.startOffset, loc.length));
}

string AstGeneratorBase::toString(const QStringRef &str) {
  return toString(str.toString());
}

string AstGeneratorBase::toString(const Location &first,
                                  const Location &second) {
  const Location loc = mergeLocs(first, second);

  return toString(loc);
}

string AstGeneratorBase::toString(UiQualifiedId *node) {
  stringstream id;

  for (UiQualifiedId *it = node; it; it = it->next) {
    id << toString(it->identifierToken);
    if (it->next)
      id << ".";
  }

  return id.str();
}

json AstGeneratorBase::getLoc(const SourceLocation &loc) {
  return static_cast<Location>(loc);
}

json AstGeneratorBase::getLoc(const Location &loc) { return loc.toJson(); }

Location AstGeneratorBase::mergeLocs(const SourceLocation &loc) {
  return Location(loc);
}

lineColumn AstGeneratorBase::getLineColumn(const int index) {
  const QString str = doc->source();

  lineColumn result;

  for (int i = 0; i < str.count(); ++i) {
    if (str.at(i) == '\n') {
      result.line++;
      result.column = 0;
      continue;
    }

    if (i == index) {
      break;
    }

    result.column++;
  }

  return result;
}

QChar AstGeneratorBase::getCharAt(const int index) {
  const QString str = doc->source();

  return str.at(index);
}

int AstGeneratorBase::getNextPrintableCharIndex(const int startFromIndex = 0) {
  const QString str = doc->source();

  for (int i = startFromIndex; i < str.count(); ++i) {
    const QChar ch = str.at(i);

    if (ch.isPrint() && !ch.isSpace()) {
      return i;
    }
  }

  return -1;
}

bool AstGeneratorBase::visit(UiPragma *node) {
  print("UiPragma", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiImport *node) {
  print("UiImport", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiObjectDefinition *node) {
  print("UiObjectDefinition", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiObjectInitializer *node) {
  print("UiObjectInitializer", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiParameterList *list) {
  print("UiParameterList", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiPublicMember *node) {
  print("UiPublicMember", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiObjectBinding *node) {
  print("UiObjectBinding", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiScriptBinding *node) {
  print("UiScriptBinding", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiArrayBinding *node) {
  print("UiArrayBinding", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiArrayMemberList *node) {
  print("UiArrayMemberList", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ThisExpression *node) {
  print("ThisExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(NullExpression *node) {
  print("NullExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(TrueLiteral *node) {
  print("TrueLiteral", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(FalseLiteral *node) {
  print("FalseLiteral", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(IdentifierExpression *node) {
  print("IdentifierExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(StringLiteral *node) {
  print("StringLiteral", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(NumericLiteral *node) {
  print("NumericLiteral", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(RegExpLiteral *node) {
  print("RegExpLiteral", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ArrayLiteral *node) {
  print("ArrayLiteral", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ObjectLiteral *node) {
  print("ObjectLiteral", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ElementList *node) {
  print("ElementList", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(PropertyAssignmentList *node) {
  print("PropertyAssignmentList", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(NestedExpression *node) {
  print("NestedExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(IdentifierPropertyName *node) {
  print("IdentifierPropertyName", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(StringLiteralPropertyName *node) {
  print("StringLiteralPropertyName", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(NumericLiteralPropertyName *node) {
  print("NumericLiteralPropertyName", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ArrayMemberExpression *node) {
  print("ArrayMemberExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(FieldMemberExpression *node) {
  print("FieldMemberExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(NewMemberExpression *node) {
  print("NewMemberExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(NewExpression *node) {
  print("NewExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(CallExpression *node) {
  print("CallExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(PostIncrementExpression *node) {
  print("PostIncrementExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(PostDecrementExpression *node) {
  print("PostDecrementExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(PreIncrementExpression *node) {
  print("PreIncrementExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(PreDecrementExpression *node) {
  print("PreDecrementExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(DeleteExpression *node) {
  print("DeleteExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(VoidExpression *node) {
  print("VoidExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(TypeOfExpression *node) {
  print("TypeOfExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UnaryPlusExpression *node) {
  print("UnaryPlusExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UnaryMinusExpression *node) {
  print("UnaryMinusExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(TildeExpression *node) {
  print("TildeExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(NotExpression *node) {
  print("NotExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(BinaryExpression *node) {
  print("BinaryExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ConditionalExpression *node) {
  print("ConditionalExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(Block *node) {
  print("Block", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(VariableStatement *node) {
  print("VariableStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(VariableDeclaration *node) {
  print("VariableDeclaration", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(EmptyStatement *node) {
  print("EmptyStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(IfStatement *node) {
  print("IfStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(DoWhileStatement *node) {
  print("DoWhileStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(WhileStatement *node) {
  print("WhileStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ForStatement *node) {
  print("ForStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(LocalForStatement *node) {
  print("LocalForStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ForEachStatement *node) {
  print("ForEachStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(LocalForEachStatement *node) {
  print("LocalForEachStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ContinueStatement *node) {
  print("ContinueStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(BreakStatement *node) {
  print("BreakStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ReturnStatement *node) {
  print("ReturnStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ThrowStatement *node) {
  print("ThrowStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(WithStatement *node) {
  print("WithStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(SwitchStatement *node) {
  print("SwitchStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(CaseBlock *node) {
  print("CaseBlock", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(CaseClause *node) {
  print("CaseClause", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(DefaultClause *node) {
  print("DefaultClause", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(LabelledStatement *node) {
  print("LabelledStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(TryStatement *node) {
  print("TryStatement", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(Catch *node) {
  print("Catch", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(Finally *node) {
  print("Finally", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(FunctionDeclaration *node) {
  print("FunctionDeclaration", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(FunctionExpression *node) {
  print("FunctionExpression", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiHeaderItemList *node) {
  print("UiHeaderItemList", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiObjectMemberList *node) {
  print("UiObjectMemberList", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiQualifiedId *node) {
  print("UiQualifiedId", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(UiQualifiedPragmaId *node) {
  print("UiQualifiedPragmaId", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(Elision *node) {
  print("Elision", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(ArgumentList *node) {
  print("ArgumentList", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(StatementList *node) {
  print("StatementList", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(SourceElements *node) {
  print("SourceElements", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(VariableDeclarationList *node) {
  print("VariableDeclarationList", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(CaseClauses *node) {
  print("CaseClauses", "not implemented");
  return false;
}

bool AstGeneratorBase::visit(FormalParameterList *node) {
  print("FormalParameterList", "not implemented");
  return false;
}
