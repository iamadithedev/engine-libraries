# Open Asset Import Library (assimp)
# ----------------------------------------------------------------------
# Copyright (c) 2006-2022, assimp team
#
# All rights reserved.
#
# Redistribution and use of this software in source and binary forms,
# with or without modification, are permitted provided that the
# following conditions are met:
#
# * Redistributions of source code must retain the above
#   copyright notice, this list of conditions and the
#   following disclaimer.
#
# * Redistributions in binary form must reproduce the above
#   copyright notice, this list of conditions and the
#   following disclaimer in the documentation and/or other
#   materials provided with the distribution.
#
# * Neither the name of the assimp team, nor the names of its
#   contributors may be used to endorse or promote products
#   derived from this software without specific prior
#   written permission of the assimp team.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#----------------------------------------------------------------------

# Listing and grouping of all the source files.
# 1) Set the file lists for each component
# 2) Create a Source Group for each component, for IDE project orginization
# 3) Add libassimp using the file lists (eliminates duplication of file names between
#    source groups and library command)
#
cmake_minimum_required( VERSION 3.10 )
SET( HEADER_PATH ../include/assimp )

SET( COMPILER_HEADERS
  ${HEADER_PATH}/Compiler/pushpack1.h
  ${HEADER_PATH}/Compiler/poppack1.h
)
SOURCE_GROUP( Compiler FILES ${COMPILER_HEADERS})

SET( PUBLIC_HEADERS
  ${HEADER_PATH}/anim.h
  ${HEADER_PATH}/aabb.h
  ${HEADER_PATH}/color4.h
  ${HEADER_PATH}/color4.inl
  ${CMAKE_CURRENT_BINARY_DIR}/../include/assimp/config.h
  ${HEADER_PATH}/ColladaMetaData.h
  ${HEADER_PATH}/commonMetaData.h
  ${HEADER_PATH}/defs.h
  ${HEADER_PATH}/cfileio.h
  ${HEADER_PATH}/material.h
  ${HEADER_PATH}/material.inl
  ${HEADER_PATH}/matrix3x3.h
  ${HEADER_PATH}/matrix3x3.inl
  ${HEADER_PATH}/matrix4x4.h
  ${HEADER_PATH}/matrix4x4.inl
  ${HEADER_PATH}/mesh.h
  ${HEADER_PATH}/ObjMaterial.h
  ${HEADER_PATH}/pbrmaterial.h
  ${HEADER_PATH}/postprocess.h
  ${HEADER_PATH}/quaternion.h
  ${HEADER_PATH}/quaternion.inl
  ${HEADER_PATH}/scene.h
  ${HEADER_PATH}/metadata.h
  ${HEADER_PATH}/texture.h
  ${HEADER_PATH}/types.h
  ${HEADER_PATH}/vector2.h
  ${HEADER_PATH}/vector2.inl
  ${HEADER_PATH}/vector3.h
  ${HEADER_PATH}/vector3.inl
  ${HEADER_PATH}/cimport.h
  ${HEADER_PATH}/AssertHandler.h
  ${HEADER_PATH}/importerdesc.h
  ${HEADER_PATH}/Importer.hpp
  ${HEADER_PATH}/IOStream.hpp
  ${HEADER_PATH}/IOSystem.hpp
  ${HEADER_PATH}/DefaultIOStream.h
  ${HEADER_PATH}/DefaultIOSystem.h
  ${HEADER_PATH}/ZipArchiveIOSystem.h
  ${HEADER_PATH}/SceneCombiner.h
  ${HEADER_PATH}/fast_atof.h
  ${HEADER_PATH}/qnan.h
  ${HEADER_PATH}/BaseImporter.h
  ${HEADER_PATH}/Hash.h
  ${HEADER_PATH}/MemoryIOWrapper.h
  ${HEADER_PATH}/ParsingUtils.h
  ${HEADER_PATH}/StreamReader.h
  ${HEADER_PATH}/StreamWriter.h
  ${HEADER_PATH}/StringComparison.h
  ${HEADER_PATH}/StringUtils.h
  ${HEADER_PATH}/SGSpatialSort.h
  ${HEADER_PATH}/GenericProperty.h
  ${HEADER_PATH}/SpatialSort.h
  ${HEADER_PATH}/SkeletonMeshBuilder.h
  ${HEADER_PATH}/SmallVector.h
  ${HEADER_PATH}/SmoothingGroups.h
  ${HEADER_PATH}/SmoothingGroups.inl
  ${HEADER_PATH}/StandardShapes.h
  ${HEADER_PATH}/RemoveComments.h
  ${HEADER_PATH}/Subdivision.h
  ${HEADER_PATH}/Vertex.h
  ${HEADER_PATH}/LineSplitter.h
  ${HEADER_PATH}/TinyFormatter.h
  ${HEADER_PATH}/Bitmap.h
  ${HEADER_PATH}/XMLTools.h
  ${HEADER_PATH}/IOStreamBuffer.h
  ${HEADER_PATH}/CreateAnimMesh.h
  ${HEADER_PATH}/XmlParser.h
  ${HEADER_PATH}/MathFunctions.h
  ${HEADER_PATH}/ByteSwapper.h
  ${HEADER_PATH}/Base64.hpp
)

