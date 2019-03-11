#include "Formats.h"

#include <string.h>

#include <assert.h>

#define STR2ENUM(str, aString, aValues) \
for (unsigned int i = 0; i < sizeof(aValues)/sizeof(aValues[0]); ++i) \
{ \
	if (!strcmp(str, aStrings[i])) \
	{ \
		return(aValues[i]); \
	} \
}

#define DECLARE_STRINGS(aStrings) static const char * aStrings [] = ENUM_LIST
#define DECLARE_VALUES(aValues) static TextureFormat aValues [] = ENUM_LIST

namespace RenderGraph
{

/**
 * @brief strToFormat
 * @param str
 * @return
 */
TextureFormat strToFormat(const char * str)
{
	#define ENUM_LIST \
	{ \
		X(R8), \
		X(RG8), \
		X(RGBA8), \
		X(R16), \
		X(RG16), \
		X(RGBA16), \
		X(R16F), \
		X(RG16F), \
		X(RGBA16F), \
		X(R32F), \
		X(RG32F), \
		X(RGBA32F), \
		X(R8I), \
		X(RG8I), \
		X(RGBA8I), \
		X(R16I), \
		X(RG16I), \
		X(RGBA16I), \
		X(R32I), \
		X(RG32I), \
		X(RGBA32I), \
		X(R8UI), \
		X(RG8UI), \
		X(RGBA8UI), \
		X(R16UI), \
		X(RG16UI), \
		X(RGBA16UI), \
		X(R32UI), \
		X(RG32UI), \
		X(RGBA32UI), \
		X(RGB10_A2), \
		X(RGB10_A2UI), \
		X(R11F_G11F_B10F), \
		X(SRGB8_ALPHA8), \
		X(DEPTH_COMPONENT16), \
		X(DEPTH_COMPONENT24), \
		X(DEPTH_COMPONENT32F), \
		X(DEPTH24_STENCIL8), \
		X(DEPTH32F_STENCIL8), \
		X(STENCIL_INDEX8) \
	}

	#define X(e) #e
	DECLARE_STRINGS(aStrings);
	#undef X

	#define X(e) e
	DECLARE_VALUES(aValues);
	#undef X

	STR2ENUM(str, aStrings, aValues)

	#undef ENUM_LIST

	assert(false);
	return(UNKNOWN);
}

}
