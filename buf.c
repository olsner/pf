
typedef struct { char* buffer; size_t size; } buffer_t[1];
static char* buf_init(buffer_t buf, size_t size)
{
	buf->buffer = size ? malloc(size) : 0;
	buf->size = size;
	return buf->buffer;
}
static void buf_init0(buffer_t buf)
{
	buf_init(buf, 0);
}
static void buf_copy(buffer_t out, const buffer_t in)
{
	memcpy(buf_init(out, in->size), in->buffer, in->size);
}
static void buf_move(buffer_t new, buffer_t old)
{
	memcpy(new, old, sizeof(buffer_t));
	memset(old, 0, sizeof(buffer_t));
}
static void buf_append(buffer_t out, const char* p, const char* end)
{
	size_t sz = end - p;
	size_t oldsz = out->size;
	size_t newsz = oldsz + sz;
	out->buffer = realloc(out->buffer, newsz);
	memcpy(out->buffer + oldsz, p, sz);
	out->size = newsz;
}
static void buf_clear(buffer_t buf)
{
	free(buf->buffer);
	memset(buf, 0, sizeof(buf));
}
