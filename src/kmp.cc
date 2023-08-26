#include "kmp.h"
#include <cstdio>

void
ComputeLPS(const char* from, size_t m, size_t* lps);

void
KMP::All(const char* text, size_t m, const char* pattern, size_t n,
         void (*callback)(size_t n))
{
        int* next = new int[n + 1];
        for (int i = 0; i < n + 1; i++) next[i] = 0;

        for (int i = 1; i < n; i++)
        {
                int j = next[i];

                while (j > 0 && pattern[j] != pattern[i])
                {
                        j = next[j];
                }

                if (j > 0 || pattern[j] == pattern[i])
                {
                        next[i + 1] = j + 1;
                }
        }

        for (int i = 0, j = 0; i < m; i++)
        {
                if (*(text + i) == *(pattern + j))
                {
                        if (++j == n)
                        {
                                callback(i - j + 1);
                        }
                }
                else if (j > 0)
                {
                        j = next[j];
                        i--; // since `i` will be incremented in the next iteration
                }
        }

        delete[] next;
}

ssize_t
KMP::First(const char* text, size_t m, const char* pattern, size_t n)
{
        int* next = new int[n + 1];
        for (int i = 0; i < n + 1; i++) next[i] = 0;

        for (int i = 1; i < n; i++)
        {
                int j = next[i];

                while (j > 0 && pattern[j] != pattern[i])
                {
                        j = next[j];
                }

                if (j > 0 || pattern[j] == pattern[i])
                {
                        next[i + 1] = j + 1;
                }
        }

        for (int i = 0, j = 0; i < m; i++)
        {
                if (*(text + i) == *(pattern + j))
                {
                        if (++j == n)
                        {
                                return i - j + 1;
                        }
                }
                else if (j > 0)
                {
                        j = next[j];
                        i--; // since `i` will be incremented in the next iteration
                }
        }

        delete[] next;
        
        return -1;
}
