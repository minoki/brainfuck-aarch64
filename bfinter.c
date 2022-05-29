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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned char ARRAY[65536];

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
    char *ip = program;
    char **stack = NULL;
    size_t stacklen = 0;
    unsigned char *ptr = ARRAY;
    while (*ip != '\0') {
        switch (*ip++) {
        case '>': ++ptr; break;
        case '<': --ptr; break;
        case '+': ++*ptr; break;
        case '-': --*ptr; break;
        case '.': putchar(*ptr); break;
        case ',': *ptr = getchar(); break;
        case '[':
            if (*ptr == 0) {
                int level = 0;
                while (*ip != '\0') {
                    char c = *ip++;
                    if (c == '[') {
                        ++level;
                    } else if (c == ']') {
                        if (level == 0) {
                            break;
                        } else {
                            --level;
                        }
                    }
                }
            } else {
                ++stacklen;
                stack = realloc(stack, sizeof(char *) * stacklen);
                stack[stacklen - 1] = ip;
            }
            break;
        case ']':
            if (*ptr == 0) {
                --stacklen;
            } else {
                ip = stack[stacklen - 1];
            }
            break;
        }
    }
}
