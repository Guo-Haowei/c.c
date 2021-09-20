//< This project is inspired by C4
//< It consists of three passes: lexer, code generation and virtual machine
//<     C source code is parsed into a list of tokens
//<     and then feed to the parser, AST will not be generated,
//<     a set of x86 like instructions and executed on virtual machine
//< Keywords such as for, do while, switch case are not supported
//< It only supports char, int, and pointer types
//< structs are not supported, enum + int array is sufficient enough
#ifndef PREPROC
#include <stdlib.h>
#include <stdio.h>
#endif

#define IS_LETTER(C) ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z'))
#define IS_DIGIT(C) (C >= '0' && C <= '9')
#define IS_HEX(C) (IS_DIGIT(C) || (C >= 'A' && C <= 'F'))
#define IS_WHITESPACE(C) (C == ' ' || C == 9 || C == 10 || C == 13)
#define IS_PUNCT(P, A, B) (*P == A && P[1] == B)
#define IS_TYPE(KIND) (KIND >= Int && KIND <= Void)
#define ALIGN(x) ((x + 3) & -4)
#define COMPILE_ERROR(...) { printf(__VA_ARGS__); exit(1); }
#define PUSH(REG, VAL) instruction(Push | (REG << 24), VAL)
#define POP(REG) instruction(Pop | (REG << 8), 0)
#define MOV(DEST, SRC, IMME) instruction(Mov | (DEST << 8) | (SRC << 24), IMME)
#define ADD(DEST, SRC1, SRC2, VAL) instruction(Add | (DEST << 8) | (SRC1 << 16) | (SRC2 << 24), (VAL))
#define SUB(DEST, SRC1, SRC2, VAL) instruction(Sub | (DEST << 8) | (SRC1 << 16) | (SRC2 << 24), (VAL))
#define MUL(DEST, SRC1, SRC2, VAL) instruction(Mul | (DEST << 8) | (SRC1 << 16) | (SRC2 << 24), (VAL))
#define SAVEW(DEST, SRC) instruction(Save | (DEST << 8) | (SRC << 16), 4);
#define CALL(ENTRY) instruction(Call, ENTRY);
#define LOADB(DEST, SRC) instruction(Load | (DEST << 8) | (SRC << 16), 1)
#define LOADW(DEST, SRC) instruction(Load | (DEST << 8) | (SRC << 16), 4)
#define CHAR_PTR (0xFF0000 | Char)
#define VOID_PTR (0xFF0000 | Void)
#define IS_PTR(TYPE) (0xFF0000 & TYPE)
#define TK_ATTRIB(IDX, ATTRIB) g_tks[((IDX) * TokenSize) + ATTRIB]
#define SYM_ATTRIB(IDX, ATTRIB) g_syms[((IDX) * SymSize) + ATTRIB]
#define OP_ATTRIB(IDX, ATTRIB) g_ops[((IDX) * OpSize) + ATTRIB]
#define CALL_ATTRIB(IDX, ATTRIB) g_calls[((IDX) * CallSize) + ATTRIB]
#define OP(op, dest, src1, src2) ((op) | (dest << 8) | (src1 << 16) | (src2 << 24))

#define MAX_PRINF_ARGS 8
#define CHUNK_SIZE (1 << 27)
#define MAX_SCOPE 128
#define MAX_CALLS 1024

enum { _TkOffset = 128,
       CInt, Id, CStr, CChar,
       TkNeq, TkEq, TkGe, TkLe,
       TkAddTo, TkSubFrom, TkInc, TkDec, TkAnd, TkOr, LShift, RShift,
       _KeywordStart, Int, Char, Void,
       Break, Cont, Else, Enum, If, Return, While,
       Printf, Fopen, Fgetc, Malloc, Memset, Exit,
       _KeywordEnd,
       Add, Sub, Mul, Div, Rem,
       Mov, Push, Pop, Load, Save,
       Neq, Eq, Gt, Ge, Lt, Le, And, Or,
       Not, Ret, Jz, Jnz, Jump, Call,
       _BreakStub, _ContStub };
enum { Undefined, Global, Param, Local, Func, Const };
enum { EAX = 1, EBX, ECX, EDX, ESP, EBP, IMME };
enum { Kind, Value, Ln, Start, End, TokenSize };
enum { TkIdx, Scope, DType, Storage, Address, SymSize };
enum { OpCode, Imme, OpSize };
enum { /* TkIndex = 0, */ InsIdx = 1, CallSize };

char *g_ram, *g_src;
int g_reserved, g_bss,
    *g_tks, g_tkCnt, g_tkIter,
    *g_syms, g_symCnt,
    *g_ops, g_opCnt,
    *g_regs,
    g_entry,
    g_scopeId, *g_scopes, g_scopeCnt,
    *g_calls, g_callCnt;

void panic(char* fmt) {
    printf("[panic] %s\n", fmt);
    exit(1);
}

int streq(char* p1, char* p2, int len) {
    while (len--) {
        if (*p1 == 0 || *p2 == 0) return 1;
        if (*p1 != *p2) return 0;
        p1 += 1; p2 += 1;
    }
    return 1;
}

int strlen(char* p) {
    int len = 0;
    while (*p++) { len += 1; }
    return len;
}

