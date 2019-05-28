#include <private/qqmljsast_p.h>

#include "AstGeneratorBase.h"
#include "AstGeneratorJavascriptBlock.h"
#include "Location.h"
#include "parser.h"

using namespace QQmlJS::AST;

json AstGeneratorJavascriptBlock::operator()(Node *node) {
  accept(node);

  string value = toString(loc);
  QString qvalue = QString::fromStdString(value);

  bool isBlock = node->kind == node->Kind_Block;

  if (isBlock) {
    ast["kind"] = "JavascriptBlock";
  } else {
    ast["kind"] = "JavascriptValue";
  }

  ast["loc"] = getLoc(loc);
  ast["object"] = !isBlock && qvalue.startsWith('{') && qvalue.endsWith('}');
  ast["value"] = value;

  return ast;
}

void AstGeneratorJavascriptBlock::accept(Node *node) {
  Node::accept(node, this);
}

void AstGeneratorJavascriptBlock::postVisit(Node *node) {
  loc = mergeLocs(loc, node->firstSourceLocation(), node->lastSourceLocation());
}
