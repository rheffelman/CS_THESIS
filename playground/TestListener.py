# Generated from Test.g4 by ANTLR 4.13.2
from antlr4 import *
if "." in __name__:
    from .TestParser import TestParser
else:
    from TestParser import TestParser

# This class defines a complete listener for a parse tree produced by TestParser.
class TestListener(ParseTreeListener):

    # Enter a parse tree produced by TestParser#program.
    def enterProgram(self, ctx:TestParser.ProgramContext):
        pass

    # Exit a parse tree produced by TestParser#program.
    def exitProgram(self, ctx:TestParser.ProgramContext):
        pass


    # Enter a parse tree produced by TestParser#ExprStatement.
    def enterExprStatement(self, ctx:TestParser.ExprStatementContext):
        pass

    # Exit a parse tree produced by TestParser#ExprStatement.
    def exitExprStatement(self, ctx:TestParser.ExprStatementContext):
        pass


    # Enter a parse tree produced by TestParser#If.
    def enterIf(self, ctx:TestParser.IfContext):
        pass

    # Exit a parse tree produced by TestParser#If.
    def exitIf(self, ctx:TestParser.IfContext):
        pass


    # Enter a parse tree produced by TestParser#Let.
    def enterLet(self, ctx:TestParser.LetContext):
        pass

    # Exit a parse tree produced by TestParser#Let.
    def exitLet(self, ctx:TestParser.LetContext):
        pass


    # Enter a parse tree produced by TestParser#Label.
    def enterLabel(self, ctx:TestParser.LabelContext):
        pass

    # Exit a parse tree produced by TestParser#Label.
    def exitLabel(self, ctx:TestParser.LabelContext):
        pass


    # Enter a parse tree produced by TestParser#Branch.
    def enterBranch(self, ctx:TestParser.BranchContext):
        pass

    # Exit a parse tree produced by TestParser#Branch.
    def exitBranch(self, ctx:TestParser.BranchContext):
        pass


    # Enter a parse tree produced by TestParser#Function.
    def enterFunction(self, ctx:TestParser.FunctionContext):
        pass

    # Exit a parse tree produced by TestParser#Function.
    def exitFunction(self, ctx:TestParser.FunctionContext):
        pass


    # Enter a parse tree produced by TestParser#block.
    def enterBlock(self, ctx:TestParser.BlockContext):
        pass

    # Exit a parse tree produced by TestParser#block.
    def exitBlock(self, ctx:TestParser.BlockContext):
        pass


    # Enter a parse tree produced by TestParser#expr.
    def enterExpr(self, ctx:TestParser.ExprContext):
        pass

    # Exit a parse tree produced by TestParser#expr.
    def exitExpr(self, ctx:TestParser.ExprContext):
        pass


    # Enter a parse tree produced by TestParser#operand.
    def enterOperand(self, ctx:TestParser.OperandContext):
        pass

    # Exit a parse tree produced by TestParser#operand.
    def exitOperand(self, ctx:TestParser.OperandContext):
        pass


    # Enter a parse tree produced by TestParser#binaryOperator.
    def enterBinaryOperator(self, ctx:TestParser.BinaryOperatorContext):
        pass

    # Exit a parse tree produced by TestParser#binaryOperator.
    def exitBinaryOperator(self, ctx:TestParser.BinaryOperatorContext):
        pass


    # Enter a parse tree produced by TestParser#argList.
    def enterArgList(self, ctx:TestParser.ArgListContext):
        pass

    # Exit a parse tree produced by TestParser#argList.
    def exitArgList(self, ctx:TestParser.ArgListContext):
        pass



del TestParser