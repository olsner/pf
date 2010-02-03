#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>

#include <gmp.h>
#include <mpfr.h>

static void print_ts_diff(const char* name, int line, struct timespec* start, struct timespec * end);

#define START() struct timespec __ts_prev; \
	clock_gettime(CLOCK_REALTIME, &__ts_prev)
#define T(name) do { \
	struct timespec ts; \
	clock_gettime(CLOCK_REALTIME, &ts); \
	print_ts_diff(name, __LINE__, &__ts_prev, &ts); \
} while (0)

#include "buf.c"
#include "mympfr.c"
#include "rewr.c"

void encode_in_base(const mpfr_t base, const mpfr_t in, buffer_t out)
{
	char* output;

	mp_prec_t bits = mpfr_get_prec(base);
	mp_prec_t bits2 = mpfr_get_prec(in);
	printf("Encoding with %d bits (input: %d bits)\n", (int)bits, (int)bits2);

	mpfr_t x;
	mpfr_init2(x, bits);
	mpfr_set(x, in, GMP_RNDN);

	long k = mpfr_digits_in_base(base, x);
	output = buf_init(out, k+1);
	
	START();
	mpfr_t base_to_the_k;
	mpfr_init2(base_to_the_k, bits);
	mpfr_pow_si(base_to_the_k, base, -k, GMP_RNDN);
	T("mpfr_pow_si");

	mpfr_mul(x, x, base_to_the_k, GMP_RNDN);
	mpfr_clear(base_to_the_k);
	T("mpfr_mul base**-k");

	mpfr_t d_k;
	mpfr_init2(d_k, bits);

	// Run loop for each j = [0..k-1]
	do
	{
		mpfr_modf(d_k, x, x, GMP_RNDN);
		*output++ = mpfr_get_uchar(d_k);
		mpfr_mul(x, x, base, GMP_RNDN);
	}
	while (k--);
	assert(output == out->buffer + out->size);
	T("mpfr_modf/mul loop");

	mpfr_mul(x, x, base, GMP_RNDN);
	printf("Residual: ");
	mpfr_out_str(stdout, 10, 0, x, GMP_RNDN);
	printf("\n");
	T("mpfr_out_str");

	mpfr_clear(d_k);
	mpfr_clear(x);
}

void decode_in_base(const mpfr_t base, mpz_t out, buffer_t in)
{
	/*
unsome :: (Floating a, RealFrac a) => a -> (Integer, [Integer]) -> a
unsome base (k,xs) = sum (zipWith (*) (iterate (/ base) (base ** fromIntegral k)) (map fromIntegral xs))
	*/

	const char* input = in->buffer;
	int k = in->size;

	// We may need log2(k) extra bits for storing the sum with full precision.
	mp_prec_t bits = mpfr_get_prec(base);
	printf("mp_prec_t has %d bits\n", (int)(sizeof(bits)*8));
	printf("decode_in_base has %d bits in base and %d digits\n", (int)bits, k);
	bits += 3000;
	printf("decode_in_base is trying to use %d bits.\n", (int)bits);

	START();
	mpfr_t sum;
	mpfr_init2(sum, 2*bits);
	mpfr_set_ui(sum, 0, GMP_RNDN);
	T("init sum");

	mpfr_t coeff;
	mpfr_init2(coeff, bits);
	mpfr_pow_ui(coeff, base, k-1, GMP_RNDN);
	T("base**(k-1)");

	mpfr_t temp;
	mpfr_init2(temp, bits);

	mpfr_t inv_base;
	mpfr_init2(inv_base, bits);
	mpfr_pow_si(inv_base, base, -1, GMP_RNDN);
	T("base**(-1)");

	while (k--)
	{
		printf("%d (%d): %d\n", (int)(input - in->buffer), k, *input);
		//mpfr_pow_ui(coeff, base, k, GMP_RNDN);
		mpfr_mul_ui(temp, coeff, *input++, GMP_RNDN);
		/*mpfr_out_str(stdout, 10, 0, coeff, GMP_RNDN);
		printf("\n");*/
		mpfr_add(sum, sum, temp, GMP_RNDN);
		mpfr_mul(coeff, coeff, inv_base, GMP_RNDN);
		//mpfr_div(coeff, coeff, base, GMP_RNDN);
	}
	assert(input == in->buffer + in->size);
	T("decode loop");

	mpfr_clear(coeff);
	mpfr_clear(temp);
	mpfr_clear(inv_base);

	mpfr_get_z(out, sum, GMP_RNDU);
	T("mpfr_get_z");

	mpfr_clear(sum);
}

void print_digits(buffer_t buffer)
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
void decode_bijective(mpz_t m)
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

void mpz_print(mpz_t op)
{
	mpz_out_str(stdout, 10, op);
	putchar('\n');
	fflush(stdout);
}

void run_program(const buffer_t in, buffer_t out, int argc, char* argv[])
{
	struct assreg* reg1 = mk_reg(1, NULL);
	buf_init0(reg1->buf);

	struct assreg* regs = mk_reg(0, reg1);
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

void print_ts_diff(const char* name, int line, struct timespec *start, struct timespec *end)
{
	double sdiff = end->tv_sec - start->tv_sec;
	double nsdiff = end->tv_nsec - start->tv_nsec;
	printf("TIME " __FILE__ ":%d %s: %fs\n", line, name, sdiff + nsdiff / 1000000000.0);
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
	const char* test_data = "Hello world!";
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
	//decode_bijective(out);
	mpz_clear(out);

	mpfr_free_cache();

	return 0;
}
