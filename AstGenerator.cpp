#include <iostream>

#include <private/qqmljsast_p.h>

#include "AstGenerator.h"
#include "AstGeneratorBase.h"
#include "AstGeneratorJavascriptBlock.h"
#include "Location.h"
#include "parser.h"

using namespace std;
using namespace QQmlJS;
using namespace QQmlJS::AST;

void AstGenerator::accept(Node *node) { Node::accept(node, this); }

json AstGenerator::operator()(Node *node) {
  accept(node);
  insertComments();

  return ast;
}

void AstGenerator::insertComments() {
  if (level != 0)
    return;

  ast["comments"] = json::array();

  const QList<SourceLocation> &comments = engine->comments();

  for (int i = 0; i < comments.size(); ++i) {
    insertComment(comments.at(i));
  }
}

Location AstGenerator::getGoodCommentLocation(const SourceLocation &badLoc) {
  const int startOffset = badLoc.offset - 2;
  const int startLine = badLoc.startLine;
  const int startColumn = badLoc.startColumn - 2;

  int endOffset = startOffset + badLoc.length + 2;

  const QString openingTag = engine->code().mid(startOffset, 2);

  if (openingTag == "/*") {
    endOffset += 2;
  }

  const lineColumn endPosition = getLineColumn(endOffset);
  const int endLine = endPosition.line;
  const int endColumn = endPosition.column;

  return Location(startColumn, startLine, startOffset, endColumn, endLine,
                  endOffset);
}

bool AstGenerator::isCommentInJavascript(const Location &loc,
                                         const json &node) {
  if (node["kind"] == "Function" || node["kind"] == "JavascriptBlock") {
    return node["loc"]["start"]["offset"] <= loc.startOffset &&
           node["loc"]["end"]["offset"] >= loc.endOffset;
  }

  if (node.contains("children") && !node["children"].empty()) {
    int size = node["children"].size();
    for (int i = 0; i < size; ++i) {
      if (isCommentInJavascript(loc, node["children"][i])) {
        return true;
      }
    }
  }

  if (node.contains("value") && node["value"].contains("kind")) {
    if (isCommentInJavascript(loc, node["value"])) {
      return true;
    }
  }

  return false;
}

