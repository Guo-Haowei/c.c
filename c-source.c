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
#define IS_WHITESPACE(C) (C == ' ' || C == '\t' || C == '\n' || C == '\r')
#define IS_PUNCT(P, A, B) (*P == A && P[1] == B)
#define IS_TYPE(KIND) (KIND >= KW_int && KIND <= KW_void)
#define ALIGN(x) ((x + 3) & -4)
// @TODO: refactor error
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
#define CHAR_PTR (0xFF0000 | KW_char)
#define VOID_PTR (0xFF0000 | KW_void)
#define IS_PTR(TYPE) (0xFF0000 & TYPE)
#define SYM_ATTRIB(IDX, ATTRIB) g_syms[((IDX) * SymSize) + ATTRIB]
#define OP_ATTRIB(IDX, ATTRIB) g_ops[((IDX) * OpSize) + ATTRIB]
#define CALL_ATTRIB(IDX, ATTRIB) g_calls[((IDX) * CallSize) + ATTRIB]
#define OP(op, dest, src1, src2) ((op) | (dest << 8) | (src1 << 16) | (src2 << 24))

#define MAX_PRINF_ARGS (8)
#define CHUNK_SIZE (1 << 27)
#define MAX_SCOPE (128)
#define MAX_CALLS (1024)

enum { Undefined, Global, Param, Local, Func, Const };
enum { EAX = 1, EBX, ECX, EDX, ESP, EBP, IMME };
enum { TkIdx, Scope, DType, Storage, Address, SymSize };
enum { OpCode, Imme, OpSize };
enum { /* TkIndex = 0, */ InsIdx = 1, CallSize };

#pragma region utils
void panic(char* fmt) {
    printf("[panic] %s\n", fmt);
    exit(1);
}

int string_equal(char* p1, char* p2, int len) {
    while (len--) {
        if (*p1 == 0 || *p2 == 0) return 1;
        if (*p1 != *p2) return 0;
        p1 += 1; p2 += 1;
    }
    return 1;
}

int string_length(char* p) {
    int len = 0;
    while (*p++) { len += 1; }
    return len;
}
#pragma endregion utils

//---------------------------------- TOKEN  ----------------------------------//
enum {
	_TK_START = 128, // 0-127 is reserved for ascii
	TK_INT,          // int
	TK_IDENT,        // identifier
	TK_STRING,       // c string
	TK_CHAR,         // char

	TK_NE,           // !=
	TK_EQ,           // ==
	TK_GE,           // >=
	TK_LE,           // <=

	TK_ADD_ASSIGN,   // +=
	TK_SUB_ASSIGN,   // -=
	TK_INC,          // ++
	TK_DEC,          // --
	TK_AND,          // &&
	TK_OR,           // ||
	TK_LSHIFT,       // <<
	TK_RSHIFT,       // >>

	_KW_START, // keywords

	KW_int, KW_char, KW_void, KW_break, KW_continue,
	KW_else, KW_enum, KW_if, KW_return, KW_while,
	KW_printf, KW_fopen, KW_fgetc, KW_calloc, KW_memset,
	KW_exit,

	_KW_END,

	// @TODO: refactor the following, because they are opcode
	Add, Sub, Mul, Div, Rem,
	Mov, Push, Pop, Load, Save,
	Neq, Eq, Gt, Ge, Lt, Le, And, Or,
	Not, Ret, Jz, Jnz, Jump, Call,
	_BreakStub, _ContStub
};

// @TODO: implement struct. Use enum and array to mimic array of struct for now
#define GET_TK_FIELD(IDX, ATTRIB) (g_token_buffer[((IDX) * _TkFieldCount) + ATTRIB])
enum {
	TkFieldKind,
	TkFieldValue, // store the value of token if char or int
	TkFieldLine, // current line of a token
	TkFieldBegin,
	TkFieldEnd,

	_TkFieldCount,
};

int* g_token_buffer; // global int array to hold token information

void check_if_token_keyword(int token_idx) {
	char* keywords = "int\0     char\0    void\0    break\0   continue\0"
		             "else\0    enum\0    if\0      return\0  while\0   "
		             "printf\0  fopen\0   fgetc\0   calloc\0  memset\0  "
		             "exit\0    ";

	char* start = GET_TK_FIELD(token_idx, TkFieldBegin);
	int token_len = (char*)GET_TK_FIELD(token_idx, TkFieldEnd) - start;

	int idx = 0;
	while (idx < (_KW_END - KW_int)) {
		char* kw = keywords + (idx * 9); // a keyword is at most 8 char, plus '\0'
		int keyword_len = string_length(kw);
		if (keyword_len == token_len && string_equal(start, kw, 8)) {
			GET_TK_FIELD(token_idx, TkFieldKind) = KW_int + idx;
			break;
		}
		++idx;
	}
    return;
}

