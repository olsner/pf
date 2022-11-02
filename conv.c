#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>

#include <gmp.h>
#include <mpfr.h>

#define DEBUG 0

static void print_ts_diff(const char* file, const char* name, int line, struct timespec *start, struct timespec *end)
{
#if DEBUG
	double sdiff = end->tv_sec - start->tv_sec;
	double nsdiff = end->tv_nsec - start->tv_nsec;
	printf("TIME %s:%d %s: %fs\n", file, line, name, sdiff + nsdiff / 1000000000.0);
	memcpy(start, end, sizeof(struct timespec));
#endif
}

#define START() struct timespec __ts_prev; \
	clock_gettime(CLOCK_REALTIME, &__ts_prev)
#define T(name) do { \
	struct timespec ts; \
	clock_gettime(CLOCK_REALTIME, &ts); \
	print_ts_diff(__FILE__, name, __LINE__, &__ts_prev, &ts); \
} while (0)

#include "buf.c"
#include "mympfr.c"
#include "base.c"

static void print_digits(buffer_t buffer)
{
	printf("%u digits: %.*s\n", (unsigned)buffer->size,
			(int)buffer->size, buffer->buffer);
	fflush(stdout);
}

static void mpz_print(mpz_t op)
{
#if DEBUG
	mpz_out_str(stdout, 10, op);
	putchar('\n');
	fflush(stdout);
#endif
}

// TODO Is this really the *de*coder? It actually *encodes* the integer as a digit-string...
static void decode_bijective(mpz_t m)
{
	while (mpz_cmp_ui(m, 0))
	{
		int r = mpz_fdiv_q_ui(m, m, 256);
		// If r == 0, then really r == 256, but that's our encoding for 0
		// anyway. Note that r == 256 means q = q-1, so we add one to m. (This
		// doesn't really make sense, but it's right...)
		if (!r)
		{
			mpz_add_ui(m, m, 1);
		}
		putchar(r);
	}
}

int main(int argc, char* argv[])
{
	mpz_t in, temp;
	mpz_t out;
	int c;
	unsigned long num_bits;

	mpz_init(in);
	mpz_init(temp);
	mpz_init(out);

	START();
	unsigned long base = 0;
	while ((c = getchar()) != EOF)
	{
		if (!c) c = 256;
		mpz_set_ui(temp, c);
		mpz_mul_2exp(temp, temp, base++*8);
		mpz_add(in, in, temp);
	}
	T("input");

	mpz_clear(temp);
	mpz_print(in);

	T("print input");

	mpfr_t e;
	mpfr_t in_f;
	mpfr_init_zprec(e, in);
	T("init e");
	mpfr_init_zprec(in_f, in);
	T("init in_f");
	mpfr_const_e(e, GMP_RNDU);
	T("mpfr_const_e #1");
	num_bits = 4 * mpfr_digits_in_base(e, in_f) + (unsigned long)mpz_sizeinbase(in, 2) + 3000;
	mpfr_set_prec(e, num_bits);
	mpfr_const_e(e, GMP_RNDU);
	T("mpfr_const_e #2");
	buffer_t buffer;
	encode_in_base(e, in_f, buffer);
	T("encode_in_base");
	mpfr_clear(in_f);

	print_digits(buffer);
	T("print_digits");
	decode_in_base(e, out, buffer);
	T("decode_in_base");
	mpfr_clear(e);
	//mpz_print(out);
	if (mpz_cmp(in, out))
	{
		printf("Ay caramba! It doesn't match.\n");
		printf("in:  ");
		mpz_print(in);
		printf("out: ");
		mpz_print(out);
		printf("diff:");
		mpz_sub(out, in, out);
		mpz_print(out);
	}
	assert(0 == mpz_cmp(in, out));
	mpz_clear(in);
	T("mpz_cmp");
	mpz_print(out);
	decode_bijective(out);
	T("decode_bijective");

	mpz_clear(out);

	mpfr_free_cache();

	return 0;
}
