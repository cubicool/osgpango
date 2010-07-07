// -*-c++-*- Copyright (C) 2010 osgPango Development Team

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <osg/Texture2D>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgPango/Context>

namespace osgPango {

CachedGlyph::CachedGlyph(
	unsigned int     _img,
	const osg::Vec2& _origin,
	const osg::Vec2& _size,
	const osg::Vec2& _bl,
	const osg::Vec2& _br,
	const osg::Vec2& _ur,
	const osg::Vec2& _ul
):
img    (_img),
origin (_origin),
size   (_size),
bl     (_bl),
br     (_br),
ur     (_ur),
ul     (_ul) {
}

const float GlyphCache::DEFAULT_BASE_X = 1.0f;
const float GlyphCache::DEFAULT_BASE_Y = 1.0f;
const float GlyphCache::DEFAULT_BASE_H = 0.0f;

GlyphCache::GlyphCache(
	GlyphRenderer* renderer,
	unsigned int   width,
	unsigned int   height
):
_renderer   (renderer),
_x          (DEFAULT_BASE_X),
_y          (DEFAULT_BASE_Y),
_h          (DEFAULT_BASE_H),
_imgWidth   (width),
_imgHeight  (height) {
	if(!_renderer.valid()) _renderer = new GlyphRenderer();
}

const CachedGlyph* GlyphCache::getCachedGlyph(unsigned int i) {
	GlyphMap::const_iterator g = _glyphs.find(i);

	// If we already have cached data, return it.
	if(g != _glyphs.end()) return &g->second;

	return 0;
};

const CachedGlyph* GlyphCache::createCachedGlyph(PangoFont* font, PangoGlyphInfo* gi) {
	unsigned int   glyph = gi->glyph;
	PangoRectangle r;

	pango_font_get_glyph_extents(font, glyph, &r, 0);
	pango_extents_to_pixels(&r, 0);

	osgCairo::Glyph g(glyph, -r.x, -r.y);

	double w = r.width;
	double h = r.height;

	if(w <= 0.0f && h <= 0.0f) {
		_glyphs[glyph] = CachedGlyph();

		return const_cast<const CachedGlyph*>(&_glyphs[glyph]);
	}

	// We can't do this in constructor because we need to give the object time
	// to allow the user to set various caching options.
	
	osgCairo::CairoScaledFont* sf = pango_cairo_font_get_scaled_font(PANGO_CAIRO_FONT(font));
	
	osg::Vec4 extents = _renderer->getExtraGlyphExtents();

	double addw = extents[2] + 1.0f;
	double addh = extents[3] + 1.0f;

	if(w + addw >= _imgWidth || h + addh >= _imgHeight) {
		osg::notify(osg::WARN)
			<< "The single glyph " << glyph 
			<< " cannot fit on the allocated texture size; this is likely a critical"
			<< " bug. Please make sure you have a large enough texture to properly"
			<< " cache the desired font." << std::endl
		;

		return 0;
	}

	// If our remaining space isn't enough to accomodate another glyph, jump to another "row."
	if(_x + w + addw >= _imgWidth) {
		_x  = DEFAULT_BASE_X;
		_y += _h + addh;
	}

	osg::notify(osg::NOTICE) << "here1" << std::endl;

	// Make sure we have enough vertical space, too.
	if(_y + h + addh >= _imgHeight || !_layers.size()) _newImageAndTexture(sf);
	
	osg::notify(osg::NOTICE) << "here2" << std::endl;
	
	// Render glyph to layers.
	for(unsigned int layerIndex = 0; layerIndex < getNumLayers(); ++layerIndex) {
		osgCairo::Image* si = _layers[layerIndex].back().first.get();
		  
		si->identityMatrix();
		si->translate(_x, _y);
		
		si->save();
	
		_renderer->renderLayer(layerIndex, si, g, w, h);

		si->restore();
	}
	
	if(h > _h) _h = h;

	double tx = _x / _imgWidth;
	double ty = (_imgHeight - (h + extents[3]) - _y) / _imgHeight;
	double tw = (_x + w + extents[2]) / _imgWidth;
	double th = (_imgHeight - _y) / _imgHeight;
	
	osg::notify(osg::NOTICE) << "here3" << std::endl;

	_glyphs[glyph] = CachedGlyph(
		_layers[0].size() - 1,
		osg::Vec2(r.x, -(h + r.y)),
		// We don't use addw/addh here because we don't want the extra 1.0f pixel,
		// which is used during linear filtering.
		osg::Vec2(w + extents[2], h + extents[3]),
		osg::Vec2(tx, ty),
		osg::Vec2(tw, ty),
		osg::Vec2(tw, th),
		osg::Vec2(tx, th)
	);
	
	osg::notify(osg::NOTICE) << "here4" << std::endl;

	_x += w + addw;
	
	return const_cast<const CachedGlyph*>(&_glyphs[glyph]);
}

void GlyphCache::writeImagesAsFiles(const std::string& prefix) const {
	for(unsigned int i = 0; i < getNumLayers(); ++i) {
		std::ostringstream str;
		str << i;
		_writeImageVectorFiles(prefix + "_layer" + str.str() +"_", "", _layers[i]);
	}
}

bool GlyphCache::_newImageAndTexture(osgCairo::CairoScaledFont* sf) {
	if(!_layers.size())
		_layers.resize(getNumLayers());
	
	for(unsigned int i = 0; i < getNumLayers(); ++i) {
		// TODO: ? get imgWidth, imgHeight from renderer ?
		osgCairo::Image* si = new osgCairo::Image(_imgWidth, _imgHeight, CAIRO_FORMAT_A8);
	
		if(!si || !si->valid() || !si->createContext()) return false;
	
		si->setScaledFont(sf);
		
		osg::Texture2D* texture = new osg::Texture2D();

		texture->setImage(si);
		texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		
		_layers[i].push_back(std::make_pair(si, texture));
	}

	// Whenever a new image is created we reset our _x, _y, and _h values.
	// It's important that you do not create a new image unless you understand that this
	// will happen and how it will affect everything.
	_x = DEFAULT_BASE_X;
	_y = DEFAULT_BASE_Y;
	_h = DEFAULT_BASE_H;

	return true;
}

void GlyphCache::_writeImageVectorFiles(
	const std::string& prefix,
	const std::string& postfix,
	const Images& images
) const {
	unsigned int num = 0;

	for(Images::const_iterator i = images.begin(); i != images.end(); i++) {
		// This should never, ever happen. :(
		if(!i->first.get()) continue;

		std::ostringstream ss;

		ss << prefix << num << postfix << ".png";

		i->first.get()->writeToPNG(ss.str().c_str());

		osg::notify()
			<< "Wrote " << ss.str()
			<< "; " << i->first.get()->getImageSizeInBytes() / 1024.0f
			<< " KB internally." << std::endl
		;

		num++;
	}
}

osgCairo::Image* GlyphCache::_getImage(unsigned int index, unsigned int layerIndex) const {
  if(layerIndex < _layers.size() && index < _layers[layerIndex].size()) 
		return _layers[layerIndex][index].first;
	else
		return 0;
}

osg::Texture* GlyphCache::_getTexture(unsigned int index, unsigned int layerIndex) const {
  if(layerIndex < _layers.size() && index < _layers[layerIndex].size()) 
		return _layers[layerIndex][index].second;
	else
		return 0;
}

osg::ref_ptr<osg::Vec3Array> GlyphGeometry::_norms;
osg::ref_ptr<osg::Vec4Array> GlyphGeometry::_cols;

GlyphGeometry::GlyphGeometry():
_numQuads(0) {
	if(!_norms.valid()) {
		_norms = new osg::Vec3Array(1);

		(*_norms)[0].set(0.0f, 0.0f, 1.0f);
		(*_norms)[0].normalize();
	}

	if(!_cols.valid()) {
		_cols = new osg::Vec4Array(1);

		(*_cols)[0].set(1.0f, 1.0f, 1.0f, 1.0f);
	}

	setVertexArray(new osg::Vec3Array());
	setTexCoordArray(0, new osg::Vec2Array());
	
	setColorArray(_cols.get());
	setNormalArray(_norms.get());
	setNormalBinding(osg::Geometry::BIND_OVERALL);
	setColorBinding(osg::Geometry::BIND_OVERALL);
}

bool GlyphGeometry::finalize() {
	addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, _numQuads * 4));

	/*
	setDataVariance(osg::Object::STATIC);
	setUseDisplayList(false);
	setUseVertexBufferObjects(true);
	*/

	setDataVariance(osg::Object::DYNAMIC);
	setUseDisplayList(false);
	setUseVertexBufferObjects(false);

	return true;
}

