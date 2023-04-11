//========================================================================
// GLFW 3.4 WGL - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

#if defined(_GLFW_WIN32)

#include <stdlib.h>
#include <assert.h>

// Return the value corresponding to the specified attribute
//
static int findPixelFormatAttribValueWGL(const int* attribs,
                                         int attribCount,
                                         const int* values,
                                         int attrib)
{
    int i;

    for (i = 0;  i < attribCount;  i++)
    {
        if (attribs[i] == attrib)
            return values[i];
    }

    return 0;
}

#define ADD_ATTRIB(a) \
{ \
    assert((size_t) attribCount < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[attribCount++] = a; \
}
#define FIND_ATTRIB_VALUE(a) \
    findPixelFormatAttribValueWGL(attribs, attribCount, values, a)

// Return a list of available and usable framebuffer configs
//
static int choosePixelFormatWGL(_GLFWwindow* window,
                                const _GLFWctxconfig* ctxconfig,
                                const _GLFWfbconfig* fbconfig)
{
    _GLFWfbconfig* usableConfigs;
    const _GLFWfbconfig* closest;
    int i, pixelFormat, nativeCount, usableCount = 0, attribCount = 0;
    int attribs[40];
    int values[sizeof(attribs) / sizeof(attribs[0])];

    nativeCount = DescribePixelFormat(window->context.wgl.dc,
                                      1,
                                      sizeof(PIXELFORMATDESCRIPTOR),
                                      NULL);

    if (_glfw.wgl.ARB_pixel_format)
    {
        ADD_ATTRIB(WGL_SUPPORT_OPENGL_ARB)
        ADD_ATTRIB(WGL_DRAW_TO_WINDOW_ARB)
        ADD_ATTRIB(WGL_PIXEL_TYPE_ARB)
        ADD_ATTRIB(WGL_ACCELERATION_ARB)
        ADD_ATTRIB(WGL_RED_BITS_ARB)
        ADD_ATTRIB(WGL_RED_SHIFT_ARB)
        ADD_ATTRIB(WGL_GREEN_BITS_ARB)
        ADD_ATTRIB(WGL_GREEN_SHIFT_ARB)
        ADD_ATTRIB(WGL_BLUE_BITS_ARB)
        ADD_ATTRIB(WGL_BLUE_SHIFT_ARB)
        ADD_ATTRIB(WGL_ALPHA_BITS_ARB)
        ADD_ATTRIB(WGL_ALPHA_SHIFT_ARB)
        ADD_ATTRIB(WGL_DEPTH_BITS_ARB)
        ADD_ATTRIB(WGL_STENCIL_BITS_ARB)
        ADD_ATTRIB(WGL_ACCUM_BITS_ARB)
        ADD_ATTRIB(WGL_ACCUM_RED_BITS_ARB)
        ADD_ATTRIB(WGL_ACCUM_GREEN_BITS_ARB)
        ADD_ATTRIB(WGL_ACCUM_BLUE_BITS_ARB)
        ADD_ATTRIB(WGL_ACCUM_ALPHA_BITS_ARB)
        ADD_ATTRIB(WGL_AUX_BUFFERS_ARB)
        ADD_ATTRIB(WGL_STEREO_ARB)
        ADD_ATTRIB(WGL_DOUBLE_BUFFER_ARB)

        if (_glfw.wgl.ARB_multisample)
            ADD_ATTRIB(WGL_SAMPLES_ARB)

        if (ctxconfig->client == GLFW_OPENGL_API)
        {
            if (_glfw.wgl.ARB_framebuffer_sRGB || _glfw.wgl.EXT_framebuffer_sRGB)
                ADD_ATTRIB(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB)
        }
        else
        {
            if (_glfw.wgl.EXT_colorspace)
                ADD_ATTRIB(WGL_COLORSPACE_EXT)
        }
    }

    usableConfigs = _glfw_calloc(nativeCount, sizeof(_GLFWfbconfig));

    for (i = 0;  i < nativeCount;  i++)
    {
        _GLFWfbconfig* u = usableConfigs + usableCount;
        pixelFormat = i + 1;

        if (_glfw.wgl.ARB_pixel_format)
        {
            // Get pixel format attributes through "modern" extension

            if (!wglGetPixelFormatAttribivARB(window->context.wgl.dc,
                                              pixelFormat, 0,
                                              attribCount,
                                              attribs, values))
            {
                _glfw_free(usableConfigs);
                return 0;
            }

            if (!FIND_ATTRIB_VALUE(WGL_SUPPORT_OPENGL_ARB) ||
                !FIND_ATTRIB_VALUE(WGL_DRAW_TO_WINDOW_ARB))
            {
                continue;
            }

            if (FIND_ATTRIB_VALUE(WGL_PIXEL_TYPE_ARB) != WGL_TYPE_RGBA_ARB)
                continue;

            if (FIND_ATTRIB_VALUE(WGL_ACCELERATION_ARB) == WGL_NO_ACCELERATION_ARB)
                continue;

            if (FIND_ATTRIB_VALUE(WGL_DOUBLE_BUFFER_ARB) != fbconfig->doublebuffer)
                continue;

            u->redBits = FIND_ATTRIB_VALUE(WGL_RED_BITS_ARB);
            u->greenBits = FIND_ATTRIB_VALUE(WGL_GREEN_BITS_ARB);
            u->blueBits = FIND_ATTRIB_VALUE(WGL_BLUE_BITS_ARB);
            u->alphaBits = FIND_ATTRIB_VALUE(WGL_ALPHA_BITS_ARB);

            u->depthBits = FIND_ATTRIB_VALUE(WGL_DEPTH_BITS_ARB);
            u->stencilBits = FIND_ATTRIB_VALUE(WGL_STENCIL_BITS_ARB);

            u->accumRedBits = FIND_ATTRIB_VALUE(WGL_ACCUM_RED_BITS_ARB);
            u->accumGreenBits = FIND_ATTRIB_VALUE(WGL_ACCUM_GREEN_BITS_ARB);
            u->accumBlueBits = FIND_ATTRIB_VALUE(WGL_ACCUM_BLUE_BITS_ARB);
            u->accumAlphaBits = FIND_ATTRIB_VALUE(WGL_ACCUM_ALPHA_BITS_ARB);

            u->auxBuffers = FIND_ATTRIB_VALUE(WGL_AUX_BUFFERS_ARB);

            if (FIND_ATTRIB_VALUE(WGL_STEREO_ARB))
                u->stereo = GLFW_TRUE;

            if (_glfw.wgl.ARB_multisample)
                u->samples = FIND_ATTRIB_VALUE(WGL_SAMPLES_ARB);

            if (ctxconfig->client == GLFW_OPENGL_API)
            {
                if (_glfw.wgl.ARB_framebuffer_sRGB ||
                    _glfw.wgl.EXT_framebuffer_sRGB)
                {
                    if (FIND_ATTRIB_VALUE(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB))
                        u->sRGB = GLFW_TRUE;
                }
            }
            else
            {
                if (_glfw.wgl.EXT_colorspace)
                {
                    if (FIND_ATTRIB_VALUE(WGL_COLORSPACE_EXT) == WGL_COLORSPACE_SRGB_EXT)
                        u->sRGB = GLFW_TRUE;
                }
            }
        }
        else
        {
            // Get pixel format attributes through legacy PFDs

            PIXELFORMATDESCRIPTOR pfd;

            if (!DescribePixelFormat(window->context.wgl.dc,
                                     pixelFormat,
                                     sizeof(PIXELFORMATDESCRIPTOR),
                                     &pfd))
            {
                _glfw_free(usableConfigs);
                return 0;
            }

            if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW) ||
                !(pfd.dwFlags & PFD_SUPPORT_OPENGL))
            {
                continue;
            }

            if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) &&
                (pfd.dwFlags & PFD_GENERIC_FORMAT))
            {
                continue;
            }

            if (pfd.iPixelType != PFD_TYPE_RGBA)
                continue;

            if (!!(pfd.dwFlags & PFD_DOUBLEBUFFER) != fbconfig->doublebuffer)
                continue;

            u->redBits = pfd.cRedBits;
            u->greenBits = pfd.cGreenBits;
            u->blueBits = pfd.cBlueBits;
            u->alphaBits = pfd.cAlphaBits;

            u->depthBits = pfd.cDepthBits;
            u->stencilBits = pfd.cStencilBits;

            u->accumRedBits = pfd.cAccumRedBits;
            u->accumGreenBits = pfd.cAccumGreenBits;
            u->accumBlueBits = pfd.cAccumBlueBits;
            u->accumAlphaBits = pfd.cAccumAlphaBits;

            u->auxBuffers = pfd.cAuxBuffers;

            if (pfd.dwFlags & PFD_STEREO)
                u->stereo = GLFW_TRUE;
        }

        u->handle = pixelFormat;
        usableCount++;
    }

    if (!usableCount)
    {
        _glfw_free(usableConfigs);
        return 0;
    }

    closest = _glfwChooseFBConfig(fbconfig, usableConfigs, usableCount);
    if (!closest)
    {
        _glfw_free(usableConfigs);
        return 0;
    }

    pixelFormat = (int) closest->handle;
    _glfw_free(usableConfigs);

    return pixelFormat;
}

