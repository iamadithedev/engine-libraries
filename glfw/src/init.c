//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@glfw.org>
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// NOTE: The global variables below comprise all mutable global data in GLFW
//       Any other mutable global variable is a bug

// This contains all mutable state shared between compilation units of GLFW
//
_GLFWlibrary _glfw = { GLFW_FALSE };

// These are outside of _glfw so they can be used before initialization and
// after termination without special handling when _glfw is cleared to zero
//
static GLFWallocator _glfwInitAllocator;
static _GLFWinitconfig _glfwInitHints =
{
    GLFW_ANY_PLATFORM, // preferred platform
    NULL            // vkGetInstanceProcAddr function
};

// The allocation function used when no custom allocator is set
//
static void* defaultAllocate(size_t size, void* user)
{
    return malloc(size);
}

// The deallocation function used when no custom allocator is set
//
static void defaultDeallocate(void* block, void* user)
{
    free(block);
}

// The reallocation function used when no custom allocator is set
//
static void* defaultReallocate(void* block, size_t size, void* user)
{
    return realloc(block, size);
}

// Terminate the library
//
static void terminate(void)
{
    int i;

    memset(&_glfw.callbacks, 0, sizeof(_glfw.callbacks));

    while (_glfw.windowListHead)
        glfwDestroyWindow((GLFWwindow*) _glfw.windowListHead);

    while (_glfw.cursorListHead)
        glfwDestroyCursor((GLFWcursor*) _glfw.cursorListHead);

    for (i = 0;  i < _glfw.monitorCount;  i++)
    {
        _GLFWmonitor* monitor = _glfw.monitors[i];
        if (monitor->originalRamp.size)
            _glfw.platform.setGammaRamp(monitor, &monitor->originalRamp);
        _glfwFreeMonitor(monitor);
    }

    _glfw_free(_glfw.monitors);
    _glfw.monitors = NULL;
    _glfw.monitorCount = 0;

    _glfwTerminateVulkan();
    _glfw.platform.terminate();

    _glfw.initialized = GLFW_FALSE;

    _glfwPlatformDestroyTls(&_glfw.contextSlot);

    memset(&_glfw, 0, sizeof(_glfw));
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Encode a Unicode code point to a UTF-8 stream
// Based on cutef8 by Jeff Bezanson (Public Domain)
//
size_t _glfwEncodeUTF8(char* s, uint32_t codepoint)
{
    size_t count = 0;

    if (codepoint < 0x80)
        s[count++] = (char) codepoint;
    else if (codepoint < 0x800)
    {
        s[count++] = (codepoint >> 6) | 0xc0;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }
    else if (codepoint < 0x10000)
    {
        s[count++] = (codepoint >> 12) | 0xe0;
        s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }
    else if (codepoint < 0x110000)
    {
        s[count++] = (codepoint >> 18) | 0xf0;
        s[count++] = ((codepoint >> 12) & 0x3f) | 0x80;
        s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }

    return count;
}

// Splits and translates a text/uri-list into separate file paths
// NOTE: This function destroys the provided string
//
char** _glfwParseUriList(char* text, int* count)
{
    const char* prefix = "file://";
    char** paths = NULL;
    char* line;

    *count = 0;

    while ((line = strtok(text, "\r\n")))
    {
        char* path;

        text = NULL;

        if (line[0] == '#')
            continue;

        if (strncmp(line, prefix, strlen(prefix)) == 0)
        {
            line += strlen(prefix);
            // TODO: Validate hostname
            while (*line != '/')
                line++;
        }

        (*count)++;

        path = _glfw_calloc(strlen(line) + 1, 1);
        paths = _glfw_realloc(paths, *count * sizeof(char*));
        paths[*count - 1] = path;

        while (*line)
        {
            if (line[0] == '%' && line[1] && line[2])
            {
                const char digits[3] = { line[1], line[2], '\0' };
                *path = (char) strtol(digits, NULL, 16);
                line += 2;
            }
            else
                *path = *line;

            path++;
            line++;
        }
    }

    return paths;
}

char* _glfw_strdup(const char* source)
{
    const size_t length = strlen(source);
    char* result = _glfw_calloc(length + 1, 1);
    strcpy(result, source);
    return result;
}

int _glfw_min(int a, int b)
{
    return a < b ? a : b;
}

int _glfw_max(int a, int b)
{
    return a > b ? a : b;
}

float _glfw_fminf(float a, float b)
{
    if (a != a)
        return b;
    else if (b != b)
        return a;
    else if (a < b)
        return a;
    else
        return b;
}

float _glfw_fmaxf(float a, float b)
{
    if (a != a)
        return b;
    else if (b != b)
        return a;
    else if (a > b)
        return a;
    else
        return b;
}

void* _glfw_calloc(size_t count, size_t size)
{
    if (count && size)
    {
        void* block;

        if (count > SIZE_MAX / size)
        {
            return NULL;
        }

        block = _glfw.allocator.allocate(count * size, _glfw.allocator.user);
        if (block)
            return memset(block, 0, count * size);
        else
        {
            return NULL;
        }
    }
    else
        return NULL;
}

void* _glfw_realloc(void* block, size_t size)
{
    if (block && size)
    {
        void* resized = _glfw.allocator.reallocate(block, size, _glfw.allocator.user);
        if (resized)
            return resized;
        else
        {
            return NULL;
        }
    }
    else if (block)
    {
        _glfw_free(block);
        return NULL;
    }
    else
        return _glfw_calloc(1, size);
}

void _glfw_free(void* block)
{
    if (block)
        _glfw.allocator.deallocate(block, _glfw.allocator.user);
}

//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwInit(void)
{
    if (_glfw.initialized)
        return GLFW_TRUE;

    memset(&_glfw, 0, sizeof(_glfw));
    _glfw.hints.init = _glfwInitHints;

    _glfw.allocator = _glfwInitAllocator;
    if (!_glfw.allocator.allocate)
    {
        _glfw.allocator.allocate   = defaultAllocate;
        _glfw.allocator.reallocate = defaultReallocate;
        _glfw.allocator.deallocate = defaultDeallocate;
    }

    if (!_glfwSelectPlatform(_glfw.hints.init.platformID, &_glfw.platform))
        return GLFW_FALSE;

    if (!_glfw.platform.init())
    {
        terminate();
        return GLFW_FALSE;
    }

    if (!_glfwPlatformCreateTls(&_glfw.contextSlot))
    {
        terminate();
        return GLFW_FALSE;
    }

    _glfw.initialized = GLFW_TRUE;

    glfwDefaultWindowHints();
    return GLFW_TRUE;
}

GLFWAPI void glfwTerminate(void)
{
    if (!_glfw.initialized)
        return;

    terminate();
}

GLFWAPI void glfwInitHint(int hint, int value)
{
    switch (hint)
    {
        case GLFW_PLATFORM:
            _glfwInitHints.platformID = value;
            return;
    }
}

GLFWAPI void glfwInitAllocator(const GLFWallocator* allocator)
{
    if (allocator)
    {
        if (allocator->allocate && allocator->reallocate && allocator->deallocate)
            _glfwInitAllocator = *allocator;
    }
    else
        memset(&_glfwInitAllocator, 0, sizeof(GLFWallocator));
}

GLFWAPI void glfwInitVulkanLoader(PFN_vkGetInstanceProcAddr loader)
{
    _glfwInitHints.vulkanLoader = loader;
}