// @TODO: refactor
int g_bss,
    g_token_iter,
    *g_syms, g_sym_count,
    *g_ops, g_opCnt,
    *g_regs,
    g_entry,
    g_scope_id, *g_scopes, g_scope_count,
    *g_calls, g_callCnt;

//---------------------------------- PARSER ----------------------------------//
int parse_escape_sequence(int letter, int ln) {
	if (letter == '0') return '\0';
	if (letter == 'n') return '\n';
	if (letter == 'r') return '\r';
	if (letter == 't') return '\t';
	if (letter == '\\') return '\\';
	if (letter == '\'') return '\'';
	if (letter == '"') return '"';

	COMPILE_ERROR("error:%d: unknown escape sequence '\\%c'\n", ln, letter);
	return 0;
}

int lex(char* p) {
    int token_idx = 0;
    int ln = 1;
	while (*p) {
		if (*p == '#' || (*p == '/' && p[1] == '/')) { // handle '#' and comment '//'
			while (*p && *p != '\n') ++p;
		} else if (IS_WHITESPACE(*p)) { // handle whitespace
			ln += (*p == '\n'); ++p;
		} else {
            GET_TK_FIELD(token_idx, TkFieldLine) = ln;
            GET_TK_FIELD(token_idx, TkFieldBegin) = (int)p;

            if (IS_LETTER(*p) || *p == '_') { // handle token or keyword
                GET_TK_FIELD(token_idx, TkFieldKind) = TK_IDENT;
                ++p;
                while (IS_LETTER(*p) || IS_DIGIT(*p) || *p == '_') {
                    ++p;
                }
                GET_TK_FIELD(token_idx, TkFieldEnd) = (int)p;
                check_if_token_keyword(token_idx);
                token_idx += 1;
            } else if (*p == '0' && p[1] == 'x') { // handle hex number
                GET_TK_FIELD(token_idx, TkFieldKind) = TK_INT;
                int result = 0;
                p += 2; while(IS_HEX(*p)) {
                    result = (result << 4) + ((*p < 'A') ? (*p - '0') : (*p - 55));
                    ++p;
                }
                GET_TK_FIELD(token_idx, TkFieldValue) = result;
                GET_TK_FIELD(token_idx++, TkFieldEnd) = p;
            } else if (IS_DIGIT(*p)) { // handle decimal number
                GET_TK_FIELD(token_idx, TkFieldKind) = TK_INT;
                int result = 0;
                while (IS_DIGIT(*p)) { result = result * 10 + (*p - '0'); ++p; }
                GET_TK_FIELD(token_idx, TkFieldValue) = result;
                GET_TK_FIELD(token_idx++, TkFieldEnd) = p;
            } else if (*p == '"') { // handle string
                GET_TK_FIELD(token_idx, TkFieldKind) = TK_STRING;
                ++p; while (*p != '"') { ++p; };
                GET_TK_FIELD(token_idx++, TkFieldEnd) = ++p;
            } else if (*p == '\'') {
                // @TODO: handle escape
                GET_TK_FIELD(token_idx, TkFieldKind) = TK_CHAR;
                int v = *(++p); // skip opening '
                if (v == '\\') {
                    v = parse_escape_sequence(*(++p), ln);
                }
                GET_TK_FIELD(token_idx, TkFieldValue) = v;
                GET_TK_FIELD(token_idx++, TkFieldEnd) = (p += 2); // skip char and closing '
            } else {
                GET_TK_FIELD(token_idx, TkFieldKind) = *p;

                if (IS_PUNCT(p, '=', '=')) { GET_TK_FIELD(token_idx, TkFieldKind) = TK_EQ; ++p; }
                else if (IS_PUNCT(p, '!', '=')) { GET_TK_FIELD(token_idx, TkFieldKind) = TK_NE; ++p; }
                else if (IS_PUNCT(p, '&', '&')) { GET_TK_FIELD(token_idx, TkFieldKind) = TK_AND; ++p; }
                else if (IS_PUNCT(p, '|', '|')) { GET_TK_FIELD(token_idx, TkFieldKind) = TK_OR; ++p; }
                else if (*p == '+') {
                    if (p[1] == '+') { GET_TK_FIELD(token_idx, TkFieldKind) = TK_INC; ++p; }
                    else if (p[1] == '=') { GET_TK_FIELD(token_idx, TkFieldKind) = TK_ADD_ASSIGN; ++p; }
                } else if (*p == '-') {
                    if (p[1] == '-') { GET_TK_FIELD(token_idx, TkFieldKind) = TK_DEC; ++p; }
                    else if (p[1] == '=') { GET_TK_FIELD(token_idx, TkFieldKind) = TK_SUB_ASSIGN; ++p; }
                } else if (*p == '>') {
                    if (p[1] == '=') { GET_TK_FIELD(token_idx, TkFieldKind) = TK_GE; ++p; }
                    else if (p[1] == '>') { GET_TK_FIELD(token_idx, TkFieldKind) = TK_RSHIFT; ++p; }
                } else if (*p == '<') {
                    if (p[1] == '=') { GET_TK_FIELD(token_idx, TkFieldKind) = TK_LE; ++p; }
                    else if (p[1] == '<') { GET_TK_FIELD(token_idx, TkFieldKind) = TK_LSHIFT; ++p; }
                }

                GET_TK_FIELD(token_idx++, TkFieldEnd) = ++p;
            }
        }
    }
    return token_idx;
}

