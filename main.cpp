/*
 * Main class of the Oberon-0 compiler.
 *
 * Created by Michael Grossniklaus on 12/14/17.
 */

#include <iostream>
#include <string>
#include "scanner/Scanner.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

int main(const int argc, const char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: oberon0c <filename>" << endl;
        exit(1);
    }
    string filename = argv[1];
    Logger logger;
    logger.setLevel(LogLevel::DEBUG);
    Scanner scanner(filename, logger);
    auto token = scanner.next();
    while (token->type() != TokenType::eof) {
        cout << *token << endl;
        token = scanner.next();
    }
    cout << *token << endl;
    string status = (logger.getErrorCount() == 0 ? "complete" : "failed");
    logger.info("Compilation " + status + ": " +
                to_string(logger.getErrorCount()) + " error(s), " +
                to_string(logger.getWarningCount()) + " warning(s), " +
                to_string(logger.getInfoCount()) + " message(s).");
    exit(logger.getErrorCount() != 0);
}