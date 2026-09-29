#include "playground.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "console.h"

#if defined(__GNUG__)
#include <cxxabi.h>
#endif

using namespace antlr4;

namespace pg {

// ---------------------------------------------------------------------------
// Colors and small helpers
// ---------------------------------------------------------------------------
bool color::enabled = true;

std::string color::paint(const std::string &text, const char *code) {
    if (!enabled || text.empty())
        return text;
    return std::string("\x1b[") + code + "m" + text + "\x1b[0m";
}

void section(const std::string &title) {
    std::cout << "\n" << color::bold(color::blue("── " + title + " ")) << "\n";
}

std::string escape(const std::string &text) {
    std::string out;
    for (char c : text) {
        switch (c) {
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:   out += c;
        }
    }
    return out;
}

static std::string collapseWhitespace(const std::string &text, size_t maxLen) {
    std::string out;
    bool inSpace = false;
    for (char c : text) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!inSpace && !out.empty())
                out += ' ';
            inSpace = true;
        } else {
            out += c;
            inSpace = false;
        }
    }
    while (!out.empty() && out.back() == ' ')
        out.pop_back();
    if (out.size() > maxLen)
        out = out.substr(0, maxLen - 3) + "...";
    return out;
}

static std::string position(Token *t) {
    return std::to_string(t->getLine()) + ":" + std::to_string(t->getCharPositionInLine());
}

static std::string tokenTypeName(size_t type, const dfa::Vocabulary &vocab) {
    if (type == Token::EOF)
        return "EOF";
    auto symbolic = vocab.getSymbolicName(type);
    if (!symbolic.empty())
        return std::string(symbolic);
    auto literal = vocab.getLiteralName(type);
    if (!literal.empty())
        return std::string(literal);
    return std::to_string(type);
}

// ---------------------------------------------------------------------------
// Token table
// ---------------------------------------------------------------------------
void printTokens(CommonTokenStream &tokens, const dfa::Vocabulary &vocab) {
    auto all = tokens.getTokens();
    size_t typeWidth = 4;
    for (Token *t : all)
        typeWidth = std::max(typeWidth, tokenTypeName(t->getType(), vocab).size());

    std::cout << color::dim("  " + std::string("#").append(4, ' ') + "pos      " + "type" +
                            std::string(typeWidth - 2, ' ') + "text")
              << "\n";
    for (Token *t : all) {
        std::ostringstream idx, pos;
        idx << std::left << std::setw(5) << t->getTokenIndex();
        pos << std::left << std::setw(9) << position(t);
        std::string type = tokenTypeName(t->getType(), vocab);
        std::string padded = type + std::string(typeWidth + 2 - type.size(), ' ');
        std::string typeColored = t->getType() == Token::EOF         ? color::dim(padded)
                                  : type.front() == '\''             ? color::yellow(padded)
                                                                     : color::green(padded);
        std::string text = t->getType() == Token::EOF ? color::dim("<EOF>") : "\"" + escape(t->getText()) + "\"";
        std::cout << "  " << color::dim(idx.str()) << color::dim(pos.str()) << typeColored << text;
        if (t->getChannel() != Token::DEFAULT_CHANNEL)
            std::cout << color::dim("  (channel " + std::to_string(t->getChannel()) + ")");
        std::cout << "\n";
    }
    std::cout << color::dim("  (tokens matched by `-> skip` rules, like whitespace, never reach this list)") << "\n";
}

// ---------------------------------------------------------------------------
// Parse tree
// ---------------------------------------------------------------------------