SET( Core_SRCS
  Common/Assimp.cpp
)

IF(MSVC OR (MINGW AND BUILD_SHARED_LIBS))
  list(APPEND Core_SRCS "res/assimp.rc")
ENDIF()

SET( Common_SRCS
  Common/Compression.cpp
  Common/Compression.h
  Common/BaseImporter.cpp
  Common/BaseProcess.cpp
  Common/BaseProcess.h
  Common/Importer.h
  Common/ScenePrivate.h
  Common/PostStepRegistry.cpp
  Common/ImporterRegistry.cpp
  Common/DefaultIOStream.cpp
  Common/IOSystem.cpp
  Common/DefaultIOSystem.cpp
  Common/ZipArchiveIOSystem.cpp
  Common/PolyTools.h
  Common/Maybe.h
  Common/Importer.cpp
  Common/SGSpatialSort.cpp
  Common/VertexTriangleAdjacency.cpp
  Common/VertexTriangleAdjacency.h
  Common/SpatialSort.cpp
  Common/SceneCombiner.cpp
  Common/ScenePreprocessor.cpp
  Common/ScenePreprocessor.h
  Common/SkeletonMeshBuilder.cpp
  Common/StandardShapes.cpp
  Common/TargetAnimation.cpp
  Common/TargetAnimation.h
  Common/RemoveComments.cpp
  Common/Subdivision.cpp
  Common/scene.cpp
  Common/Bitmap.cpp
  Common/Version.cpp
  Common/CreateAnimMesh.cpp
  Common/simd.h
  Common/simd.cpp
  Common/material.cpp
  Common/AssertHandler.cpp
  Common/Base64.cpp
)
SOURCE_GROUP(Common FILES ${Common_SRCS})

SET( CApi_SRCS
  CApi/CInterfaceIOWrapper.cpp
  CApi/CInterfaceIOWrapper.h
)
SOURCE_GROUP(CApi FILES ${CApi_SRCS})

SET(Geometry_SRCS
  Geometry/GeometryUtils.h
  Geometry/GeometryUtils.cpp
)
SOURCE_GROUP(Geometry FILES ${Geometry_SRCS})

IF ( ASSIMP_BUILD_NONFREE_C4D_IMPORTER )
  SET( C4D_SRCS
    AssetLib/C4D/C4DImporter.cpp
    AssetLib/C4D/C4DImporter.h
  )
  SOURCE_GROUP( C4D FILES ${C4D_SRCS})
ENDIF ()

# if this variable is set to TRUE, the user can manually disable importers by setting
# ASSIMP_BUILD_XXX_IMPORTER to FALSE for each importer
# if this variable is set to FALSE, the user can manually enable importers by setting
# ASSIMP_BUILD_XXX_IMPORTER to TRUE for each importer
OPTION(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT "default value of all ASSIMP_BUILD_XXX_IMPORTER values" TRUE)

