#pragma once

#include "crc32c.h"
#include "log.h"

// Primary hash calculation for all shader file names.

// 64 bit magic FNV-0 and FNV-1 prime
#define FNV_64_PRIME ((UINT64)0x100000001b3ULL)
static UINT64 fnv_64_buf(const void* buf, size_t len)
{
	UINT64 hval = 0;
	unsigned const char* bp = (unsigned const char*)buf;	/* start of buffer */
	unsigned const char* be = bp + len;		/* beyond end of buffer */

	// FNV-1 hash each octet of the buffer
	while (bp < be)
	{
		// multiply by the 64 bit FNV magic prime mod 2^64 */
		hval *= FNV_64_PRIME;
		// xor the bottom with the current octet
		hval ^= (UINT64)*bp++;
	}
	return hval;
}

// Create hash code for textures or buffers.  

// Wrapped in try/catch because it can crash in Dirt Rally,
// because of noncontiguous or non-mapped memory for the texture.  Not sure this
// is the best strategy.

// Now switching to use crc32_append instead of fnv_64_buf for performance. This
// implementation of crc32c uses the SSE 4.2 instructions in the CPU to calculate,
// and is some 30x faster than fnv_64_buf.
// 
// Not changing shader hash calculation as there are thousands of shaders already
// in the field, and there is no known bottleneck for that calculation.

static uint32_t crc32c_hw(uint32_t seed, const void* buffer, size_t length)
{
	try
	{
		const uint8_t* cast_buffer = static_cast<const uint8_t*>(buffer);

		return crc32c_append(seed, cast_buffer, length);
	}
	catch (...)
	{
		// Fatal error, but catch it and return null for hash.
		LogInfo("   ******* Exception caught while calculating crc32c_hw hash ******\n");
		return 0;
	}
}