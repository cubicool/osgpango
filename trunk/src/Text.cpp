// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <sstream>
#include <algorithm>
#include <osg/io_utils>
#include <osg/Math>
#include <osg/Image>
#include <osg/Geode>
#include <osg/TexEnvCombine>
#include <osgUtil/TransformAttributeFunctor>		
#include <osgPango/Text>

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
_colorMode       (cm),
_lastTransform   (osg::Matrix::identity()),
_coordinateAlign (COORDINATE_ALIGN_AUTO) {
	clear();
}

Text::~Text() {
}

void Text::clear() {
	_ggMap.clear();
	_size          = osg::Vec2();
	_origin        = osg::Vec2();
	_scale         = 1;
	_baseline      = 0;
	_alpha         = 1.0f;
	_init          = false;
	_finalized     = false;
	_lastTransform = osg::Matrix::identity();
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
	GlyphRenderer* gr = Context::instance().getGlyphRenderer(_glyphRenderer);

	if(!gr) return;

	GlyphCache* gc = gr->getOrCreateGlyphCache(font);

	if(!gc) return;

	osg::Vec2 layoutPos(x / PANGO_SCALE, -(y / PANGO_SCALE));

	osg::Vec4 extents = gr->getExtraGlyphExtents();
	ColorPair color   = Context::instance().getColorPair();
	
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
			// TODO: This is, I think, where we'd generate just a block
			// or something indicating our glyph is invalid in some way.
			//
			// PangoFontMetrics* metrics = pango_font_get_metrics(font, 0);
			// pango_font_metrics_unref(metrics);

			continue;
		}

		const CachedGlyph* cg = gc->getCachedGlyph(gi->glyph);

		if(!cg) cg = gc->createCachedGlyph(font, gi);

		if(!cg) continue;

		GlyphGeometryIndex& ggi = _ggMap[GlyphGeometryMapKey(gc, color)];

		if(cg->size.x() > 0.0f && cg->size.y() > 0.0f) {
			osg::Vec2 pos(
				(gi->geometry.x_offset / PANGO_SCALE) - extents[0],
				(gi->geometry.y_offset / PANGO_SCALE) - extents[1]
			);

			if(!ggi[cg->img]) ggi[cg->img] = createGlyphGeometry();

			ggi[cg->img]->pushCachedGlyphAt(
				cg,
				pos + layoutPos
			);
		}

		layoutPos += osg::Vec2(gi->geometry.width / PANGO_SCALE, 0.0f);
	}

	// Use the lowest baseline of all the texts that are added.
	int baseline = y / PANGO_SCALE;

	if(!_init || baseline > _baseline) _baseline = baseline;
}

void Text::setColorPalette(const ColorPalette& cp) {
	_palette = cp;
}

