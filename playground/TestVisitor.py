# Generated from Test.g4 by ANTLR 4.13.2
from antlr4 import *
if "." in __name__:
    from .TestParser import TestParser
else:
    from TestParser import TestParser

# This class defines a complete generic visitor for a parse tree produced by TestParser.

class TestVisitor(ParseTreeVisitor):

    # Visit a parse tree produced by TestParser#program.
    def visitProgram(self, ctx:TestParser.ProgramContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#ExprStatement.
    def visitExprStatement(self, ctx:TestParser.ExprStatementContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#If.
    def visitIf(self, ctx:TestParser.IfContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#Let.
    def visitLet(self, ctx:TestParser.LetContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#Label.
    def visitLabel(self, ctx:TestParser.LabelContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#Branch.
    def visitBranch(self, ctx:TestParser.BranchContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#Function.
    def visitFunction(self, ctx:TestParser.FunctionContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#block.
    def visitBlock(self, ctx:TestParser.BlockContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#expr.
    def visitExpr(self, ctx:TestParser.ExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#operand.
    def visitOperand(self, ctx:TestParser.OperandContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#binaryOperator.
    def visitBinaryOperator(self, ctx:TestParser.BinaryOperatorContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by TestParser#argList.
    def visitArgList(self, ctx:TestParser.ArgListContext):
        return self.visitChildren(ctx)



del TestParser