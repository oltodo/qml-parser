#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsdocument.h>

#include "AstGeneratorBase.h"
#include "AstGeneratorJavascriptBlock.h"
#include "Location.h"
#include "parser.h"

using namespace QmlJS;
using namespace QmlJS::AST;

json AstGeneratorJavascriptBlock::operator()(Node *node) {
  accept(node);

  if (Block *t = dynamic_cast<Block *>(node)) {
    ast["kind"] = "JavascriptBlock";
  } else {
    ast["kind"] = "JavascriptValue";
  }

  ast["loc"] = getLoc(loc);
  ast["value"] = toString(loc);

  return ast;
}

void AstGeneratorJavascriptBlock::accept(Node *node) {
  Node::accept(node, this);
}

void AstGeneratorJavascriptBlock::postVisit(Node *node) {
  loc = mergeLocs(loc, node->firstSourceLocation(), node->lastSourceLocation());
}
