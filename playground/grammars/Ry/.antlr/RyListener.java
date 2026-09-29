// Generated from d:/GitHub/CS_THESIS/playground/grammars/Ry/Ry.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link RyParser}.
 */
public interface RyListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link RyParser#program}.
	 * @param ctx the parse tree
	 */
	void enterProgram(RyParser.ProgramContext ctx);
	/**
	 * Exit a parse tree produced by {@link RyParser#program}.
	 * @param ctx the parse tree
	 */
	void exitProgram(RyParser.ProgramContext ctx);
	/**
	 * Enter a parse tree produced by the {@code letStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterLetStmt(RyParser.LetStmtContext ctx);
	/**
	 * Exit a parse tree produced by the {@code letStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitLetStmt(RyParser.LetStmtContext ctx);
	/**
	 * Enter a parse tree produced by the {@code assignStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterAssignStmt(RyParser.AssignStmtContext ctx);
	/**
	 * Exit a parse tree produced by the {@code assignStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitAssignStmt(RyParser.AssignStmtContext ctx);
	/**
	 * Enter a parse tree produced by the {@code labelStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterLabelStmt(RyParser.LabelStmtContext ctx);
	/**
	 * Exit a parse tree produced by the {@code labelStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitLabelStmt(RyParser.LabelStmtContext ctx);
	/**
	 * Enter a parse tree produced by the {@code branchStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterBranchStmt(RyParser.BranchStmtContext ctx);
	/**
	 * Exit a parse tree produced by the {@code branchStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitBranchStmt(RyParser.BranchStmtContext ctx);
	/**
	 * Enter a parse tree produced by the {@code ifStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterIfStmt(RyParser.IfStmtContext ctx);
	/**
	 * Exit a parse tree produced by the {@code ifStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitIfStmt(RyParser.IfStmtContext ctx);
	/**
	 * Enter a parse tree produced by the {@code returnStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterReturnStmt(RyParser.ReturnStmtContext ctx);
	/**
	 * Exit a parse tree produced by the {@code returnStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitReturnStmt(RyParser.ReturnStmtContext ctx);
	/**
	 * Enter a parse tree produced by the {@code exprStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterExprStmt(RyParser.ExprStmtContext ctx);
	/**
	 * Exit a parse tree produced by the {@code exprStmt}
	 * labeled alternative in {@link RyParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitExprStmt(RyParser.ExprStmtContext ctx);
	/**
	 * Enter a parse tree produced by {@link RyParser#block}.
	 * @param ctx the parse tree
	 */
	void enterBlock(RyParser.BlockContext ctx);
	/**
	 * Exit a parse tree produced by {@link RyParser#block}.
	 * @param ctx the parse tree
	 */
	void exitBlock(RyParser.BlockContext ctx);
	/**
	 * Enter a parse tree produced by {@link RyParser#expr}.
	 * @param ctx the parse tree
	 */
	void enterExpr(RyParser.ExprContext ctx);
	/**
	 * Exit a parse tree produced by {@link RyParser#expr}.
	 * @param ctx the parse tree
	 */
	void exitExpr(RyParser.ExprContext ctx);
	/**
	 * Enter a parse tree produced by {@link RyParser#operand}.
	 * @param ctx the parse tree
	 */
	void enterOperand(RyParser.OperandContext ctx);
	/**
	 * Exit a parse tree produced by {@link RyParser#operand}.
	 * @param ctx the parse tree
	 */
	void exitOperand(RyParser.OperandContext ctx);
	/**
	 * Enter a parse tree produced by {@link RyParser#binOp}.
	 * @param ctx the parse tree
	 */
	void enterBinOp(RyParser.BinOpContext ctx);
	/**
	 * Exit a parse tree produced by {@link RyParser#binOp}.
	 * @param ctx the parse tree
	 */
	void exitBinOp(RyParser.BinOpContext ctx);
	/**
	 * Enter a parse tree produced by {@link RyParser#paramList}.
	 * @param ctx the parse tree
	 */
	void enterParamList(RyParser.ParamListContext ctx);
	/**
	 * Exit a parse tree produced by {@link RyParser#paramList}.
	 * @param ctx the parse tree
	 */
	void exitParamList(RyParser.ParamListContext ctx);
	/**
	 * Enter a parse tree produced by {@link RyParser#argList}.
	 * @param ctx the parse tree
	 */
	void enterArgList(RyParser.ArgListContext ctx);
	/**
	 * Exit a parse tree produced by {@link RyParser#argList}.
	 * @param ctx the parse tree
	 */
	void exitArgList(RyParser.ArgListContext ctx);
	/**
	 * Enter a parse tree produced by {@link RyParser#literal}.
	 * @param ctx the parse tree
	 */
	void enterLiteral(RyParser.LiteralContext ctx);
	/**
	 * Exit a parse tree produced by {@link RyParser#literal}.
	 * @param ctx the parse tree
	 */
	void exitLiteral(RyParser.LiteralContext ctx);
}