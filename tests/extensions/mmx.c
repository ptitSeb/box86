#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include<limits.h>
#include<immintrin.h>
#include<cpuid.h>



typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define I8_MAX 127
#define I8_MIN -128
#define U8_MAX 255
#define U8_MIN 0

#define I16_MAX 32767
#define I16_MIN -32768
#define U16_MAX 65535
#define U16_MIN 0

#define I32_MAX 2147483647
#define I32_MIN -2147483648
#define U32_MAX 4294967295
#define U32_MIN 0

#define MMX_TEST_STRUCT(sz) \
    typedef struct mmx_##sz##_test { \
        sz a; \
        sz b; \
        sz result; \
    } mmx_##sz##_test_t

MMX_TEST_STRUCT(u8);
MMX_TEST_STRUCT(i8);
MMX_TEST_STRUCT(u16);
MMX_TEST_STRUCT(i16);
MMX_TEST_STRUCT(u32);
MMX_TEST_STRUCT(i32);

// Binary compare two mm registers
bool mm_raw_compare(__m64 a, __m64 b) {
	__m64 a_upper_reg = _mm_srli_si64(a, 32);
	__m64 b_upper_reg = _mm_srli_si64(b, 32);

	int a_lower = _m_to_int(a);
	int a_upper = _m_to_int(a_upper_reg);

	int b_lower = _m_to_int(b);
	int b_upper = _m_to_int(b_upper_reg);

	return (a_lower == b_lower) && (a_upper == b_upper);
}

#define MMX_ARITH_TEST(name, testcases, testcase_type, type, size, testfunc) \
bool name() { \
	printf("TEST: " #name "\n"); \
	int errors = 0; \
\
	for (size_t i = 0; i < ARRAY_SIZE(testcases); i++ ) { \
		testcase_type test_data = testcases[i]; \
\
		__m64 a = _mm_set1_pi##size(test_data.a); \
		__m64 b = _mm_set1_pi##size(test_data.b); \
		__m64 expected = _mm_set1_pi##size(test_data.result); \
		__m64 result = testfunc(a, b); \
\
		bool success = mm_raw_compare(expected, result); \
		errors += (int) (!success); \
	} \
\
	_m_empty(); \
	printf("TEST: finished with: %d errors\n", errors); \
	return errors; \
}


mmx_i8_test_t mmx_i8_add_test_data[] = {
    { .a = 1, .b = 2, .result = 3 },
    { .a = 0, .b = 1, .result = 1 },
    { .a = I8_MAX, .b = 1, .result = I8_MIN },
    { .a = I8_MIN, .b = -1, .result = I8_MAX },
    { .a = 0, .b = U8_MAX, .result = U8_MAX },
};
mmx_i8_test_t mmx_i8_add_sat_test_data[] = {
    { .a = 1, .b = 2, .result = 3 },
    { .a = 0, .b = 1, .result = 1 },
    { .a = I8_MAX, .b = 1, .result = I8_MAX },
    { .a = I8_MIN, .b = -1, .result = I8_MIN },
};
mmx_u8_test_t mmx_u8_add_sat_test_data[] = {
    { .a = 1, .b = 2, .result = 3 },
    { .a = 0, .b = 1, .result = 1 },
    { .a = U8_MAX, .b = 1, .result = U8_MAX },
    { .a = 0, .b = U8_MAX, .result = U8_MAX },
};

mmx_i16_test_t mmx_i16_add_test_data[] = {
    { .a = 1, .b = 2, .result = 3 },
    { .a = 0, .b = 1, .result = 1 },
    { .a = I16_MAX, .b = 1, .result = I16_MIN },
    { .a = I16_MIN, .b = -1, .result = I16_MAX },
};
mmx_i16_test_t mmx_i16_add_sat_test_data[] = {
    { .a = 1, .b = 2, .result = 3 },
    { .a = 0, .b = 1, .result = 1 },
    { .a = I16_MAX, .b = 1, .result = I16_MAX },
    { .a = I16_MIN, .b = -1, .result = I16_MIN },
};
mmx_u16_test_t mmx_u16_add_sat_test_data[] = {
    { .a = 1, .b = 2, .result = 3 },
    { .a = 0, .b = 1, .result = 1 },
    { .a = U16_MAX, .b = 1, .result = U16_MAX },
    { .a = 0, .b = U16_MAX, .result = U16_MAX },
};

mmx_i32_test_t mmx_i32_add_test_data[] = {
    { .a = 1, .b = 2, .result = 3 },
    { .a = 0, .b = 1, .result = 1 },
    { .a = I32_MAX, .b = 1, .result = I32_MIN },
    { .a = I32_MIN, .b = -1, .result = I32_MAX },
};

MMX_ARITH_TEST(test_mmx_add_pi8, mmx_i8_add_test_data, mmx_i8_test_t, i8, 8, _m_paddb);
MMX_ARITH_TEST(test_mmx_add_sat_pi8, mmx_i8_add_sat_test_data, mmx_i8_test_t, i8, 8, _m_paddsb);
MMX_ARITH_TEST(test_mmx_add_sat_pu8, mmx_u8_add_sat_test_data, mmx_u8_test_t, u8, 8, _m_paddusb);

MMX_ARITH_TEST(test_mmx_add_pi16, mmx_i16_add_test_data, mmx_i16_test_t, i16, 16, _m_paddw);
MMX_ARITH_TEST(test_mmx_add_sat_pi16, mmx_i16_add_sat_test_data, mmx_i16_test_t, i16, 16, _m_paddsw);
MMX_ARITH_TEST(test_mmx_add_sat_pu16, mmx_u16_add_sat_test_data, mmx_u16_test_t, u16, 16, _m_paddusw);

MMX_ARITH_TEST(test_mmx_add_pi32, mmx_i32_add_test_data, mmx_i32_test_t, i32, 32, _m_paddd);

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

	errors += (int) test_mmx_add_pi8();
	errors += (int) test_mmx_add_sat_pi8();
	errors += (int) test_mmx_add_sat_pu8();

	errors += (int) test_mmx_add_pi16();
	errors += (int) test_mmx_add_sat_pi16();
	errors += (int) test_mmx_add_sat_pu16();

	errors += (int) test_mmx_add_pi32();

	printf("Errors: %d\n", errors);
	return errors;
}