# macro to add the CMake Option ADD_ASSIMP_IMPORTER_<name> which enables compile of loader
# this way selective loaders can be compiled (reduces filesize + compile time)
MACRO(ADD_ASSIMP_IMPORTER name)
  IF (ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT)
    set(ASSIMP_IMPORTER_ENABLED TRUE)
    IF (DEFINED ASSIMP_BUILD_${name}_IMPORTER AND NOT ASSIMP_BUILD_${name}_IMPORTER)
      set(ASSIMP_IMPORTER_ENABLED FALSE)
    ENDIF ()
  ELSE ()
    set(ASSIMP_IMPORTER_ENABLED ${ASSIMP_BUILD_${name}_IMPORTER})
  ENDIF ()
  IF (ASSIMP_IMPORTER_ENABLED)
    LIST(APPEND ASSIMP_LOADER_SRCS ${ARGN})
    SET(ASSIMP_IMPORTERS_ENABLED "${ASSIMP_IMPORTERS_ENABLED} ${name}")
    SOURCE_GROUP(AssetLib\\${name} FILES ${ARGN})
  ELSE()
    SET(${name}_SRC "")
    SET(ASSIMP_IMPORTERS_DISABLED "${ASSIMP_IMPORTERS_DISABLED} ${name}")
    add_definitions(-DASSIMP_BUILD_NO_${name}_IMPORTER)
  ENDIF()
ENDMACRO()

if (NOT ASSIMP_NO_EXPORT)

  # if this variable is set to TRUE, the user can manually disable exporters by setting
  # ASSIMP_BUILD_XXX_EXPORTER to FALSE for each exporter
  # if this variable is set to FALSE, the user can manually enable exporters by setting
  # ASSIMP_BUILD_XXX_EXPORTER to TRUE for each exporter
  OPTION(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT "default value of all ASSIMP_BUILD_XXX_EXPORTER values" TRUE)

  # macro to add the CMake Option ADD_ASSIMP_EXPORTER_<name> which enables compilation of an exporter
  # this way selective exporters can be compiled (reduces filesize + compile time)
  MACRO(ADD_ASSIMP_EXPORTER name)
    IF (ASSIMP_NO_EXPORT)
      set(ASSIMP_EXPORTER_ENABLED FALSE)
    ELSEIF (ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT)
      set(ASSIMP_EXPORTER_ENABLED TRUE)
      IF (DEFINED ASSIMP_BUILD_${name}_EXPORTER AND NOT ASSIMP_BUILD_${name}_EXPORTER)
        set(ASSIMP_EXPORTER_ENABLED FALSE)
      ENDIF ()
    ELSE ()
      set(ASSIMP_EXPORTER_ENABLED ${ASSIMP_BUILD_${name}_EXPORTER})
    ENDIF ()

    IF (ASSIMP_EXPORTER_ENABLED)
      SET(ASSIMP_EXPORTERS_ENABLED "${ASSIMP_EXPORTERS_ENABLED} ${name}")
      LIST(APPEND ASSIMP_EXPORTER_SRCS ${ARGN})
      SOURCE_GROUP(AssetLib\\${name} FILES ${ARGN})
    ELSE()
      SET(ASSIMP_EXPORTERS_DISABLED "${ASSIMP_EXPORTERS_DISABLED} ${name}")
      add_definitions(-DASSIMP_BUILD_NO_${name}_EXPORTER)
    ENDIF()
  ENDMACRO()

endif()

SET(ASSIMP_LOADER_SRCS "")
SET(ASSIMP_IMPORTERS_ENABLED "") # list of enabled importers
SET(ASSIMP_IMPORTERS_DISABLED "") # disabled importers list (used to print)

SET(ASSIMP_EXPORTER_SRCS "")
SET(ASSIMP_EXPORTERS_ENABLED "") # list of enabled exporters
SET(ASSIMP_EXPORTERS_DISABLED "") # disabled exporters list (used to print)

ADD_ASSIMP_IMPORTER( COLLADA
  AssetLib/Collada/ColladaHelper.cpp
  AssetLib/Collada/ColladaHelper.h
  AssetLib/Collada/ColladaLoader.cpp
  AssetLib/Collada/ColladaLoader.h
  AssetLib/Collada/ColladaParser.cpp
  AssetLib/Collada/ColladaParser.h
)

