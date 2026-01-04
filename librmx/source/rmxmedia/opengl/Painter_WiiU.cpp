/*
 * Wii U Painter implementation using WHB/GX2.
 * This is a best-effort mapping of the existing Painter API to WHB helper calls.
 * Adjust WHB function names if your WUT version differs.
 */

#include "rmxmedia.h"
#include "rmxmedia/opengl/Painter.h"
#include "rmxmedia/opengl/Texture.h"
#include "rmxmedia/opengl/OpenGLFontOutput.h"

#if defined(PLATFORM_WIIU)
#include "../framework/GX2Renderer.h"
#if defined(__has_include)
# if __has_include(<whb/gfx.h>)
#  include <whb/gfx.h>
# else
#  include "../framework/wiiu_shim_gx2.h"
# endif
#else
# include "../framework/wiiu_shim_gx2.h"
#endif

namespace rmx
{
	Painter::Painter()
	{
		reset();
	}

	Painter::~Painter()
	{
	}

	void Painter::reset()
	{
		mColor = Color::WHITE;
		mTexturesEnabled = false;
		// WHB/GX2 state is handled in VideoManager; nothing to do here.
	}

	void Painter::begin()
	{
		FTX::Video->setPixelView();
		reset();
	}

	void Painter::end()
	{
		resetScissor();
	}

	void Painter::enableTextures(bool enable)
	{
		mTexturesEnabled = enable;
	}

	void Painter::resetColor()
	{
		mColor = Color::WHITE;
	}

	void Painter::setColor(const Color& color)
	{
		mColor = color;
	}

	void Painter::drawRect(const Rectf& rect, const Color& color)
	{
		// Draw a solid color rectangle using renderer wrapper
		GX2Renderer::instance().drawTexturedQuad((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, 0, color.getRGBA32());
	}

	void Painter::drawRect(const Rectf& rect, const Texture& texture, const Color& color)
	{
		// Draw textured rect using renderer wrapper
		GX2Renderer::instance().drawTexturedQuad((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, (int)texture.getHandle(), color.getRGBA32());
	}

	void Painter::drawRect(const Rectf& rect, const Texture& texture, const Vec2f& uv0, const Vec2f& uv1, const Color& color)
	{
		// TODO: Use UVs when drawing textured quad with WHB helper
		drawRect(rect, texture, color);
	}

	void Painter::drawQuadPatch(int numVertX, int numVertY, float* verticesX, float* verticesY, float* texcrdsX, float* texcrdsY)
	{
		// Fallback: draw triangles using textured quads
		RMX_ASSERT(false, "Quad patch not implemented for Wii U Painter");
	}

	void Painter::print(Font& font, const Vec2i& pos, const StringReader& text, int alignment, const Color& color)
	{
		// Build type infos
		std::vector<Font::TypeInfo> typeinfos;
		font.getTypeInfos(typeinfos, pos, text, 0);

		// Use OpenGLFontOutput's atlas building to obtain vertex groups (uses SpriteAtlas)
		OpenGLFontOutput fontOutput(font);
		OpenGLFontOutput::VertexGroups vertexGroups;
		fontOutput.buildVertexGroups(vertexGroups, typeinfos);

		// For each vertex group, draw the triangles using WHB textured draws
		for (const auto& vg : vertexGroups.mVertexGroups)
		{
			Texture* tex = vg.mTexture;
			if (nullptr == tex)
				continue;
			int start = (int)vg.mStartIndex;
			int count = (int)vg.mNumVertices;
			// Bind texture (platform-specific)
			WHBGfxBindTexture((int)tex->getHandle());
			for (int i = 0; i < count; i += 3)
			{
				const OpenGLFontOutput::Vertex& v0 = vertexGroups.mVertices[start + i + 0];
				const OpenGLFontOutput::Vertex& v1 = vertexGroups.mVertices[start + i + 1];
				const OpenGLFontOutput::Vertex& v2 = vertexGroups.mVertices[start + i + 2];

				// Issue a textured triangle draw. This helper should map to an efficient GX2 draw path.
				WHBGfxDrawTexturedTriangle(
					(int)roundf(v0.mPosition.x), (int)roundf(v0.mPosition.y), v0.mTexcoords.x, v0.mTexcoords.y,
					(int)roundf(v1.mPosition.x), (int)roundf(v1.mPosition.y), v1.mTexcoords.x, v1.mTexcoords.y,
					(int)roundf(v2.mPosition.x), (int)roundf(v2.mPosition.y), v2.mTexcoords.x, v2.mTexcoords.y
				);
			}
			WHBGfxUnbindTexture();
		}
	}

	void Painter::print(Font& font, const Vec2i& pos, const StringReader& text, const PrintOptions& printOptions)
	{
		print(font, pos, text, (int)printOptions.mAlignment, printOptions.mTintColor);
	}

	void Painter::print(Font& font, const Recti& rect, const StringReader& text, int alignment, const Color& color)
	{
		print(font, rect.topLeft(), text, alignment, color);
	}

	void Painter::print(Font& font, const Recti& rect, const StringReader& text, const PrintOptions& printOptions)
	{
		print(font, rect.topLeft(), text, printOptions);
	}

	void Painter::resetScissor()
	{
		mScissorStack.clear();
		// Disable scissor
		WHBGfxDisableScissor();
	}

	void Painter::setScissor(const Recti& rect)
	{
		mScissorStack.clear();
		mScissorStack.push_back(rect);
		WHBGfxEnableScissor(rect.x, rect.y, rect.width, rect.height);
	}

	void Painter::pushScissor(const Recti& rect)
	{
		Recti old = mScissorStack.empty() ? FTX::screenRect() : mScissorStack.back();
		Recti newScissor;
		newScissor.intersect(old, rect);
		mScissorStack.push_back(newScissor);
		WHBGfxEnableScissor(newScissor.x, newScissor.y, newScissor.width, newScissor.height);
	}

	void Painter::popScissor()
	{
		if (mScissorStack.empty()) return;
		mScissorStack.pop_back();
		if (mScissorStack.empty())
		{
			WHBGfxDisableScissor();
		}
		else
		{
			const Recti& r = mScissorStack.back();
			WHBGfxEnableScissor(r.x, r.y, r.width, r.height);
		}
	}

} // namespace rmx

#endif // PLATFORM_WIIU
