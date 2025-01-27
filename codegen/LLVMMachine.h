//
// Created by moltmanns on 1/26/25.
//

#ifndef LLVMMACHINE_H
#define LLVMMACHINE_H
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

#include "typechecker/TypeChecker.h"


class LLVMMachine {
private:
public:
    llvm::TargetMachine *TM = nullptr;

    LLVMMachine();

    void emit(llvm::Module *llvm_module, const std::string &name, OutputFileType type) const;

    ~LLVMMachine() = default;
};



#endif //LLVMMACHINE_H
