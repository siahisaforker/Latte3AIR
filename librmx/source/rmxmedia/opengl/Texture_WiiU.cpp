/*
 * Wii U Texture wrapper using WHB/GX2 helpers.
 * Best-effort mapping: adjust WHB function names if your WUT version differs.
 */

#include "rmxmedia.h"
#include "rmxmedia/opengl/Texture.h"

#if defined(PLATFORM_WIIU)

#include <whb/gfx.h>
#include <gx2.h>

Texture::Texture()
{
    initialize();
}

Texture::Texture(const Bitmap& bitmap)
{
    initialize();
    load(bitmap);
}

Texture::Texture(const String& filename)
{
    initialize();
    load(filename);
}

Texture::Texture(Texture&& other)
{
    mHandle = other.mHandle;
    other.mHandle = 0;
    mType = other.mType;
    mFormat = other.mFormat;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mFilterLinear = other.mFilterLinear;
    mHasMipmaps = other.mHasMipmaps;
}

Texture::~Texture()
{
    if (mHandle != 0)
    {
        WHBGfxDestroyTexture((int)mHandle);
        mHandle = 0;
    }
}

void Texture::generate()
{
    // No-op: WHB returns integer texture IDs during creation
}

void Texture::create(GLenum type)
{
    // Map to default 2D creation
    create(64, 64, rmx::OpenGLHelper::FORMAT_RGBA);
}

void Texture::create(GLint format)
{
    // Create a small default texture
    create(64, 64, format);
}

void Texture::create(const Vec2i& size, GLint format)
{
    create(size.x, size.y, format);
}

void Texture::createCubemap(GLint format)
{
    // Cubemaps not implemented in this Wii U shim
}

void Texture::createCubemap(int width, int height, GLint format)
{
    // Cubemaps not implemented in this Wii U shim
}

void Texture::createCubemap(const Vec2i& size, GLint format)
{
    // Cubemaps not implemented in this Wii U shim
}

void Texture::updateAll(const void* data)
{
    if (mHandle == 0) return;
    WHBGfxUpdateTexture((int)mHandle, 0, 0, mWidth, mHeight, data);
}

void Texture::updateRect(const void* data, const Recti& rect)
{
    if (mHandle == 0) return;
    WHBGfxUpdateTexture((int)mHandle, rect.x, rect.y, rect.width, rect.height, data);
}

void Texture::updateRect(const Bitmap& bitmap, int px, int py)
{
    updateRect(bitmap.getData(), Recti(px, py, bitmap.getWidth(), bitmap.getHeight()));
}

void Texture::copyFramebuffer(const Recti& rect)
{
    // Not implemented: framebuffer copy to texture
}

void Texture::copyFramebufferCubemap(const Recti& rect, int side)
{
    // Not implemented
}

void Texture::buildMipmaps()
{
    // Not implemented
}

void Texture::initialize()
{
    mHandle = 0;
    mType = 0;
    mFormat = 0;
    mWidth = 0;
    mHeight = 0;
    mFilterLinear = true;
    mHasMipmaps = false;
}

void Texture::create(int width, int height, GLint format)
{
    mWidth = width;
    mHeight = height;
    mFormat = format;
    // Create WHB texture from empty data
    int texId = WHBGfxCreateTexture(width, height, WHB_GX2_FORMAT_RGBA8, nullptr);
    mHandle = texId;
}

void Texture::load(const void* data, int width, int height)
{
    if (nullptr == data) return;
    mWidth = width;
    mHeight = height;
    mFormat = rmx::OpenGLHelper::FORMAT_RGBA;
    int texId = WHBGfxCreateTexture(width, height, WHB_GX2_FORMAT_RGBA8, data);
    mHandle = texId;
}

void Texture::load(const Bitmap& bitmap)
{
    load(bitmap.getData(), bitmap.getWidth(), bitmap.getHeight());
}

void Texture::load(const String& filename)
{
    Bitmap bitmap;
    if (bitmap.load(filename.toWString()))
    {
        load(bitmap);
    }
}

void Texture::bind() const
{
    if (mHandle == 0) return;
    WHBGfxBindTexture((int)mHandle);
}

void Texture::unbind() const
{
    WHBGfxUnbindTexture();
}

void Texture::setFilterNearest()
{
}

void Texture::setFilterLinear()
{
}

void Texture::setWrapClamp()
{
}

void Texture::setWrapRepeat()
{
}

void Texture::setWrapRepeatMirror()
{
}

#endif
