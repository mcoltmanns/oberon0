/*
 * Main class of the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/14/17.
 */

#include <iostream>
#include <string>

#include "codegen/Generator.h"
#include "codegen/Generator.h"
#include "codegen/LLVMMachine.h"
#include "parser/Parser.h"
#include "scanner/Scanner.h"
#include "scoper/Scoper.h"
#include "scoper/symbols/Constant.h"
#include "scoper/symbols/Module.h"
#include "scoper/symbols/Procedure.h"
#include "scoper/symbols/Variable.h"
#include "scoper/symbols/types/ConstructedTypes.h"
#include "typechecker/TypeChecker.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

int main(const int argc, const char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: oberon0c <filename>" << endl;
        exit(1);
    }
    path filename = argv[1];
    Logger logger;
    logger.setLevel(LogLevel::DEBUG);
    Scanner scanner(filename, logger);
    Parser parser{scanner, logger};
    auto module_node = parser.parse(); // get an ast
    if (logger.getErrorCount() == 0) {
        module_node->print(cout);
        auto module_name = std::dynamic_pointer_cast<IdentNode>(module_node->children().at(0))->name();
        auto outer_scope = std::make_shared<Scope>(logger, "EXTERN"); // declare outer scope - this is also where basic types and external procedures go
        Scoper scoper = Scoper(outer_scope, logger); // get a scoper
        scoper.visit(module_node); // build the scope
        if (logger.getErrorCount() == 0) {
            outer_scope->print(cout);
            auto module_symbol = outer_scope->lookup_by_name<Module>(module_name);
            auto tm = LLVMMachine();
            auto gen = Generator();
            auto ctx = llvm::LLVMContext();
            auto code = gen.generate_module(module_symbol.get(), ctx, tm.TM->createDataLayout(), tm.TM->getTargetTriple());
            if (logger.getErrorCount() != 0) goto print_status;
            tm.emit(code, module_symbol->name(), OutputFileType::LLVMIRFile);
        }
    }

print_status:
    string status = (logger.getErrorCount() == 0 ? "complete" : "failed");
    logger.info("Compilation " + status + ": " +
                to_string(logger.getErrorCount()) + " error(s), " +
                to_string(logger.getWarningCount()) + " warning(s), " +
                to_string(logger.getInfoCount()) + " message(s).");
    exit(logger.getErrorCount() != 0);
}