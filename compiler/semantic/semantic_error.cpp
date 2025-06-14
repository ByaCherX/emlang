
#include "semantic/semantic_error.h"
#include <iostream>
#include <sstream>
#include <vector>

namespace emlang {

// SemanticError constructor implementation
SemanticError::SemanticError(const std::string& message, size_t line, size_t column)
    : std::runtime_error(message), line(line), column(column) {
}

std::string SemanticError::getFormattedMessage() const {
    std::stringstream ss;
    ss << "Semantic Error";
    if (line > 0) {
        ss << " [" << line << ":" << column << "]";
    }
    ss << ": " << what();
    return ss.str();
}

// SemanticErrorReporter constructor implementation
SemanticErrorReporter::SemanticErrorReporter(bool showWarnings) : showWarnings(showWarnings) {
}

void SemanticErrorReporter::reportError(const std::string& message, size_t line, size_t column) {
    SemanticError error(message, line, column);
    errors.push_back(error);
    
    // Print error immediately
    std::cerr << error.getFormattedMessage() << std::endl;
}

void SemanticErrorReporter::reportWarning(const std::string& message, size_t line, size_t column) {
    if (showWarnings) {
        std::cerr << "Semantic Warning";
        if (line > 0) {
            std::cerr << " [" << line << ":" << column << "]";
        }
        std::cerr << ": " << message << std::endl;
    }
    
    // Store warning for counting
    warnings.push_back(message);
}

bool SemanticErrorReporter::hasErrors() const {
    return !errors.empty();
}

bool SemanticErrorReporter::hasWarnings() const {
    return !warnings.empty();
}

size_t SemanticErrorReporter::getErrorCount() const {
    return errors.size();
}

size_t SemanticErrorReporter::getWarningCount() const {
    return warnings.size();
}

void SemanticErrorReporter::clear() {
    errors.clear();
    warnings.clear();
}

void SemanticErrorReporter::printSummary() const {
    if (hasErrors()) {
        std::cerr << "\nSemantic analysis failed with " << errors.size() << " error(s)";
        if (hasWarnings()) {
            std::cerr << " and " << warnings.size() << " warning(s)";
        }
        std::cerr << "." << std::endl;
    } else if (hasWarnings()) {
        std::cerr << "\nSemantic analysis completed with " << warnings.size() << " warning(s)." << std::endl;
    } else {
        std::cout << "\nSemantic analysis completed successfully." << std::endl;
    }
}

const std::vector<SemanticError>& SemanticErrorReporter::getErrors() const {
    return errors;
}

const std::vector<std::string>& SemanticErrorReporter::getWarnings() const {
    return warnings;
}

} // namespace emlang