#undef ADD_ATTRIB
#undef FIND_ATTRIB_VALUE

static void makeContextCurrentWGL(_GLFWwindow* window)
{
    if (window)
    {
        if (wglMakeCurrent(window->context.wgl.dc, window->context.wgl.handle))
            _glfwPlatformSetTls(&_glfw.contextSlot, window);
        else
        {
            _glfwPlatformSetTls(&_glfw.contextSlot, NULL);
        }
    }
    else
    {
        if (!wglMakeCurrent(NULL, NULL))
        {
        }

        _glfwPlatformSetTls(&_glfw.contextSlot, NULL);
    }
}

static void swapBuffersWGL(_GLFWwindow* window)
{
    if (!window->monitor)
    {
        // HACK: Use DwmFlush when desktop composition is enabled on Windows Vista and 7
        if (!IsWindows8OrGreater() && IsWindowsVistaOrGreater())
        {
            BOOL enabled = FALSE;

            if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled)
            {
                int count = abs(window->context.wgl.interval);
                while (count--)
                    DwmFlush();
            }
        }
    }

    SwapBuffers(window->context.wgl.dc);
}

static void swapIntervalWGL(int interval)
{
    _GLFWwindow* window = _glfwPlatformGetTls(&_glfw.contextSlot);

    window->context.wgl.interval = interval;

    if (!window->monitor)
    {
        // HACK: Disable WGL swap interval when desktop composition is enabled on Windows
        //       Vista and 7 to avoid interfering with DWM vsync
        if (!IsWindows8OrGreater() && IsWindowsVistaOrGreater())
        {
            BOOL enabled = FALSE;

            if (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled)
                interval = 0;
        }
    }

    if (_glfw.wgl.EXT_swap_control)
        wglSwapIntervalEXT(interval);
}