void lex() {
    int ln = 1, range = _KeywordEnd - _KeywordStart - 1;
    char *p = g_src;
    while (*p) {
        if (*p == '#' || (*p == '/' && p[1] == '/')) { while (*p && *p != 10) ++p; }
        else if (IS_WHITESPACE(*p)) { ln += (*p == 10); ++p; }
        else {
            TK_ATTRIB(g_tkCnt, Ln) = ln;
            TK_ATTRIB(g_tkCnt, Start) = p;

            if (IS_LETTER(*p) || *p == '_') {
                TK_ATTRIB(g_tkCnt, Kind) = Id;
                ++p; while (IS_LETTER(*p) || IS_DIGIT(*p) || *p == '_') { ++p; }
                TK_ATTRIB(g_tkCnt, End) = p;
                char *keywords = "int\0     char\0    void\0    break\0   continue\0"
                                 "else\0    enum\0    if\0      return\0  while\0   "
                                 "printf\0  fopen\0   fgetc\0   malloc\0  memset\0  "
                                 "exit\0    ";
                int i = 0, start = TK_ATTRIB(g_tkCnt, Start), len = TK_ATTRIB(g_tkCnt, End) - start;
                while (i < range) {
                    char* kw = keywords + (i * 9);
                    int kwlen = strlen(kw);
                    if (kwlen == len && streq(start, kw, 8)) {
                        TK_ATTRIB(g_tkCnt, Kind) = _KeywordStart + i + 1;
                        break;
                    }
                    ++i;
                }
                g_tkCnt += 1;
            } else if (*p == '0' && p[1] == 'x') {
                TK_ATTRIB(g_tkCnt, Kind) = CInt;
                int result = 0;
                p += 2; while(IS_HEX(*p)) {
                    result = (result << 4) + ((*p < 'A') ? (*p - '0') : (*p - 55));
                    ++p;
                }
                TK_ATTRIB(g_tkCnt, Value) = result;
                TK_ATTRIB(g_tkCnt++, End) = p;
            } else if (IS_DIGIT(*p)) {
                TK_ATTRIB(g_tkCnt, Kind) = CInt;
                int result = 0;
                while (IS_DIGIT(*p)) { result = result * 10 + (*p - '0'); ++p; }
                TK_ATTRIB(g_tkCnt, Value) = result;
                TK_ATTRIB(g_tkCnt++, End) = p;
            } else if (*p == '"') {
                TK_ATTRIB(g_tkCnt, Kind) = CStr;
                ++p; while (*p != '"') { ++p; };
                TK_ATTRIB(g_tkCnt++, End) = ++p;
            } else if (*p == 39) { // ascii '''
                TK_ATTRIB(g_tkCnt, Kind) = CChar;
                TK_ATTRIB(g_tkCnt, Value) = p[1];
                TK_ATTRIB(g_tkCnt++, End) = (p += 3);
            } else {
                TK_ATTRIB(g_tkCnt, Kind) = *p;

                if (IS_PUNCT(p, '=', '=')) { TK_ATTRIB(g_tkCnt, Kind) = TkEq; ++p; }
                else if (IS_PUNCT(p, '!', '=')) { TK_ATTRIB(g_tkCnt, Kind) = TkNeq; ++p; }
                else if (IS_PUNCT(p, '&', '&')) { TK_ATTRIB(g_tkCnt, Kind) = TkAnd; ++p; }
                else if (IS_PUNCT(p, '|', '|')) { TK_ATTRIB(g_tkCnt, Kind) = TkOr; ++p; }
                else if (*p == '+') {
                    if (p[1] == '+') { TK_ATTRIB(g_tkCnt, Kind) = TkInc; ++p; }
                    else if (p[1] == '=') { TK_ATTRIB(g_tkCnt, Kind) = TkAddTo; ++p; }
                } else if (*p == '-') {
                    if (p[1] == '-') { TK_ATTRIB(g_tkCnt, Kind) = TkDec; ++p; }
                    else if (p[1] == '=') { TK_ATTRIB(g_tkCnt, Kind) = TkSubFrom; ++p; }
                } else if (*p == '>') {
                    if (p[1] == '=') { TK_ATTRIB(g_tkCnt, Kind) = TkGe; ++p; }
                    else if (p[1] == '>') { TK_ATTRIB(g_tkCnt, Kind) = RShift; ++p; }
                } else if (*p == '<') {
                    if (p[1] == '=') { TK_ATTRIB(g_tkCnt, Kind) = TkLe; ++p; }
                    else if (p[1] == '<') { TK_ATTRIB(g_tkCnt, Kind) = LShift; ++p; }
                }

                TK_ATTRIB(g_tkCnt++, End) = ++p;
            }
        }
    }
    return;
}

// debug
void dump_tokens() {
    printf("-------- lex --------\n");
    int indent = 0, i = 0, ln = 0;
    while (i < g_tkCnt) {
        int tkln = TK_ATTRIB(i, Ln);
        int kind = TK_ATTRIB(i, Kind);
        int start = TK_ATTRIB(i, Start);
        int end = TK_ATTRIB(i, End);
        int len = end - start;
        if (kind == '{') { indent += 1; }
        else if (kind == '}') { indent -= 1; }
        if (ln != tkln) {
            printf("\n%-3d:%.*s", tkln, indent * 4, "                                        ");
            ln = tkln;
        }
        char* names = "Int   Char  Void  Break Cont  Else  Enum  If    "
                      "Ret   While Print Fopen Fgetc MallocMemsetExit  ";
        printf("%.*s", len, start);
        if (kind >= Int) {
            printf("{");
            char *p = names + 6 * (kind - Int); int ii = 0;
            while (ii < 6) {
                if (*p == ' ') break;
                printf("%c", *p);
                ++ii; ++p;
            }
            printf("}");
        }
        printf(" ");
        ++i;
    }
    printf("\n");
    return;
}

void enter_scope() {
    if (g_scopeCnt >= MAX_SCOPE) {
        panic("scope overflow");
    }

    g_scopes[g_scopeCnt++] = ++g_scopeId;
    return;
}

