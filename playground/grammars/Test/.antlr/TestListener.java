// Generated from d:/GitHub/CS_THESIS/playground/grammars/Test/Test.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link TestParser}.
 */
public interface TestListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link TestParser#program}.
	 * @param ctx the parse tree
	 */
	void enterProgram(TestParser.ProgramContext ctx);
	/**
	 * Exit a parse tree produced by {@link TestParser#program}.
	 * @param ctx the parse tree
	 */
	void exitProgram(TestParser.ProgramContext ctx);
	/**
	 * Enter a parse tree produced by {@link TestParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterStatement(TestParser.StatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link TestParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitStatement(TestParser.StatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link TestParser#block}.
	 * @param ctx the parse tree
	 */
	void enterBlock(TestParser.BlockContext ctx);
	/**
	 * Exit a parse tree produced by {@link TestParser#block}.
	 * @param ctx the parse tree
	 */
	void exitBlock(TestParser.BlockContext ctx);
	/**
	 * Enter a parse tree produced by {@link TestParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterExpr(TestParser.ExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link TestParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitExpr(TestParser.ExprContext ctx);
	/**
	 * Enter a parse tree produced by {@link TestParser#operand}.
	 * @param ctx the parse tree
	 */
	void enterOperand(TestParser.OperandContext ctx);
	/**
	 * Exit a parse tree produced by {@link TestParser#operand}.
	 * @param ctx the parse tree
	 */
	void exitOperand(TestParser.OperandContext ctx);
	/**
	 * Enter a parse tree produced by {@link TestParser#binaryOperator}.
	 * @param ctx the parse tree
	 */
	void enterBinaryOperator(TestParser.BinaryOperatorContext ctx);
	/**
	 * Exit a parse tree produced by {@link TestParser#binaryOperator}.
	 * @param ctx the parse tree
	 */
	void exitBinaryOperator(TestParser.BinaryOperatorContext ctx);
	/**
	 * Enter a parse tree produced by {@link TestParser#argList}.
	 * @param ctx the parse tree
	 */
	void enterArgList(TestParser.ArgListContext ctx);
	/**
	 * Exit a parse tree produced by {@link TestParser#argList}.
	 * @param ctx the parse tree
	 */
	void exitArgList(TestParser.ArgListContext ctx);
}