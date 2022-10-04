static void encode_in_base(const mpfr_t base, const mpfr_t in, buffer_t out)
{
	char* output;

	mp_prec_t bits = mpfr_get_prec(base);
#if DEBUG
	mp_prec_t bits2 = mpfr_get_prec(in);
	printf("Encoding with %d bits (input: %d bits)\n", (int)bits, (int)bits2);
#endif

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
#if DEBUG
	printf("Residual: ");
	mpfr_out_str(stdout, 10, 0, x, GMP_RNDN);
	printf("\n");
	T("mpfr_out_str");
#endif

	mpfr_clear(d_k);
	mpfr_clear(x);
}

static void decode_in_base(const mpfr_t base, mpz_t out, buffer_t in)
{
	/*
unsome :: (Floating a, RealFrac a) => a -> (Integer, [Integer]) -> a
unsome base (k,xs) = sum (zipWith (*) (iterate (/ base) (base ** fromIntegral k)) (map fromIntegral xs))
	*/

	const char* input = in->buffer;
	int k = in->size;

	// We may need log2(k) extra bits for storing the sum with full precision.
	mp_prec_t bits = mpfr_get_prec(base);
#if DEBUG
	printf("mp_prec_t has %d bits\n", (int)(sizeof(bits)*8));
	printf("decode_in_base has %d bits in base and %d digits\n", (int)bits, k);
#endif
	bits += 3000;
#if DEBUG
	printf("decode_in_base is trying to use %d bits.\n", (int)bits);
#endif

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
#if DEBUG
		printf("%d (%d): %d\n", (int)(input - in->buffer), k, *input);
#endif
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