void exit_scope() {
    if (g_scopeCnt <= 0) {
        panic("scope overflow");
    }

    int i = g_symCnt - 1;
    while (SYM_ATTRIB(i, Scope) == g_scopes[g_scopeCnt - 1]) {
        --g_symCnt, --i;
    }
    
    --g_scopeCnt;
    return;
}

int expect(int kind) {
    if (TK_ATTRIB(g_tkIter, Kind) != kind) {
        int start = TK_ATTRIB(g_tkIter, Start), end = TK_ATTRIB(g_tkIter, End);
        COMPILE_ERROR("error:%d: expected token '%c'(%d), got '%.*s'\n",
            TK_ATTRIB(g_tkIter, Ln), kind < 128 ? kind : ' ', kind, end - start, start);
    }
    return g_tkIter++;
}

int expect_type() {
    int base_type = TK_ATTRIB(g_tkIter, Kind);
    if (IS_TYPE(base_type)) {
        ++g_tkIter;
        int ptr = 0;
        while (TK_ATTRIB(g_tkIter, Kind) == '*') { ptr = (ptr << 8) | 0xFF; ++g_tkIter; }
        return (ptr << 16) | base_type;
    }

    int start = TK_ATTRIB(g_tkIter, Start), end = TK_ATTRIB(g_tkIter, End);
    COMPILE_ERROR("error:%d: expected type specifier, got '%.*s'\n", TK_ATTRIB(g_tkIter, Ln), end - start, start);
}

void instruction(int op, int imme) {
    OP_ATTRIB(g_opCnt, OpCode) = op;
    OP_ATTRIB(g_opCnt, Imme) = imme;
    ++g_opCnt;
    return;
}

int primary_expr() {
    int tkIdx = g_tkIter++;
    char* start = TK_ATTRIB(tkIdx, Start);
    char* end = TK_ATTRIB(tkIdx, End);
    int ln = TK_ATTRIB(tkIdx, Ln);
    int kind = TK_ATTRIB(tkIdx, Kind);
    int value = TK_ATTRIB(tkIdx, Value);
    int len = end - start;
    if (kind == CInt || kind == CChar) {
        MOV(EAX, IMME, value);
        return Int;
    }
    
    if (kind == CStr) {
        MOV(EAX, IMME, g_bss);
        while (1) {
            len = len - 1;
            int i = 1;
            while (i < len) {
                int c = start[i];
                if (c == 92) { // '\'
                    c = start[i += 1];
                    if (c == 'n') { c = 10; }
                    else if (c == '0') { c = 0; }
                    else { COMPILE_ERROR("error:%d: unknown escape sequence '%c'\n", ln, c); }
                }
                *((char*)g_bss++) = c;
                ++i;
            }

            if (TK_ATTRIB(g_tkIter, Kind) != CStr) break;
            start = TK_ATTRIB(g_tkIter, Start);
            end = TK_ATTRIB(g_tkIter, End);
            ln = TK_ATTRIB(g_tkIter, Ln);
            len = end - start;
            ++g_tkIter;
        }
        
        *((char*)g_bss++) = 0;
        g_bss = ALIGN(g_bss);
        return CHAR_PTR;
    }

    if (kind == '(') {
        int data_type = expr();
        expect(')');
        return data_type;
    }
    
    if (kind == Id) {
        if (TK_ATTRIB(g_tkIter, Kind) == '(') {
            ++g_tkIter;
            int argc = 0;
            while (TK_ATTRIB(g_tkIter, Kind) != ')') {
                if (argc > 0) expect(',');
                assign_expr();
                PUSH(EAX, 0);
                ++argc;
            }

            CALL_ATTRIB(g_callCnt, TkIdx) = tkIdx;
            CALL_ATTRIB(g_callCnt++, InsIdx) = g_opCnt;

            CALL(0);
            if (argc) { ADD(ESP, ESP, IMME, argc << 2); }
            expect(')');
            return Int;
        }

        int address = 0, type = Undefined, data_type = 0, i = g_symCnt - 1;
        while (i >= 0) {
            int tmp = SYM_ATTRIB(i, TkIdx);
            char *tmpstart = TK_ATTRIB(tmp, Start), *tmpend = TK_ATTRIB(tmp, End);
            if (len == (tmpend - tmpstart) && streq(start, tmpstart, len)) {
                address = SYM_ATTRIB(i, Address);
                type = SYM_ATTRIB(i, Storage);
                data_type = SYM_ATTRIB(i, DType);
                break;
            }
            --i;
        }

        if (type == Global) {
            MOV(EDX, IMME, address);
            LOADW(EAX, EDX);
            return data_type;
        }
        if (type == Const) {
            MOV(EAX, IMME, address);
            return Int;
        }
        if (type == Undefined) {
            COMPILE_ERROR("error:%d: '%.*s' undeclared\n", ln, len, start);
        }

        SUB(EDX, EBP, IMME, address);
        LOADW(EAX, EDX);
        return data_type;
    }

    if (kind == Printf) {
        expect('(');
        int argc = 0;
        while (TK_ATTRIB(g_tkIter, Kind) != ')') {
            if (argc > 0) expect(',');
            assign_expr();
            PUSH(EAX, 0);
            ++argc;
        }
        if (argc > MAX_PRINF_ARGS) panic("printf supports at most %d args");
        SUB(ESP, ESP, IMME, (MAX_PRINF_ARGS - argc) << 2);
        instruction(Printf, argc);
        ADD(ESP, ESP, IMME, MAX_PRINF_ARGS << 2);
        expect(')');
        return Int;
    }

    int paramCnt = 0, ret = Void, i = 0;
    if (kind == Fopen) { paramCnt = 2; ret = VOID_PTR; }
    else if (kind == Fgetc) { paramCnt = 1; ret = Int; }
    else if (kind == Malloc) { paramCnt = 1; ret = VOID_PTR; }
    else if (kind == Exit) { paramCnt = 1; ret = Void; }
    else { COMPILE_ERROR("error:%d: expected expression, got '%.*s'\n", ln, len, start); }

    expect('(');
    while (i < paramCnt) {
        if (i++ > 0) { expect(','); }
        assign_expr();
        PUSH(EAX, 0);
    }
    instruction(kind, 0);
    ADD(ESP, ESP, IMME, 4 * paramCnt);
    expect(')');
    return ret;
}

