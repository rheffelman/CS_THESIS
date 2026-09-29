// Calc: evaluates the parse tree with a VISITOR.
//
// A visitor has one visitX() method per labeled alternative in Calc.g4
// (# Let, # Print, # Add, ...). Each method returns a value (std::any), and
// you decide when to visit children by calling visit(child). That makes
// visitors a natural fit for evaluating expressions bottom-up.

#include <map>
#include <stdexcept>

#include "CalcBaseVisitor.h"
#include "CalcLexer.h"
#include "CalcParser.h"
#include "playground.h"

class Evaluator : public CalcBaseVisitor {
public:
    std::map<std::string, long> vars;

    // 'let' ID '=' expr ';'
    std::any visitLet(CalcParser::LetContext *ctx) override {
        long value = eval(ctx->expr());
        vars[ctx->ID()->getText()] = value;
        return {};
    }

    // 'print' expr ';'
    std::any visitPrint(CalcParser::PrintContext *ctx) override {
        std::cout << eval(ctx->expr()) << "\n";
        return {};
    }

    // '(' expr ')'  - the parentheses only affect the tree's shape; the value is just the inner expr.
    std::any visitParens(CalcParser::ParensContext *ctx) override { return eval(ctx->expr()); }

    // expr '*' expr
    std::any visitMul(CalcParser::MulContext *ctx) override { return eval(ctx->expr(0)) * eval(ctx->expr(1)); }

    // expr '+' expr
    std::any visitAdd(CalcParser::AddContext *ctx) override { return eval(ctx->expr(0)) + eval(ctx->expr(1)); }

    // NUMBER
    std::any visitNum(CalcParser::NumContext *ctx) override { return std::stol(ctx->NUMBER()->getText()); }

    // ID
    std::any visitVar(CalcParser::VarContext *ctx) override {
        std::string name = ctx->ID()->getText();
        auto it = vars.find(name);
        if (it == vars.end())
            throw std::runtime_error("undefined variable '" + name + "' at line " +
                                     std::to_string(ctx->getStart()->getLine()));
        return it->second;
    }

private:
    long eval(antlr4::tree::ParseTree *node) { return std::any_cast<long>(visit(node)); }
};

int main(int argc, char **argv) {
    pg::Playground<CalcLexer, CalcParser> play("Calc");

    // Start rules you can pick with --rule. The first one is the default.
    play.startRule("program", [](CalcParser &p) { return p.program(); });
    play.startRule("expr", [](CalcParser &p) { return p.expr(); });

    // Lives outside the lambda so variables survive between lines in --repl mode.
    Evaluator evaluator;

    play.onTree([&](antlr4::tree::ParseTree *tree, const std::string &rule) {
        std::any result = evaluator.visit(tree);
        if (rule == "expr")
            std::cout << "= " << std::any_cast<long>(result) << "\n";
    });

    return play.run(argc, argv);
}
