#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsdocument.h>

#include "AstGeneratorBase.h"
#include "AstGeneratorJavascriptBlock.h"
#include "Location.h"

using namespace QmlJS;
using namespace QmlJS::AST;
using namespace nlohmann;

json AstGeneratorJavascriptBlock::operator()(Node *node) {
  accept(node);

  ast["kind"] = "JavascriptBlock";
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
