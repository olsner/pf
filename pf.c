#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>

#include <gmp.h>
#include <mpfr.h>

static void print_ts_diff(const char* file, const char* name, int line, struct timespec* start, struct timespec * end);

#define START() struct timespec __ts_prev; \
	clock_gettime(CLOCK_REALTIME, &__ts_prev)
#define T(name) do { \
	struct timespec ts; \
	clock_gettime(CLOCK_REALTIME, &ts); \
	print_ts_diff(__FILE__, name, __LINE__, &__ts_prev, &ts); \
} while (0)

#include "buf.c"
#include "mympfr.c"
#include "rewr.c"
#include "base.c"

static void print_digits(buffer_t buffer)
{
	int i;
	printf("Digits: %u.\n", (unsigned)buffer->size);
	for (i = 0; i < buffer->size; i++)
	{
		printf("%d\t%u\n", i, buffer->buffer[i]);
	}
	printf(".\n");
	fflush(stdout);
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

static void mpz_print(mpz_t op)
{
	mpz_out_str(stdout, 10, op);
	putchar('\n');
	fflush(stdout);
}

static void run_program(const buffer_t in, buffer_t out, int argc, char* argv[])
{
	struct reg* reg1 = mk_reg(1, NULL);
	buf_init0(reg1->buf);

	struct reg* regs = mk_reg(0, reg1);
	buf_copy(regs->buf, in);

	maybe(argc > 1);
	interp(&regs, (const char**)argv, 1, argc);
	buf_move(out, reg1->buf);
	while (regs) regs = dl_reg(regs);

	char* p = out->buffer;
	const char* end = p + out->size;
	while (p < end)
		*p++ &= 3;
}

static void print_ts_diff(const char* file, const char* name, int line, struct timespec *start, struct timespec *end)
{
	double sdiff = end->tv_sec - start->tv_sec;
	double nsdiff = end->tv_nsec - start->tv_nsec;
	printf("TIME %s:%d %s: %fs\n", file, line, name, sdiff + nsdiff / 1000000000.0);
	memcpy(start, end, sizeof(struct timespec));
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
	//const char* test_data = "Hello world!";
	unsigned long base = 0;
	while ((c = getchar()) != EOF)
	//while ((c = *test_data++))
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

	mpfr_t pi;
	mpfr_t in_f;
	mpfr_init_zprec(pi, in);
	T("init pi");
	mpfr_init_zprec(in_f, in);
	T("init in_f");
	mpfr_const_pi(pi, GMP_RNDN);
	T("mpfr_const_pi #1");
	num_bits = 4*mpfr_digits_in_base(pi, in_f) + (unsigned long)mpz_sizeinbase(in, 2);
	mpfr_set_prec(pi, num_bits);
	mpfr_const_pi(pi, GMP_RNDN);
	T("mpfr_const_pi #2");
	buffer_t buffer;
	encode_in_base(pi, in_f, buffer);
	T("encode_in_base");
	mpfr_clear(in_f);

	print_digits(buffer);
	T("print_digits");
	decode_in_base(pi, out, buffer);
	T("decode_in_base");
	mpfr_clear(pi);
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
	decode_bijective(out);
	T("decode_bijective");

	buffer_t out_buffer;
	run_program(buffer, out_buffer, argc, argv);
	buf_clear(buffer);
	T("run program");

	mpfr_t e;
	mpfr_init2(e, 256);
	mpfr_exp_ui(e, out_buffer->size, GMP_RNDU);
	mpfr_log2(e, e, GMP_RNDU);
	num_bits = mpfr_get_ui(e, GMP_RNDU);

	mpfr_set_prec(e, num_bits);
	mpfr_const_e(e, GMP_RNDN);

	decode_in_base(e, out, out_buffer);
	mpfr_clear(e);
	buf_clear(out_buffer);

	mpz_print(out);
	decode_bijective(out);
	mpz_clear(out);

	mpfr_free_cache();

	return 0;
}
