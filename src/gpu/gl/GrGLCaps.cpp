/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLCaps.h"
#include "GrGLContext.h"
#include "SkTSearch.h"
#include "SkTSort.h"

GrGLCaps::GrGLCaps() {
    this->reset();
}

void GrGLCaps::reset() {
    INHERITED::reset();

    fVerifiedColorConfigs.reset();
    fStencilFormats.reset();
    fStencilVerifiedColorConfigs.reset();
    fMSFBOType = kNone_MSFBOType;
    fFBFetchType = kNone_FBFetchType;
    fMaxFragmentUniformVectors = 0;
    fMaxVertexAttributes = 0;
    fMaxFragmentTextureUnits = 0;
    fMaxFixedFunctionTextureCoords = 0;
    fRGBA8RenderbufferSupport = false;
    fBGRAFormatSupport = false;
    fBGRAIsInternalFormat = false;
    fTextureSwizzleSupport = false;
    fUnpackRowLengthSupport = false;
    fUnpackFlipYSupport = false;
    fPackRowLengthSupport = false;
    fPackFlipYSupport = false;
    fTextureUsageSupport = false;
    fTexStorageSupport = false;
    fTextureRedSupport = false;
    fImagingSupport = false;
    fTwoFormatLimit = false;
    fFragCoordsConventionSupport = false;
    fVertexArrayObjectSupport = false;
    fUseNonVBOVertexAndIndexDynamicData = false;
    fIsCoreProfile = false;
    fFixedFunctionSupport = false;
    fDiscardFBSupport = false;
    fFullClearIsFree = false;
}

GrGLCaps::GrGLCaps(const GrGLCaps& caps) : GrDrawTargetCaps() {
    *this = caps;
}

GrGLCaps& GrGLCaps::operator = (const GrGLCaps& caps) {
    INHERITED::operator=(caps);
    fVerifiedColorConfigs = caps.fVerifiedColorConfigs;
    fStencilFormats = caps.fStencilFormats;
    fStencilVerifiedColorConfigs = caps.fStencilVerifiedColorConfigs;
    fMaxFragmentUniformVectors = caps.fMaxFragmentUniformVectors;
    fMaxVertexAttributes = caps.fMaxVertexAttributes;
    fMaxFragmentTextureUnits = caps.fMaxFragmentTextureUnits;
    fMaxFixedFunctionTextureCoords = caps.fMaxFixedFunctionTextureCoords;
    fMSFBOType = caps.fMSFBOType;
    fFBFetchType = caps.fFBFetchType;
    fRGBA8RenderbufferSupport = caps.fRGBA8RenderbufferSupport;
    fBGRAFormatSupport = caps.fBGRAFormatSupport;
    fBGRAIsInternalFormat = caps.fBGRAIsInternalFormat;
    fTextureSwizzleSupport = caps.fTextureSwizzleSupport;
    fUnpackRowLengthSupport = caps.fUnpackRowLengthSupport;
    fUnpackFlipYSupport = caps.fUnpackFlipYSupport;
    fPackRowLengthSupport = caps.fPackRowLengthSupport;
    fPackFlipYSupport = caps.fPackFlipYSupport;
    fTextureUsageSupport = caps.fTextureUsageSupport;
    fTexStorageSupport = caps.fTexStorageSupport;
    fTextureRedSupport = caps.fTextureRedSupport;
    fImagingSupport = caps.fImagingSupport;
    fTwoFormatLimit = caps.fTwoFormatLimit;
    fFragCoordsConventionSupport = caps.fFragCoordsConventionSupport;
    fVertexArrayObjectSupport = caps.fVertexArrayObjectSupport;
    fUseNonVBOVertexAndIndexDynamicData = caps.fUseNonVBOVertexAndIndexDynamicData;
    fIsCoreProfile = caps.fIsCoreProfile;
    fFixedFunctionSupport = caps.fFixedFunctionSupport;
    fDiscardFBSupport = caps.fDiscardFBSupport;
    fFullClearIsFree = caps.fFullClearIsFree;

    return *this;
}