//--------------------------------- CODEGEN ----------------------------------//
void enter_scope() {
    if (g_scope_count >= MAX_SCOPE) {
        panic("scope overflow");
    }

    g_scopes[g_scope_count++] = ++g_scope_id;
    return;
}

void exit_scope() {
    if (g_scope_count <= 0) {
        panic("scope overflow");
    }

    int i = g_sym_count - 1;
    while (SYM_ATTRIB(i, Scope) == g_scopes[g_scope_count - 1]) {
        --g_sym_count, --i;
    }

    --g_scope_count;
    return;
}

int expect(int kind) {
    if (GET_TK_FIELD(g_token_iter, TkFieldKind) != kind) {
        int start = GET_TK_FIELD(g_token_iter, TkFieldBegin), end = GET_TK_FIELD(g_token_iter, TkFieldEnd);
        COMPILE_ERROR("error:%d: expected token '%c'(%d), got '%.*s'\n",
            GET_TK_FIELD(g_token_iter, TkFieldLine), kind < 128 ? kind : ' ', kind, end - start, start);
    }
    return g_token_iter++;
}

int expect_type() {
    int base_type = GET_TK_FIELD(g_token_iter, TkFieldKind);
    if (IS_TYPE(base_type)) {
        ++g_token_iter;
        int ptr = 0;
        while (GET_TK_FIELD(g_token_iter, TkFieldKind) == '*') { ptr = (ptr << 8) | 0xFF; ++g_token_iter; }
        return (ptr << 16) | base_type;
    }

    int start = GET_TK_FIELD(g_token_iter, TkFieldBegin), end = GET_TK_FIELD(g_token_iter, TkFieldEnd);
    COMPILE_ERROR("error:%d: expected type specifier, got '%.*s'\n", GET_TK_FIELD(g_token_iter, TkFieldLine), end - start, start);
}

void instruction(int op, int imme) {
    OP_ATTRIB(g_opCnt, OpCode) = op;
    OP_ATTRIB(g_opCnt, Imme) = imme;
    ++g_opCnt;
    return;
}

