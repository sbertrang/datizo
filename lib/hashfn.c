/* postgresql: postgresql-9.2.2/src/backend/utils/hash/hashfn.c */
/*-------------------------------------------------------------------------
 *
 * hashfn.c
 *		Hash functions for use in dynahash.c hashtables
 *
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/utils/hash/hashfn.c
 *
 * NOTES
 *	  It is expected that every bit of a hash function's 32-bit result is
 *	  as random as every other; failure to ensure this is likely to lead
 *	  to poor performance of hash tables.  In most cases a hash
 *	  function should use hash_any() or its variant hash_uint32().
 *
 *-------------------------------------------------------------------------
 */

#include <assert.h>
#include <string.h>

#include "datizo.h"
#include "bitmapset.h"

/*
#include "access/hash.h"
*/


/*
 * string_hash: hash function for keys that are NUL-terminated strings.
 *
 * NOTE: this is the default hash function if none is specified.
 */
uint32_t
string_hash(const void *key, size_t keysize)
{
	/*
	 * If the string exceeds keysize-1 bytes, we want to hash only that many,
	 * because when it is copied into the hash table it will be truncated at
	 * that length.
	 */
	size_t		s_len = strlen((const char *) key);

	s_len = Min(s_len, keysize - 1);
	return hash_any((const unsigned char *) key,
								   (int) s_len);
}

/*
 * tag_hash: hash function for fixed-size tag values
 */
uint32_t
tag_hash(const void *key, size_t keysize)
{
	return hash_any((const unsigned char *) key,
								   (int) keysize);
}

/*
 * oid_hash: hash function for keys that are OIDs
 *
 * (tag_hash works for this case too, but is slower)
 */
#if 0
uint32_t
oid_hash(const void *key, size_t keysize)
{
	assert(keysize == sizeof(Oid));
	return hash_uint32((uint32_t) *((const Oid *) key));
}
#endif

/*
 * bitmap_hash: hash function for keys that are (pointers to) Bitmapsets
 *
 * Note: don't forget to specify bitmap_match as the match function!
 */
uint32_t
bitmap_hash(const void *key, size_t keysize)
{
	assert(keysize == sizeof(Bitmapset *));
	return bms_hash_value(*((const Bitmapset *const *) key));
}

/*
 * bitmap_match: match function to use with bitmap_hash
 */
int
bitmap_match(const void *key1, const void *key2, size_t keysize)
{
	assert(keysize == sizeof(Bitmapset *));
	return !bms_equal(*((const Bitmapset *const *) key1),
					  *((const Bitmapset *const *) key2));
}
