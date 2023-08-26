#ifndef FILTER_CUDA_KMP_H
#define FILTER_CUDA_KMP_H

#include <cstddef>
#include <cstdio>

struct KMP final
{
	static void All(const char* from, size_t m, const char* what, size_t n, void(* callback)(size_t n));

	static ssize_t First(const char* from, size_t m, const char* what, size_t n);
};

#endif //FILTER_CUDA_KMP_H