void GrGLCaps::init(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli) {

    this->reset();
    if (!ctxInfo.isInitialized()) {
        return;
    }

    GrGLBinding binding = ctxInfo.binding();
    GrGLVersion version = ctxInfo.version();

    /**************************************************************************
     * Caps specific to GrGLCaps
     **************************************************************************/

    if (kES_GrGLBinding == binding) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS,
                          &fMaxFragmentUniformVectors);
    } else {
        SkASSERT(kDesktop_GrGLBinding == binding);
        GrGLint max;
        GR_GL_GetIntegerv(gli, GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &max);
        fMaxFragmentUniformVectors = max / 4;
        if (version >= GR_GL_VER(3, 2)) {
            GrGLint profileMask;
            GR_GL_GetIntegerv(gli, GR_GL_CONTEXT_PROFILE_MASK, &profileMask);
            fIsCoreProfile = SkToBool(profileMask & GR_GL_CONTEXT_CORE_PROFILE_BIT);
        }
        if (!fIsCoreProfile) {
            fFixedFunctionSupport = true;
            GR_GL_GetIntegerv(gli, GR_GL_MAX_TEXTURE_COORDS, &fMaxFixedFunctionTextureCoords);
            // Sanity check
            SkASSERT(fMaxFixedFunctionTextureCoords > 0 && fMaxFixedFunctionTextureCoords < 128);
        }
    }
    GR_GL_GetIntegerv(gli, GR_GL_MAX_VERTEX_ATTRIBS, &fMaxVertexAttributes);
    GR_GL_GetIntegerv(gli, GR_GL_MAX_TEXTURE_IMAGE_UNITS, &fMaxFragmentTextureUnits);

    if (kDesktop_GrGLBinding == binding) {
        fRGBA8RenderbufferSupport = true;
    } else {
        fRGBA8RenderbufferSupport = version >= GR_GL_VER(3,0) ||
                                    ctxInfo.hasExtension("GL_OES_rgb8_rgba8") ||
                                    ctxInfo.hasExtension("GL_ARM_rgba8");
    }

    if (kDesktop_GrGLBinding == binding) {
        fBGRAFormatSupport = version >= GR_GL_VER(1,2) ||
                             ctxInfo.hasExtension("GL_EXT_bgra");
    } else {
        if (ctxInfo.hasExtension("GL_APPLE_texture_format_BGRA8888")) {
            fBGRAFormatSupport = true;
        } else if (ctxInfo.hasExtension("GL_EXT_texture_format_BGRA8888")) {
            fBGRAFormatSupport = true;
            fBGRAIsInternalFormat = true;
        }
        SkASSERT(fBGRAFormatSupport ||
                 kSkia8888_GrPixelConfig != kBGRA_8888_GrPixelConfig);
    }

    if (kDesktop_GrGLBinding == binding) {
        fTextureSwizzleSupport = version >= GR_GL_VER(3,3) ||
                                 ctxInfo.hasExtension("GL_ARB_texture_swizzle");
    } else {
        fTextureSwizzleSupport = version >= GR_GL_VER(3,0);
    }

    if (kDesktop_GrGLBinding == binding) {
        fUnpackRowLengthSupport = true;
        fUnpackFlipYSupport = false;
        fPackRowLengthSupport = true;
        fPackFlipYSupport = false;
    } else {
        fUnpackRowLengthSupport = version >= GR_GL_VER(3,0) ||
                                  ctxInfo.hasExtension("GL_EXT_unpack_subimage");
        fUnpackFlipYSupport = ctxInfo.hasExtension("GL_CHROMIUM_flipy");
        fPackRowLengthSupport = version >= GR_GL_VER(3,0) ||
                                ctxInfo.hasExtension("GL_NV_pack_subimage");
        fPackFlipYSupport =
            ctxInfo.hasExtension("GL_ANGLE_pack_reverse_row_order");
    }

    fTextureUsageSupport = (kES_GrGLBinding == binding) &&
                            ctxInfo.hasExtension("GL_ANGLE_texture_usage");

    if (kDesktop_GrGLBinding == binding) {
        // The EXT version can apply to either GL or GLES.
        fTexStorageSupport = version >= GR_GL_VER(4,2) ||
                             ctxInfo.hasExtension("GL_ARB_texture_storage") ||
                             ctxInfo.hasExtension("GL_EXT_texture_storage");
    } else {
        // Qualcomm Adreno drivers appear to have issues with texture storage.
        fTexStorageSupport = (version >= GR_GL_VER(3,0) &&
                              kQualcomm_GrGLVendor != ctxInfo.vendor()) ||
                             ctxInfo.hasExtension("GL_EXT_texture_storage");
    }

    // ARB_texture_rg is part of OpenGL 3.0, but mesa doesn't support it if
    // it doesn't have ARB_texture_rg extension.
    if (kDesktop_GrGLBinding == binding) {
        if (ctxInfo.isMesa()) {
            fTextureRedSupport = ctxInfo.hasExtension("GL_ARB_texture_rg");
        } else {
            fTextureRedSupport = version >= GR_GL_VER(3,0) ||
                                 ctxInfo.hasExtension("GL_ARB_texture_rg");
        }
    } else {
        fTextureRedSupport =  version >= GR_GL_VER(3,0) ||
                              ctxInfo.hasExtension("GL_EXT_texture_rg");
    }

    fImagingSupport = kDesktop_GrGLBinding == binding &&
                      ctxInfo.hasExtension("GL_ARB_imaging");

    // ES 2 only guarantees RGBA/uchar + one other format/type combo for
    // ReadPixels. The other format has to checked at run-time since it
    // can change based on which render target is bound
    fTwoFormatLimit = kES_GrGLBinding == binding;

    // Known issue on at least some Intel platforms:
    // http://code.google.com/p/skia/issues/detail?id=946
    if (kIntel_GrGLVendor != ctxInfo.vendor()) {
        fFragCoordsConventionSupport = ctxInfo.glslGeneration() >= k150_GrGLSLGeneration ||
                                       ctxInfo.hasExtension("GL_ARB_fragment_coord_conventions");
    }

    // SGX and Mali GPUs that are based on a tiled-deferred architecture that have trouble with
    // frequently changing VBOs. We've measured a performance increase using non-VBO vertex
    // data for dynamic content on these GPUs. Perhaps we should read the renderer string and
    // limit this decision to specific GPU families rather than basing it on the vendor alone.
    if (!GR_GL_MUST_USE_VBO &&
        (kARM_GrGLVendor == ctxInfo.vendor() || kImagination_GrGLVendor == ctxInfo.vendor())) {
        fUseNonVBOVertexAndIndexDynamicData = true;
    }

    fDiscardFBSupport = ctxInfo.hasExtension("GL_EXT_discard_framebuffer");

    if (kARM_GrGLVendor == ctxInfo.vendor() || kImagination_GrGLVendor == ctxInfo.vendor()) {
        fFullClearIsFree = true;
    }

    if (kDesktop_GrGLBinding == binding) {
        fVertexArrayObjectSupport = version >= GR_GL_VER(3, 0) ||
                                    ctxInfo.hasExtension("GL_ARB_vertex_array_object");
    } else {
        fVertexArrayObjectSupport = version >= GR_GL_VER(3, 0) ||
                                    ctxInfo.hasExtension("GL_OES_vertex_array_object");
    }

    if (kES_GrGLBinding == binding) {
        if (ctxInfo.hasExtension("GL_EXT_shader_framebuffer_fetch")) {
            fFBFetchType = kEXT_FBFetchType;
        } else if (ctxInfo.hasExtension("GL_NV_shader_framebuffer_fetch")) {
            fFBFetchType = kNV_FBFetchType;
        }
    }

    this->initFSAASupport(ctxInfo, gli);
    this->initStencilFormats(ctxInfo);

    /**************************************************************************
     * GrDrawTargetCaps fields
     **************************************************************************/
    GrGLint numFormats;
    GR_GL_GetIntegerv(gli, GR_GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numFormats);
    if (numFormats) {
        SkAutoSTMalloc<10, GrGLint> formats(numFormats);
        GR_GL_GetIntegerv(gli, GR_GL_COMPRESSED_TEXTURE_FORMATS, formats);
        for (int i = 0; i < numFormats; ++i) {
            if (formats[i] == GR_GL_PALETTE8_RGBA8) {
                f8BitPaletteSupport = true;
                break;
            }
        }
    }

    if (kDesktop_GrGLBinding == binding) {
        // we could also look for GL_ATI_separate_stencil extension or
        // GL_EXT_stencil_two_side but they use different function signatures
        // than GL2.0+ (and than each other).
        fTwoSidedStencilSupport = (ctxInfo.version() >= GR_GL_VER(2,0));
        // supported on GL 1.4 and higher or by extension
        fStencilWrapOpsSupport = (ctxInfo.version() >= GR_GL_VER(1,4)) ||
                                  ctxInfo.hasExtension("GL_EXT_stencil_wrap");
    } else {
        // ES 2 has two sided stencil and stencil wrap
        fTwoSidedStencilSupport = true;
        fStencilWrapOpsSupport = true;
    }

    if (kDesktop_GrGLBinding == binding) {
        fBufferLockSupport = true; // we require VBO support and the desktop VBO extension includes
                                   // glMapBuffer.
    } else {
        fBufferLockSupport = ctxInfo.hasExtension("GL_OES_mapbuffer");
    }

    if (kDesktop_GrGLBinding == binding) {
        SkASSERT(ctxInfo.version() >= GR_GL_VER(2,0) ||
                 ctxInfo.hasExtension("GL_ARB_texture_non_power_of_two"));
        fNPOTTextureTileSupport = true;
        fMipMapSupport = true;
    } else {
        // Unextended ES2 supports NPOT textures with clamp_to_edge and non-mip filters only
        // ES3 has no limitations.
        fNPOTTextureTileSupport = ctxInfo.version() >= GR_GL_VER(3,0) ||
                                  ctxInfo.hasExtension("GL_OES_texture_npot");
        // ES2 supports MIP mapping for POT textures but our caps don't allow for limited MIP
        // support. The OES extension or ES 3.0 allow for MIPS on NPOT textures. So, apparently,
        // does the undocumented GL_IMG_texture_npot extension. This extension does not seem to
        // to alllow arbitrary wrap modes, however.
        fMipMapSupport = fNPOTTextureTileSupport || ctxInfo.hasExtension("GL_IMG_texture_npot");
    }

    fHWAALineSupport = (kDesktop_GrGLBinding == binding);

    GR_GL_GetIntegerv(gli, GR_GL_MAX_TEXTURE_SIZE, &fMaxTextureSize);
    GR_GL_GetIntegerv(gli, GR_GL_MAX_RENDERBUFFER_SIZE, &fMaxRenderTargetSize);
    // Our render targets are always created with textures as the color
    // attachment, hence this min:
    fMaxRenderTargetSize = GrMin(fMaxTextureSize, fMaxRenderTargetSize);

    fPathRenderingSupport = GR_GL_USE_NV_PATH_RENDERING &&
                            ctxInfo.hasExtension("GL_NV_path_rendering");
    SkASSERT(!fPathRenderingSupport || fFixedFunctionSupport);

    fDstReadInShaderSupport = kNone_FBFetchType != fFBFetchType;

    // Disable scratch texture reuse on Mali and Adreno devices
    fReuseScratchTextures = kARM_GrGLVendor != ctxInfo.vendor() &&
                            kQualcomm_GrGLVendor != ctxInfo.vendor();

    // Enable supported shader-related caps
    if (kDesktop_GrGLBinding == binding) {
        fDualSourceBlendingSupport = ctxInfo.version() >= GR_GL_VER(3,3) ||
                                     ctxInfo.hasExtension("GL_ARB_blend_func_extended");
        fShaderDerivativeSupport = true;
        // we don't support GL_ARB_geometry_shader4, just GL 3.2+ GS
        fGeometryShaderSupport = ctxInfo.version() >= GR_GL_VER(3,2) &&
                                 ctxInfo.glslGeneration() >= k150_GrGLSLGeneration;
    } else {
        fShaderDerivativeSupport = ctxInfo.hasExtension("GL_OES_standard_derivatives");
    }

    if (GrGLCaps::kES_IMG_MsToTexture_MSFBOType == fMSFBOType) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_SAMPLES_IMG, &fMaxSampleCount);
    } else if (GrGLCaps::kNone_MSFBOType != fMSFBOType) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_SAMPLES, &fMaxSampleCount);
    }

    this->initConfigRenderableTable(ctxInfo);
}

