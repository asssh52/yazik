my language, some info will be later

Грамматика языка:

```
G       ::= CompOp@
CompOp  ::= {OpSep}+
Op      ::= Id '=' Expr | Print | While | Def | If | Ret
Expr    ::= P {[+-*/] P}* 
If      ::= 'skibidi' CompOp 'ohio' Expr 
P       ::= '('Expr')'| Call | Id | Num
Id      ::= [a-z, A-Z][a-z, A-Z, 0-9, _ ]*
Num     ::= 'число'
Sep     ::= '$'
Prnt    ::= 'sigma' Expr
While   ::= 'amogus' CompOp 'imposter' Expr
Def     ::= 'kfc'    CompOp 'xd'       ID   {'whopper' ID}* 
Call    ::= 'wakie wakey' ID {'whopper' Expr}*
Ret     ::= 'banana' Expr
```
