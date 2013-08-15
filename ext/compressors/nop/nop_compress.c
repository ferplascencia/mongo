/*-
 * Public Domain 2008-2013 WiredTiger, Inc.
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <wiredtiger.h>
#include <wiredtiger_ext.h>

static int
nop_compress(WT_COMPRESSOR *, WT_SESSION *,
    uint8_t *, size_t, uint8_t *, size_t, size_t *, int *);
static int
nop_decompress(WT_COMPRESSOR *, WT_SESSION *,
    uint8_t *, size_t, uint8_t *, size_t, size_t *);
static int
nop_pre_size(WT_COMPRESSOR *, WT_SESSION *, uint8_t *, size_t, size_t *);
static int
nop_terminate(WT_COMPRESSOR *, WT_SESSION *);

/*! [WT_COMPRESSOR initialization] */
/* Local compressor structure. */
typedef struct {
	WT_COMPRESSOR compressor;		/* Must come first */

	WT_EXTENSION_API *wt_api;		/* Extension API */
} NOP_COMPRESSOR;

/*
 * A simple shared library compression example.
 */
int
wiredtiger_extension_init(WT_CONNECTION *connection, WT_CONFIG_ARG *config)
{
	NOP_COMPRESSOR *nop_compressor;

	(void)config;				/* Unused parameters */

	if ((nop_compressor = calloc(1, sizeof(NOP_COMPRESSOR))) == NULL)
		return (errno);

	/*
	 * Allocate a local compressor structure, with a WT_COMPRESSOR structure
	 * as the first field, allowing us to treat references to either type of
	 * structure as a reference to the other type.
	 *
	 * This could be simplified if only a single database is opened in the
	 * application, we could use a static WT_COMPRESSOR structure, and a
	 * static reference to the WT_EXTENSION_API methods, then we don't need
	 * to allocate memory when the compressor is initialized or free it when
	 * the compressor is terminated.  However, this approach is more general
	 * purpose and supports multiple databases per application.
	 */
	nop_compressor->compressor.compress = nop_compress;
	nop_compressor->compressor.compress_raw = NULL;
	nop_compressor->compressor.decompress = nop_decompress;
	nop_compressor->compressor.pre_size = nop_pre_size;
	nop_compressor->compressor.terminate = nop_terminate;

	nop_compressor->wt_api = connection->get_extension_api(connection);

						/* Load the compressor */
	return (connection->add_compressor(
	    connection, "nop", (WT_COMPRESSOR *)nop_compressor, NULL));
}
/*! [WT_COMPRESSOR initialization] */

/*! [WT_COMPRESSOR compress] */
/*
 * A simple compression example that passes data through unchanged.
 */
static int
nop_compress(WT_COMPRESSOR *compressor, WT_SESSION *session,
    uint8_t *src, size_t src_len,
    uint8_t *dst, size_t dst_len,
    size_t *result_lenp, int *compression_failed)
{
	(void)compressor;			/* Unused parameters */
	(void)session;

	*compression_failed = 0;
	if (dst_len < src_len) {
		*compression_failed = 1;
		return (0);
	}

	memcpy(dst, src, src_len);
	*result_lenp = src_len;

	return (0);
}
/*! [WT_COMPRESSOR compress] */

/*! [WT_COMPRESSOR decompress] */
/*
 * A simple compression example that passes data through unchanged.
 */
static int
nop_decompress(WT_COMPRESSOR *compressor, WT_SESSION *session,
    uint8_t *src, size_t src_len,
    uint8_t *dst, size_t dst_len,
    size_t *result_lenp)
{
	(void)compressor;			/* Unused parameters */
	(void)session;
	(void)src_len;

	/*
	 * The destination length is the number of uncompressed bytes we're
	 * expected to return.
	 */
	memcpy(dst, src, dst_len);
	*result_lenp = dst_len;
	return (0);
}
/*! [WT_COMPRESSOR decompress] */

/*! [WT_COMPRESSOR presize] */
/*
 * A simple pre-size example that returns the source length.
 */
static int
nop_pre_size(WT_COMPRESSOR *compressor, WT_SESSION *session,
    uint8_t *src, size_t src_len,
    size_t *result_lenp)
{
	/* Unused parameters */
	(void)compressor;
	(void)session;
	(void)src;

	*result_lenp = src_len;
	return (0);
}
/*! [WT_COMPRESSOR presize] */

/*! [WT_COMPRESSOR terminate] */
/*
 * A simple termination example that frees the allocated memory.
 */
static int
nop_terminate(WT_COMPRESSOR *compressor, WT_SESSION *session)
{
	(void)session;				/* Unused parameters */

	free(compressor);
	return (0);
}
/*! [WT_COMPRESSOR terminate] */
