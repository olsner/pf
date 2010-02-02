
int mpfr_fits_uchar_p(mpfr_t op)
{
	return mpfr_fits_ushort_p(op, GMP_RNDN) && mpfr_get_ui(op, GMP_RNDN) < 256;
}

char mpfr_get_uchar(mpfr_t op)
{
	assert(mpfr_integer_p(op));
	assert(mpfr_fits_uchar_p(op));

	return mpfr_get_ui(op, GMP_RNDN);
}

void mpfr_log_base(mpfr_t rop, const mpfr_t base, const mpfr_t op1)
{
	mpfr_t log_base, log_op1;
	mpfr_init2(log_base, mpfr_get_prec(rop));
	mpfr_init2(log_op1, mpfr_get_prec(rop));
	mpfr_log(log_base, base, GMP_RNDN);
	mpfr_log(log_op1, op1, GMP_RNDN);
	mpfr_div(rop, log_op1, log_base, GMP_RNDN);
	mpfr_clear(log_base);
	mpfr_clear(log_op1);
}

void split_num(mpfr_t integral, mpfr_t frac, const mpfr_t in)
{
	mpfr_trunc(integral, in);
}

void mpfr_init_zprec(mpfr_t out, const mpz_t prec)
{
	size_t bits = mpz_sizeinbase(prec, 2);
	mpfr_init2(out, bits); // Magic constant: Get some more precision than we should perhaps need, just in case.
	mpfr_set_z(out, prec, GMP_RNDN);
}


void mpfr_exp_ui(mpfr_t out, unsigned long ui, mp_rnd_t rnd)
{
	mpfr_set_ui(out, ui, rnd);
	mpfr_exp(out, out, rnd);
}

unsigned long mpfr_digits_in_base(const mpfr_t base, const mpfr_t x)
{
	mpfr_t lx;
	mpfr_init2(lx, mpfr_get_prec(x));
	mpfr_log_base(lx, base, x);

	unsigned long ret = mpfr_get_ui(lx, GMP_RNDD);
	mpfr_clear(lx);
	return ret;
}

#define mpfr_const_e(out, rnd) mpfr_exp_ui(out, 1, rnd)

