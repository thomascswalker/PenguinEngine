#include "Font.h"

float TTF::getScaleForPixelHeight(FontInfo* fontInfo, int32 lineHeight)
{
	int deltaHeight = fontInfo->hhea->ascender - fontInfo->hhea->descender;
	return (float)lineHeight / (float)deltaHeight;
}
