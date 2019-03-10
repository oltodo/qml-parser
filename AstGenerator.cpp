#include <iostream>
#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsdocument.h>

#include "AstGenerator.h"
#include "AstGeneratorBase.h"
#include "AstGeneratorJavascriptBlock.h"
#include "Location.h"
#include "parser.h"

using namespace std;
using namespace QmlJS;
using namespace QmlJS::AST;

void AstGenerator::accept(Node *node) { Node::accept(node, this); }

json AstGenerator::operator()(Node *node) {
  accept(node);
  return ast;
}

void AstGenerator::appendItems(const json &items) {
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

bool AstGenerator::visit(UiImport *node) {
  print("UiImport");

  json item;

  item["kind"] = "Import";
  item["loc"] =
      getLoc(node->importToken, node->fileNameToken, node->versionToken,
             node->asToken, node->importIdToken, node->semicolonToken);

  if (!node->fileName.isNull())
    item["path"] = toString(node->fileName);
  else
    item["identifier"] = toString(node->importUri);

  if (node->versionToken.isValid())
    item["version"] = toString(node->versionToken);

  if (!node->importId.isNull())
    item["as"] = toString(node->importIdToken);

  ast = item;

  return false;
}

bool AstGenerator::visit(UiObjectDefinition *node) {
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

bool AstGenerator::visit(UiObjectInitializer *node) {
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

bool AstGenerator::visit(UiParameterList *list) {
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

bool AstGenerator::visit(UiPublicMember *node) {
  print("UiPublicMember", toString(node->identifierToken));

  json item;

  if (node->type == UiPublicMember::Property) {
    item["kind"] = "Property";

    item["loc"] =
        getLoc(node->defaultToken, node->readonlyToken, node->propertyToken,
               node->typeModifierToken, node->typeToken, node->identifierToken,
               node->colonToken, node->semicolonToken);

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
      item["value"] = gen(node->binding);
      // json value = gen(node->binding);
      // item["value"].push_back(value);
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

bool AstGenerator::visit(UiObjectBinding *node) {
  print("UiObjectBinding");

  json item;

  if (node->hasOnToken) {
    item["kind"] = "ObjectDefinition";
    item["identifier"] = toString(node->qualifiedTypeNameId);
    item["on"] = toString(node->qualifiedId);
    item["loc"] =
        getLoc(node->firstSourceLocation(), node->lastSourceLocation());
  } else {
    item["kind"] = "ObjectDefinition";
    item["identifier"] = toString(node->qualifiedTypeNameId);
  }

  AstGenerator gen(doc, level + 1);
  item["children"] = gen(node->initializer);

  ast = item;

  return false;
}

bool AstGenerator::visit(UiScriptBinding *node) {
  print("UiScriptBinding");

  json item;
  item["kind"] = "Attribute";
  item["identifier"] = toString(node->qualifiedId);

  AstGeneratorJavascriptBlock gen(doc, level + 1);
  item["value"] = gen(node->statement);

  ast = item;

  return false;
}

bool AstGenerator::visit(UiArrayBinding *node) {
  print("UiArrayBinding");

  json item;
  item["kind"] = "ArrayBinding";
  item["identifier"] = toString(node->qualifiedId);

  AstGenerator gen(doc, level + 1);
  item["children"] = gen(node->members);

  ast = item;

  return false;
}

bool AstGenerator::visit(UiArrayMemberList *node) {
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

bool AstGenerator::visit(FunctionDeclaration *node) {
  print("FunctionDeclaration");

  return visit(static_cast<FunctionExpression *>(node));
}

bool AstGenerator::visit(FunctionExpression *node) {
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

bool AstGenerator::visit(UiHeaderItemList *node) {
  print("UiHeaderItemList");

  ast = json::object();

  const int lastOffset = doc->source().count() - 1;
  const lineColumn position = getLineColumn(lastOffset);
  const SourceLocation firstLocation = SourceLocation(0, 1, 1, 1);
  const SourceLocation lastLocation =
      SourceLocation(lastOffset, 1, position.line, position.column);

  ast["kind"] = "Program";
  ast["loc"] = getLoc(firstLocation, lastLocation);
  ast["children"] = json::array();

  for (UiHeaderItemList *it = node; it; it = it->next) {
    AstGenerator gen(doc, level + 1);
    const json item = gen(it->headerItem);
    ast["children"].push_back(item);
  }

  return false;
}

bool AstGenerator::visit(UiObjectMemberList *node) {
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
