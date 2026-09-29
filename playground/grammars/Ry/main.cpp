// Ry: walks the parse tree with a LISTENER.
//
// A listener has enterX()/exitX() methods that ANTLR's ParseTreeWalker calls
// automatically as it walks the whole tree depth-first. You don't control the
// walk and methods don't return values, which makes listeners good for
// "notice things as you go" jobs: collecting names, checking rules, printing
// an outline. (Compare grammars/Calc/main.cpp, which uses a visitor.)

#include <map>
#include <set>

#include "RyBaseListener.h"
#include "RyLexer.h"
#include "RyParser.h"
#include "playground.h"

class Outline : public RyBaseListener {
public:
    // let x = ...;
    void enterLetStmt(RyParser::LetStmtContext *ctx) override {
        std::string name = ctx->ID()->getText();
        declared.insert(name);
        show(ctx, "let    " + name);
    }

    // x = ...;
    void enterAssignStmt(RyParser::AssignStmtContext *ctx) override {
        std::string name = ctx->ID()->getText();
        show(ctx, "assign " + name);
        if (!declared.count(name))
            problems.push_back(where(ctx) + "assignment to '" + name + "' before any `let " + name + "`");
    }

    // label one:
    void enterLabelStmt(RyParser::LabelStmtContext *ctx) override {
        std::string name = ctx->ID()->getText();
        labels[name] = ctx->getStart()->getLine();
        show(ctx, "label  " + name + ":");
    }

    // branch one;
    void enterBranchStmt(RyParser::BranchStmtContext *ctx) override {
        addBranch(ctx, ctx->ID()->getText());
        show(ctx, "branch -> " + ctx->ID()->getText());
    }

    // if (cond) branch one;   or   if (cond) { ... }
    void enterIfStmt(RyParser::IfStmtContext *ctx) override {
        std::string text = "if     (" + ctx->expr()->getText() + ")";
        if (ctx->ID()) {
            addBranch(ctx, ctx->ID()->getText());
            text += " branch -> " + ctx->ID()->getText();
        }
        show(ctx, text);
    }

    // sum = branch add(x, y);  (the expression form of branch)
    void enterExpr(RyParser::ExprContext *ctx) override {
        if (ctx->ID())
            addBranch(ctx, ctx->ID()->getText());
    }

    // { ... } blocks: indent their contents in the outline.
    void enterBlock(RyParser::BlockContext *) override { ++depth; }
    void exitBlock(RyParser::BlockContext *) override { --depth; }

    // Runs last, after the whole tree has been walked.
    void exitProgram(RyParser::ProgramContext *) override {
        for (auto &[target, line] : branches)
            if (!labels.count(target))
                problems.push_back("line " + std::to_string(line) + ": branch to undefined label '" + target + "'");

        std::cout << "\n";
        if (problems.empty()) {
            std::cout << pg::color::green("checks passed") << pg::color::dim(" (labels exist, variables declared)")
                      << "\n";
        } else {
            for (auto &p : problems)
                std::cout << pg::color::red("problem: ") << p << "\n";
        }
    }

private:
    std::set<std::string> declared;
    std::map<std::string, size_t> labels;
    std::vector<std::pair<std::string, size_t>> branches;
    std::vector<std::string> problems;
    int depth = 0;

    static std::string where(antlr4::ParserRuleContext *ctx) {
        return "line " + std::to_string(ctx->getStart()->getLine()) + ": ";
    }
    void addBranch(antlr4::ParserRuleContext *ctx, const std::string &target) {
        branches.emplace_back(target, ctx->getStart()->getLine());
    }
    void show(antlr4::ParserRuleContext *ctx, const std::string &text) {
        std::string line = std::to_string(ctx->getStart()->getLine());
        std::cout << pg::color::dim(std::string(4 - std::min<size_t>(line.size(), 4), ' ') + line + "  ")
                  << std::string(depth * 4, ' ') << text << "\n";
    }
};

int main(int argc, char **argv) {
    pg::Playground<RyLexer, RyParser> play("Ry");

    play.startRule("program", [](RyParser &p) { return p.program(); });
    play.startRule("statement", [](RyParser &p) { return p.statement(); });
    play.startRule("expr", [](RyParser &p) { return p.expr(); });

    play.onTree([](antlr4::tree::ParseTree *tree, const std::string &) {
        Outline outline;  // fresh each run
        antlr4::tree::ParseTreeWalker::DEFAULT.walk(&outline, tree);
    });

    return play.run(argc, argv);
}