int post_expr() {
    int data_type = primary_expr();
    while (1) {
        int kind = TK_ATTRIB(g_tkIter, Kind);
        int ln = TK_ATTRIB(g_tkIter, Ln);
        if (kind == '[') {
            ++g_tkIter;
            if (!IS_PTR(data_type)) {
                COMPILE_ERROR("error:%d: attempted to dereference a non-pointer type 0x%X\n", ln, data_type);
            }
            PUSH(EAX, 0);
            assign_expr();
            int is_charptr = data_type == CHAR_PTR;
            if (!is_charptr) {
                MUL(EAX, EAX, IMME, 4);
            }
            POP(EBX);
            ADD(EAX, EBX, EAX, 0);
            MOV(EDX, EAX, 0);
            if (is_charptr) { LOADB(EAX, EAX); }
            else { LOADW(EAX, EAX); }
            expect(']');
            data_type = ((data_type >> 8) & 0xFF0000) | ((data_type & 0xFFFF));
        } else if (kind == TkInc || kind == TkDec) {
            ++g_tkIter;
            LOADW(EAX, EDX);
            MOV(EBX, EAX, 0);
            int value = (IS_PTR(data_type) && data_type != CHAR_PTR) ? 4 : 1;
            int op = kind == TkInc ? Add : Sub;
            instruction(OP(op, EBX, EBX, IMME), value);
            SAVEW(EDX, EBX);
        } else {
            break;
        }
    }
    return data_type;
}

int unary_expr() {
    int kind = TK_ATTRIB(g_tkIter, Kind);
    int ln = TK_ATTRIB(g_tkIter, Ln);
    if (kind == '!') {
        ++g_tkIter;
        int data_type = unary_expr();
        instruction(OP(Not, EAX, 0, 0), 0);
        return data_type;
    }
    if (kind == '+') {
        ++g_tkIter;
        return unary_expr();
    }
    if (kind == '-') {
        ++g_tkIter;
        int data_type = unary_expr();
        MOV(EBX, IMME, 0);
        SUB(EAX, EBX, EAX, 0);
        return data_type;
    }
    if (kind == '*') {
        ++g_tkIter;
        int data_type = unary_expr();
        if (!IS_PTR(data_type)) {
            COMPILE_ERROR("error:%d: attempted to dereference a non-pointer type 0x%X\n", ln, data_type);
        }

        MOV(EDX, EAX, 0);
        if (data_type == CHAR_PTR) LOADB(EAX, EDX);
        else LOADW(EAX, EDX);
        return ((data_type >> 8) & 0xFF0000) | (0xFFFF & data_type);
    }
    if (kind == TkInc || kind == TkDec) {
        ++g_tkIter;
        int data_type = unary_expr();
        LOADW(EAX, EDX);
        int value = (IS_PTR(data_type) && data_type != CHAR_PTR) ? 4 : 1;
        int op = kind == TkInc ? Add : Sub;
        instruction(OP(op, EAX, EAX, IMME), value);
        SAVEW(EDX, EAX);
        return data_type;
    }
    return post_expr();
}

int cast_expr() {
    if (TK_ATTRIB(g_tkIter, Kind) == '(') {
        int kind = TK_ATTRIB(g_tkIter + 1, Kind);
        if (IS_TYPE(kind)) {
            ++g_tkIter; // skip '('
            int data_type = expect_type();
            expect(')');
            cast_expr();
            return data_type;
        }
    }

    return unary_expr();
}

int mul_expr() {
    int data_type = cast_expr();
    while (1) {
        int kind = TK_ATTRIB(g_tkIter, Kind), opcode;
        if (kind == '*') opcode = Mul;
        else if (kind == '/') opcode = Div;
        else if (kind == '%') opcode = Rem;
        else break;
        ++g_tkIter;
        PUSH(EAX, 0);
        cast_expr();
        POP(EBX);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }

    return data_type;
}

int add_expr() {
    int data_type = mul_expr();
    while (1) {
        int kind = TK_ATTRIB(g_tkIter, Kind), opcode;
        if (kind == '+') opcode = Add;
        else if (kind == '-') opcode = Sub;
        else break;
        ++g_tkIter;
        PUSH(EAX, 0);
        int rhs = mul_expr();
        if (IS_PTR(data_type) && IS_PTR(rhs)) {
            if (data_type != rhs) {
                COMPILE_ERROR("error:%d: type mismatch", TK_ATTRIB(g_tkIter, Ln));
            }
            if (data_type != CHAR_PTR) {
                panic("TODO: handle subtraction other than char* - char*");
            }
            data_type = Int;
        }
        POP(EBX);
        if (IS_PTR(data_type) && data_type != CHAR_PTR) { MUL(EAX, EAX, IMME, 4); }
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }

    return data_type;
}

int shift_expr() {
    int data_type = add_expr();
    while (1) {
        int kind = TK_ATTRIB(g_tkIter, Kind);
        if (kind != LShift && kind != RShift) break;
        ++g_tkIter;
        PUSH(EAX, 0);
        add_expr();
        POP(EBX);
        instruction(OP(kind, EAX, EBX, EAX), 0);
    }
    return data_type;
}


