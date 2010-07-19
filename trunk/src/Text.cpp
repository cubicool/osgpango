// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <sstream>
#include <algorithm>
#include <osg/io_utils>
#include <osg/Math>
#include <osg/Image>
#include <osgUtil/TransformAttributeFunctor>		
#include <osgPango/Context>

namespace osgPango {

bool TextOptions::setupPangoLayout(PangoLayout* layout) const {
	if(alignment != TEXT_ALIGN_JUSTIFY) {
		PangoAlignment pa = PANGO_ALIGN_LEFT;

		if(alignment == TEXT_ALIGN_CENTER) pa = PANGO_ALIGN_CENTER;

		else if(alignment == TEXT_ALIGN_RIGHT) pa = PANGO_ALIGN_RIGHT;
	
		pango_layout_set_alignment(layout, pa);
	}

	else pango_layout_set_justify(layout, true);

	if(width > 0) pango_layout_set_width(layout, width * PANGO_SCALE);

	if(height > 0) pango_layout_set_height(layout, height * PANGO_SCALE);

	if(indent > 0) pango_layout_set_indent(layout, indent * PANGO_SCALE);

	if(spacing > 0) pango_layout_set_spacing(layout, spacing * PANGO_SCALE);

	return true;
}

Text::Text(ColorMode cm):
_colorMode(cm),
_lastTransform(osg::Matrixd::identity()) {
	clear();
}

Text::~Text() {
}

void Text::clear() {
	for(GlyphGeometryMap::iterator g = _ggMap.begin(); g != _ggMap.end(); g++) {
		GlyphGeometryIndex& ggi = g->second;

		ggi.clear();
	}

	_size      = osg::Vec2();
	_origin    = osg::Vec2();
	_lastX     = 0;
	_lastY     = 0;
	_baseline  = 0;
	_init      = false;
	_newGlyphs = false;
	_finalized = false;
	_lastTransform = osg::Matrixd::identity();
}

GlyphGeometry* createGlyphGeometry() {
	static unsigned int ggId = 0;

	std::ostringstream ss;

	ss << "GlyphGeometry_" << ggId;

	ggId++;

	GlyphGeometry* gg = new GlyphGeometry();

	gg->setName(ss.str());

	return gg;
}

void Text::drawGlyphs(PangoFont* font, PangoGlyphString* glyphs, int x, int y) {
	// Get the GlyphCache from a key, which may or may not be set via PangoAttr if I
	// can it to work properly. :)
	GlyphCache* gc = Context::instance().getGlyphCache(font, _glyphRenderer);

	if(!gc) return;

	osg::Vec2::value_type currentY = -(y / PANGO_SCALE);

	osg::Vec2 layoutPos(x / PANGO_SCALE, currentY);

	if(static_cast<int>(currentY) == _lastY) layoutPos[0] += _lastX;

	else _lastX = 0.0f;

	// TODO: Enabling optional honoring of extents...
	// osg::Vec4 extents = gc->getGlyphRenderer()->getExtraGlyphExtents();
	osg::Vec4 extents = osg::Vec4();

	ColorPair color = Context::instance().getColorPair();
	
	if(_colorMode == COLOR_MODE_PALETTE_ONLY) {
		if(_palette.size() >= 2) {
			color.first.set(_palette[0]);
			color.second.set(_palette[1]);
		}

		else osg::notify(osg::WARN)
			<< "You have set the ColorMode to COLOR_MODE_PALETTE_ONLY on a Text object but "
			<< "did not actually provide the ColorPalette before calling addText(); this will "
			<< "cause strange effects and create extraneous StateSet objects, but is not fatal."
			<< std::endl
		;
	}

	for(int i = 0; i < glyphs->num_glyphs; i++) {
		PangoGlyphInfo* gi = glyphs->glyphs + i;

		if((gi->glyph & PANGO_GLYPH_UNKNOWN_FLAG)) {
			PangoFontMetrics* metrics = pango_font_get_metrics(font, 0);

			pango_font_metrics_unref(metrics);

			continue;
		}

		const CachedGlyph* cg = gc->getCachedGlyph(gi->glyph);

		if(!cg) {
			cg = gc->createCachedGlyph(font, gi);

			_newGlyphs = true;
		}

		if(!cg) continue;

		GlyphGeometryIndex& ggi = _ggMap[GlyphGeometryMapKey(gc, color)];

		if(cg->size.x() > 0.0f && cg->size.y() > 0.0f) {
			osg::Vec2 pos(
				(gi->geometry.x_offset / PANGO_SCALE) + extents[0],
				(gi->geometry.y_offset / PANGO_SCALE) + extents[1]
			);

			if(!ggi[cg->img]) ggi[cg->img] = createGlyphGeometry();

			ggi[cg->img]->pushCachedGlyphAt(
				cg,
				pos + layoutPos
			);
		}

		layoutPos += osg::Vec2((gi->geometry.width / PANGO_SCALE) + extents[0], 0.0f);
		_lastX    += extents[0];
	}

	_lastY = currentY;

	// Use the lowest baseline of all the texts that are added.
	int baseline = y / PANGO_SCALE;

	if(!_init || baseline > _baseline) _baseline = baseline;
}

void Text::addText(const std::string& str, int x, int y, const TextOptions& to) {
	String       text;
	PangoLayout* layout = pango_layout_new(Context::instance().getPangoContext());

	if(str.size()) {
		text.set(str, OSGPANGO_ENCODING);

		std::string utf8 = text.createUTF8EncodedString();

		if(to.markup) pango_layout_set_markup(layout, utf8.c_str(), -1);
		
		else pango_layout_set_text(layout, utf8.c_str(), -1);
	}

	to.setupPangoLayout(layout);

	_glyphRenderer = to.renderer;

	Context::instance().drawLayout(this, layout, x, y);

	// Get text dimensions and whatnot; we'll accumulate this data after each rendering
	// to keep it accurate.
	PangoRectangle rect;

	pango_layout_get_pixel_extents(layout, &rect, 0);

	g_object_unref(layout);

	const GlyphRenderer* gr = Context::instance().getGlyphRenderer(to.renderer);

	if(!gr) return;

	const osg::Vec4& extents = gr->getExtraGlyphExtents();

	osg::Vec2::value_type ox = -(x + rect.x); //- extents[0];
	osg::Vec2::value_type oy = y + rect.y - extents[3];
	osg::Vec2::value_type sw = rect.width;
	osg::Vec2::value_type sh = rect.height + extents[3];

	if(!_init) {
		_origin.set(ox, oy);
		_size.set(sw, sh);
	}
	
	else {
		if(ox > _origin[0]) _origin[0] = ox;

		if(oy > _origin[1]) _origin[1] = oy;

		if(sw > _size[0]) _size[0] = sw;

		if(sh > _size[1]) _size[1] = sh;
	}

	// We've run ONCE, so we're initialized to some state. Everything else from
	// here is based on this position, greater or lower.
	_init = true;
}

void Text::setColorPalette(const ColorPalette& cp) {
	_palette = cp;
}

osg::Vec2 Text::getOriginBaseline() const {
	return osg::Vec2(_origin.x(), _baseline);
}

osg::Vec2 Text::getOriginTranslated() const {
	return osg::Vec2(_origin.x(), _size.y() + _origin.y());
}

bool Text::_finalizeGeometry(osg::Group* group) {
	typedef std::map<GlyphRenderer*, GeometryList> RendererGeometry;

	RendererGeometry rg;

	for(GlyphGeometryMap::iterator g = _ggMap.begin(); g != _ggMap.end(); g++) {
		GlyphGeometryIndex& ggi = g->second;

		for(GlyphGeometryIndex::iterator i = ggi.begin(); i != ggi.end(); i++) {
			GlyphCache* gc    = g->first.first;
			ColorPair   color = g->first.second;
			
			for(unsigned int layer = 0; layer < gc->getNumLayers(); ++layer) {
				osg::Image* texture = gc->getImage(i->first, layer);

				if(_newGlyphs && texture) texture->dirty();
			}

			if(!i->second->finalize()) continue;

			if(rg.find(gc->getGlyphRenderer()) == rg.end())
				rg.insert(std::make_pair(gc->getGlyphRenderer(), GeometryList()))
			;

			rg[gc->getGlyphRenderer()].push_back(std::make_pair(
				i->second, 
				GlyphGeometryState()
			));
			
			GlyphGeometryState& ggs = rg[gc->getGlyphRenderer()].back().second;

			if(_palette.size() < 2) _palette.resize(2);

			if(_colorMode == COLOR_MODE_MARKUP_OVERWRITE) {
				_palette[0] = color.first;
				_palette[1] = color.second;
			}

			for(unsigned int layer = 0; layer < gc->getNumLayers(); ++layer) {
				ggs.textures.push_back(gc->getTexture(i->first, layer));
				ggs.colors.push_back(_palette[layer]);
			}
		}
	}

	unsigned int maxPasses = 0;

	// First create/update geometry states which are common for each pass. During iteration update maximum
	// number of passes.
	for(RendererGeometry::const_iterator ct = rg.begin(); ct != rg.end(); ++ct) {
		GlyphRenderer*      renderer = ct->first;
		const GeometryList& gl       = ct->second;

		maxPasses = std::max(renderer->getNumPasses(), maxPasses);

		for(GeometryList::const_iterator i = gl.begin(); i != gl.end(); i++) {
			renderer->updateOrCreateState(i->first, i->second);
		}
	}

	// Create structure for passes.
	for(unsigned int i = 0; i < maxPasses; ++i) {
		osg::Group* pass = new osg::Group();

		pass->getOrCreateStateSet()->setRenderBinDetails(i, "RenderBin");

		group->addChild(pass);
	}

	// Assign renderers to passes.
	for(RendererGeometry::const_iterator ct = rg.begin(); ct != rg.end(); ++ct) {
		GlyphRenderer*      renderer  = ct->first;
		const GeometryList& gl        = ct->second;

		for(unsigned int i = 0; i < renderer->getNumPasses(); ++i) {
			// Each renderer has own geode node with assigned state required for pass.
			osg::Geode* pass = new osg::Geode();

			renderer->updateOrCreateState(i, pass);

			// Attach renderer pass to common group.
			osg::Group* attachTo = dynamic_cast<osg::Group*>(group->getChild(maxPasses - renderer->getNumPasses() + i));

			if(attachTo) {
				attachTo->addChild(pass);

				// Attach geometries.
				for(GeometryList::const_iterator glit = gl.begin(); glit != gl.end(); glit++) {
					pass->addDrawable(glit->first);
				}
			}
		}
	}

	_finalized = true;
	
	return true;
}

void Text::_applyTransform(const osg::Matrixd &transform)
{
	osgUtil::TransformAttributeFunctor functor(_lastTransform * transform);
	for(GlyphGeometryMap::const_iterator g = _ggMap.begin(); g != _ggMap.end(); g++) {
		const GlyphGeometryIndex& ggi = g->second;
		for(GlyphGeometryIndex::const_iterator ct = ggi.begin(); ct != ggi.end(); ++ct) {
			ct->second->accept(functor);
		}
	}
	_lastTransform = osg::Matrixd::inverse(transform);
}


}