bool GlyphGeometry::pushCachedGlyphAt(
	const CachedGlyph* cg,
	const osg::Vec2&   pos
) {
	static float z = 0.0f;
	static const osg::Matrix m(
		osg::Matrix::translate(0.0f, -1.0, 0.0f) *
		osg::Matrix::scale(1.0f, -1.0f, 1.0f));

	osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(getVertexArray());
	osg::Vec2Array* texs  = dynamic_cast<osg::Vec2Array*>(getTexCoordArray(0));

	if(!verts || !texs) return false;

	osg::Vec2 origin = pos + cg->origin;

	verts->push_back(osg::Vec3(origin, z));
	verts->push_back(osg::Vec3(origin + osg::Vec2(cg->size.x(), 0.0f), z));
	verts->push_back(osg::Vec3(origin + cg->size, z));
	verts->push_back(osg::Vec3(origin + osg::Vec2(0.0f, cg->size.y()), z));

	// z -= 0.001f;
	osg::Vec4 bl = osg::Vec4(cg->bl.x(), cg->bl.y(), 0.0, 1.0) * m;
	osg::Vec4 br = osg::Vec4(cg->br.x(), cg->br.y(), 0.0, 1.0) * m;
	osg::Vec4 ur = osg::Vec4(cg->ur.x(), cg->ur.y(), 0.0, 1.0) * m;
	osg::Vec4 ul = osg::Vec4(cg->ul.x(), cg->ul.y(), 0.0, 1.0) * m;

	texs->push_back(osg::Vec2(bl.x(), bl.y()));
	texs->push_back(osg::Vec2(br.x(), br.y()));
	texs->push_back(osg::Vec2(ur.x(), ur.y()));
	texs->push_back(osg::Vec2(ul.x(), ul.y()));

	_numQuads++;

	return true;
}