int relation_expr() {
    int data_type = shift_expr();
    while (1) {
        int kind = TK_ATTRIB(g_tkIter, Kind), opcode;
        if (kind == TkNeq) opcode = Neq;
        else if (kind == TkEq) opcode = Eq;
        else if (kind == '<') opcode = Lt;
        else if (kind == '>') opcode = Gt;
        else if (kind == TkGe) opcode = Ge;
        else if (kind == '<') opcode = Lt;
        else if (kind == TkLe) opcode = Le;
        else break;
        ++g_tkIter;
        PUSH(EAX, 0);
        shift_expr();
        POP(EBX);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }
    return data_type;
}

int bit_expr() {
    int data_type = relation_expr();
    while (1) {
        int kind = TK_ATTRIB(g_tkIter, Kind), opcode;
        if (kind == '&') opcode = And;
        else if (kind == '|') opcode = Or;
        else break;
        ++g_tkIter;
        PUSH(EAX, 0);
        relation_expr();
        POP(EBX);
        instruction(OP(opcode, EAX, EBX, EAX), 0);
    }
    return data_type;
}

int logical_expr() {
    int data_type = bit_expr();
    while (1) {
        int kind = TK_ATTRIB(g_tkIter, Kind), opcode;
        if (kind == TkAnd) opcode = Jz;
        else if (kind == TkOr) opcode = Jnz;
        else break;

        ++g_tkIter;
        int skip = g_opCnt;
        instruction(opcode, 0);
        bit_expr();
        OP_ATTRIB(skip, Imme) = g_opCnt;
        continue;
    }

    return data_type;
}

int assign_expr() {
    int data_type = logical_expr();
    while (1) {
        int kind = TK_ATTRIB(g_tkIter, Kind);
        if (kind == '=') {
            ++g_tkIter;
            PUSH(EDX, 0);
            logical_expr();
            POP(EDX);
            instruction(OP(Save, EDX, EAX, 0), data_type == Char ? 1 : 4);
            continue;
        }

        if (kind == TkAddTo) {
            ++g_tkIter;
            PUSH(EDX, 0);
            relation_expr();
            POP(EDX);
            LOADW(EBX, EDX);
            if (IS_PTR(data_type) && data_type != CHAR_PTR) { MUL(EAX, EAX, IMME, 4); }
            ADD(EAX, EBX, EAX, 0);
            SAVEW(EDX, EAX);
            continue;
        }

        if (kind == TkSubFrom) {
            ++g_tkIter;
            PUSH(EDX, 0);
            relation_expr();
            POP(EDX);
            LOADW(EBX, EDX);
            if (IS_PTR(data_type) && data_type != CHAR_PTR) { MUL(EAX, EAX, IMME, 4); }
            SUB(EAX, EBX, EAX, 0);
            SAVEW(EDX, EAX);
            continue;
        }

        if (kind == '?') {
            ++g_tkIter;
            int goto_L1 = g_opCnt;
            instruction(Jz, 0);
            int lhs = expr();
            expect(':');
            int goto_L2 = g_opCnt;
            instruction(Jump, g_opCnt + 1);
            OP_ATTRIB(goto_L1, Imme) = g_opCnt;
            int rhs = assign_expr();
            OP_ATTRIB(goto_L2, Imme) = g_opCnt;
            continue;
        }

        break;
    }

    return data_type;
}

int expr() {
    int type = assign_expr();
    while (TK_ATTRIB(g_tkIter, Kind) == ',') {
        g_tkIter += 1;
        type = assign_expr();
    }
    return type;
}