static int extensionSupportedWGL(const char* extension)
{
    const char* extensions = NULL;

    if (_glfw.wgl.GetExtensionsStringARB)
        extensions = wglGetExtensionsStringARB(wglGetCurrentDC());
    else if (_glfw.wgl.GetExtensionsStringEXT)
        extensions = wglGetExtensionsStringEXT();

    if (!extensions)
        return GLFW_FALSE;

    return _glfwStringInExtensionString(extension, extensions);
}

static GLFWglproc getProcAddressWGL(const char* procname)
{
    const GLFWglproc proc = (GLFWglproc) wglGetProcAddress(procname);
    if (proc)
        return proc;

    return (GLFWglproc) _glfwPlatformGetModuleSymbol(_glfw.wgl.instance, procname);
}

static void destroyContextWGL(_GLFWwindow* window)
{
    if (window->context.wgl.handle)
    {
        wglDeleteContext(window->context.wgl.handle);
        window->context.wgl.handle = NULL;
    }
}

// Initialize WGL
//
GLFWbool _glfwInitWGL(void)
{
    PIXELFORMATDESCRIPTOR pfd;
    HGLRC prc, rc;
    HDC pdc, dc;

    if (_glfw.wgl.instance)
        return GLFW_TRUE;

    _glfw.wgl.instance = _glfwPlatformLoadModule("opengl32.dll");
    if (!_glfw.wgl.instance)
    {
        return GLFW_FALSE;
    }

    _glfw.wgl.CreateContext = (PFN_wglCreateContext)
        _glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglCreateContext");
    _glfw.wgl.DeleteContext = (PFN_wglDeleteContext)
        _glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglDeleteContext");
    _glfw.wgl.GetProcAddress = (PFN_wglGetProcAddress)
        _glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetProcAddress");
    _glfw.wgl.GetCurrentDC = (PFN_wglGetCurrentDC)
        _glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetCurrentDC");
    _glfw.wgl.GetCurrentContext = (PFN_wglGetCurrentContext)
        _glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglGetCurrentContext");
    _glfw.wgl.MakeCurrent = (PFN_wglMakeCurrent)
        _glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglMakeCurrent");
    _glfw.wgl.ShareLists = (PFN_wglShareLists)
        _glfwPlatformGetModuleSymbol(_glfw.wgl.instance, "wglShareLists");

    // NOTE: A dummy context has to be created for opengl32.dll to load the
    //       OpenGL ICD, from which we can then query WGL extensions
    // NOTE: This code will accept the Microsoft GDI ICD; accelerated context
    //       creation failure occurs during manual pixel format enumeration

    dc = GetDC(_glfw.win32.helperWindowHandle);

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    if (!SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd))
    {
        return GLFW_FALSE;
    }

    rc = wglCreateContext(dc);
    if (!rc)
    {
        return GLFW_FALSE;
    }

    pdc = wglGetCurrentDC();
    prc = wglGetCurrentContext();

    if (!wglMakeCurrent(dc, rc))
    {
        wglMakeCurrent(pdc, prc);
        wglDeleteContext(rc);
        return GLFW_FALSE;
    }

    // NOTE: Functions must be loaded first as they're needed to retrieve the
    //       extension string that tells us whether the functions are supported
    _glfw.wgl.GetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)
        wglGetProcAddress("wglGetExtensionsStringEXT");
    _glfw.wgl.GetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
        wglGetProcAddress("wglGetExtensionsStringARB");
    _glfw.wgl.CreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    _glfw.wgl.SwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
        wglGetProcAddress("wglSwapIntervalEXT");
    _glfw.wgl.GetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
        wglGetProcAddress("wglGetPixelFormatAttribivARB");

    // NOTE: WGL_ARB_extensions_string and WGL_EXT_extensions_string are not
    //       checked below as we are already using them
    _glfw.wgl.ARB_multisample =
        extensionSupportedWGL("WGL_ARB_multisample");
    _glfw.wgl.ARB_framebuffer_sRGB =
        extensionSupportedWGL("WGL_ARB_framebuffer_sRGB");
    _glfw.wgl.EXT_framebuffer_sRGB =
        extensionSupportedWGL("WGL_EXT_framebuffer_sRGB");
    _glfw.wgl.ARB_create_context =
        extensionSupportedWGL("WGL_ARB_create_context");
    _glfw.wgl.ARB_create_context_profile =
        extensionSupportedWGL("WGL_ARB_create_context_profile");
    _glfw.wgl.ARB_create_context_robustness =
        extensionSupportedWGL("WGL_ARB_create_context_robustness");
    _glfw.wgl.EXT_swap_control =
        extensionSupportedWGL("WGL_EXT_swap_control");
    _glfw.wgl.EXT_colorspace =
        extensionSupportedWGL("WGL_EXT_colorspace");
    _glfw.wgl.ARB_pixel_format =
        extensionSupportedWGL("WGL_ARB_pixel_format");
    _glfw.wgl.ARB_context_flush_control =
        extensionSupportedWGL("WGL_ARB_context_flush_control");

    wglMakeCurrent(pdc, prc);
    wglDeleteContext(rc);
    return GLFW_TRUE;
}

