// #include <qmljs/parser/qmljsastvisitor_p.h>
// #include <qmljs/parser/qmljsengine_p.h>

#include <qmljs/parser/qmljsastvisitor_p.h>

#include "ast_generator.h"

using namespace QmlJS;
using namespace QmlJS::AST;

class AstGenerator
{
    Document::Ptr _doc;

  private:
    /* data */
  public:
    AstGenerator(Document::Ptr doc)
        : _doc(doc)
    {
    }
};

// AstGenerator::AstGenerator(doc){};

// AstGenerator::~AstGenerator(){};