void stmt() {
    int kind = TK_ATTRIB(g_tkIter, Kind);
    if (kind == Return) {
        if (TK_ATTRIB(++g_tkIter, Kind) != ';') { assign_expr(); }
        MOV(ESP, EBP, 0);
        POP(EBP);
        instruction(Ret, 0);
        expect(';');
        return;
    }

    if (kind == If) {
        //     eax == 0; goto L1 |     eax == 0; goto L1
        //     ...               |     ...
        //     goto L2           | L1: ...
        // L1: ...               |
        // L2: ...               |
        ++g_tkIter;
        expect('('); expr(); expect(')');
        int goto_L1 = g_opCnt;
        instruction(Jz, 0);
        stmt();

        if (TK_ATTRIB(g_tkIter, Kind) != Else) {
            OP_ATTRIB(goto_L1, Imme) = g_opCnt;
            return;
        }

        ++g_tkIter; // skip else
        int goto_L2 = g_opCnt;
        instruction(Jump, g_opCnt + 1);
        OP_ATTRIB(goto_L1, Imme) = g_opCnt;
        stmt();
        OP_ATTRIB(goto_L2, Imme) = g_opCnt;
        return;
    }

    if (kind == While) {
        // CONT: ...
        //       eax == 0; goto BREAK
        //       ...
        //       jump CONT;
        // BREAK:
        int label_cont = g_opCnt;
        ++g_tkIter;
        expect('('); expr(); expect(')');
        int goto_end = g_opCnt;
        instruction(Jz, 0);
        stmt();
        instruction(Jump, label_cont);
        int label_break = g_opCnt;
        OP_ATTRIB(goto_end, Imme) = label_break;
        int i = g_opCnt - 1;
        while (i > label_cont) {
            if (OP_ATTRIB(i, OpCode) == _BreakStub) { OP_ATTRIB(i, OpCode) = Jump; OP_ATTRIB(i, Imme) = label_break; }
            else if (OP_ATTRIB(i, OpCode) == _ContStub) { OP_ATTRIB(i, OpCode) = Jump; OP_ATTRIB(i, Imme) = label_cont; }
            i -= 1;
        }
        return;
    }

    if (kind == Break) {
        ++g_tkIter;
        instruction(_BreakStub, 0);
        expect(';');
        return;
    }

    if (kind == Cont) {
        ++g_tkIter;
        instruction(_ContStub, 0);
        expect(';');
        return;
    }

    if (kind == '{') {
        enter_scope();
        ++g_tkIter;
        int restore = 0;
        while (TK_ATTRIB(g_tkIter, Kind) != '}') {
            kind = TK_ATTRIB(g_tkIter, Kind);
            if (IS_TYPE(kind)) {
                ++g_tkIter;
                int base_type = kind, varNum = 0;
                while (1) {
                    if (varNum > 0) {
                        expect(',');
                    }

                    int ptr = 0;
                    while (TK_ATTRIB(g_tkIter, Kind) == '*') { ptr = (ptr << 8) | 0xFF; ++g_tkIter; }
                    int id = expect(Id), prev = g_symCnt - 1;
                    SYM_ATTRIB(g_symCnt, Address) = 4;

                    if (prev >= 0 && SYM_ATTRIB(prev, Storage) == Local) {
                        SYM_ATTRIB(g_symCnt, Address) += SYM_ATTRIB(prev, Address);
                    }

                    SYM_ATTRIB(g_symCnt, Storage) = Local;
                    SYM_ATTRIB(g_symCnt, TkIdx) = id;
                    SYM_ATTRIB(g_symCnt, Scope) = g_scopes[g_scopeCnt - 1];
                    SYM_ATTRIB(g_symCnt, DType) = (ptr << 16) | base_type;

                    SUB(ESP, ESP, IMME, 4);
                    if (TK_ATTRIB(g_tkIter, Kind) == '=') {
                        ++g_tkIter;
                        assign_expr();
                        SUB(EDX, EBP, IMME, SYM_ATTRIB(g_symCnt, Address));
                        SAVEW(EDX, EAX);
                    }

                    ++restore, ++varNum, ++g_symCnt;
                    if (TK_ATTRIB(g_tkIter, Kind) == ';') { break; }
                }

                ++g_tkIter;
            } else {
                stmt();
            }
        }
        ++g_tkIter;

        if (restore) { ADD(ESP, ESP, IMME, restore << 2); }
        exit_scope();
        return;
    }

    if (kind == ';') {
        ++g_tkIter;
        return;
    }

    expr();
    expect(';');
    return;
}

// an object could be a global variable, an enum or a function
void obj() {
    int kind = TK_ATTRIB(g_tkIter, Kind);
    if (kind == Enum) {
        ++g_tkIter;
        expect('{');
        int val = 0;
        while (TK_ATTRIB(g_tkIter, Kind) != '}') {
            int idx = expect(Id);
            SYM_ATTRIB(g_symCnt, TkIdx) = idx;
            SYM_ATTRIB(g_symCnt, Storage) = Const;
            SYM_ATTRIB(g_symCnt, DType) = Int;
            SYM_ATTRIB(g_symCnt, Scope) = g_scopes[g_scopeCnt - 1];

            if (TK_ATTRIB(g_tkIter, Kind) == '=') {
                ++g_tkIter;
                idx = expect(CInt);
                val = TK_ATTRIB(idx, Value);
            }

            SYM_ATTRIB(g_symCnt++, Address) = val++;

            if (TK_ATTRIB(g_tkIter, Kind) == '}') { break; }
            expect(',');
        }
        ++g_tkIter;
        expect(';');
        return;
    }

    int ln = TK_ATTRIB(g_tkIter, Ln);
    int start = TK_ATTRIB(g_tkIter, Start);
    int end = TK_ATTRIB(g_tkIter++, End);
    if (!IS_TYPE(kind)) {
        COMPILE_ERROR("error:%d: unexpected token '%.*s'\n", ln, end - start, start);
    }

    while (TK_ATTRIB(g_tkIter, Kind) != ';') {
        int data_type = kind, ptr = 0;
        while (TK_ATTRIB(g_tkIter, Kind) == '*') {
            ptr = (ptr << 8) | 0xFF;
            ++g_tkIter;
        }
        data_type = (ptr << 16) | data_type;

        int id = expect(Id);

        if (TK_ATTRIB(g_tkIter, Kind) != '(') {
            SYM_ATTRIB(g_symCnt, Storage) = Global;
            SYM_ATTRIB(g_symCnt, TkIdx) = id;
            SYM_ATTRIB(g_symCnt, Scope) = g_scopes[g_scopeCnt - 1];
            SYM_ATTRIB(g_symCnt, DType) = data_type;
            *((int*)g_bss) = 0;
            SYM_ATTRIB(g_symCnt++, Address) = g_bss;
            g_bss += 4;
            if (TK_ATTRIB(g_tkIter, Kind) != ';') { expect(','); }
            continue;
        }

        if (streq("main", TK_ATTRIB(id, Start), 4)) {
            g_entry = g_opCnt;
        } else {
            SYM_ATTRIB(g_symCnt, Storage) = Func;
            SYM_ATTRIB(g_symCnt, TkIdx) = id;
            SYM_ATTRIB(g_symCnt, DType) = data_type;
            SYM_ATTRIB(g_symCnt, Scope) = g_scopes[g_scopeCnt - 1];
            SYM_ATTRIB(g_symCnt++, Address) = g_opCnt;
        }

        enter_scope();
        expect('(');
        int argCnt = 0, i = 1;
        while (TK_ATTRIB(g_tkIter, Kind) != ')') {
            if (argCnt > 0) { expect(','); }
            int data_type = expect_type();
            int ptr = 0;
            while (TK_ATTRIB(g_tkIter, Kind) == '*') { ptr = (ptr << 8) | 0xFF; ++g_tkIter; }
            data_type = (ptr << 16) | data_type;
            SYM_ATTRIB(g_symCnt, TkIdx) = expect(Id);
            SYM_ATTRIB(g_symCnt, Scope) = g_scopes[g_scopeCnt - 1];
            SYM_ATTRIB(g_symCnt, DType) = data_type;
            SYM_ATTRIB(g_symCnt++, Storage) = Param;
            ++argCnt;
        }
        expect(')');
        while (i <= argCnt) {
            SYM_ATTRIB(g_symCnt - i, Address) = -((i + 1) << 2);
            ++i;
        }

        // save frame
        PUSH(EBP, 0);
        MOV(EBP, ESP, 0);
        stmt();
        exit_scope();
        return;
    }
    ++g_tkIter;
    return;
}

