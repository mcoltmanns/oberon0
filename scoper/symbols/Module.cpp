//
// Created by moltmanns on 12/24/24.
//

#include "Module.h"

#include <llvm/IR/Verifier.h>

#include "Constant.h"
#include "Procedure.h"

Module::Module(std::string name, const FilePos &pos, std::shared_ptr<Node> sseq_node, std::shared_ptr<Scope> module_scope) : Symbol(std::move(name), pos, 1), sseq_node(std::move(sseq_node)), scope_(std::move(module_scope)) {
    kind_ = SymbolKind::MODULE;
}

void Module::print(std::ostream &s, const int tabs) {
    for (int i = 0; i < tabs; i++) s << "\t";
    s << "MODULE " << name_ << " declared at " << declared_at_.fileName << ": " << declared_at_.lineNo << ":" << declared_at_.charNo << std::endl;
    scope_->print(s, tabs + 1);
}
