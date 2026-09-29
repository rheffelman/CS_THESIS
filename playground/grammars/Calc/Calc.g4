// Calculator with variables. Edit freely - `play Calc ...` regenerates
// and rebuilds automatically whenever this file changes.
grammar Calc;

program : stmt* EOF ;

stmt    : 'let' ID '=' expr ';'    # Let
        | 'print' expr ';'         # Print
        ;

// Earlier alternatives bind tighter: * before +.
expr    : '(' expr ')'             # Parens
        | expr '*' expr            # Mul
        | expr '+' expr            # Add
        | NUMBER                   # Num
        | ID                       # Var
        ;

NUMBER       : [0-9]+ ;
ID           : [a-z]+ ;
LINE_COMMENT : '//' ~[\r\n]* -> skip ;
WS           : [ \t\r\n]+ -> skip ;