void GrGLCaps::initConfigRenderableTable(const GrGLContextInfo& ctxInfo) {

    // OpenGL < 3.0
    //  no support for render targets unless the GL_ARB_framebuffer_object
    //  extension is supported (in which case we get ALPHA, RED, RG, RGB,
    //  RGBA (ALPHA8, RGBA4, RGBA8) for OpenGL > 1.1). Note that we
    //  probably don't get R8 in this case.

    // OpenGL 3.0
    //  base color renderable: ALPHA, RED, RG, RGB, and RGBA
    //  sized derivatives: ALPHA8, R8, RGBA4, RGBA8

    // >= OpenGL 3.1
    //  base color renderable: RED, RG, RGB, and RGBA
    //  sized derivatives: R8, RGBA4, RGBA8
    //  if the GL_ARB_compatibility extension is supported then we get back
    //  support for GL_ALPHA and ALPHA8

    // GL_EXT_bgra adds BGRA render targets to any version

    // ES 2.0
    //  color renderable: RGBA4, RGB5_A1, RGB565
    //  GL_EXT_texture_rg adds support for R8 as a color render target
    //  GL_OES_rgb8_rgba8 and/or GL_ARM_rgba8 adds support for RGBA8
    //  GL_EXT_texture_format_BGRA8888 and/or GL_APPLE_texture_format_BGRA8888 added BGRA support

    // ES 3.0
    // Same as ES 2.0 except R8 and RGBA8 are supported without extensions (the functions called
    // below already account for this).

    enum {
        kNo_MSAA = 0,
        kYes_MSAA = 1,
    };

    if (kDesktop_GrGLBinding == ctxInfo.binding()) {
        // Post 3.0 we will get R8
        // Prior to 3.0 we will get ALPHA8 (with GL_ARB_framebuffer_object)
        if (ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object")) {
            fConfigRenderSupport[kAlpha_8_GrPixelConfig][kNo_MSAA] = true;
            fConfigRenderSupport[kAlpha_8_GrPixelConfig][kYes_MSAA] = true;
        }
    } else {
        // On ES we can only hope for R8
        fConfigRenderSupport[kAlpha_8_GrPixelConfig][kNo_MSAA] = fTextureRedSupport;
        fConfigRenderSupport[kAlpha_8_GrPixelConfig][kYes_MSAA] = fTextureRedSupport;
    }

    if (kDesktop_GrGLBinding != ctxInfo.binding()) {
        // only available in ES
        fConfigRenderSupport[kRGB_565_GrPixelConfig][kNo_MSAA] = true;
        fConfigRenderSupport[kRGB_565_GrPixelConfig][kYes_MSAA] = true;
    }

    // we no longer support 444 as a render target
    fConfigRenderSupport[kRGBA_4444_GrPixelConfig][kNo_MSAA]  = false;
    fConfigRenderSupport[kRGBA_4444_GrPixelConfig][kYes_MSAA]  = false;

    if (this->fRGBA8RenderbufferSupport) {
        fConfigRenderSupport[kRGBA_8888_GrPixelConfig][kNo_MSAA]  = true;
        fConfigRenderSupport[kRGBA_8888_GrPixelConfig][kYes_MSAA]  = true;
    }

    if (this->fBGRAFormatSupport) {
        fConfigRenderSupport[kBGRA_8888_GrPixelConfig][kNo_MSAA]  = true;
        // The GL_EXT_texture_format_BGRA8888 extension does not add BGRA to the list of
        // configs that are color-renderable and can be passed to glRenderBufferStorageMultisample.
        // Chromium may have an extension to allow BGRA renderbuffers to work on desktop platforms.
        if (ctxInfo.extensions().has("GL_CHROMIUM_renderbuffer_format_BGRA8888")) {
            fConfigRenderSupport[kBGRA_8888_GrPixelConfig][kYes_MSAA] = true;
        } else {
            fConfigRenderSupport[kBGRA_8888_GrPixelConfig][kYes_MSAA] =
                !fBGRAIsInternalFormat || !this->usesMSAARenderBuffers();
        }
    }

    // If we don't support MSAA then undo any places above where we set a config as renderable with
    // msaa.
    if (kNone_MSFBOType == fMSFBOType) {
        for (int i = 0; i < kGrPixelConfigCnt; ++i) {
            fConfigRenderSupport[i][kYes_MSAA] = false;
        }
    }
}

