#pragma once

#ifndef AST_GENERATOR_JAVASCRIPT_BLOCK_H
#define AST_GENERATOR_JAVASCRIPT_BLOCK_H

#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsdocument.h>

using namespace std;
using namespace QmlJS;
using namespace QmlJS::AST;
using namespace nlohmann;

class AstGeneratorJavascriptBlock : protected AstGeneratorBase {
  using AstGeneratorBase::AstGeneratorBase;

  json ast;
  Location loc;

protected:
  void accept(Node *node);
  void postVisit(Node *node) override;

public:
  json operator()(Node *node);
};

#endif // AST_GENERATOR_JAVASCRIPT_BLOCK_H