void Text::addText(
	String::Encoding   encoding,
	const std::string& str,
	const std::string& descr,
	int                x,
	int                y,
	const TextOptions& to
) {	
	GlyphRenderer* gr = Context::instance().getGlyphRenderer(_glyphRenderer);

	if(!gr) return;

	String       text;
	PangoLayout* layout = pango_layout_new(Context::instance().getPangoContext());

	if(str.size()) {
		text.set(str, encoding);

		std::string utf8 = text.createUTF8EncodedString();

		if(descr.empty()) pango_layout_set_markup(layout, utf8.c_str(), -1);

		else {
			pango_layout_set_font_description(
				layout,
				pango_font_description_from_string(descr.c_str())
			);

			pango_layout_set_text(layout, utf8.c_str(), -1);
		}
	}

	to.setupPangoLayout(layout);
	
	Context::instance().drawLayout(this, layout, x, y);

	// Get text dimensions and whatnot; we'll accumulate this data after each rendering
	// to keep it accurate.
	PangoRectangle rect;

	pango_layout_get_pixel_extents(layout, &rect, 0);

	g_object_unref(layout);

	const osg::Vec4& extents = gr->getExtraGlyphExtents();

	osg::Vec2::value_type ox = -(x + rect.x);
	osg::Vec2::value_type oy = y + rect.y;
	osg::Vec2::value_type sw = rect.width;
	osg::Vec2::value_type sh = rect.height;

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

osg::Vec2 Text::getOriginBaseline() const {
	return osg::Vec2(_origin.x(), _baseline);
}

osg::Vec2 Text::getOriginTranslated() const {
	return osg::Vec2(_origin.x(), _size.y() + _origin.y());
}

bool Text::_finalizeGeometry(osg::Group* group) {
	typedef std::map<const GlyphRenderer*, GeometryList> RendererGeometry;

	RendererGeometry rg;

	for(GlyphGeometryMap::iterator g = _ggMap.begin(); g != _ggMap.end(); g++) {
		GlyphCache*         gc    = g->first.first;
		ColorPair           color = g->first.second;
		GlyphGeometryIndex& ggi   = g->second;

		for(GlyphGeometryIndex::iterator i = ggi.begin(); i != ggi.end(); i++) {
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

			for(unsigned int layer = 0; layer < gc->getLayers().size(); ++layer) {
				ggs.textures.push_back(gc->getTexture(i->first, layer));
				ggs.colors.push_back(_palette[layer]);
			}
		}
	}

	unsigned int maxPasses = 0;

	// First create/update geometry states which are common for each pass. During iteration update maximum
	// number of passes.
	for(RendererGeometry::const_iterator ct = rg.begin(); ct != rg.end(); ct++) {
		const GlyphRenderer* renderer = ct->first;
		const GeometryList&        gl = ct->second;

		maxPasses = std::max(renderer->getNumPasses(), maxPasses);

		for(GeometryList::const_iterator i = gl.begin(); i != gl.end(); i++) {
			if(!renderer->updateOrCreateState(i->first, i->second)) {
				osg::notify(osg::WARN)
					<< "Failed to call updateOrCreateState for Renderer '"
					<< renderer->getName() << "' during GeometryList update. "
					<< std::endl
				;
			}
		}
	}

	// Create structure for passes.
	for(unsigned int i = 0; i < maxPasses; i++) {
		osg::Group*    pass  = new osg::Group();
		osg::StateSet* state = pass->getOrCreateStateSet();

		state->setRenderBinDetails(i, "RenderBin");
		state->getOrCreateUniform("pangoAlpha", osg::Uniform::FLOAT)->set(_alpha);

		group->addChild(pass);
	}
	
	// Assign renderers to passes.
	for(RendererGeometry::const_iterator ct = rg.begin(); ct != rg.end(); ct++) {
		const GlyphRenderer* renderer = ct->first;
		const GeometryList&        gl = ct->second;

		for(unsigned int i = 0; i < renderer->getNumPasses(); i++) {
			// Each renderer has own geode node with assigned state required for pass.
			osg::Geode* pass = new osg::Geode();

			if(!renderer->updateOrCreateState(i, pass)) {
				osg::notify(osg::WARN)
					<< "Failed to call updateOrCreateState for Renderer '"
					<< renderer->getName() << "' on pass number "
					<< i << "."
					<< std::endl
				;
			}

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
	
	_ggMap.clear();
	
	return true;
}

class ApplyTransformsVisitor: public osg::NodeVisitor {
public:	
	typedef std::set<osg::Drawable*> DrawableSet;

	ApplyTransformsVisitor (const osg::Matrix& matrix):
	osg::NodeVisitor       (osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
	_functor               (matrix) {
	}
	
	virtual void apply(osg::Geode& geode) {
		for(unsigned int i = 0; i < geode.getNumDrawables(); i++) {
			_drawables.insert(geode.getDrawable(i));
		}
	}
	
	void transform(bool pixelAlign)  {
		for(
			DrawableSet::iterator i = _drawables.begin();
			i != _drawables.end();
			i++
		) {
			(*i)->accept(_functor);
			(*i)->dirtyDisplayList();
			(*i)->dirtyBound();
			
			if(osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(*i)) {
				if(pixelAlign) {
					// Here we do some pixel-alignment calculations. What this
					// means is that we iterate through all of vertices that make
					// up our text and make sure they occur on integer-compatible
					// coordinates. This is only important when a scale is applied
					// to the text.

					osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(
						geometry->getVertexArray()
					);

					if(!verts) continue;

					for(
						osg::Vec3Array::iterator v = verts->begin();
						v != verts->end();
						v++
					) {
						roundVec3(*v);
					}
				}
				
				if(geometry->getVertexArray()) geometry->getVertexArray()->dirty();
			}
		}
	}

private:
	osgUtil::TransformAttributeFunctor _functor;
	DrawableSet                        _drawables;
};

void Text::_applyTransform(osg::Node* node, const osg::Matrix& transform) {
	ApplyTransformsVisitor nv(_lastTransform * transform);
	
	node->accept(nv);

	bool align = false;

	switch(_coordinateAlign) {
		case COORDINATE_ALIGN_NONE:
			break;
		
		case COORDINATE_ALIGN_AUTO:
			align = fmodf(_scale, static_cast<int>(_scale)) == 0.0f;
			
			break;
		
		case COORDINATE_ALIGN_ALWAYS:
			align = true;

			break;
	}

	nv.transform(align);

	_lastTransform = osg::Matrix::inverse(transform);
}

}