bool GrGLCaps::readPixelsSupported(const GrGLInterface* intf,
                                   GrGLenum format,
                                   GrGLenum type) const {
    if (GR_GL_RGBA == format && GR_GL_UNSIGNED_BYTE == type) {
        // ES 2 guarantees this format is supported
        return true;
    }

    if (!fTwoFormatLimit) {
        // not limited by ES 2's constraints
        return true;
    }

    GrGLint otherFormat = GR_GL_RGBA;
    GrGLint otherType = GR_GL_UNSIGNED_BYTE;

    // The other supported format/type combo supported for ReadPixels
    // can change based on which render target is bound
    GR_GL_GetIntegerv(intf,
                      GR_GL_IMPLEMENTATION_COLOR_READ_FORMAT,
                      &otherFormat);

    GR_GL_GetIntegerv(intf,
                      GR_GL_IMPLEMENTATION_COLOR_READ_TYPE,
                      &otherType);

    return (GrGLenum)otherFormat == format && (GrGLenum)otherType == type;
}

void GrGLCaps::initFSAASupport(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli) {

    fMSFBOType = kNone_MSFBOType;
    if (kDesktop_GrGLBinding != ctxInfo.binding()) {
        // We prefer the EXT/IMG extension over ES3 MSAA because we've observed
        // ES3 driver bugs on at least one device with a tiled GPU (N10).
        if (ctxInfo.hasExtension("GL_EXT_multisampled_render_to_texture")) {
            fMSFBOType = kES_EXT_MsToTexture_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_IMG_multisampled_render_to_texture")) {
            fMSFBOType = kES_IMG_MsToTexture_MSFBOType;
        } else if (!GR_GL_IGNORE_ES3_MSAA && ctxInfo.version() >= GR_GL_VER(3,0)) {
            fMSFBOType = GrGLCaps::kES_3_0_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_CHROMIUM_framebuffer_multisample")) {
            // chrome's extension is equivalent to the EXT msaa
            // and fbo_blit extensions.
            fMSFBOType = kDesktop_EXT_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_APPLE_framebuffer_multisample")) {
            fMSFBOType = kES_Apple_MSFBOType;
        }
    } else {
        if ((ctxInfo.version() >= GR_GL_VER(3,0)) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object")) {
            fMSFBOType = GrGLCaps::kDesktop_ARB_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_EXT_framebuffer_multisample") &&
                   ctxInfo.hasExtension("GL_EXT_framebuffer_blit")) {
            fMSFBOType = GrGLCaps::kDesktop_EXT_MSFBOType;
        }
    }
}

