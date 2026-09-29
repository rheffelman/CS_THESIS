#pragma once
// The playground driver: reads input, runs the generated lexer + parser, and
// prints tokens / parse trees / errors. Each grammar's main.cpp creates a
// pg::Playground<XLexer, XParser>, registers start rules and an action, then
// calls run(). See grammars/Calc/main.cpp for an example.

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "antlr4-runtime.h"

namespace pg {

// ---------------------------------------------------------------------------
// Command-line options
// ---------------------------------------------------------------------------
struct Options {
    std::string inputFile;   // positional argument
    std::string inlineText;  // -e "text"
    bool hasInlineText = false;
    bool repl = false;
    std::string rule;        // start rule; empty = default
    bool tokens = false;
    bool tree = true;
    bool lisp = false;
    bool trace = false;
    bool diag = false;
    bool run = true;
    bool color = true;
    bool listRules = false;
    bool help = false;
};

// Returns false on a bad argument (message already printed).
bool parseArgs(int argc, char **argv, Options &opts);
void printUsage(const std::string &grammarName, const std::vector<std::string> &registeredRules);

// ---------------------------------------------------------------------------
// Output helpers
// ---------------------------------------------------------------------------
namespace color {
extern bool enabled;
std::string paint(const std::string &text, const char *code);
inline std::string bold(const std::string &s)    { return paint(s, "1"); }
inline std::string dim(const std::string &s)     { return paint(s, "2"); }
inline std::string red(const std::string &s)     { return paint(s, "31"); }
inline std::string green(const std::string &s)   { return paint(s, "32"); }
inline std::string yellow(const std::string &s) { return paint(s, "33"); }
inline std::string blue(const std::string &s)    { return paint(s, "34"); }
inline std::string magenta(const std::string &s) { return paint(s, "35"); }
inline std::string cyan(const std::string &s)    { return paint(s, "36"); }
} // namespace color

void section(const std::string &title);
std::string escape(const std::string &text);

// Token table: index, position, token type, text.
void printTokens(antlr4::CommonTokenStream &tokens, const antlr4::dfa::Vocabulary &vocab);

// Box-drawing parse tree with rule names, alternative labels, text and positions.
void printTree(antlr4::tree::ParseTree *tree, const std::vector<std::string> &ruleNames,
               const antlr4::dfa::Vocabulary &vocab);

// Lists parser rules and token types of a grammar.
void printGrammarInfo(const std::string &grammarName, const std::vector<std::string> &ruleNames,
                      const antlr4::dfa::Vocabulary &vocab, const std::vector<std::string> &registeredRules);

// Prints errors with the offending source line and a caret underneath.
class PrettyErrorListener : public antlr4::BaseErrorListener {
public:
    PrettyErrorListener(const std::string &source, std::string stage);
    void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                     size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override;
    void setStage(std::string stage) { _stage = std::move(stage); }
    int errors = 0;
    int notes = 0;

private:
    std::vector<std::string> _lines;
    std::string _stage;
};

// --trace: logs, as parsing happens, each rule entered/exited and each token
// consumed, indented by nesting depth.
class TraceListener : public antlr4::tree::ParseTreeListener {
public:
    explicit TraceListener(antlr4::Parser &parser) : _parser(parser) {}
    void enterEveryRule(antlr4::ParserRuleContext *ctx) override;
    void exitEveryRule(antlr4::ParserRuleContext *ctx) override;
    void visitTerminal(antlr4::tree::TerminalNode *node) override;
    void visitErrorNode(antlr4::tree::ErrorNode *node) override;

private:
    std::string indent() const { return std::string(_depth * 2, ' '); }
    antlr4::Parser &_parser;
    int _depth = 0;
};

bool readFile(const std::string &path, std::string &out);

// ---------------------------------------------------------------------------
// Playground
// ---------------------------------------------------------------------------
template <class Lexer, class Parser>
class Playground {
public:
    using StartFn = std::function<antlr4::ParserRuleContext *(Parser &)>;
    using Action = std::function<void(antlr4::tree::ParseTree *tree, const std::string &rule)>;