// The generated class for a labeled alternative is e.g. CalcParser::AddContext.
// We recover "Add" from the C++ type name so the tree shows which alternative
// matched.
static std::string altLabel(ParserRuleContext *ctx, const std::string &ruleName) {
    const char *raw = typeid(*ctx).name();
    std::string name = raw;
#if defined(__GNUG__)
    int status = 0;
    char *demangled = abi::__cxa_demangle(raw, nullptr, nullptr, &status);
    if (status == 0 && demangled)
        name = demangled;
    std::free(demangled);
#endif
    auto colons = name.rfind("::");
    if (colons != std::string::npos)
        name = name.substr(colons + 2);
    const std::string suffix = "Context";
    if (name.size() > suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0)
        name = name.substr(0, name.size() - suffix.size());

    std::string capitalized = ruleName;
    if (!capitalized.empty())
        capitalized[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(capitalized[0])));
    if (name == capitalized || name == "ParserRule" || name == "InterpreterRule")
        return "";
    return name;
}

static std::string sourceText(ParserRuleContext *ctx) {
    Token *start = ctx->getStart();
    Token *stop = ctx->getStop();
    if (!start || !stop || start->getType() == Token::EOF || stop->getTokenIndex() < start->getTokenIndex())
        return "";
    size_t a = start->getStartIndex();
    size_t b = stop->getType() == Token::EOF ? stop->getStartIndex() - 1 : stop->getStopIndex();
    if (a == INVALID_INDEX || b == INVALID_INDEX || b < a)
        return "";
    return start->getInputStream()->getText(misc::Interval(a, b));
}

namespace {
struct TreePrinter {
    const std::vector<std::string> &ruleNames;
    const dfa::Vocabulary &vocab;

    void print(tree::ParseTree *node, const std::string &prefix, bool last, bool root) {
        std::string branch = root ? "" : (last ? "└── " : "├── ");
        std::cout << prefix << color::dim(branch) << label(node) << "\n";

        std::string childPrefix = root ? "" : prefix + color::dim(last ? "    " : "│   ");
        for (size_t i = 0; i < node->children.size(); ++i)
            print(node->children[i], childPrefix, i + 1 == node->children.size(), false);
    }

    std::string label(tree::ParseTree *node) {
        switch (node->getTreeType()) {
        case tree::ParseTreeType::RULE: {
            auto *ctx = static_cast<ParserRuleContext *>(node);
            std::string rule = ruleNames.at(ctx->getRuleIndex());
            std::string out = color::bold(color::cyan(rule));
            std::string alt = altLabel(ctx, rule);
            if (!alt.empty())
                out += " " + color::magenta("(" + alt + ")");
            std::string text = collapseWhitespace(sourceText(ctx), 50);
            if (!text.empty())
                out += "  " + color::dim("\"" + text + "\"");
            if (ctx->exception)
                out += "  " + color::red("✗ this rule hit a syntax error");
            return out;
        }
        case tree::ParseTreeType::ERROR: {
            auto *err = static_cast<tree::ErrorNode *>(node);
            Token *t = err->getSymbol();
            return color::red("✗ error node \"" + escape(t->getText()) + "\"") + "  " + color::dim("@" + position(t));
        }
        case tree::ParseTreeType::TERMINAL:
        default: {
            auto *term = static_cast<tree::TerminalNode *>(node);
            Token *t = term->getSymbol();
            if (t->getType() == Token::EOF)
                return color::dim("<EOF>");
            std::string type = tokenTypeName(t->getType(), vocab);
            std::string pos = "  " + color::dim("@" + position(t));
            if (type.front() == '\'')
                return color::yellow(type) + pos;
            return color::green(type) + " \"" + escape(t->getText()) + "\"" + pos;
        }
        }
    }
};
} // namespace

void printTree(tree::ParseTree *tree, const std::vector<std::string> &ruleNames, const dfa::Vocabulary &vocab) {
    TreePrinter printer{ruleNames, vocab};
    printer.print(tree, "", true, true);
}

