import os
import sys
from explore import explore
from antlr4 import CommonTokenStream, FileStream, Token, InputStream
from antlr4.tree.Tree import ErrorNode, TerminalNode

from generated.TestLexer import TestLexer
from generated.TestParser import TestParser
from generated.TestVisitor import TestVisitor

os.system("")
sys.stdout.reconfigure(encoding="utf-8")


def walk(node, depth=0):
    if isinstance(node, TerminalNode):
        print("  " * depth + "token: " + node.getText())
        return
    print("  " * depth + node.parser.ruleNames[node.getRuleIndex()])
    for child in node.getChildren():
        walk(child, depth + 1)

variables = []
def error_check(node):
    def traverse(node):
        if isinstance(node, TerminalNode):
            return
            #if node.symbol.text == "i":
                #explore(node)
                #variables.append(node.parentCtx.children[1].children[0].children[0].symbol.text)
        
        
        if isinstance(node, TestParser.OperandContext):
            if node.ID() is not None:
                variables.append(("var", node.children[0].symbol.text))

        if isinstance(node, TestParser.LetContext):
            variables.append(("let", node.children[1].children[0].children[0].symbol.text))


        for child in node.getChildren():
            traverse(child)
    traverse(node)
    let = []
    for item in variables:
        if item[0] == "let":
            let.append(item[1])
        if item[1] not in let:
            print("error! " + item[0] + " \""+ item[1] + "\" used before it's declared!")
    print(let)

IR = []
LABEL_COUNT = 0
def statements(node):
    if isinstance(node, TerminalNode):
        return

    elif isinstance(node, TestParser.LetContext):
        ID = node.children[1].children[0].children[0].symbol.text
        value = node.children[1].children[2].children[0].symbol.text
        IR.append(("LET", ID, value))

    elif isinstance(node, TestParser.BranchContext):
        ID = node.children[1].children[0].children[0].symbol.text
        IR.append(("BRANCH", ID))

    elif isinstance(node, TestParser.LabelContext):
        ID = node.children[1].children[0].children[0].symbol.text
        IR.append(("LABEL", ID))
        
    elif isinstance(node, TestParser.ExprContext):
        explore(node)
    # elif isinstance(node, TestParser.IfContext):

    for child in node.children:
        statements(child)

def main():
    text = FileStream(sys.argv[1], encoding="utf-8")
    lexer = TestLexer(text)
    tokens = CommonTokenStream(lexer)

    parser = TestParser(tokens)
    tree = parser.program()
    #error_check(tree)
    statements(tree)

    for stmt in IR:
        print(stmt)

if __name__ == "__main__":
    main()
