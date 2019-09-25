#include<stdio.h>
#include<stdbool.h>
#include<cpuid.h>

bool test_mmx_cpuid() {
	printf("TEST: test_mmx_cpuid\n");

	unsigned int eax, ebx, ecx, edx;
	asm volatile(
		"cpuid"
		: "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) 
		: "a" (1), "c" (0)
	);

	int has_mmx = !!(edx & (1 << 23));
	if (has_mmx) {
		return 0;
	} else {
		return 1;
	}
}

int main() {
	int errors = 0;
	errors += (int) test_mmx_cpuid();

	printf("Errors: %d\n", errors);
	return errors;
}

