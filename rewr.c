
struct assreg
{
	int index;
	buffer_t buf;

	struct assreg* next;
};

struct assreg* mk_reg(int i, struct assreg* next)
{
	struct assreg* ret = (struct assreg*)calloc(1, sizeof(struct assreg));
	ret->index = i;
	ret->next = next;
	return ret;
}
struct assreg* dl_reg(struct assreg* reg)
{
	struct assreg* ret = reg->next;
	free(reg);
	return ret;
}

void no()
{
	fprintf(stderr, "No.\n");
	exit(1);
}
#define maybe(i) if (i) ; else no()

buffer_t* lup(struct assreg** reg, int i)
{
	if (!reg || !*reg)
	{
		return &(*reg = mk_reg(i, *reg))->buf;
	}
	struct assreg* a = *reg;
	if (a->index == i)
	{
		return &a->buf;
	}
	struct assreg* b = a->next;
	if (b && b->index == i)
	{
		// * -> a -> b -> c => * -> b -> a -> c
		//
		// a.next = c
		// b.next = a
		// *.next = b
		struct assreg* c = b->next;
		*reg = b;
		a->next = c;
		b->next = a;
		return &b->buf;
	}
	return lup(&a->next, i);
}

void doop(buffer_t* in, const char* f, const char* fe, const char* t, const char* te, buffer_t* out)
{
	maybe(in);
	maybe(out);

	maybe(f < fe);
	maybe(t < te);

	const char* r = (*in)->buffer;
	const char* re = r + (*in)->size;
	const char* rw = r;

	while (r < re)
	{
		const char* r2 = r;
		const char* fr = f;
		while (fr < fe && r2 < re && *fr++ == *r2++);
		if (fr <= fe)
		{
			r++;
			continue;
		}

		buf_append(*out, rw, r);
		buf_append(*out, t, te);
		rw = r = r2;
	}
}

void interp(struct assreg* regs, const char** prog, size_t ip, size_t proglen)
{
	while (ip < proglen)
	{
		printf("Run %d: \"%s\"\n", (int)ip, prog[ip]);

		char* end, *s, *s2, *s3;
		int i0 = strtol(prog[ip], &end, 10);
		maybe(*end++ == '/');
		if (!*end) { printf("GOTO %d\n", i0); ip = i0; continue; }
		s = end;
		s2 = strchr(s, '/');
		maybe(s2++);
		s3 = strchr(s2, '/');
		maybe(s3++);

		int i1 = strtol(s3, &end, 10);
		maybe(end != s3);
		maybe(!*end);

		doop(lup(&regs, i0), s, s2-1, s2, s3-1, lup(&regs, i1));

		if (i1 == 1) { printf("Output written. Done.\n"); break; }
		ip++;
	}
}

