
#include "Nodes.h"
#include <Parser/ParserVal.h>
using namespace tinydbpp::ast;

SysManagement::SysManagement(const ParserVal &target, Statement::Type type)
        :Statement(type), target(std::make_shared<ParserVal>(target)){ }

std::shared_ptr<const tinydbpp::ParserVal> SysManagement::getTarget() const {
  return target;
}

void Statements::addStatementToFront(std::shared_ptr<Statement> stmt) {
  statements.insert(statements.begin(), stmt);
}