SET( MaterialSystem_SRCS
  Material/MaterialSystem.cpp
  Material/MaterialSystem.h
)
SOURCE_GROUP( MaterialSystem FILES ${MaterialSystem_SRCS})

ADD_ASSIMP_IMPORTER( OBJ
  AssetLib/Obj/ObjFileData.h
  AssetLib/Obj/ObjFileImporter.cpp
  AssetLib/Obj/ObjFileImporter.h
  AssetLib/Obj/ObjFileMtlImporter.cpp
  AssetLib/Obj/ObjFileMtlImporter.h
  AssetLib/Obj/ObjFileParser.cpp
  AssetLib/Obj/ObjFileParser.h
  AssetLib/Obj/ObjTools.h
)

if (ASSIMP_BUILD_IFC_IMPORTER)
  if (MSVC)
    set_source_files_properties(Importer/IFC/IFCReaderGen1_2x3.cpp Importer/IFC/IFCReaderGen2_2x3.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
  elseif(MINGW)
    set_source_files_properties(Importer/IFC/IFCReaderGen1_2x3.cpp Importer/IFC/IFCReaderGen2_2x3.cpp PROPERTIES COMPILE_FLAGS "-O2 -Wa,-mbig-obj")
  endif()
endif ()

ADD_ASSIMP_IMPORTER( FBX
  AssetLib/FBX/FBXImporter.cpp
  AssetLib/FBX/FBXCompileConfig.h
  AssetLib/FBX/FBXImporter.h
  AssetLib/FBX/FBXParser.cpp
  AssetLib/FBX/FBXParser.h
  AssetLib/FBX/FBXTokenizer.cpp
  AssetLib/FBX/FBXTokenizer.h
  AssetLib/FBX/FBXImportSettings.h
  AssetLib/FBX/FBXConverter.h
  AssetLib/FBX/FBXConverter.cpp
  AssetLib/FBX/FBXUtil.h
  AssetLib/FBX/FBXUtil.cpp
  AssetLib/FBX/FBXDocument.h
  AssetLib/FBX/FBXDocument.cpp
  AssetLib/FBX/FBXProperties.h
  AssetLib/FBX/FBXProperties.cpp
  AssetLib/FBX/FBXMeshGeometry.h
  AssetLib/FBX/FBXMeshGeometry.cpp
  AssetLib/FBX/FBXMaterial.cpp
  AssetLib/FBX/FBXModel.cpp
  AssetLib/FBX/FBXAnimation.cpp
  AssetLib/FBX/FBXNodeAttribute.cpp
  AssetLib/FBX/FBXDeformer.cpp
  AssetLib/FBX/FBXBinaryTokenizer.cpp
  AssetLib/FBX/FBXDocumentUtil.cpp
  AssetLib/FBX/FBXCommon.h
)

SET( PostProcessing_SRCS
  PostProcessing/CalcTangentsProcess.cpp
  PostProcessing/CalcTangentsProcess.h
  PostProcessing/ComputeUVMappingProcess.cpp
  PostProcessing/ComputeUVMappingProcess.h
  PostProcessing/ConvertToLHProcess.cpp
  PostProcessing/ConvertToLHProcess.h
  PostProcessing/EmbedTexturesProcess.cpp
  PostProcessing/EmbedTexturesProcess.h
  PostProcessing/FindDegenerates.cpp
  PostProcessing/FindDegenerates.h
  PostProcessing/FindInstancesProcess.cpp
  PostProcessing/FindInstancesProcess.h
  PostProcessing/FindInvalidDataProcess.cpp
  PostProcessing/FindInvalidDataProcess.h
  PostProcessing/FixNormalsStep.cpp
  PostProcessing/FixNormalsStep.h
  PostProcessing/DropFaceNormalsProcess.cpp
  PostProcessing/DropFaceNormalsProcess.h
  PostProcessing/GenFaceNormalsProcess.cpp
  PostProcessing/GenFaceNormalsProcess.h
  PostProcessing/GenVertexNormalsProcess.cpp
  PostProcessing/GenVertexNormalsProcess.h
  PostProcessing/PretransformVertices.cpp
  PostProcessing/PretransformVertices.h
  PostProcessing/LimitBoneWeightsProcess.cpp
  PostProcessing/LimitBoneWeightsProcess.h
  PostProcessing/RemoveRedundantMaterials.cpp
  PostProcessing/RemoveRedundantMaterials.h
  PostProcessing/RemoveVCProcess.cpp
  PostProcessing/RemoveVCProcess.h
  PostProcessing/TextureTransform.cpp
  PostProcessing/TextureTransform.h
  PostProcessing/TriangulateProcess.cpp
  PostProcessing/TriangulateProcess.h
  PostProcessing/OptimizeGraph.cpp
  PostProcessing/OptimizeGraph.h
  PostProcessing/OptimizeMeshes.cpp
  PostProcessing/OptimizeMeshes.h
  PostProcessing/DeboneProcess.cpp
  PostProcessing/DeboneProcess.h
  PostProcessing/ProcessHelper.h
  PostProcessing/ProcessHelper.cpp
  PostProcessing/ScaleProcess.cpp
  PostProcessing/ScaleProcess.h
  PostProcessing/ArmaturePopulate.cpp
  PostProcessing/ArmaturePopulate.h
  PostProcessing/GenBoundingBoxesProcess.cpp
  PostProcessing/GenBoundingBoxesProcess.h
  PostProcessing/SplitByBoneCountProcess.cpp
  PostProcessing/SplitByBoneCountProcess.h
)
SOURCE_GROUP( PostProcessing FILES ${PostProcessing_SRCS})

# Workaround for issue #2406 - force problematic large file to be optimized to prevent string table overflow error
# Used -Os instead of -O2 as previous issues had mentioned, since -Os is roughly speaking -O2, excluding any
# optimizations that take up extra space. Given that the issue is a string table overflowing, -Os seemed appropriate
# Also, I'm not positive if both link & compile flags are needed, but this hopefully ensures that the issue should not
# recur for edge cases such as static builds.
if ((MINGW) AND (CMAKE_BUILD_TYPE MATCHES Debug))
  message("-- Applying MinGW StepFileGen1.cpp Debug Workaround")
  SET_SOURCE_FILES_PROPERTIES(Importer/StepFile/StepFileGen1.cpp PROPERTIES COMPILE_FLAGS -Os )
  SET_SOURCE_FILES_PROPERTIES(Importer/StepFile/StepFileGen1.cpp PROPERTIES LINK_FLAGS -Os )
  SET_SOURCE_FILES_PROPERTIES(Importer/StepFile/StepFileGen1.cpp PROPERTIES STATIC_LIBRARY_FLAGS -Os )
endif()

SET( Extra_SRCS
  MD4FileData.h
)
SOURCE_GROUP( Extra FILES ${Extra_SRCS})

# pugixml
IF(ASSIMP_HUNTER_ENABLED)
  hunter_add_package(pugixml)
  find_package(pugixml CONFIG REQUIRED)
ELSE()
  SET( Pugixml_SRCS
    ../contrib/pugixml/src/pugiconfig.hpp
    ../contrib/pugixml/src/pugixml.hpp
  )
  INCLUDE_DIRECTORIES("../contrib/pugixml/src")
  SOURCE_GROUP( Contrib\\Pugixml FILES ${Pugixml_SRCS})
ENDIF()

# utf8
IF(ASSIMP_HUNTER_ENABLED)
  hunter_add_package(utf8)
  find_package(utf8cpp CONFIG REQUIRED)
ELSE()
  # utf8 is header-only, so Assimp doesn't need to do anything.
ENDIF()

# minizip/unzip
IF(ASSIMP_HUNTER_ENABLED)
  hunter_add_package(minizip)
  find_package(minizip CONFIG REQUIRED)
ELSE()
  SET( unzip_SRCS
    ../contrib/unzip/crypt.c
    ../contrib/unzip/crypt.h
    ../contrib/unzip/ioapi.c
    ../contrib/unzip/ioapi.h
    ../contrib/unzip/unzip.c
    ../contrib/unzip/unzip.h
  )
  SOURCE_GROUP(Contrib\\unzip FILES ${unzip_SRCS})
ENDIF()

# zip (https://github.com/kuba--/zip)
separate_arguments(ASSIMP_EXPORTERS_LIST UNIX_COMMAND ${ASSIMP_EXPORTERS_ENABLED})
IF(3MF IN_LIST ASSIMP_EXPORTERS_LIST)
  IF(ASSIMP_HUNTER_ENABLED)
    hunter_add_package(zip)
    find_package(zip CONFIG REQUIRED)
  ELSE()
    SET( ziplib_SRCS
      ../contrib/zip/src/miniz.h
      ../contrib/zip/src/zip.c
      ../contrib/zip/src/zip.h
    )

    # TODO if cmake required version has been updated to >3.12.0, collapse this to the second case only
    if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
      add_definitions(-DMINIZ_USE_UNALIGNED_LOADS_AND_STORES=0)
    else()
      add_compile_definitions(MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0)
    endif()

    SOURCE_GROUP( ziplib FILES ${ziplib_SRCS} )
  ENDIF()
ENDIF()

# VC2010 fixes
if(MSVC10)
  option( VC10_STDINT_FIX "Fix for VC10 Compiler regarding pstdint.h redefinition errors" OFF )
  if( VC10_STDINT_FIX )
    ADD_DEFINITIONS( -D_STDINT )
  endif()
endif()

IF( MSVC OR "${CMAKE_CXX_SIMULATE_ID}" MATCHES "MSVC") # clang with MSVC ABI
  ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
endif ()

IF(NOT ASSIMP_HUNTER_ENABLED)
  if (UNZIP_FOUND)
    SET (unzip_compile_SRCS "")
  else ()
    SET (unzip_compile_SRCS ${unzip_SRCS})
    INCLUDE_DIRECTORIES( "../contrib/unzip/" )
  endif ()
ENDIF()

MESSAGE(STATUS "Enabled importer formats:${ASSIMP_IMPORTERS_ENABLED}")
MESSAGE(STATUS "Disabled importer formats:${ASSIMP_IMPORTERS_DISABLED}")

if (NOT ASSIMP_NO_EXPORT)
  MESSAGE(STATUS "Enabled exporter formats:${ASSIMP_EXPORTERS_ENABLED}")
  MESSAGE(STATUS "Disabled exporter formats:${ASSIMP_EXPORTERS_DISABLED}")
endif()

SOURCE_GROUP( include\\assimp    FILES ${PUBLIC_HEADERS} )

SET( assimp_src
  # Assimp Files
  ${Core_SRCS}
  ${CApi_SRCS}
  ${Common_SRCS}
  ${Geometry_SRCS}
  ${Exporter_SRCS}
  ${PostProcessing_SRCS}
  ${MaterialSystem_SRCS}

  # Model Support
  ${ASSIMP_LOADER_SRCS}
  ${ASSIMP_EXPORTER_SRCS}

  # Third-party libraries
  ${unzip_compile_SRCS}
  ${ziplib_SRCS}
  ${Pugixml_SRCS}
  ${stb_SRCS}
  # Necessary to show the headers in the project when using the VC++ generator:

  ${PUBLIC_HEADERS}
  ${COMPILER_HEADERS}
)

ADD_LIBRARY( assimp ${assimp_src} )
ADD_LIBRARY(assimp::assimp ALIAS assimp)

# Add or remove dllexport tags depending on the library type.
IF (BUILD_SHARED_LIBS)
  TARGET_COMPILE_DEFINITIONS(assimp PRIVATE ASSIMP_BUILD_DLL_EXPORT)
ELSE ()
  TARGET_COMPILE_DEFINITIONS(assimp PRIVATE OPENDDL_STATIC_LIBARY)
ENDIF ()

TARGET_USE_COMMON_OUTPUT_DIRECTORY(assimp)

IF (ASSIMP_WARNINGS_AS_ERRORS)
  MESSAGE(STATUS "Treating all warnings as errors (for assimp library only)")
  IF (MSVC)

    IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang" ) # clang-cl
      TARGET_COMPILE_OPTIONS(assimp PRIVATE -Wall -Werror
        -Wno-unused-function
        -Wno-microsoft-enum-value
        -Wno-switch-enum
        -Wno-covered-switch-default
        -Wno-reserved-identifier
        -Wno-c++98-compat-pedantic
        -Wno-c++98-compat
        -Wno-documentation
        -Wno-documentation-unknown-command
        -Wno-deprecated-dynamic-exception-spec
        -Wno-undef
        -Wno-suggest-destructor-override
        -Wno-suggest-override
        -Wno-inconsistent-missing-destructor-override
        -Wno-zero-as-null-pointer-constant
        -Wno-global-constructors
        -Wno-exit-time-destructors
        -Wno-extra-semi-stmt
        -Wno-missing-prototypes
        -Wno-old-style-cast
        -Wno-cast-align
        -Wno-cast-qual
        -Wno-float-equal
        -Wno-implicit-int-float-conversion
        -Wno-sign-conversion
        -Wno-implicit-float-conversion
        -Wno-implicit-int-conversion
        -Wno-float-conversion
        -Wno-double-promotion
        -Wno-unused-macros
        -Wno-disabled-macro-expansion
        -Wno-shadow-field-in-constructor
        -Wno-shadow-field
        -Wno-shadow
        -Wno-language-extension-token
        -Wno-header-hygiene
        -Wno-tautological-value-range-compare
        -Wno-tautological-type-limit-compare
        -Wno-missing-noreturn
        -Wno-missing-variable-declarations
        -Wno-extra-semi
        -Wno-nonportable-system-include-path
        -Wno-undefined-reinterpret-cast
        -Wno-shift-sign-overflow
        -Wno-deprecated-copy-with-user-provided-dtor
        -Wno-deprecated-copy-with-dtor
        -Wno-deprecated
        -Wno-format-nonliteral
        -Wno-format-non-iso
        -Wno-comma
        -Wno-unreachable-code-break
        -Wno-unreachable-code-return
        -Wno-unreachable-code
        -Wno-implicit-fallthrough
        -Wno-unused-template
        -Wno-undefined-func-template
        -Wno-nested-anon-types
        -Wno-declaration-after-statement
      )
    ELSE()
      TARGET_COMPILE_OPTIONS(assimp PRIVATE /W4 /WX)
    ENDIF()
  ELSE()
    TARGET_COMPILE_OPTIONS(assimp PRIVATE -Wall -Werror)
  ENDIF()
ENDIF()

# adds C_FLAGS required to compile zip.c on old GCC 4.x compiler
TARGET_COMPILE_FEATURES(assimp PRIVATE c_std_99)

TARGET_INCLUDE_DIRECTORIES ( assimp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
  $<INSTALL_INTERFACE:${ASSIMP_INCLUDE_INSTALL_DIR}>
)

IF(ASSIMP_HUNTER_ENABLED)
  TARGET_LINK_LIBRARIES(assimp
      PUBLIC
      minizip::minizip
      ZLIB::zlib
      utf8cpp
      pugixml
      stb::stb
  )
  if(TARGET zip::zip)
    target_link_libraries(assimp PUBLIC zip::zip)
  endif()
ELSE()
  TARGET_LINK_LIBRARIES(assimp ${ZLIB_LIBRARIES} ${OPENDDL_PARSER_LIBRARIES})
ENDIF()

if( MSVC )
  # in order to prevent DLL hell, each of the DLLs have to be suffixed with the major version and msvc prefix
  # CMake 3.12 added a variable for this
  if(MSVC_TOOLSET_VERSION)
    set(MSVC_PREFIX "vc${MSVC_TOOLSET_VERSION}")
  else()
    if( MSVC70 OR MSVC71 )
      set(MSVC_PREFIX "vc70")
    elseif( MSVC80 )
      set(MSVC_PREFIX "vc80")
    elseif( MSVC90 )
      set(MSVC_PREFIX "vc90")
    elseif( MSVC10 )
      set(MSVC_PREFIX "vc100")
    elseif( MSVC11 )
      set(MSVC_PREFIX "vc110")
    elseif( MSVC12 )
      set(MSVC_PREFIX "vc120")
    elseif( MSVC_VERSION LESS 1910)
      set(MSVC_PREFIX "vc140")
    elseif( MSVC_VERSION LESS 1920)
      set(MSVC_PREFIX "vc141")
    elseif( MSVC_VERSION LESS 1930)
      set(MSVC_PREFIX "vc142")
    else()
      MESSAGE(WARNING "unknown msvc version ${MSVC_VERSION}")
      set(MSVC_PREFIX "vc150")
    endif()
  endif()
  set(LIBRARY_SUFFIX "${ASSIMP_LIBRARY_SUFFIX}-${MSVC_PREFIX}-mt" CACHE STRING "the suffix for the assimp windows library")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "WindowsStore")
    target_compile_definitions(assimp PUBLIC WindowsStore)
    TARGET_LINK_LIBRARIES(assimp advapi32)
