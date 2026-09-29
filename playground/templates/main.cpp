#include "TEMPLATEBaseVisitor.h"
#include "TEMPLATELexer.h"
#include "TEMPLATEParser.h"
#include "playground.h"

// Your code goes here. Override one visitX() method per rule (or per # label)
// you care about; anything you don't override just visits its children.
// Look at build/generated/TEMPLATE/TEMPLATEParser.h to see the generated
// context classes and their accessor methods.
class MyVisitor : public TEMPLATEBaseVisitor {
public:
    // item : ID '=' value ';'
    std::any visitItem(TEMPLATEParser::ItemContext *ctx) override {
        std::cout << "found item " << ctx->ID()->getText() << " = " << ctx->value()->getText() << "\n";
        return visitChildren(ctx);
    }
};

int main(int argc, char **argv) {
    pg::Playground<TEMPLATELexer, TEMPLATEParser> play("TEMPLATE");

    // Start rules you can pick with --rule (first one = default).
    play.startRule("program", [](TEMPLATEParser &p) { return p.program(); });
    play.startRule("value", [](TEMPLATEParser &p) { return p.value(); });

    play.onTree([](antlr4::tree::ParseTree *tree, const std::string &rule) {
        MyVisitor visitor;
        visitor.visit(tree);
    });

    return play.run(argc, argv);
}