// Terminate WGL
//
void _glfwTerminateWGL(void)
{
    if (_glfw.wgl.instance)
        _glfwPlatformFreeModule(_glfw.wgl.instance);
}

#define SET_ATTRIB(a, v) \
{ \
    assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}

// Create the OpenGL or OpenGL ES context
//
GLFWbool _glfwCreateContextWGL(_GLFWwindow* window,
                               const _GLFWctxconfig* ctxconfig,
                               const _GLFWfbconfig* fbconfig)
{
    int attribs[40];
    int pixelFormat;
    PIXELFORMATDESCRIPTOR pfd;
    HGLRC share = NULL;

    window->context.wgl.dc = GetDC(window->win32.handle);
    if (!window->context.wgl.dc)
    {
        return GLFW_FALSE;
    }

    pixelFormat = choosePixelFormatWGL(window, ctxconfig, fbconfig);
    if (!pixelFormat)
        return GLFW_FALSE;

    if (!DescribePixelFormat(window->context.wgl.dc,
                             pixelFormat, sizeof(pfd), &pfd))
    {
        return GLFW_FALSE;
    }

    if (!SetPixelFormat(window->context.wgl.dc, pixelFormat, &pfd))
    {
        return GLFW_FALSE;
    }

    if (ctxconfig->client == GLFW_OPENGL_API)
    {
        if (ctxconfig->profile)
        {
            if (!_glfw.wgl.ARB_create_context_profile)
            {
                return GLFW_FALSE;
            }
        }
    }
    else
    {
        if (!_glfw.wgl.ARB_create_context ||
            !_glfw.wgl.ARB_create_context_profile)
        {
            return GLFW_FALSE;
        }
    }

    if (_glfw.wgl.ARB_create_context)
    {
        int index = 0, mask = 0, flags = 0;

        if (ctxconfig->client == GLFW_OPENGL_API)
        {
            if (ctxconfig->profile == GLFW_OPENGL_CORE_PROFILE)
                mask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
        }

        if (ctxconfig->robustness)
        {
            if (_glfw.wgl.ARB_create_context_robustness)
            {
                if (ctxconfig->robustness == GLFW_NO_RESET_NOTIFICATION)
                {
                    SET_ATTRIB(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                               WGL_NO_RESET_NOTIFICATION_ARB)
                }
                else if (ctxconfig->robustness == GLFW_LOSE_CONTEXT_ON_RESET)
                {
                    SET_ATTRIB(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                               WGL_LOSE_CONTEXT_ON_RESET_ARB)
                }

                flags |= WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (ctxconfig->release)
        {
            if (_glfw.wgl.ARB_context_flush_control)
            {
                if (ctxconfig->release == GLFW_RELEASE_BEHAVIOR_NONE)
                {
                    SET_ATTRIB(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB,
                               WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB)
                }
                else if (ctxconfig->release == GLFW_RELEASE_BEHAVIOR_FLUSH)
                {
                    SET_ATTRIB(WGL_CONTEXT_RELEASE_BEHAVIOR_ARB,
                               WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB)
                }
            }
        }

        // NOTE: Only request an explicitly versioned context when necessary, as
        //       explicitly requesting version 1.0 does not always return the
        //       highest version supported by the driver
        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            SET_ATTRIB(WGL_CONTEXT_MAJOR_VERSION_ARB, ctxconfig->major)
            SET_ATTRIB(WGL_CONTEXT_MINOR_VERSION_ARB, ctxconfig->minor)
        }

        if (flags)
            SET_ATTRIB(WGL_CONTEXT_FLAGS_ARB, flags)

        if (mask)
            SET_ATTRIB(WGL_CONTEXT_PROFILE_MASK_ARB, mask)

        SET_ATTRIB(0, 0)

        window->context.wgl.handle =
            wglCreateContextAttribsARB(window->context.wgl.dc, share, attribs);
        if (!window->context.wgl.handle)
        {
            return GLFW_FALSE;
        }
    }
    else
    {
        window->context.wgl.handle = wglCreateContext(window->context.wgl.dc);
        if (!window->context.wgl.handle)
        {
            return GLFW_FALSE;
        }

        if (share)
        {
            if (!wglShareLists(share, window->context.wgl.handle))
            {
                return GLFW_FALSE;
            }
        }
    }

    window->context.makeCurrent = makeContextCurrentWGL;
    window->context.swapBuffers = swapBuffersWGL;
    window->context.swapInterval = swapIntervalWGL;
    window->context.extensionSupported = extensionSupportedWGL;
    window->context.getProcAddress = getProcAddressWGL;
    window->context.destroy = destroyContextWGL;

    return GLFW_TRUE;
}

#undef SET_ATTRIB

GLFWAPI HGLRC glfwGetWGLContext(GLFWwindow* handle)
{
    _GLFWwindow* window = (_GLFWwindow*) handle;

    if (_glfw.platform.platformID != GLFW_PLATFORM_WIN32)
    {
        return NULL;
    }

    if (window->context.source != GLFW_NATIVE_CONTEXT_API)
    {
        return NULL;
    }

    return window->context.wgl.handle;
}

#endif // _GLFW_WIN32