void printGrammarInfo(const std::string &grammarName, const std::vector<std::string> &ruleNames,
                      const dfa::Vocabulary &vocab, const std::vector<std::string> &registeredRules) {
    section(grammarName + ": parser rules");
    for (const auto &rule : ruleNames) {
        bool registered = std::find(registeredRules.begin(), registeredRules.end(), rule) != registeredRules.end();
        std::cout << "  " << color::cyan(rule)
                  << (registered ? color::dim("   (registered in main.cpp: runs your action)") : "") << "\n";
    }
    section(grammarName + ": token types");
    for (size_t type = 1; type <= vocab.getMaxTokenType(); ++type) {
        std::string symbolic(vocab.getSymbolicName(type));
        std::string literal(vocab.getLiteralName(type));
        std::cout << "  " << color::dim(std::to_string(type) + (type < 10 ? "   " : "  "));
        if (!symbolic.empty())
            std::cout << color::green(symbolic) << (literal.empty() ? "" : "  " + color::yellow(literal));
        else
            std::cout << color::yellow(literal) << color::dim("   (implicit token from a literal in a parser rule)");
        std::cout << "\n";
    }
}

// ---------------------------------------------------------------------------
// Trace
// ---------------------------------------------------------------------------
void TraceListener::enterEveryRule(ParserRuleContext *ctx) {
    Token *next = _parser.getCurrentToken();
    std::string nextText = next->getType() == Token::EOF ? "<EOF>" : "\"" + escape(next->getText()) + "\"";
    std::cout << indent() << color::cyan("enter " + _parser.getRuleNames()[ctx->getRuleIndex()])
              << color::dim("   next token: " + nextText) << "\n";
    ++_depth;
}

void TraceListener::exitEveryRule(ParserRuleContext *ctx) {
    --_depth;
    std::cout << indent() << color::dim("exit  " + _parser.getRuleNames()[ctx->getRuleIndex()]) << "\n";
}

void TraceListener::visitTerminal(tree::TerminalNode *node) {
    Token *t = node->getSymbol();
    std::string type = tokenTypeName(t->getType(), _parser.getVocabulary());
    std::cout << indent() << "match " << (type.front() == '\'' ? color::yellow(type) : color::green(type));
    if (type.front() != '\'' && t->getType() != Token::EOF)
        std::cout << " \"" << escape(t->getText()) << "\"";
    std::cout << "\n";
}

void TraceListener::visitErrorNode(tree::ErrorNode *node) {
    std::cout << indent() << color::red("error recovery: \"" + escape(node->getSymbol()->getText()) + "\"") << "\n";
}

// ---------------------------------------------------------------------------
// Errors
// ---------------------------------------------------------------------------
PrettyErrorListener::PrettyErrorListener(const std::string &source, std::string stage) : _stage(std::move(stage)) {
    std::string line;
    std::istringstream in(source);
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        std::replace(line.begin(), line.end(), '\t', ' ');
        _lines.push_back(line);
    }
}

void PrettyErrorListener::syntaxError(Recognizer *, Token *offendingSymbol, size_t line, size_t col,
                                      const std::string &msg, std::exception_ptr) {
    // DiagnosticErrorListener (--diag) reports through this same channel;
    // those are informational, not errors.
    bool note = msg.rfind("report", 0) == 0;
    if (note)
        ++notes;
    else
        ++errors;

    std::cout << (note ? color::yellow("note") : color::red("error")) << color::dim(" [" + _stage + "] ")
              << color::bold(std::to_string(line) + ":" + std::to_string(col)) << "  " << msg << "\n";

    if (line >= 1 && line <= _lines.size()) {
        const std::string &src = _lines[line - 1];
        std::ostringstream num;
        num << std::setw(5) << line;
        std::cout << color::dim(num.str() + " | ") << src << "\n";

        size_t width = 1;
        if (offendingSymbol && offendingSymbol->getType() != Token::EOF &&
            offendingSymbol->getStopIndex() >= offendingSymbol->getStartIndex())
            width = offendingSymbol->getStopIndex() - offendingSymbol->getStartIndex() + 1;
        if (col < src.size())
            width = std::min(width, src.size() - col);
        std::cout << color::dim("      | ") << std::string(col, ' ')
                  << (note ? color::yellow(std::string(width, '^')) : color::red(std::string(width, '^'))) << "\n";
    }
}