endif()

SET_TARGET_PROPERTIES( assimp PROPERTIES
  VERSION ${ASSIMP_VERSION}
  SOVERSION ${ASSIMP_SOVERSION} # use full version
  OUTPUT_NAME assimp${LIBRARY_SUFFIX}
)

if (APPLE)
  if (ASSIMP_BUILD_FRAMEWORK)
    SET_TARGET_PROPERTIES( assimp PROPERTIES
      FRAMEWORK TRUE
      FRAMEWORK_VERSION C
      MACOSX_FRAMEWORK_IDENTIFIER net.sf.assimp
      PUBLIC_HEADER "${PUBLIC_HEADERS}"
    )

    # PUBLIC_HEADER option does not support directory structure creation
    # add ./Compiler/*.h to assimp.framework via copy command
    ADD_CUSTOM_COMMAND(TARGET assimp POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_directory
         "${HEADER_PATH}/Compiler"
         assimp.framework/Headers/Compiler
      COMMENT "Copying public ./Compiler/ header files to framework bundle's Headers/Compiler/")
  ENDIF()
ENDIF()

# Build against external unzip, or add ../contrib/unzip so
# assimp can #include "unzip.h"
IF(NOT ASSIMP_HUNTER_ENABLED)
  if (UNZIP_FOUND)
    INCLUDE_DIRECTORIES(${UNZIP_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(assimp ${UNZIP_LIBRARIES})
  else ()
    INCLUDE_DIRECTORIES("../")
  endif ()
ENDIF()

if(MSVC AND ASSIMP_INSTALL_PDB)
  # When only the static library is built, these properties must
  # be set to ensure the static lib .pdb is staged for installation.
  IF(NOT BUILD_SHARED_LIBS)
    SET_TARGET_PROPERTIES( assimp PROPERTIES
      COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMPILE_PDB_NAME assimp${LIBRARY_SUFFIX}
      COMPILE_PDB_NAME_DEBUG assimp${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}
    )
  ENDIF()

  IF(CMAKE_GENERATOR MATCHES "^Visual Studio")
    install(FILES ${Assimp_BINARY_DIR}/code/Debug/assimp${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}.pdb
      DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
      CONFIGURATIONS Debug
    )
    install(FILES ${Assimp_BINARY_DIR}/code/RelWithDebInfo/assimp${LIBRARY_SUFFIX}.pdb
      DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
      CONFIGURATIONS RelWithDebInfo
    )
  ELSE()
    install(FILES ${Assimp_BINARY_DIR}/code/assimp${LIBRARY_SUFFIX}${CMAKE_DEBUG_POSTFIX}.pdb
      DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
      CONFIGURATIONS Debug
    )
    install(FILES ${Assimp_BINARY_DIR}/code/assimp${LIBRARY_SUFFIX}.pdb
      DESTINATION ${ASSIMP_LIB_INSTALL_DIR}
      CONFIGURATIONS RelWithDebInfo
    )
  ENDIF()
ENDIF ()