void gen(int argc, char** argv) {
    enter_scope();

    while (g_tkIter < g_tkCnt) {
        obj();
    }

    int i = 0;
    while (i < g_callCnt) {
        int idx = CALL_ATTRIB(i, TkIdx);
        int start = TK_ATTRIB(idx, Start);
        int end = TK_ATTRIB(idx, End);
        int ln = TK_ATTRIB(idx, Ln);
        int len = end - start;

        int found = 0, j = 0;
        while (j < g_symCnt) {
            if (SYM_ATTRIB(j, Storage) == Func) {
                int funcIdx = SYM_ATTRIB(j, TkIdx);
                if (streq(start, TK_ATTRIB(funcIdx, Start), len)) {
                    found = 1;
                    /// NOTE: potential error here?
                    OP_ATTRIB(CALL_ATTRIB(i, InsIdx), Imme) = SYM_ATTRIB(j, Address);
                    break;
                }
            }
            ++j;
        }

        if (!found) {
            COMPILE_ERROR("error:%d: unknown reference to call %.*s\n", ln, len, start);
        }
        ++i;
    }

    exit_scope();

    // copy args to "bss" section
    int argptr = g_bss, it = 0;
    char** argStart = g_bss;
    char* stringStart = argStart + argc;
    while (it < argc) {
        argStart[it] = stringStart;
        char* p = argv[it];
        while (*p) {
            *stringStart++ = *p++;
            ++g_bss;
        }
        *stringStart++ = 0;
        ++g_bss, ++it;
    }

    g_bss = ALIGN(g_bss);

    // start
    int entry = g_opCnt;
    PUSH(IMME, argc);
    PUSH(IMME, argptr);
    CALL(g_entry);

    g_entry = entry;

    return;
}

