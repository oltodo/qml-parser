#include <private/qqmljsast_p.h>

#include "AstGeneratorBase.h"
#include "AstGeneratorJavascriptBlock.h"
#include "Location.h"
#include "parser.h"

using namespace QQmlJS::AST;

json AstGeneratorJavascriptBlock::operator()(Statement *node) {
  accept(node);

  QString value = QString::fromStdString(toString(loc));

  bool isBlock = node->kind == node->Kind_Block;
  bool isObject = !isBlock && value.startsWith('{') && value.endsWith('}');
  bool hasSemicolon = !isBlock && value.endsWith(';');
  Location newLoc = hasSemicolon ? loc - 1 : loc;

  ast["kind"] = isBlock ? "JavascriptBlock" : "JavascriptValue";
  ast["loc"] = getLoc(newLoc);
  ast["object"] = isObject;
  ast["value"] = toString(newLoc);

  return ast;
}

void AstGeneratorJavascriptBlock::accept(Node *node) {
  Node::accept(node, this);
}

void AstGeneratorJavascriptBlock::postVisit(Node *node) {
  loc = mergeLocs(loc, node->firstSourceLocation(), node->lastSourceLocation());
}
