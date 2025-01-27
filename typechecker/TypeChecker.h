//
// Created by moltmanns on 1/10/25.
// Checks types in statement nodes
//

#ifndef TYPECHECKER_H
#define TYPECHECKER_H
#include "parser/ast/visitor/NodeVisitor.h"

enum class OutputFileType {
    AssemblyFile, LLVMIRFile, ObjectFile
};

class TypeChecker final : public NodeVisitor {
public:
    explicit TypeChecker(std::shared_ptr<Scope> scope, Logger& logger) : NodeVisitor(std::move(scope), logger) {}
    ~TypeChecker() noexcept override = default;

    std::shared_ptr<Type> get_type(const std::shared_ptr<Node> &node);

    void visit(const std::shared_ptr<Node>& node) override;
};



#endif //TYPECHECKER_H