//GlyphRendererOutline::GlyphRendererOutline(unsigned int size):
//_outline(size) {
//}
//
//osg::Vec4 GlyphRendererOutline::getExtraGlyphExtents() const {
//	return osg::Vec4(_outline, _outline, _outline * 2, _outline * 2);
//}
//
//bool GlyphRendererOutline::renderGlyph(
//	osgCairo::Surface*     si,
//	const osgCairo::Glyph& g,
//	unsigned int           w,
//	unsigned int           h
//) {
//	if(!si) return false;
//
//	si->translate(_outline, _outline);
//
//	GlyphRenderer::renderGlyph(si, g, w, h);
//
//	return true;
//}
//
//bool GlyphRendererOutline::renderGlyphEffects(
//	osgCairo::Surface*     si,
//	const osgCairo::Glyph& g,
//	unsigned int           w,
//	unsigned int           h
//) {
//	if(!si) return false;
//
//	si->setLineJoin(CAIRO_LINE_JOIN_ROUND);
//	si->setLineWidth((_outline * 2) - 0.5f);
//	si->setAntialias(CAIRO_ANTIALIAS_SUBPIXEL);
//	si->translate(_outline, _outline);
//	si->glyphPath(g);
//	si->strokePreserve();
//	si->fill();
//	si->setOperator(CAIRO_OPERATOR_CLEAR);
//	
//	GlyphRenderer::renderGlyph(si, g, w, h);
//
//	return true;
//}
//
//GlyphRendererShadowOffset::GlyphRendererShadowOffset(int x, int y):
//_xOffset (x),
//_yOffset (y) {
//}
//
//osg::Vec4 GlyphRendererShadowOffset::getExtraGlyphExtents() const {
//	return osg::Vec4(0.0f, 0.0f, std::abs(_xOffset), std::abs(_yOffset));
//}
//
//bool GlyphRendererShadowOffset::renderGlyph(
//	osgCairo::Surface*     si,
//	const osgCairo::Glyph& g,
//	unsigned int           w,
//	unsigned int           h
//) {
//	if(!si) return false;
//	
//	if(_xOffset < 0) si->translate(std::abs(_xOffset), 0.0f);
//
//	if(_yOffset < 0) si->translate(0.0f, std::abs(_yOffset));
//
//	GlyphRenderer::renderGlyph(si, g, w, h);
//
//	return true;
//}
//
//bool GlyphRendererShadowOffset::renderGlyphEffects(
//	osgCairo::Surface*     si,
//	const osgCairo::Glyph& g,
//	unsigned int           w,
//	unsigned int           h
//) {
//	if(!si) return false;
//
//	si->save();
//
//	if(_xOffset > 0) si->translate(_xOffset, 0.0f);
//
//	if(_yOffset > 0) si->translate(0.0f, _yOffset);
//
//	GlyphRenderer::renderGlyph(si, g, w, h);
//
//	si->restore();
//	si->setOperator(CAIRO_OPERATOR_CLEAR);
//
//	if(_xOffset < 0) si->translate(std::abs(_xOffset), 0.0f);
//
//	if(_yOffset < 0) si->translate(0.0f, std::abs(_yOffset));
//
//	GlyphRenderer::renderGlyph(si, g, w, h);
//
//	return true;
//}
//
//GlyphRendererShadowGaussian::GlyphRendererShadowGaussian(unsigned int radius):
//_radius(radius) {
//}
//
//osg::Vec4 GlyphRendererShadowGaussian::getExtraGlyphExtents() const {
//	return osg::Vec4(_radius * 2, _radius * 2, _radius * 4, _radius * 4);
//}
//
//bool GlyphRendererShadowGaussian::renderGlyph(
//	osgCairo::Surface*     si,
//	const osgCairo::Glyph& g,
//	unsigned int           w,
//	unsigned int           h
//) {
//	if(!si) return false;
//
//	si->translate(_radius * 2, _radius * 2);
//
//	GlyphRenderer::renderGlyph(si, g, w, h);
//
//	return true;
//}
//
//bool GlyphRendererShadowGaussian::renderGlyphEffects(
//	osgCairo::Surface*     si,
//	const osgCairo::Glyph& g,
//	unsigned int           w,
//	unsigned int           h
//) {
//	if(!si) return false;
//
//	double add = _radius * 4.0f;
//	
//	// Create a temporary small surface and then copy that to the bigger one.
//	osgCairo::Surface tmp(w + add, h + add, CAIRO_FORMAT_ARGB32);
//
//	if(!tmp.createContext()) return false;
// 
//	osgCairo::CairoScaledFont* sf = si->getScaledFont();
//
//	tmp.setScaledFont(sf);
//	tmp.setLineJoin(CAIRO_LINE_JOIN_ROUND);
//	tmp.setLineWidth(_radius - 0.5f);
//	tmp.setAntialias(CAIRO_ANTIALIAS_SUBPIXEL);
//	tmp.translate(_radius * 2, _radius * 2);
//	tmp.glyphPath(g);
//	tmp.strokePreserve();
//	tmp.fill();
//
//	osgCairo::util::gaussianBlur(&tmp, _radius);
//	
//	tmp.setOperator(CAIRO_OPERATOR_CLEAR);
//	
//	GlyphRenderer::renderGlyph(&tmp, g, w, h);
//
//	si->setSourceSurface(&tmp, 0, 0);
//	si->paint();
//
//	return true;
//}
//
//GlyphRendererShadowGaussianMultipass::GlyphRendererShadowGaussianMultipass(unsigned int radius):
//GlyphRendererShadowGaussian(radius) {
//}
//
//unsigned int GlyphRendererShadowGaussianMultipass::getNumPasses() const {
//	return 2;
//}
//
//bool GlyphRendererShadowGaussianMultipass::updateOrCreateState(int pass, osg::Geode* geode) {
//		
//	std::string VERTEX_SHADER =
//		"#version 120\n"
//		"varying vec4 pangoTexCoord;"
//		"void main() {"
//		"	pangoTexCoord = gl_MultiTexCoord0;"
//		"	gl_Position = ftransform();"
//		"}"
//		;
//
//	std::string FRAGMENT_SHADER =
//		"varying vec4 pangoTexCoord;"
//		"uniform vec3 pangoColor[8];"
//		"uniform sampler2D pangoTex0;"
//		"uniform sampler2D pangoTex1;"
//		"uniform float pangoAlpha;"
//		"void main() {"
//		"	float tex0 = texture2D(pangoTex0, pangoTexCoord.st).a;"   
//		"	float tex1 = texture2D(pangoTex1, pangoTexCoord.st).a;"   
//		" vec3 color0 = pangoColor[0].rgb * tex0;"
//		" vec3 color1 = pangoColor[1].rgb * tex1;"
//		" float alpha0 = tex0;"
//		" float alpha1 = tex1;"
//		;
//
//	if(pass == 0)		
//	{
//		FRAGMENT_SHADER += 
//			"gl_FragColor = vec4(color1, alpha1);"
//			"}";
//	}
//
//	if(pass == 1)
//	{
//		FRAGMENT_SHADER += 
//			"gl_FragColor = vec4(color0, alpha0);"
//			"}";
//	}
//	
//	osg::Program* program = new osg::Program();
//	osg::Shader*  vert    = new osg::Shader(osg::Shader::VERTEX, VERTEX_SHADER);
//	osg::Shader*  frag    = new osg::Shader(osg::Shader::FRAGMENT, FRAGMENT_SHADER);
//
//	vert->setName("pangoRendererVert");
//	frag->setName("pangoRendererFrag");
//	program->setName("pangoRenderer");
//
//	program->addShader(vert);
//	program->addShader(frag);
//
//	osg::StateSet* state = geode->getOrCreateStateSet();
//
//	state->setAttributeAndModes(program);
//	state->setMode(GL_BLEND, osg::StateAttribute::ON);
//	state->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.01f));
//	state->setAttribute(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
//	
//	state->getOrCreateUniform("pangoAlpha", osg::Uniform::FLOAT)->set(1.0f);
//	state->getOrCreateUniform("pangoTex0", osg::Uniform::INT)->set(0);
//	state->getOrCreateUniform("pangoTex1", osg::Uniform::INT)->set(1);
//
//	return true;
//}
//
//bool GlyphRendererShadowGaussianMultipass::updateOrCreateState(osg::Geometry* geometry, const GlyphGeometryState& gs) {
//	osg::Uniform* pangoColor = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "pangoColor", 8);
//
//	pangoColor->setElement(0, gs.color);
//	pangoColor->setElement(1, gs.effectsColor);
//	
//	osg::StateSet* state = geometry->getOrCreateStateSet();
//
//	state->setTextureAttributeAndModes(0, gs.texture, osg::StateAttribute::ON);
//	state->setTextureAttributeAndModes(1, gs.effectsTexture, osg::StateAttribute::ON);
//	state->addUniform(pangoColor);
//
//	return true;
//}

}
