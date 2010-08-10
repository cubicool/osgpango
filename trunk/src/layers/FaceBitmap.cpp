// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgCairo/Util>
#include <osgCairo/ReadFile>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerFaceBitmap::GlyphLayerFaceBitmap(const std::string& path):
GlyphLayer (CAIRO_FORMAT_ARGB32),
_repeatX   (1), 
_repeatY   (1),
_pattern   (0) {
	setBitmap(path);
}

GlyphLayerFaceBitmap::~GlyphLayerFaceBitmap() {
	cairo_pattern_destroy(_pattern);
}

/*
void setSubImageAlpha(
	cairo_surface_t* surface,
	unsigned int x,
	unsigned int y,
	unsigned int w,
	unsigned int h,
	unsigned char alpha
) {
	unsigned char* data   = cairo_image_surface_get_data(surface);
	cairo_format_t format = cairo_image_surface_get_format(surface);
	int            stride = cairo_image_surface_get_stride(surface);

	unsigned int channels = 0;
	unsigned int offset = 0;

	if(format == CAIRO_FORMAT_ARGB32) {
		channels = 4;
		offset = 3;
	}

	else if(format == CAIRO_FORMAT_A8) { 
		channels = 1;
		offset = 0;
	}

	else return;

	for(unsigned int iY = y; iY < y + h; iY++) {
		for(unsigned int iX = x; iX < x + w; iX++) {
			unsigned char* dataPtr = &data[iY * stride + iX * channels];
			dataPtr[offset] = alpha;
		}
	}
}
*/

bool GlyphLayerFaceBitmap::render(
	cairo_t*       c,
	cairo_glyph_t* glyph,
	unsigned int   width,
	unsigned int   height
) {
	if(cairo_status(c) || !glyph || !_bitmap || cairo_pattern_status(_pattern)) return false;

	double bw = _bitmap->getSurfaceWidth();
	double bh = _bitmap->getSurfaceHeight();

	cairo_matrix_t scale_matrix;

	cairo_matrix_init_scale(&scale_matrix, bw / width * _repeatX, bh / height * _repeatY);
	cairo_pattern_set_matrix(_pattern, &scale_matrix);

	cairo_set_source(c, _pattern);

	cairo_glyph_path(c, glyph, 1);
	cairo_fill(c);

	return true;
}
	
void GlyphLayerFaceBitmap::setBitmap(const std::string& path) {
	if(_path == path || path.empty()) return;

	_path   = path;
	_bitmap = osgCairo::readImageFile(_path);

	if(!_bitmap || !_bitmap->valid())  {
		OSG_WARN
			<< "osgPango::GlyphLayerFaceBitmap::setBitmap: Can't load image: "
			<< _path
			<< std::endl
		; 
		
		return;
	}

	// TODO: Consider if we can flip in osgCairo::readImageFile
	_bitmap->flipVertical();

	_pattern = cairo_pattern_create_for_surface(_bitmap->getSurface());
	
	if(cairo_pattern_status(_pattern)) return;

	cairo_pattern_set_extend(_pattern, CAIRO_EXTEND_REPEAT);
	cairo_pattern_set_filter(_pattern, CAIRO_FILTER_BEST);
}

} 