    explicit Playground(std::string grammarName) : _grammar(std::move(grammarName)) {}

    // Register a start rule you can pick with --rule NAME. The first one
    // registered is the default. Rules you don't register can still be used
    // with --rule; they're parsed by ANTLR's interpreter, so the tree prints
    // fine, but your action (which needs the generated classes) is skipped.
    void startRule(const std::string &name, StartFn fn) {
        if (_rules.empty())
            _defaultRule = name;
        _rules[name] = std::move(fn);
        _ruleOrder.push_back(name);
    }

    // Your code: runs on the parse tree after a successful parse.
    void onTree(Action action) { _action = std::move(action); }

    int run(int argc, char **argv);

private:
    int parseOnce(const std::string &text);
    void repl();
    void withRuleInfo(const std::function<void(const std::vector<std::string> &,
                                               const antlr4::dfa::Vocabulary &)> &fn);

    std::string _grammar;
    std::map<std::string, StartFn> _rules;
    std::vector<std::string> _ruleOrder;
    std::string _defaultRule;
    Action _action;
    Options _opts;
};

void initConsole(const Options &opts);
bool loadInput(const Options &opts, std::string &text);

template <class Lexer, class Parser>
void Playground<Lexer, Parser>::withRuleInfo(
    const std::function<void(const std::vector<std::string> &, const antlr4::dfa::Vocabulary &)> &fn) {
    antlr4::ANTLRInputStream empty("");
    Lexer lexer(&empty);
    antlr4::CommonTokenStream tokens(&lexer);
    Parser parser(&tokens);
    fn(parser.getRuleNames(), parser.getVocabulary());
}

template <class Lexer, class Parser>
int Playground<Lexer, Parser>::run(int argc, char **argv) {
    if (!parseArgs(argc, argv, _opts)) {
        std::cerr << "Run with --help for usage.\n";
        return 2;
    }
    initConsole(_opts);

    if (_opts.help) {
        printUsage(_grammar, _ruleOrder);
        return 0;
    }
    if (_opts.listRules) {
        withRuleInfo([&](auto &rules, auto &vocab) { printGrammarInfo(_grammar, rules, vocab, _ruleOrder); });
        return 0;
    }
    if (_opts.rule.empty()) {
        if (!_defaultRule.empty())
            _opts.rule = _defaultRule;
        else
            withRuleInfo([&](auto &rules, auto &) { _opts.rule = rules.at(0); });
    }
    if (_opts.repl) {
        repl();
        return 0;
    }
    if (_opts.inputFile.empty() && !_opts.hasInlineText) {
        printUsage(_grammar, _ruleOrder);
        return 2;
    }
    std::string text;
    if (!loadInput(_opts, text))
        return 2;
    return parseOnce(text);
}

template <class Lexer, class Parser>
int Playground<Lexer, Parser>::parseOnce(const std::string &text) {
    PrettyErrorListener errors(text, "lexer");

    // 1. LEXER: characters -> tokens
    antlr4::ANTLRInputStream input(text);
    Lexer lexer(&input);
    lexer.removeErrorListeners();
    lexer.addErrorListener(&errors);
    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();

    if (_opts.tokens) {
        section("tokens");
        printTokens(tokens, lexer.getVocabulary());
    }

    // 2. PARSER: tokens -> parse tree
    errors.setStage("parser");
    Parser parser(&tokens);
    std::unique_ptr<antlr4::ParserInterpreter> interpreter;
    antlr4::DiagnosticErrorListener diagnostics(true);
    std::unique_ptr<TraceListener> trace;

    auto configure = [&](antlr4::Parser &p) {
        p.removeErrorListeners();
        p.addErrorListener(&errors);
        if (_opts.diag) {
            p.addErrorListener(&diagnostics);
            p.template getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode(
                antlr4::atn::PredictionMode::LL_EXACT_AMBIG_DETECTION);
        }
        if (_opts.trace) {
            trace = std::make_unique<TraceListener>(p);
            p.addParseListener(trace.get());
        }
    };

    if (_opts.trace)
        section("trace (rule enter/exit, token consumption)");

    antlr4::ParserRuleContext *tree = nullptr;
    bool typedTree = false;
    auto registered = _rules.find(_opts.rule);
    if (registered != _rules.end()) {
        configure(parser);
        tree = registered->second(parser);
        typedTree = true;
        parser.removeParseListeners();
    } else {
        const auto &names = parser.getRuleNames();
        auto it = std::find(names.begin(), names.end(), _opts.rule);
        if (it == names.end()) {
            std::cerr << color::red("error: ") << "no rule named '" << _opts.rule << "'. Rules in " << _grammar
                      << ": ";
            for (size_t i = 0; i < names.size(); ++i)
                std::cerr << (i ? ", " : "") << names[i];
            std::cerr << "\n";
            return 2;
        }
        interpreter = std::make_unique<antlr4::ParserInterpreter>(
            parser.getGrammarFileName(), parser.getVocabulary(), names, parser.getATN(), &tokens);
        configure(*interpreter);
        tree = interpreter->parse(static_cast<size_t>(it - names.begin()));
        interpreter->removeParseListeners();
    }

    const auto &ruleNames = parser.getRuleNames();
    if (_opts.tree) {
        section("parse tree (start rule: " + _opts.rule + ")");
        printTree(tree, ruleNames, parser.getVocabulary());
    }
    if (_opts.lisp) {
        section("LISP-style tree");
        std::cout << tree->toStringTree(interpreter ? static_cast<antlr4::Parser *>(interpreter.get()) : &parser)
                  << "\n";
    }

    if (errors.errors > 0) {
        std::cout << "\n"
                  << color::red(std::to_string(errors.errors) + " syntax error(s)")
                  << (_opts.run && _action ? color::dim(" - skipping the run step") : "") << "\n";
        return 1;
    }

    // 3. YOUR CODE: walk the tree
    if (_opts.run && _action) {
        if (!typedTree) {
            std::cout << "\n"
                      << color::dim("(rule '" + _opts.rule +
                                    "' isn't registered in main.cpp, so it was parsed by ANTLR's generic "
                                    "interpreter: no # labels in the tree, and the action was skipped)")
                      << "\n";
        } else {
            section("run");
            try {
                _action(tree, _opts.rule);
            } catch (const std::exception &ex) {
                std::cout << color::red("runtime error: ") << ex.what() << "\n";
                return 1;
            }
        }
    }
    return 0;
}

template <class Lexer, class Parser>
void Playground<Lexer, Parser>::repl() {
    std::cout << color::bold(_grammar + " REPL") << color::dim("  (start rule: " + _opts.rule + ")") << "\n"
              << color::dim("Each line is parsed on its own. Commands: :tokens :tree :lisp :trace :diag :run "
                            "(toggle), :rule NAME, :rules, :q") << "\n";
    std::string line;
    while (true) {
        std::cout << "\n" << color::cyan(_grammar + "> ") << std::flush;
        if (!std::getline(std::cin, line))
            break;
        if (line.empty())
            continue;
        if (line[0] == ':') {
            auto toggle = [&](const char *name, bool &flag) {
                flag = !flag;
                std::cout << name << (flag ? " on" : " off") << "\n";
            };
            if (line == ":q" || line == ":quit") break;
            else if (line == ":tokens") toggle("tokens", _opts.tokens);
            else if (line == ":tree")   toggle("tree", _opts.tree);
            else if (line == ":lisp")   toggle("lisp", _opts.lisp);
            else if (line == ":trace")  toggle("trace", _opts.trace);
            else if (line == ":diag")   toggle("diag", _opts.diag);
            else if (line == ":run")    toggle("run", _opts.run);
            else if (line == ":rules")
                withRuleInfo([&](auto &rules, auto &vocab) { printGrammarInfo(_grammar, rules, vocab, _ruleOrder); });
            else if (line.rfind(":rule ", 0) == 0) {
                _opts.rule = line.substr(6);
                std::cout << "start rule: " << _opts.rule << "\n";
            } else
                std::cout << "unknown command " << line << "\n";
            continue;
        }
        parseOnce(line);
    }
}

} // namespace pg