void AstGenerator::insertComment(const SourceLocation &initialLoc) {
  const Location loc = getGoodCommentLocation(initialLoc);

  if (isCommentInJavascript(loc, ast)) {
    return;
  }

  const string value = toString(loc);

  json comment;

  if (value.substr(0, 2) == "//") {
    comment["kind"] = "CommentLine";
  } else {
    comment["kind"] = "CommentBlock";
  }

  comment["loc"] = getLoc(loc);
  comment["value"] = value;

  ast["comments"].push_back(comment);
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

bool AstGenerator::visit(UiPragma *node) {
  print("UiPragma");

  json item;

  item["kind"] = "Pragma";
  item["loc"] = getLoc(node->pragmaToken, node->lastSourceLocation());
  item["type"] = toString(node->name);

  ast = item;

  return false;
}

bool AstGenerator::visit(UiImport *node) {
  print("UiImport");

  json item;

  item["kind"] = "Import";

  if (node->version) {
    item["loc"] =
        getLoc(node->importToken, node->fileNameToken,
               node->version->majorToken, node->version->minorToken,
               node->asToken, node->importIdToken, node->semicolonToken);
  } else {
    item["loc"] = getLoc(node->importToken, node->fileNameToken, node->asToken,
                         node->importIdToken, node->semicolonToken);
  }

  if (!node->fileName.isNull())
    item["path"] = toString(node->fileName);
  else
    item["identifier"] = toString(node->importUri);

  if (node->version)
    item["version"] =
        toString(node->version->majorToken, node->version->minorToken);

  if (!node->importId.isNull())
    item["as"] = toString(node->importIdToken);

  ast = item;

  return false;
}

bool AstGenerator::visit(UiObjectDefinition *node) {
  print("UiObjectDefinition", toString(node->qualifiedTypeNameId));

  ast["kind"] = "ObjectDefinition";
  ast["loc"] = getLoc(node->firstSourceLocation(), node->lastSourceLocation());
  ast["identifier"] = toString(node->qualifiedTypeNameId);
  ast["children"] = json::array();

  if (node->initializer) {
    AstGenerator gen(engine, level + 1);
    const json item = gen(node->initializer);
    ast["children"] = item;
  }

  return false;
}

bool AstGenerator::visit(UiObjectInitializer *node) {
  print("UiObjectInitializer");

  AstGenerator gen(engine, level + 1);
  appendItems(gen(node->members));

  return false;
}

bool AstGenerator::visit(UiParameterList *list) {
  print("UiParameterList");

  json items;

  for (UiParameterList *it = list; it; it = it->next) {
    json item;

    item["type"] = toString(it->type);
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
    item["loc"];

    Location loc = mergeLocs(node->defaultToken, node->readonlyToken,
                             node->propertyToken, node->typeModifierToken,
                             node->typeToken, node->identifierToken,
                             node->colonToken, node->semicolonToken);

    if (node->isDefaultMember)
      item["default"] = true;
    else if (node->isReadonlyMember)
      item["readonly"] = true;

    if (!node->typeModifier.isNull())
      item["typeModifier"] = toString(node->typeModifierToken);

    item["type"] = toString(node->memberType);

    if (node->statement) {
      item["identifier"] = toString(node->identifierToken);

      AstGeneratorJavascriptBlock gen(engine, level + 1);
      item["value"] = gen(node->statement);

      loc = mergeLocs(loc, Location(item["value"]["loc"]));
    } else if (node->binding) {
      item["identifier"] = toString(node->identifierToken);

      AstGenerator gen(engine, level + 1);
      item["value"] = gen(node->binding);

      if (item["value"]["kind"] == "Attribute")
        item["value"] = item["value"]["value"];

      loc = mergeLocs(loc, Location(item["value"]["loc"]));
    } else
      item["identifier"] = toString(node->identifierToken);

    item["loc"] = getLoc(loc);
  } else {
    item["kind"] = "Signal";
    item["identifier"] = toString(node->identifierToken);
    item["asBrackets"] = false;

    Location loc;

    if (node->parameters) {
      AstGenerator gen(engine, level + 1);
      item["parameters"] = gen(node->parameters);
      item["asBrackets"] = true;

      // Where is the closing bracket?!
      const Location lastLoc =
          static_cast<Location>(node->parameters->lastSourceLocation());

      const int closingBracketOffset =
          engine->code().indexOf(')', lastLoc.endOffset);

      const lineColumn position = getLineColumn(closingBracketOffset);

      const Location bracketLocation =
          Location(position.column, position.line, closingBracketOffset, 1);

      loc = mergeLocs(node->firstSourceLocation(), bracketLocation);
    } else {
      Location lastLoc = static_cast<Location>(node->identifierToken);

      const int nextCharIndex = getNextPrintableCharIndex(lastLoc.endOffset);
      const QChar nextChar = getCharAt(nextCharIndex);

      if (nextChar == '(') {
        item["asBrackets"] = true;

        const int closingBracketIndex =
            getNextPrintableCharIndex(nextCharIndex);
        const lineColumn position = getLineColumn(closingBracketIndex);

        lastLoc = Location(position.column + 1, position.line,
                           closingBracketIndex + 1, 1);
      }

      loc = mergeLocs(node->firstSourceLocation(), lastLoc);
    }

    item["loc"] = getLoc(loc);
  }

  ast = item;

  return false;
}

bool AstGenerator::visit(UiObjectBinding *node) {
  print("UiObjectBinding");

  json item;

  if (node->hasOnToken) {
    item["kind"] = "ObjectDefinition";
    item["loc"] =
        getLoc(node->firstSourceLocation(), node->lastSourceLocation());
    item["identifier"] = toString(node->qualifiedTypeNameId);
    item["on"] = toString(node->qualifiedId);

    AstGenerator gen(engine, level + 1);
    item["children"] = gen(node->initializer);
  } else {
    item["kind"] = "Attribute";
    item["loc"];
    item["identifier"] = toString(node->qualifiedId);

    json value;
    value["kind"] = "ObjectDefinition";
    value["loc"] = getLoc(node->qualifiedTypeNameId->firstSourceLocation(),
                          node->initializer->lastSourceLocation());
    value["identifier"] = toString(node->qualifiedTypeNameId);

    AstGenerator gen(engine, level + 1);
    value["children"] = gen(node->initializer);

    item["value"] = value;
    item["loc"] = getLoc(node->firstSourceLocation(), item["value"]["loc"]);
  }

  ast = item;

  return false;
}

bool AstGenerator::visit(UiScriptBinding *node) {
  const string identifier = toString(node->qualifiedId);

  print("UiScriptBinding", identifier);

  json item;

  AstGeneratorJavascriptBlock gen(engine, level + 1);
  const json value = gen(node->statement);

  if (identifier == "id") {
    item["kind"] = "ObjectIdentifier";
    item["loc"] =
        Location(node->firstSourceLocation(), node->lastSourceLocation());
    item["value"] = value["value"];
  } else {
    item["kind"] = "Attribute";
    item["loc"];
    item["identifier"] = identifier;
    item["value"] = value;
    item["loc"] = mergeLocs(node->qualifiedId->firstSourceLocation(),
                            Location(item["value"]["loc"]));
  }

  ast = item;

  return false;
}

bool AstGenerator::visit(UiArrayBinding *node) {
  print("UiArrayBinding");

  json item;
  item["kind"] = "ArrayBinding";
  item["loc"] = getLoc(node->lbracketToken, node->rbracketToken);

  item["identifier"] = toString(node->qualifiedId);

  AstGenerator gen(engine, level + 1);
  item["children"] = gen(node->members);

  ast = item;

  return false;
}

bool AstGenerator::visit(UiArrayMemberList *node) {
  print("UiArrayMemberList");

  json items;

  for (UiArrayMemberList *it = node; it; it = it->next) {
    AstGenerator gen(engine, level + 1);
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

  ast = item;

  return false;
}

bool AstGenerator::visit(UiHeaderItemList *node) {
  print("UiHeaderItemList");

  ast = json::object();

  const int lastOffset = engine->code().count();
  const lineColumn position = getLineColumn(lastOffset);
  const Location loc =
      Location(0, 1, 0, position.column, position.line, lastOffset);

  ast["kind"] = "Program";
  ast["loc"] = loc.toJson();
  ast["children"] = json::array();

  for (UiHeaderItemList *it = node; it; it = it->next) {
    AstGenerator gen(engine, level + 1);
    const json item = gen(it->headerItem);
    ast["children"].push_back(item);
  }

  return false;
}

bool AstGenerator::visit(UiObjectMemberList *node) {
  print("UiObjectMemberList");

  json items;

  for (UiObjectMemberList *it = node; it; it = it->next) {
    AstGenerator gen(engine, level + 1);
    const json item = gen(it->member);
    items.push_back(item);
  }

  appendItems(items);

  return false;
}
