/*
*	Part of the Oxygen Engine / Sonic 3 A.I.R. software distribution.
*	Copyright (C) 2017-2025 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include <vector>

#include <rmxmedia.h>

#include "oxygen/drawing/DrawerTexture.h"
#include "oxygen/rendering/Geometry.h"

class RenderParts;


class Renderer
{
public:
	Renderer(int8 rendererTypeId, RenderParts& renderParts, DrawerTexture& outputTexture) :
		mRendererTypeId(rendererTypeId),
		mRenderParts(renderParts),
		mGameScreenTexture(outputTexture)
	{
	}

	virtual ~Renderer() = default;

	virtual void initialize() = 0;
	virtual void reset() = 0;
	virtual void setGameResolution(const Vec2i& gameResolution) = 0;
	virtual void clearGameScreen() = 0;
	virtual void renderGameScreen(const std::vector<Geometry*>& geometries) = 0;
	virtual void renderDebugDraw(int debugDrawMode, const Recti& rect) = 0;

protected:
	inline void startRendering() {}
	inline bool progressRendering() { return true; }

	inline bool isUsingSpriteMask(const std::vector<Geometry*>& geometries) const
	{
		for (const Geometry* geometry : geometries)
		{
			if (nullptr == geometry)
				continue;

			if (geometry->getType() != Geometry::Type::SPRITE)
				continue;

			const SpriteGeometry& sg = static_cast<const SpriteGeometry&>(*geometry);
			if (sg.mSpriteInfo.getType() == RenderItem::Type::SPRITE_MASK)
				return true;
		}
		return false;
	}

protected:
	const int8 mRendererTypeId;
	RenderParts& mRenderParts;
	DrawerTexture& mGameScreenTexture;
};