// debug
void dump_code() {
    #define REG2STR(REG) 3, regs + 3 * REG
    printf("-------- code --------\n");
    char* regs = "   eaxebxecxedxespebp";
    int pc = 0;
    while (pc < g_opCnt) {
        int op = OP_ATTRIB(pc, OpCode);
        int imme = OP_ATTRIB(pc, Imme);
        int dest = (op & 0xFF00) >> 8;
        int src1 = (op & 0xFF0000) >> 16;
        int src2 = (op & 0xFF000000) >> 24;
        op = op & 0xFF;
        char* width = imme == 4 ? "word" : "byte";
        printf("[ %4d ] ", pc);
        if (op == Mov) {
            if (src2 == IMME) printf("  mov %.*s, %d(0x%08X)\n", REG2STR(dest), imme, imme);
            else printf("  mov %.*s, %.*s\n", REG2STR(dest), REG2STR(src2));
        } else if (op == Ret) {
            printf("  ret\n");
        } else if (op == Add || op == Sub || op == Mul || op == Div || op == Rem) {
            char* opstr = op == Add ? "add" : op == Sub ? "sub" : op == Mul ? "mul" : op == Div ? "div" : "rem";
            if (src2 == IMME) printf("  %s %.*s, %.*s, %d\n", opstr, REG2STR(dest), REG2STR(src1), imme);
            else printf("  %s %.*s, %.*s, %.*s\n", opstr, REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == Eq || op == Neq || op == Gt || op == Ge || op == Lt || op == Le) {
            char* opstr = op == Eq ? "==" : op == Neq ? "!=" : op == Gt ? ">" : op == Ge ? ">=" : op == Lt ? "<" : "<=";
            printf("  %s %.*s, %.*s, %.*s\n", opstr, REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == And) {
            printf("  and %.*s, %.*s, %.*s\n", REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == Or) {
            printf("  or %.*s, %.*s, %.*s\n", REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == Not) {
            printf("  not %.*s\n", REG2STR(dest));
        } else if (op == LShift) {
            printf("  lshift %.*s, %.*s, %.*s\n", REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == RShift) {
            printf("  rshift %.*s, %.*s, %.*s\n", REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == Push) {
            if (src2 == IMME) printf("  push %d(0x%08X)\n", imme, imme);
            else printf("  push %.*s\n", REG2STR(src2));
        } else if (op == Pop) {
            printf("  pop %.*s\n", REG2STR(dest));
        } else if (op == Load) {
            printf("  load %.*s, %s[%.*s]\n", REG2STR(dest), width, REG2STR(src1));
        } else if (op == Save) {
            printf("  save %s[%.*s], %.*s\n", width, REG2STR(dest), REG2STR(src1));
        } else if (op == Jump || op == Jz || op == Jnz || op == Call) {
            char* opstr = op == Jump ? "jmp" : op == Jz ? "jz" : op == Jnz ? "jnz" : "call";
            printf("  %s %d\n", opstr, imme);
        } else if (op == Printf || op == Fopen || op == Fgetc || op == Malloc || op == Exit) {
            char* opstr = op == Printf ? "printf" : op == Fopen ? "fopen" : op == Fgetc ? "fgetc" : op == Malloc ? "malloc" : "exit";
            printf("  %s\n", opstr);
        } else {
            panic("invalid op code");
        }
        ++pc;
    }
    return;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s file [args...]\n", *argv);
        return 1;
    }

    void* fp = fopen(argv[1], "r");
    if (!fp) {
        printf("file '%s' does not exist\n", argv[1]);
        return 1;
    }

    g_reserved = 2 * CHUNK_SIZE * argc;
    g_ram = malloc(g_reserved); // NOTE: whatever, too lazy to free it

    // memory layout
    // | instructions | global variables | ... script memory ... | stack |
    int src_reserved = 1 << 18;
    int tk_reserved = 4 * TokenSize * (src_reserved >> 2);
    int sym_reserved = 4 * SymSize * (tk_reserved >> 8);
    int opcode_reserved = 4 * OpSize * (src_reserved >> 3);
    int scope_reserved = 4 * MAX_SCOPE;
    int call_reserved = 4 * CallSize * MAX_CALLS;
    g_src = g_ram + (g_reserved - src_reserved);
    g_tks = g_ram + (g_reserved - src_reserved - tk_reserved);
    g_syms = g_ram + (g_reserved - src_reserved - tk_reserved - sym_reserved);
    g_scopes = g_ram + (g_reserved - src_reserved - tk_reserved - sym_reserved - scope_reserved);
    g_calls = g_ram + (g_reserved - src_reserved - tk_reserved - sym_reserved - scope_reserved - call_reserved);
    g_bss = g_ram + opcode_reserved;
    g_ops = g_ram;

    // read source file
    int src_len = 0, c;
    while ((c = fgetc(fp)) != -1) { g_src[src_len++] = c; }
    g_src[src_len] = 0;

    // lexing
    lex();

    // code generation
    gen(argc - 1, argv + 1);

    // run
    g_regs = g_ram + g_reserved - 4 * IMME;
    g_regs[ESP] = g_ram + g_reserved - 4 * IMME;

    int pc = g_entry;
    while (pc < g_opCnt) {
        int op = OP_ATTRIB(pc, OpCode);
        int imme = OP_ATTRIB(pc, Imme);
        int dest = (op & 0xFF00) >> 8;
        int src1 = (op & 0xFF0000) >> 16;
        int src2 = (op & 0xFF000000) >> 24;
        int value = src2 == IMME ? imme : g_regs[src2];
        op = op & 0xFF;

        if (op == Call) {
            g_regs[ESP] -= 4;
            *((int*)g_regs[ESP]) = pc + 1;
            pc = imme;
            continue;
        }

        if ((op == Jump)) {
            pc = imme;
            continue;
        }

        if (op == Jz) {
            if (g_regs[EAX] == 0) pc = imme;
            else pc = pc + 1;
            continue;
        }

        if (op == Jnz) {
            if (g_regs[EAX]) pc = imme;
            else pc = pc + 1;
            continue;
        }

        if (op == Ret) {
            pc = *((int*)g_regs[ESP]);
            g_regs[ESP] += 4;
            continue;
        }

        pc = pc + 1;

        if (op == Mov) { g_regs[dest] = value; }
        else if (op == Push) { g_regs[ESP] -= 4; *((int*)g_regs[ESP]) = value; }
        else if (op == Pop) { g_regs[dest] = *((int*)g_regs[ESP]); g_regs[ESP] += 4; }
        else if (op == Add) { g_regs[dest] = g_regs[src1] + value; }
        else if (op == Sub) { g_regs[dest] = g_regs[src1] - value; }
        else if (op == Mul) { g_regs[dest] = g_regs[src1] * value; }
        else if (op == Div) { g_regs[dest] = g_regs[src1] / value; }
        else if (op == Rem) { g_regs[dest] = g_regs[src1] % value; }
        else if (op == Eq) { g_regs[dest] = g_regs[src1] == value; }
        else if (op == Neq) { g_regs[dest] = g_regs[src1] != value; }
        else if (op == Ge) { g_regs[dest] = g_regs[src1] >= value; }
        else if (op == Gt) { g_regs[dest] = g_regs[src1] > value; }
        else if (op == Le) { g_regs[dest] = g_regs[src1] <= value; }
        else if (op == Lt) { g_regs[dest] = g_regs[src1] < value; }
        else if (op == And) { g_regs[dest] = g_regs[src1] & value; }
        else if (op == Or) { g_regs[dest] = g_regs[src1] | value; }
        else if (op == Not) { g_regs[dest] = !g_regs[dest]; }
        else if (op == LShift) { g_regs[dest] = g_regs[src1] << value; }
        else if (op == RShift) { g_regs[dest] = g_regs[src1] >> value; }
        else if (op == Save) {
            if (imme == 4) *((int*)g_regs[dest]) = g_regs[src1];
            else *((char*)g_regs[dest]) = g_regs[src1];
        }
        else if (op == Load) {
            if (imme == 4) g_regs[dest] = *((int*)g_regs[src1]);
            else g_regs[dest] = *((char*)g_regs[src1]);
        }
        else if (op == Printf) {
            int* p = g_regs[ESP];
            printf((char*)(p[7]), p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
        } else if (op == Fgetc) {
            int* p = g_regs[ESP];
            g_regs[EAX] = fgetc((void*)(p[0]));
        } else if (op == Fopen) {
            int* p = g_regs[ESP];
            g_regs[EAX] = fopen((char*)(p[1]), (char*)(p[0]));
        } else if (op == Malloc) {
            g_regs[EAX] = g_ram + CHUNK_SIZE;
        } else if (op == Exit) {
            g_regs[EAX] = *((int*)g_regs[ESP]);
            break;
        } else { panic("Invalid op code"); }
    }
    return 0;
}