int primary_expr() {
    int tkIdx = g_token_iter++;
    char* start = GET_TK_FIELD(tkIdx, TkFieldBegin);
    char* end = GET_TK_FIELD(tkIdx, TkFieldEnd);
    int ln = GET_TK_FIELD(tkIdx, TkFieldLine);
    int kind = GET_TK_FIELD(tkIdx, TkFieldKind);
    int value = GET_TK_FIELD(tkIdx, TkFieldValue);
    int len = end - start;
    if (kind == TK_INT || kind == TK_CHAR) {
        MOV(EAX, IMME, value);
        return KW_int;
    }
    
    if (kind == TK_STRING) {
        MOV(EAX, IMME, g_bss);
        while (1) {
            len = len - 1;
            int i = 1;
            while (i < len) {
                int c = start[i];
                if (c == '\\') {
                    c = parse_escape_sequence(start[i += 1], ln);
                }
                *((char*)g_bss++) = c;
                ++i;
            }

            if (GET_TK_FIELD(g_token_iter, TkFieldKind) != TK_STRING) break;
            start = GET_TK_FIELD(g_token_iter, TkFieldBegin);
            end = GET_TK_FIELD(g_token_iter, TkFieldEnd);
            ln = GET_TK_FIELD(g_token_iter, TkFieldLine);
            len = end - start;
            ++g_token_iter;
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
    
    if (kind == TK_IDENT) {
        if (GET_TK_FIELD(g_token_iter, TkFieldKind) == '(') {
            ++g_token_iter;
            int argc = 0;
            while (GET_TK_FIELD(g_token_iter, TkFieldKind) != ')') {
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
            return KW_int;
        }

        int address = 0, type = Undefined, data_type = 0, i = g_sym_count - 1;
        while (i >= 0) {
            int tmp = SYM_ATTRIB(i, TkIdx);
            char *tmpstart = GET_TK_FIELD(tmp, TkFieldBegin), *tmpend = GET_TK_FIELD(tmp, TkFieldEnd);
            if (len == (tmpend - tmpstart) && string_equal(start, tmpstart, len)) {
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
            return KW_int;
        }
        if (type == Undefined) {
            COMPILE_ERROR("error:%d: '%.*s' undeclared\n", ln, len, start);
        }

        SUB(EDX, EBP, IMME, address);
        LOADW(EAX, EDX);
        return data_type;
    }

    if (kind == KW_printf) {
        expect('(');
        int argc = 0;
        while (GET_TK_FIELD(g_token_iter, TkFieldKind) != ')') {
            if (argc > 0) expect(',');
            assign_expr();
            PUSH(EAX, 0);
            ++argc;
        }
        if (argc > MAX_PRINF_ARGS) panic("printf supports at most %d args");
        SUB(ESP, ESP, IMME, (MAX_PRINF_ARGS - argc) << 2);
        instruction(KW_printf, argc);
        ADD(ESP, ESP, IMME, MAX_PRINF_ARGS << 2);
        expect(')');
        return KW_int;
    }

    int paramCnt = 0, ret = KW_void, i = 0;
    if (kind == KW_fopen) { paramCnt = 2; ret = VOID_PTR; }
    else if (kind == KW_fgetc) { paramCnt = 1; ret = KW_int; }
    else if (kind == KW_calloc) { paramCnt = 2; ret = VOID_PTR; }
    else if (kind == KW_exit) { paramCnt = 1; ret = KW_void; }
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
        int kind = GET_TK_FIELD(g_token_iter, TkFieldKind);
        int ln = GET_TK_FIELD(g_token_iter, TkFieldLine);
        if (kind == '[') {
            ++g_token_iter;
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
        } else if (kind == TK_INC || kind == TK_DEC) {
            ++g_token_iter;
            LOADW(EAX, EDX);
            MOV(EBX, EAX, 0);
            int value = (IS_PTR(data_type) && data_type != CHAR_PTR) ? 4 : 1;
            int op = kind == TK_INC ? Add : Sub;
            instruction(OP(op, EBX, EBX, IMME), value);
            SAVEW(EDX, EBX);
        } else {
            break;
        }
    }
    return data_type;
}

int unary_expr() {
    int kind = GET_TK_FIELD(g_token_iter, TkFieldKind);
    int ln = GET_TK_FIELD(g_token_iter, TkFieldLine);
    if (kind == '!') {
        ++g_token_iter;
        int data_type = unary_expr();
        instruction(OP(Not, EAX, 0, 0), 0);
        return data_type;
    }
    if (kind == '+') {
        ++g_token_iter;
        return unary_expr();
    }
    if (kind == '-') {
        ++g_token_iter;
        int data_type = unary_expr();
        MOV(EBX, IMME, 0);
        SUB(EAX, EBX, EAX, 0);
        return data_type;
    }
    if (kind == '*') {
        ++g_token_iter;
        int data_type = unary_expr();
        if (!IS_PTR(data_type)) {
            COMPILE_ERROR("error:%d: attempted to dereference a non-pointer type 0x%X\n", ln, data_type);
        }

        MOV(EDX, EAX, 0);
        if (data_type == CHAR_PTR) LOADB(EAX, EDX);
        else LOADW(EAX, EDX);
        return ((data_type >> 8) & 0xFF0000) | (0xFFFF & data_type);
    }
    if (kind == TK_INC || kind == TK_DEC) {
        ++g_token_iter;
        int data_type = unary_expr();
        LOADW(EAX, EDX);
        int value = (IS_PTR(data_type) && data_type != CHAR_PTR) ? 4 : 1;
        int op = kind == TK_INC ? Add : Sub;
        instruction(OP(op, EAX, EAX, IMME), value);
        SAVEW(EDX, EAX);
        return data_type;
    }
    return post_expr();
}

int cast_expr() {
    if (GET_TK_FIELD(g_token_iter, TkFieldKind) == '(') {
        int kind = GET_TK_FIELD(g_token_iter + 1, TkFieldKind);
        if (IS_TYPE(kind)) {
            ++g_token_iter; // skip '('
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
        int kind = GET_TK_FIELD(g_token_iter, TkFieldKind), opcode;
        if (kind == '*') opcode = Mul;
        else if (kind == '/') opcode = Div;
        else if (kind == '%') opcode = Rem;
        else break;
        ++g_token_iter;
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
        int kind = GET_TK_FIELD(g_token_iter, TkFieldKind), opcode;
        if (kind == '+') opcode = Add;
        else if (kind == '-') opcode = Sub;
        else break;
        ++g_token_iter;
        PUSH(EAX, 0);
        int rhs = mul_expr();
        if (IS_PTR(data_type) && IS_PTR(rhs)) {
            if (data_type != rhs) {
                COMPILE_ERROR("error:%d: type mismatch", GET_TK_FIELD(g_token_iter, TkFieldLine));
            }
            if (data_type != CHAR_PTR) {
                panic("TODO: handle subtraction other than char* - char*");
            }
            data_type = KW_int;
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
        int kind = GET_TK_FIELD(g_token_iter, TkFieldKind);
        if (kind != TK_LSHIFT && kind != TK_RSHIFT) break;
        ++g_token_iter;
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
        int kind = GET_TK_FIELD(g_token_iter, TkFieldKind), opcode;
        if (kind == TK_NE) opcode = Neq;
        else if (kind == TK_EQ) opcode = Eq;
        else if (kind == '<') opcode = Lt;
        else if (kind == '>') opcode = Gt;
        else if (kind == TK_GE) opcode = Ge;
        else if (kind == '<') opcode = Lt;
        else if (kind == TK_LE) opcode = Le;
        else break;
        ++g_token_iter;
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
        int kind = GET_TK_FIELD(g_token_iter, TkFieldKind), opcode;
        if (kind == '&') opcode = And;
        else if (kind == '|') opcode = Or;
        else break;
        ++g_token_iter;
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
        int kind = GET_TK_FIELD(g_token_iter, TkFieldKind), opcode;
        if (kind == TK_AND) opcode = Jz;
        else if (kind == TK_OR) opcode = Jnz;
        else break;

        ++g_token_iter;
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
        int kind = GET_TK_FIELD(g_token_iter, TkFieldKind);
        if (kind == '=') {
            ++g_token_iter;
            PUSH(EDX, 0);
            logical_expr();
            POP(EDX);
            instruction(OP(Save, EDX, EAX, 0), data_type == KW_char ? 1 : 4);
            continue;
        }

        if (kind == TK_ADD_ASSIGN) {
            ++g_token_iter;
            PUSH(EDX, 0);
            relation_expr();
            POP(EDX);
            LOADW(EBX, EDX);
            if (IS_PTR(data_type) && data_type != CHAR_PTR) { MUL(EAX, EAX, IMME, 4); }
            ADD(EAX, EBX, EAX, 0);
            SAVEW(EDX, EAX);
            continue;
        }

        if (kind == TK_SUB_ASSIGN) {
            ++g_token_iter;
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
            ++g_token_iter;
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
    while (GET_TK_FIELD(g_token_iter, TkFieldKind) == ',') {
        g_token_iter += 1;
        type = assign_expr();
    }
    return type;
}

void stmt() {
    int kind = GET_TK_FIELD(g_token_iter, TkFieldKind);
    if (kind == KW_return) {
        if (GET_TK_FIELD(++g_token_iter, TkFieldKind) != ';') { assign_expr(); }
        MOV(ESP, EBP, 0);
        POP(EBP);
        instruction(Ret, 0);
        expect(';');
        return;
    }

    if (kind == KW_if) {
        //     eax == 0; goto L1 |     eax == 0; goto L1
        //     ...               |     ...
        //     goto L2           | L1: ...
        // L1: ...               |
        // L2: ...               |
        ++g_token_iter;
        expect('('); expr(); expect(')');
        int goto_L1 = g_opCnt;
        instruction(Jz, 0);
        stmt();

        if (GET_TK_FIELD(g_token_iter, TkFieldKind) != KW_else) {
            OP_ATTRIB(goto_L1, Imme) = g_opCnt;
            return;
        }

        ++g_token_iter; // skip else
        int goto_L2 = g_opCnt;
        instruction(Jump, g_opCnt + 1);
        OP_ATTRIB(goto_L1, Imme) = g_opCnt;
        stmt();
        OP_ATTRIB(goto_L2, Imme) = g_opCnt;
        return;
    }

    if (kind == KW_while) {
        // CONT: ...
        //       eax == 0; goto BREAK
        //       ...
        //       jump CONT;
        // BREAK:
        int label_cont = g_opCnt;
        ++g_token_iter;
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

    if (kind == KW_break) {
        ++g_token_iter;
        instruction(_BreakStub, 0);
        expect(';');
        return;
    }

    if (kind == KW_continue) {
        ++g_token_iter;
        instruction(_ContStub, 0);
        expect(';');
        return;
    }

    if (kind == '{') {
        enter_scope();
        ++g_token_iter;
        int restore = 0;
        while (GET_TK_FIELD(g_token_iter, TkFieldKind) != '}') {
            kind = GET_TK_FIELD(g_token_iter, TkFieldKind);
            if (IS_TYPE(kind)) {
                ++g_token_iter;
                int base_type = kind, varNum = 0;
                while (1) {
                    if (varNum > 0) {
                        expect(',');
                    }

                    int ptr = 0;
                    while (GET_TK_FIELD(g_token_iter, TkFieldKind) == '*') { ptr = (ptr << 8) | 0xFF; ++g_token_iter; }
                    int id = expect(TK_IDENT), prev = g_sym_count - 1;
                    SYM_ATTRIB(g_sym_count, Address) = 4;

                    if (prev >= 0 && SYM_ATTRIB(prev, Storage) == Local) {
                        SYM_ATTRIB(g_sym_count, Address) += SYM_ATTRIB(prev, Address);
                    }

                    SYM_ATTRIB(g_sym_count, Storage) = Local;
                    SYM_ATTRIB(g_sym_count, TkIdx) = id;
                    SYM_ATTRIB(g_sym_count, Scope) = g_scopes[g_scope_count - 1];
                    SYM_ATTRIB(g_sym_count, DType) = (ptr << 16) | base_type;

                    SUB(ESP, ESP, IMME, 4);
                    if (GET_TK_FIELD(g_token_iter, TkFieldKind) == '=') {
                        ++g_token_iter;
                        assign_expr();
                        SUB(EDX, EBP, IMME, SYM_ATTRIB(g_sym_count, Address));
                        SAVEW(EDX, EAX);
                    }

                    ++restore, ++varNum, ++g_sym_count;
                    if (GET_TK_FIELD(g_token_iter, TkFieldKind) == ';') { break; }
                }

                ++g_token_iter;
            } else {
                stmt();
            }
        }
        ++g_token_iter;

        if (restore) { ADD(ESP, ESP, IMME, restore << 2); }
        exit_scope();
        return;
    }

    if (kind == ';') {
        ++g_token_iter;
        return;
    }

    expr();
    expect(';');
    return;
}

// an object could be a global variable, an enum or a function
void parse_declaration() {
    int kind = GET_TK_FIELD(g_token_iter, TkFieldKind);

    // parse enum
    if (kind == KW_enum) {
        ++g_token_iter;
        expect('{');
        int val = 0;
        while (GET_TK_FIELD(g_token_iter, TkFieldKind) != '}') {
            int idx = expect(TK_IDENT);
            SYM_ATTRIB(g_sym_count, TkIdx) = idx;
            SYM_ATTRIB(g_sym_count, Storage) = Const;
            SYM_ATTRIB(g_sym_count, DType) = KW_int;
            SYM_ATTRIB(g_sym_count, Scope) = g_scopes[g_scope_count - 1];

            if (GET_TK_FIELD(g_token_iter, TkFieldKind) == '=') {
                ++g_token_iter;
                idx = expect(TK_INT);
                val = GET_TK_FIELD(idx, TkFieldValue);
            }

            SYM_ATTRIB(g_sym_count++, Address) = val++;

            if (GET_TK_FIELD(g_token_iter, TkFieldKind) == '}') { break; }
            expect(',');
        }
        ++g_token_iter;
        expect(';');
        return;
    }

    // @TODO: parse struct

    // parse function/global variable
    int ln = GET_TK_FIELD(g_token_iter, TkFieldLine);
    int start = GET_TK_FIELD(g_token_iter, TkFieldBegin);
    int end = GET_TK_FIELD(g_token_iter++, TkFieldEnd);
    if (!IS_TYPE(kind)) {
        COMPILE_ERROR("error:%d: unexpected token '%.*s'\n", ln, end - start, start);
    }

    while (GET_TK_FIELD(g_token_iter, TkFieldKind) != ';') {
        int data_type = kind, ptr = 0;
        while (GET_TK_FIELD(g_token_iter, TkFieldKind) == '*') {
            ptr = (ptr << 8) | 0xFF;
            ++g_token_iter;
        }
        data_type = (ptr << 16) | data_type;

        int id = expect(TK_IDENT);

        if (GET_TK_FIELD(g_token_iter, TkFieldKind) != '(') {
            SYM_ATTRIB(g_sym_count, Storage) = Global;
            SYM_ATTRIB(g_sym_count, TkIdx) = id;
            SYM_ATTRIB(g_sym_count, Scope) = g_scopes[g_scope_count - 1];
            SYM_ATTRIB(g_sym_count, DType) = data_type;
            *((int*)g_bss) = 0;
            SYM_ATTRIB(g_sym_count++, Address) = g_bss;
            g_bss += 4;
            if (GET_TK_FIELD(g_token_iter, TkFieldKind) != ';') { expect(','); }
            continue;
        }

        if (string_equal("main", GET_TK_FIELD(id, TkFieldBegin), 4)) {
            g_entry = g_opCnt;
        } else {
            SYM_ATTRIB(g_sym_count, Storage) = Func;
            SYM_ATTRIB(g_sym_count, TkIdx) = id;
            SYM_ATTRIB(g_sym_count, DType) = data_type;
            SYM_ATTRIB(g_sym_count, Scope) = g_scopes[g_scope_count - 1];
            SYM_ATTRIB(g_sym_count++, Address) = g_opCnt;
        }

        enter_scope();
        expect('(');
        int argCnt = 0, i = 1;
        while (GET_TK_FIELD(g_token_iter, TkFieldKind) != ')') {
            if (argCnt > 0) { expect(','); }
            int data_type = expect_type();
            int ptr = 0;
            while (GET_TK_FIELD(g_token_iter, TkFieldKind) == '*') { ptr = (ptr << 8) | 0xFF; ++g_token_iter; }
            data_type = (ptr << 16) | data_type;
            SYM_ATTRIB(g_sym_count, TkIdx) = expect(TK_IDENT);
            SYM_ATTRIB(g_sym_count, Scope) = g_scopes[g_scope_count - 1];
            SYM_ATTRIB(g_sym_count, DType) = data_type;
            SYM_ATTRIB(g_sym_count++, Storage) = Param;
            ++argCnt;
        }
        expect(')');
        while (i <= argCnt) {
            SYM_ATTRIB(g_sym_count - i, Address) = -((i + 1) << 2);
            ++i;
        }

        // save frame
        PUSH(EBP, 0);
        MOV(EBP, ESP, 0);
        stmt();
        exit_scope();
        return;
    }
    ++g_token_iter;
    return;
}

void gen(int argc, char** argv, int token_count) {
    enter_scope();

    // parse external declaration
    while (g_token_iter < token_count) {
        parse_declaration();
    }

    int i = 0;
    while (i < g_callCnt) {
        int idx = CALL_ATTRIB(i, TkIdx);
        int start = GET_TK_FIELD(idx, TkFieldBegin);
        int end = GET_TK_FIELD(idx, TkFieldEnd);
        int ln = GET_TK_FIELD(idx, TkFieldLine);
        int len = end - start;

        int found = 0, j = 0;
        while (j < g_sym_count) {
            if (SYM_ATTRIB(j, Storage) == Func) {
                int funcIdx = SYM_ATTRIB(j, TkIdx);
                if (string_equal(start, GET_TK_FIELD(funcIdx, TkFieldBegin), len)) {
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
        } else if (op == TK_LSHIFT) {
            printf("  lshift %.*s, %.*s, %.*s\n", REG2STR(dest), REG2STR(src1), REG2STR(src2));
        } else if (op == TK_RSHIFT) {
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
        } else if (op == KW_printf || op == KW_fopen || op == KW_fgetc || op == KW_calloc || op == KW_exit) {
            char* opstr = op == KW_printf ? "printf" : op == KW_fopen ? "fopen" : op == KW_fgetc ? "fgetc" : op == KW_calloc ? "calloc" : "exit";
            printf("  %s\n", opstr);
        } else {
            panic("invalid op code");
        }
        ++pc;
    }
    return;
}

#define FATAL_ERROR(fmt, ...) { printf("c.c: \033[31mfatal error\033[0m: " fmt "\ncompilation terminated.\n", ##__VA_ARGS__); exit(1); }

int main(int argc, char **argv) {
	if (argc == 1) {
        FATAL_ERROR("no input files");
		return 1;
	}

	void* fp = fopen(argv[1], "r");
	if (!fp) {
		FATAL_ERROR("%s: No such file or directory", *(argv + 1));
		return 1;
	}

	int reserved_size_in_byte = 2 * CHUNK_SIZE * argc;
	char* ram = calloc(reserved_size_in_byte, 1);

    // memory layout
    // | instructions | global variables | ... script memory ... | stack |
    int src_reserved = 1 << 18;
    int tk_reserved = 4 * _TkFieldCount * (src_reserved >> 2);
    int sym_reserved = 4 * SymSize * (tk_reserved >> 8);
    int opcode_reserved = 4 * OpSize * (src_reserved >> 3);
    int scope_reserved = 4 * MAX_SCOPE;
    int call_reserved = 4 * CallSize * MAX_CALLS;
    char* source_code = ram + (reserved_size_in_byte - src_reserved);
    g_token_buffer = ram + (reserved_size_in_byte - src_reserved - tk_reserved);
    g_syms = ram + (reserved_size_in_byte - src_reserved - tk_reserved - sym_reserved);
    g_scopes = ram + (reserved_size_in_byte - src_reserved - tk_reserved - sym_reserved - scope_reserved);
    g_calls = ram + (reserved_size_in_byte - src_reserved - tk_reserved - sym_reserved - scope_reserved - call_reserved);
    g_bss = ram + opcode_reserved;
    g_ops = ram;

    // read source code
	int src_len = 0, c;
	while ((c = fgetc(fp)) != -1) {
		source_code[src_len++] = c;
	}
    source_code[src_len] = 0;

    // lexing
    int token_count = lex(source_code);

    // code generation
    gen(argc - 1, argv + 1, token_count);

    // run
    g_regs = ram + reserved_size_in_byte - 4 * IMME;
    g_regs[ESP] = ram + reserved_size_in_byte - 4 * IMME;

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
        else if (op == TK_LSHIFT) { g_regs[dest] = g_regs[src1] << value; }
        else if (op == TK_RSHIFT) { g_regs[dest] = g_regs[src1] >> value; }
        else if (op == Save) {
            if (imme == 4) *((int*)g_regs[dest]) = g_regs[src1];
            else *((char*)g_regs[dest]) = g_regs[src1];
        }
        else if (op == Load) {
            if (imme == 4) g_regs[dest] = *((int*)g_regs[src1]);
            else g_regs[dest] = *((char*)g_regs[src1]);
        }
        else if (op == KW_printf) {
            int* p = g_regs[ESP];
            printf((char*)(p[7]), p[6], p[5], p[4], p[3], p[2], p[1], p[0]);
        } else if (op == KW_fgetc) {
            int* p = g_regs[ESP];
            g_regs[EAX] = fgetc((void*)(p[0]));
        } else if (op == KW_fopen) {
            int* p = g_regs[ESP];
            g_regs[EAX] = fopen((char*)(p[1]), (char*)(p[0]));
        } else if (op == KW_calloc) {
            g_regs[EAX] = ram + CHUNK_SIZE; // @HACK: not really allocating desired amount of memory
        } else if (op == KW_exit) {
            g_regs[EAX] = *((int*)g_regs[ESP]);
            break;
        } else { panic("Invalid op code"); }
    }
    return 0;
}
