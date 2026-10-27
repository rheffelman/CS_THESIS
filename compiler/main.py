import os
import sys
import re
from pathlib import Path
from explore import explore
from antlr4 import CommonTokenStream, FileStream
from antlr4.tree.Tree import TerminalNode

from generated.RyLexer import RyLexer
from generated.RyParser import RyParser

os.system("")
sys.stdout.reconfigure(encoding="utf-8")


def walk(node, depth=0):
    if isinstance(node, TerminalNode):
        print("  " * depth + "token: " + node.getText())
        return
    print("  " * depth + node.parser.ruleNames[node.getRuleIndex()])
    for child in node.getChildren():
        walk(child, depth + 1)

IR = []
LABEL_COUNT = 0
TEMP_COUNT = 0

def new_label():
    global LABEL_COUNT
    name = "_L" + str(LABEL_COUNT)
    LABEL_COUNT += 1
    return name

def new_temp():
    global TEMP_COUNT
    name = "_t" + str(TEMP_COUNT)
    TEMP_COUNT += 1
    return name

def lower_expr(node):
    left = lower_operand(node.operand(0))
    if node.binaryOperator() is None:
        return left
    op = node.binaryOperator().getText()
    right = lower_operand(node.operand(1))
    temp = new_temp()
    IR.append(("OP", temp, left, op, right))
    return temp

def lower_operand(node):
    if node.expr() is not None:  # '(' expr ')'
        return lower_expr(node.expr())
    if node.ID() is not None and node.getChildCount() > 1:  # ID '(' argList? ')'
        name = node.ID().getText()
        args = []
        if node.argList() is not None:
            args = [lower_expr(arg) for arg in node.argList().expr()]
        temp = new_temp()
        IR.append(("CALL", temp, name, args))
        return temp
    return node.getText()  # ID, NUMBER, FLOAT, STRING, or BOOL (getText keeps a leading '-')

def statements(node):
    
    if isinstance(node, TerminalNode):
        return

    elif isinstance(node, RyParser.LetContext):
        expr = node.expr()
        ID = expr.operand(0).getText()
        value = None  # `let x;` with no value
        if expr.binaryOperator() is not None:
            value = lower_operand(expr.operand(1))
        IR.append(("LET", ID, value))
        return

    elif isinstance(node, RyParser.ExprStatementContext):
        expr = node.expr()
        op = expr.binaryOperator()
        if op is not None and op.getText() == "=":
            ID = expr.operand(0).getText()
            value = lower_operand(expr.operand(1))
            IR.append(("ASSIGN", ID, value))
        else:
            lower_expr(expr)  # e.g. print(hw); the result is unused
        return

    elif isinstance(node, RyParser.BranchContext):
        ID = node.children[1].children[0].children[0].symbol.text
        IR.append(("BRANCH", ID))

    elif isinstance(node, RyParser.LabelContext):
        ID = node.children[1].children[0].children[0].symbol.text
        IR.append(("LABEL", ID))

    elif isinstance(node, RyParser.IfContext):
        cond = lower_expr(node.expr())
        end = new_label()
        IR.append(("BRANCH_IF_FALSE", cond, end))
        statements(node.block())
        IR.append(("LABEL", end))
        return

    for child in node.children:
        statements(child)

ARITHMETIC = {'+', '-', '*', '/'}
COMPARISON = {'<', '>', '==', '!='}
LOGICAL = {'and', 'or', 'AND', 'OR'}

def literal_type(value):
    if re.match(r'^-?[0-9]+$', value): return 'int'
    if re.match(r'^-?([0-9]+\.[0-9]*([eE][+-]?[0-9]+)?|\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+)$', value): return 'float'
    if re.match(r'^(true|false)$', value): return 'bool'
    if re.match(r'^".*"$', value): return 'string'
    return None

def op_type(op, left, right):
    numbers = {'int', 'float'}
    if op in ARITHMETIC:
        if left == right == 'int': return 'int'
        if left in numbers and right in numbers: return 'float'
        if op == '+' and left == right == 'string': return 'string'
    elif op in COMPARISON:
        if left == right or (left in numbers and right in numbers): return 'bool'
    elif op in LOGICAL:
        if left == right == 'bool': return 'bool'
    raise TypeError("can't apply " + op + " to " + left + " and " + right)

def type_deduction():
    types = {}  # variable or temp name -> type

    def type_of(value):
        if value in types:
            return types[value]
        t = literal_type(value)
        if t is None:
            raise NameError("\"" + value + "\" has no known type")
        return t

    for i in range(len(IR)):
        stmt = IR[i]
        if stmt[0] == 'OP':
            _, temp, left, op, right = stmt
            types[temp] = op_type(op, type_of(left), type_of(right))
        elif stmt[0] == 'LET' and stmt[2] is not None:
            types[stmt[1]] = type_of(stmt[2])
            IR[i] = stmt + (types[stmt[1]],)
        elif stmt[0] == 'ASSIGN':
            if type_of(stmt[2]) != types.get(stmt[1]):
                raise TypeError("can't assign " + type_of(stmt[2]) + " to " + stmt[1])

    return types

C_TYPES = {'int': 'int', 'float': 'double', 'bool': 'bool', 'string': 'const char *'}
C_OPERATORS = {'and': '&&', 'AND': '&&', 'or': '||', 'OR': '||'}

def c_value(value):
    if literal_type(value) == 'int':
        return str(int(value))  # C reads a leading 0 as octal, so 007 -> 7 and 08 would not compile
    return value

def raise_stmts(types):
    output_string = Path(__file__).with_name("template.c").read_text(encoding="utf-8")

    # every variable and temp is declared at the top of main so a goto never jumps past a declaration
    declarations = [f"{C_TYPES[t]} {name};" for name, t in types.items()]

    code = []
    for stmt in IR:
        if stmt[0] in ('LET', 'ASSIGN'):
            if stmt[2] is not None:  # `let x;` only needs its declaration
                code.append(f"{stmt[1]} = {c_value(stmt[2])};")
        elif stmt[0] == 'OP':
            _, temp, left, op, right = stmt
            if op == '+' and types[temp] == 'string':
                code.append(f"{temp} = concat({c_value(left)}, {c_value(right)});")
            else:
                code.append(f"{temp} = {c_value(left)} {C_OPERATORS.get(op, op)} {c_value(right)};")
        elif stmt[0] == 'CALL':
            _, temp, name, args = stmt
            call = f"{name}({', '.join(c_value(arg) for arg in args)});"
            code.append(f"{temp} = {call}" if temp in types else call)  # untyped temp: result unused, e.g. print
        elif stmt[0] == 'LABEL':
            code.append(f"{stmt[1]}:;")
        elif stmt[0] == 'BRANCH':
            code.append(f"goto {stmt[1]};")
        elif stmt[0] == 'BRANCH_IF_FALSE':
            code.append(f"if (!{stmt[1]}) goto {stmt[2]};")

    output_string = output_string.replace("// @DECLARATIONS", "\n    ".join(declarations))
    output_string = output_string.replace("// @CODE", "\n    ".join(code))
    return output_string

def main():
    text = FileStream(sys.argv[1], encoding="utf-8")
    lexer = RyLexer(text)
    tokens = CommonTokenStream(lexer)

    parser = RyParser(tokens)
    tree = parser.program()

    statements(tree)
    types = type_deduction()
    for stmt in IR:
        print(stmt)
    Path(sys.argv[1]).with_suffix(".c").write_text(raise_stmts(types), encoding="utf-8")

if __name__ == "__main__":
    main()