namespace {
const GrGLuint kUnknownBitCount = GrGLStencilBuffer::kUnknownBitCount;
}

void GrGLCaps::initStencilFormats(const GrGLContextInfo& ctxInfo) {

    // Build up list of legal stencil formats (though perhaps not supported on
    // the particular gpu/driver) from most preferred to least.

    // these consts are in order of most preferred to least preferred
    // we don't bother with GL_STENCIL_INDEX1 or GL_DEPTH32F_STENCIL8

    static const StencilFormat
                  // internal Format      stencil bits      total bits        packed?
        gS8    = {GR_GL_STENCIL_INDEX8,   8,                8,                false},
        gS16   = {GR_GL_STENCIL_INDEX16,  16,               16,               false},
        gD24S8 = {GR_GL_DEPTH24_STENCIL8, 8,                32,               true },
        gS4    = {GR_GL_STENCIL_INDEX4,   4,                4,                false},
    //  gS     = {GR_GL_STENCIL_INDEX,    kUnknownBitCount, kUnknownBitCount, false},
        gDS    = {GR_GL_DEPTH_STENCIL,    kUnknownBitCount, kUnknownBitCount, true };

    if (kDesktop_GrGLBinding == ctxInfo.binding()) {
        bool supportsPackedDS =
            ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_EXT_packed_depth_stencil") ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object");

        // S1 thru S16 formats are in GL 3.0+, EXT_FBO, and ARB_FBO since we
        // require FBO support we can expect these are legal formats and don't
        // check. These also all support the unsized GL_STENCIL_INDEX.
        fStencilFormats.push_back() = gS8;
        fStencilFormats.push_back() = gS16;
        if (supportsPackedDS) {
            fStencilFormats.push_back() = gD24S8;
        }
        fStencilFormats.push_back() = gS4;
        if (supportsPackedDS) {
            fStencilFormats.push_back() = gDS;
        }
    } else {
        // ES2 has STENCIL_INDEX8 without extensions but requires extensions
        // for other formats.
        // ES doesn't support using the unsized format.

        fStencilFormats.push_back() = gS8;
        //fStencilFormats.push_back() = gS16;
        if (ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_OES_packed_depth_stencil")) {
            fStencilFormats.push_back() = gD24S8;
        }
        if (ctxInfo.hasExtension("GL_OES_stencil4")) {
            fStencilFormats.push_back() = gS4;
        }
    }
    SkASSERT(0 == fStencilVerifiedColorConfigs.count());
    fStencilVerifiedColorConfigs.push_back_n(fStencilFormats.count());
}