// ---------------------------------------------------------------------------
// CLI
// ---------------------------------------------------------------------------
bool parseArgs(int argc, char **argv, Options &opts) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto needValue = [&](std::string &into) {
            if (i + 1 >= argc) {
                std::cerr << "error: " << a << " needs a value\n";
                return false;
            }
            into = argv[++i];
            return true;
        };
        if (a == "-e" || a == "--expr") {
            if (!needValue(opts.inlineText))
                return false;
            opts.hasInlineText = true;
        } else if (a == "-r" || a == "--rule") {
            if (!needValue(opts.rule))
                return false;
        } else if (a == "-i" || a == "--repl")   opts.repl = true;
        else if (a == "-t" || a == "--tokens")   opts.tokens = true;
        else if (a == "--no-tree")               opts.tree = false;
        else if (a == "-l" || a == "--lisp")     opts.lisp = true;
        else if (a == "--trace")                 opts.trace = true;
        else if (a == "--diag")                  opts.diag = true;
        else if (a == "--no-run")                opts.run = false;
        else if (a == "--no-color")              opts.color = false;
        else if (a == "--rules")                 opts.listRules = true;
        else if (a == "-h" || a == "--help")     opts.help = true;
        else if (!a.empty() && a[0] == '-') {
            std::cerr << "error: unknown option " << a << "\n";
            return false;
        } else if (opts.inputFile.empty())
            opts.inputFile = a;
        else {
            std::cerr << "error: more than one input file given (" << opts.inputFile << ", " << a << ")\n";
            return false;
        }
    }
    return true;
}

void printUsage(const std::string &g, const std::vector<std::string> &registeredRules) {
    std::string rules;
    for (size_t i = 0; i < registeredRules.size(); ++i)
        rules += (i ? ", " : "") + registeredRules[i];
    std::cout << "usage: play " << g << " FILE [options]\n"
              << "       play " << g << " -e \"TEXT\" [options]\n"
              << "       play " << g << " --repl [options]\n"
              << "\ninput:\n"
              << "  FILE              file to parse (also looked up inside grammars/" << g << "/)\n"
              << "  -e, --expr TEXT   parse TEXT instead of a file\n"
              << "  -i, --repl        interactive: parse each line you type\n"
              << "\nwhat to show:\n"
              << "  -t, --tokens      token table (lexer output)\n"
              << "      --no-tree     hide the parse tree (shown by default)\n"
              << "  -l, --lisp        also print the tree in LISP form, like antlr4-parse -tree\n"
              << "      --trace       log every rule the parser enters/exits and every token it consumes\n"
              << "      --diag        report ambiguities in the grammar\n"
              << "      --no-run      don't run the action from main.cpp (visitor/listener)\n"
              << "      --no-color    plain output\n"
              << "\nother:\n"
              << "  -r, --rule NAME   start rule (default: " << (rules.empty() ? "first rule" : registeredRules[0])
              << (rules.empty() ? "" : "; registered: " + rules) << ")\n"
              << "      --rules       list the grammar's parser rules and token types\n"
              << "  -h, --help        this message\n";
}

bool readFile(const std::string &path, std::string &out) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    out = ss.str();
    // Drop a UTF-8 byte-order mark (Notepad likes adding one).
    if (out.rfind("\xEF\xBB\xBF", 0) == 0)
        out.erase(0, 3);
    return true;
}

bool loadInput(const Options &opts, std::string &text) {
    if (opts.hasInlineText) {
        text = opts.inlineText;
        return true;
    }
    if (readFile(opts.inputFile, text))
        return true;
    if (const char *dir = std::getenv("PG_GRAMMAR_DIR")) {
        if (readFile(std::string(dir) + "/" + opts.inputFile, text))
            return true;
    }
    std::cerr << color::red("error: ") << "can't read " << opts.inputFile << "\n";
    return false;
}

void initConsole(const Options &opts) {
    enableConsole();
    color::enabled = opts.color && stdoutIsTerminal() && std::getenv("NO_COLOR") == nullptr;
}

} // namespace pg
