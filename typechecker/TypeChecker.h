//
// Created by moltmanns on 1/10/25.
// Checks types in statement nodes
//

#ifndef TYPECHECKER_H
#define TYPECHECKER_H
#include "parser/ast/visitor/NodeVisitor.h"
#include "scoper/symbols/types/Type.h"

enum class OutputFileType {
    AssemblyFile, LLVMIRFile, ObjectFile
};

class TypeChecker final : public NodeVisitor {
public:
    explicit TypeChecker(std::shared_ptr<Scope> scope, Logger& logger) : NodeVisitor(std::move(scope), logger) {}
    ~TypeChecker() noexcept override = default;

    std::shared_ptr<Type> get_type(const std::shared_ptr<Node> &node);

    void visit(const std::shared_ptr<Node>& node) override;

    static bool types_compatible(const std::string &a, const std::string &b);
};



#endif //TYPECHECKER_H
