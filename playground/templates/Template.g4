// A new grammar. The name after `grammar` must match the file name.
grammar TEMPLATE;

// Parser rules (lowercase): structure built from tokens and other rules.
program : item* EOF ;

item    : ID '=' value ';' ;

value   : NUMBER              # Number
        | STRING              # String
        | '[' value* ']'      # List
        ;

// Lexer rules (UPPERCASE): the "words" of the language.
NUMBER  : [0-9]+ ;
STRING  : '"' ~["\r\n]* '"' ;
ID      : [a-zA-Z_] [a-zA-Z_0-9]* ;
WS      : [ \t\r\n]+ -> skip ;
