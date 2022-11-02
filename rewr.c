
struct reg
{
	int index;
	buffer_t buf;

	struct reg* next;
};

struct reg* mk_reg(int i, struct reg* next)
{
	struct reg* ret = (struct reg*)calloc(1, sizeof(struct reg));
	ret->index = i;
	ret->next = next;
	return ret;
}
struct reg* dl_reg(struct reg* reg)
{
	struct reg* ret = reg->next;
	buf_clear(reg->buf);
	free(reg);
	return ret;
}

void no()
{
	fprintf(stderr, "No.\n");
	exit(1);
}
#define maybe(i) if (i) ; else no()

struct reg* lup(struct reg** reg, int i)
{
	if (!reg || !*reg)
	{
		return *reg = mk_reg(i, *reg);
	}
	struct reg* a = *reg;
	if (a->index == i)
	{
		return a;
	}
	struct reg* b = a->next;
	if (b && b->index == i)
	{
		// * -> a -> b -> c => * -> b -> a -> c
		//
		// a.next = c
		// b.next = a
		// *.next = b
		struct reg* c = b->next;
		*reg = b;
		a->next = c;
		b->next = a;
		return b;
	}
	return lup(&a->next, i);
}

bool doop(buffer_t* in, const char* f, const char* fe, const char* t, const char* te, buffer_t* out)
{
	maybe(in);
	maybe(out);

	maybe(f < fe);
	maybe(t < te);

	bool replaced = false;
	const char* r = (*in)->buffer;
	const char* re = r + (*in)->size;
	const char* rw = r;

	while (r < re)
	{
        // TODO This is doing strstr/memmem
		const char* r2 = r;
		const char* fr = f;
		while (fr < fe && r2 < re && *fr == *r2) fr++, r2++;
		if (fr < fe)
		{
			r++;
			continue;
		}

		replaced = true;
		buf_append(*out, rw, r);
		buf_append(*out, t, te);
		rw = r = r2;
	}
	buf_append(*out, rw, re);
	return replaced;
}

void interp(struct reg** regs, const char** prog, size_t ip, size_t proglen)
{
    bool flag = false;
	while (ip < proglen)
	{
		if (DEBUG) printf("Run %d: \"%s\"\n", (int)ip, prog[ip]);

		char* end = NULL;
		const int i0 = strtol(prog[ip], &end, 10);
		maybe(*end++ == '/');
		if (!*end)
		{
			maybe(i0);
			if (DEBUG) printf("GOTO %d\n", i0);
			if (i0 < 0)
			{
				size_t new_ip = flag ? -i0 : ip + 1;
				flag = false;
				ip = new_ip;
			}
			else
			{
				ip = i0;
			}
			continue;
		}
		const char* const s = end;
		const char* s2 = strchr(s, '/');
		maybe(s2++);
		const char* s3 = strchr(s2, '/');
		maybe(s3++);

		const int i1 = strtol(s3, &end, 10);
		maybe(end != s3);
		maybe(!*end);

		struct reg* r0 = lup(regs, i0);
		struct reg* r1 = lup(regs, i1);
		if (doop(&r0->buf, s, s2-1, s2, s3-1, &r1->buf)) {
			flag = true;
		}

		if (i1 == 1) { if (DEBUG) printf("Output written. Done.\n"); break; }
		ip++;
	}
}
