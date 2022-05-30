/*
 * Copyright (c) 2022 ARATA Mizuki
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Usage: %s program.bf\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        fprintf(stderr, "Failed to open input\n");
        return 1;
    }
    char *program = NULL;
    size_t progsize = 0;
    {
        int seekresult = fseek(f, 0, SEEK_END);
        assert(seekresult == 0);
        long size = ftell(f);
        assert(size >= 0);
        progsize = size;
        seekresult = fseek(f, 0, SEEK_SET);
        assert(seekresult == 0);
        program = malloc(progsize + 1);
        if (program == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            fclose(f);
            return 1;
        }
        size_t result = fread(program, 1, progsize, f);
        assert(result == progsize);
        program[result] = '\0';
    }
    fclose(f);
    FILE *out = fopen("out.s", "w");
#if defined(__APPLE__)
    const char *ARRAY_sym = "_ARRAY";
    const char *main_sym = "_main";
    const char *getchar_sym = "_getchar";
    const char *putchar_sym = "_putchar";
#else
    const char *ARRAY_sym = "ARRAY";
    const char *main_sym = "main";
    const char *getchar_sym = "getchar";
    const char *putchar_sym = "putchar";
#endif
    fputs("\t.data\n", out);
    fprintf(out, "%s:\n", ARRAY_sym);
    fputs("\t.zero 65536\n", out);
    fputs("\t.text\n", out);
    fprintf(out, "\t.globl %s\n", main_sym);
    fputs("\t.align 2\n", out); // 4-byte align
    fprintf(out, "%s:\n", main_sym);
    fputs("\tstp x29, x30, [sp, #-16]!\n", out); // save x29 (frame pointer), x30 (link register)
    fputs("\tstr x19, [sp, #-16]!\n", out); // save x19
    fputs("\tadd x29, sp, #32\n", out); // set frame pointer
    // ptr = x19 (callee save)
#if defined(__APPLE__)
    fprintf(out, "\tadrp x19, %s@PAGE\n", ARRAY_sym);
    fprintf(out, "\tadd x19, x19, %s@PAGEOFF\n", ARRAY_sym);
#else
    fprintf(out, "\tadrp x19, %s\n", ARRAY_sym);
    fprintf(out, "\tadd x19, x19, :lo12:%s\n", ARRAY_sym);
#endif
    char *ip = program;
    int *stack = NULL;
    size_t stacklen = 0;
    int labelcounter = 0;
    while (*ip != '\0') {
        switch (*ip++) {
        case '>':
            {
                int n = 1;
                while (*ip == '>' && n < 4095) {
                    ++n;
                    ++ip;
                }
                assert(n <= 4095);
                fprintf(out, "\tadd x19, x19, #%d\n", n); // ptr += n
                break;
            }
        case '<':
            {
                int n = 1;
                while (*ip == '<' && n < 4095) {
                    ++n;
                    ++ip;
                }
                assert(n <= 4095);
                fprintf(out, "\tsub x19, x19, #%d\n", n); // ptr -= n
                break;
            }
        case '+':
            {
                int n = 1;
                while (*ip == '+' && n < 4095) {
                    ++n;
                    ++ip;
                }
                assert(n <= 4095);
                fputs("\tldrb w0, [x19]\n", out); // w0 = *ptr
                fprintf(out, "\tadd w0, w0, #%d\n", n); // w0 += n
                fputs("\tstrb w0, [x19]\n", out); // *ptr = w0
                break;
            }
        case '-':
            {
                int n = 1;
                while (*ip == '-' && n < 4095) {
                    ++n;
                    ++ip;
                }
                assert(n <= 4095);
                fputs("\tldrb w0, [x19]\n", out); // w0 = *ptr
                fprintf(out, "\tsub w0, w0, #%d\n", n); // w0 -= n
                fputs("\tstrb w0, [x19]\n", out); // *ptr = w0
                break;
            }
        case '.':
            fputs("\tldrb w0, [x19]\n", out); // w0 = *ptr
            fprintf(out, "\tbl %s\n", putchar_sym);
            break;
        case ',':
            fprintf(out, "\tbl %s\n", getchar_sym);
            fputs("\tstrb w0, [x19]\n", out);
            break;
        case '[':
            {
                int label = labelcounter++;
                ++stacklen;
                stack = realloc(stack, sizeof(int) * stacklen);
                stack[stacklen - 1] = label;
                fprintf(out, "L%d_start:\n", label);
                fputs("\tldrb w0, [x19]\n", out);
                fprintf(out, "\tcbnz w0, L%d_body\n", label);
                fprintf(out, "\tb L%d_end\n", label);
                fprintf(out, "L%d_body:\n", label);
                break;
            }
        case ']':
            {
                if (stacklen == 0) {
                    fputs("Unmatched brackets.\n", stderr);
                    break;
                }
                int label = stack[--stacklen];
                fprintf(out, "\tb L%d_start\n", label);
                fprintf(out, "L%d_end:\n", label);
                break;
            }
        }
    }
    if (stacklen != 0) {
        fputs("Unmatched brackets.\n", stderr);
    }
    fputs("\tldr x19, [sp], #16\n", out); // restore x19
    fputs("\tldp x29, x30, [sp], #16\n", out); // restore x29, x30
    fputs("\tmov w0, #0\n", out);
    fputs("\tret\n", out);
    fclose(out);
}
