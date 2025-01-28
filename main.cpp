/*
 * Main class of the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/14/17.
 */

#include <iostream>
#include <string>

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
    auto mod = parser.parse(); // get an ast
    if (logger.getErrorCount() != 0) goto print_status;
    else {
        mod->print(cout);
        auto outer_scope = std::make_shared<Scope>(logger, ""); // declare empty outer scope
        Scoper scoper = Scoper(outer_scope, logger); // get a scoper
        scoper.visit(mod); // build the scope
        if (logger.getErrorCount() != 0) goto print_status;
        else {
            outer_scope->print(cout);
            auto module = outer_scope->lookup_by_name<Module>("Sort0");
            auto tm = LLVMMachine();
            llvm::LLVMContext ctx;
            auto gen = Generator(*tm.TM, ctx, logger);
            auto code = gen.gen_module(module.get());
            if (logger.getErrorCount() != 0) goto print_status;
            tm.emit(code, module->name(), OutputFileType::LLVMIRFile);
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