void GrGLCaps::markColorConfigAndStencilFormatAsVerified(
                                    GrPixelConfig config,
                                    const GrGLStencilBuffer::Format& format) {
#if !GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT
    return;
#endif
    SkASSERT((unsigned)config < (unsigned)kGrPixelConfigCnt);
    SkASSERT(fStencilFormats.count() == fStencilVerifiedColorConfigs.count());
    int count = fStencilFormats.count();
    // we expect a really small number of possible formats so linear search
    // should be OK
    SkASSERT(count < 16);
    for (int i = 0; i < count; ++i) {
        if (format.fInternalFormat ==
            fStencilFormats[i].fInternalFormat) {
            fStencilVerifiedColorConfigs[i].markVerified(config);
            return;
        }
    }
    GrCrash("Why are we seeing a stencil format that "
            "GrGLCaps doesn't know about.");
}

bool GrGLCaps::isColorConfigAndStencilFormatVerified(
                                GrPixelConfig config,
                                const GrGLStencilBuffer::Format& format) const {
#if !GR_GL_CHECK_FBO_STATUS_ONCE_PER_FORMAT
    return false;
#endif
    SkASSERT((unsigned)config < (unsigned)kGrPixelConfigCnt);
    int count = fStencilFormats.count();
    // we expect a really small number of possible formats so linear search
    // should be OK
    SkASSERT(count < 16);
    for (int i = 0; i < count; ++i) {
        if (format.fInternalFormat ==
            fStencilFormats[i].fInternalFormat) {
            return fStencilVerifiedColorConfigs[i].isVerified(config);
        }
    }
    GrCrash("Why are we seeing a stencil format that "
            "GLCaps doesn't know about.");
    return false;
}

