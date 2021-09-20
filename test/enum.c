#include <stdio.h>

enum { _TK_OFFSET = 128,
       CInt, Id, CStr, CChar,
       TkNeq, TkEq, TkGe, TkLe,
       TkAddTo, TkSubFrom, TkInc, TkDec, TkAnd, TkOr, TkLshift, TkRshift,
       Int, Char, Void,
       Break, Cont, Do, Else, Enum, For, If, Return, Sizeof, While,
       Printf, Fopen, Fgetc, Malloc, Memset, Exit,
       Add, Sub, Mul, Div, Rem,
       Mov, Push, Pop, Load, Save,
       Neq, Eq, Gt, Ge, Lt, Le,
       Not, Ret, Jz, Jnz, Jump, Call,
       _BreakStub, _ContStub, };

enum { Undefined, Global, Param, Local, Func, Const, };
enum { EAX = 1, EBX, ECX, EDX, ESP, EBP, IMME, };

int main() {
    printf("CInt is %d\n", CInt);
    printf("Call is %d\n", Call);
    printf("Undefined is %d\n", Undefined);
    printf("Const is %d\n", Const);
    printf("EAX is %d\n", EAX);
    printf("ESP is %d\n", ESP);
    return 0;
}
