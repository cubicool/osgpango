// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgCairo/Util>
#include <osgCairo/ReadFile>
#include <osgPango/GlyphLayer>

namespace osgPango {

GlyphLayerFaceBitmap::GlyphLayerFaceBitmap(const std::string &path):
GlyphLayer(CAIRO_FORMAT_ARGB32),
_pattern(0), 
_repeatX(1), 
_repeatY(1)
{
	setFaceBitmap(path);
}

GlyphLayerFaceBitmap::~GlyphLayerFaceBitmap()
{
	cairo_pattern_destroy(_pattern);
}

	
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
	cairo_pattern_set_matrix (_pattern, &scale_matrix);

	cairo_set_source(c, _pattern);

	cairo_glyph_path(c, glyph, 1);
	cairo_fill(c);

	return true;
}
	
void GlyphLayerFaceBitmap::setFaceBitmap(const std::string &path) {
  if(_path == path || path.empty()) return;
  
  _path = path;
  _bitmap = osgCairo::readImageFile(_path);
  
  if(!_bitmap || !_bitmap->valid()) 
  {
    OSG_WARN << "(GlyphLayerFaceBitmap::setFaceBitmap): Can't load image: " << _path << std::endl; 
    return;
  }
  // TODO: Consider if we can flip in osgCairo::readImageFile
  _bitmap->flipVertical();
  
  _pattern = cairo_pattern_create_for_surface(_bitmap->getSurface());
  if(cairo_pattern_status(_pattern)) return;
  
  cairo_pattern_set_extend(_pattern, CAIRO_EXTEND_REPEAT);
}

} 