SkString GrGLCaps::dump() const {

    SkString r = INHERITED::dump();

    r.appendf("--- GL-Specific ---\n");
    for (int i = 0; i < fStencilFormats.count(); ++i) {
        r.appendf("Stencil Format %d, stencil bits: %02d, total bits: %02d\n",
                 i,
                 fStencilFormats[i].fStencilBits,
                 fStencilFormats[i].fTotalBits);
    }

    static const char* kMSFBOExtStr[] = {
        "None",
        "ARB",
        "EXT",
        "ES 3.0",
        "Apple",
        "IMG MS To Texture",
        "EXT MS To Texture",
    };
    GR_STATIC_ASSERT(0 == kNone_MSFBOType);
    GR_STATIC_ASSERT(1 == kDesktop_ARB_MSFBOType);
    GR_STATIC_ASSERT(2 == kDesktop_EXT_MSFBOType);
    GR_STATIC_ASSERT(3 == kES_3_0_MSFBOType);
    GR_STATIC_ASSERT(4 == kES_Apple_MSFBOType);
    GR_STATIC_ASSERT(5 == kES_IMG_MsToTexture_MSFBOType);
    GR_STATIC_ASSERT(6 == kES_EXT_MsToTexture_MSFBOType);
    GR_STATIC_ASSERT(GR_ARRAY_COUNT(kMSFBOExtStr) == kLast_MSFBOType + 1);

    static const char* kFBFetchTypeStr[] = {
        "None",
        "EXT",
        "NV",
    };
    GR_STATIC_ASSERT(0 == kNone_FBFetchType);
    GR_STATIC_ASSERT(1 == kEXT_FBFetchType);
    GR_STATIC_ASSERT(2 == kNV_FBFetchType);
    GR_STATIC_ASSERT(GR_ARRAY_COUNT(kFBFetchTypeStr) == kLast_FBFetchType + 1);


    r.appendf("Core Profile: %s\n", (fIsCoreProfile ? "YES" : "NO"));
    r.appendf("Fixed Function Support: %s\n", (fFixedFunctionSupport ? "YES" : "NO"));
    r.appendf("MSAA Type: %s\n", kMSFBOExtStr[fMSFBOType]);
    r.appendf("FB Fetch Type: %s\n", kFBFetchTypeStr[fFBFetchType]);
    r.appendf("Max FS Uniform Vectors: %d\n", fMaxFragmentUniformVectors);
    r.appendf("Max FS Texture Units: %d\n", fMaxFragmentTextureUnits);
    if (fFixedFunctionSupport) {
        r.appendf("Max Fixed Function Texture Coords: %d\n", fMaxFixedFunctionTextureCoords);
    }
    r.appendf("Max Vertex Attributes: %d\n", fMaxVertexAttributes);
    r.appendf("Support RGBA8 Render Buffer: %s\n", (fRGBA8RenderbufferSupport ? "YES": "NO"));
    r.appendf("BGRA support: %s\n", (fBGRAFormatSupport ? "YES": "NO"));
    r.appendf("BGRA is an internal format: %s\n", (fBGRAIsInternalFormat ? "YES": "NO"));
    r.appendf("Support texture swizzle: %s\n", (fTextureSwizzleSupport ? "YES": "NO"));
    r.appendf("Unpack Row length support: %s\n", (fUnpackRowLengthSupport ? "YES": "NO"));
    r.appendf("Unpack Flip Y support: %s\n", (fUnpackFlipYSupport ? "YES": "NO"));
    r.appendf("Pack Row length support: %s\n", (fPackRowLengthSupport ? "YES": "NO"));
    r.appendf("Pack Flip Y support: %s\n", (fPackFlipYSupport ? "YES": "NO"));

    r.appendf("Texture Usage support: %s\n", (fTextureUsageSupport ? "YES": "NO"));
    r.appendf("Texture Storage support: %s\n", (fTexStorageSupport ? "YES": "NO"));
    r.appendf("GL_R support: %s\n", (fTextureRedSupport ? "YES": "NO"));
    r.appendf("GL_ARB_imaging support: %s\n", (fImagingSupport ? "YES": "NO"));
    r.appendf("Two Format Limit: %s\n", (fTwoFormatLimit ? "YES": "NO"));
    r.appendf("Fragment coord conventions support: %s\n",
             (fFragCoordsConventionSupport ? "YES": "NO"));
    r.appendf("Vertex array object support: %s\n", (fVertexArrayObjectSupport ? "YES": "NO"));
    r.appendf("Use non-VBO for dynamic data: %s\n",
             (fUseNonVBOVertexAndIndexDynamicData ? "YES" : "NO"));
    r.appendf("Discard FrameBuffer support: %s\n", (fDiscardFBSupport ? "YES" : "NO"));
    r.appendf("Full screen clear is free: %s\n", (fFullClearIsFree ? "YES" : "NO"));
    return r;
}
