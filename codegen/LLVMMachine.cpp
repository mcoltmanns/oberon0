//
// Created by moltmanns on 1/26/25.
//

#include "LLVMMachine.h"

#include <iostream>

LLVMMachine::LLVMMachine() {
    // like the demo on gitlab
    // init llvm
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    // default host triple
    std::string triple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!target) {
        std::cerr << error << std::endl;
    }
    else {
        std::string cpu = "generic";
        std::string features;
        llvm::TargetOptions options;
#ifdef _LLVM_LEGACY
        auto model = llvm::Optional<llvm::Reloc::Model>();
#else
        auto model = std::optional<llvm::Reloc::Model>();
#endif
        TM = target->createTargetMachine(triple, cpu, features, options, model);
    }
}

void LLVMMachine::emit(llvm::Module *llvm_module, const std::string &name, OutputFileType type) const {
    std::string ext;
    switch (type) {
        case OutputFileType::AssemblyFile:
            ext = ".s";
            break;
        case OutputFileType::LLVMIRFile:
            ext = ".ll";
            break;
        default:
#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
            ext = ".obj"
#else
            ext = ".o";
#endif
            break;
    }
    std::string file = name + ext;

    // serialize llvm module to file
    std::error_code ec;
    // open output stream
    llvm::raw_fd_ostream output(file, ec, llvm::sys::fs::OF_None);
    if (ec) {
        std::cerr << ec.message() << std::endl;
        exit(ec.value());
    }
    if (type == OutputFileType::LLVMIRFile) {
        llvm_module->print(output, nullptr);
        output.flush();
        output.close();
        return;
    }
    llvm::CodeGenFileType ft;
    switch (type) {
        case OutputFileType::AssemblyFile:
            std::cerr << "Assembly files are currently unsupported" << std::endl;
/*
#ifdef _LLVM_18
            ft = llvm::CodeGenFileType::AssemblyFile;
#else
            ft = llvm::CodeGenFileType::CGFT_AssemblyFile;
#endif
*/
            break;
        default:
#ifdef _LLVM_18
            ft = llvm::CodeGenFileType::ObjectFile;
#else
            ft = llvm::CodeGenFileType::CGFT_ObjectFile;
#endif
            break;
    }
    llvm::legacy::PassManager pass_manager;
    if (TM->addPassesToEmitFile(pass_manager, output, nullptr, ft)) {
        std::cerr << "Error emitting LLVM IR file" << std::endl;
        return;
    }
    pass_manager.run(*llvm_module);
    output.flush();
    output.close();
}
