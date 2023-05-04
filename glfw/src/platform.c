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

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

static const struct
{
    int ID;
    GLFWbool (*connect)(int,_GLFWplatform*);
} supportedPlatforms[] =
{
#if defined(_GLFW_WIN32)
    { GLFW_PLATFORM_WIN32, _glfwConnectWin32 },
#endif
};

GLFWbool _glfwSelectPlatform(int desiredID, _GLFWplatform* platform)
{
    const size_t count = sizeof(supportedPlatforms) / sizeof(supportedPlatforms[0]);
    size_t i;

    if (desiredID != GLFW_ANY_PLATFORM &&
        desiredID != GLFW_PLATFORM_WIN32 &&
        desiredID != GLFW_PLATFORM_NULL)
    {
        return GLFW_FALSE;
    }

    if (desiredID == GLFW_ANY_PLATFORM)
    {
        return supportedPlatforms[0].connect(supportedPlatforms[0].ID, platform);
    }
    else
    {
        for (i = 0;  i < count;  i++)
        {
            if (supportedPlatforms[i].ID == desiredID)
                return supportedPlatforms[i].connect(desiredID, platform);
        }
    }

    return GLFW_FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwGetPlatform(void)
{
    return _glfw.platform.platformID;
}

GLFWAPI int glfwPlatformSupported(int platformID)
{
    const size_t count = sizeof(supportedPlatforms) / sizeof(supportedPlatforms[0]);
    size_t i;

    if (platformID != GLFW_PLATFORM_WIN32 &&
        platformID != GLFW_PLATFORM_NULL)
    {
        return GLFW_FALSE;
    }

    if (platformID == GLFW_PLATFORM_NULL)
        return GLFW_TRUE;

    for (i = 0;  i < count;  i++)
    {
        if (platformID == supportedPlatforms[i].ID)
            return GLFW_TRUE;
    }

    return GLFW_FALSE;
}