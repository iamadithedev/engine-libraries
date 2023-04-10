/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2022, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/
/** @file  Assimp.cpp
 *  @brief Implementation of the Plain-C API
 */

#include <assimp/BaseImporter.h>
#include <assimp/GenericProperty.h>
#include <assimp/cimport.h>
#include <assimp/importerdesc.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "CApi/CInterfaceIOWrapper.h"
#include "Importer.h"
#include "ScenePrivate.h"

#include <list>

// ------------------------------------------------------------------------------------------------
#ifndef ASSIMP_BUILD_SINGLETHREADED
#include <mutex>
#include <thread>
#endif
// ------------------------------------------------------------------------------------------------
using namespace Assimp;

namespace Assimp {
// underlying structure for aiPropertyStore
typedef BatchLoader::PropertyMap PropertyMap;

#if defined(__has_warning)
#if __has_warning("-Wordered-compare-function-pointers")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wordered-compare-function-pointers"
#endif
#endif

#if defined(__has_warning)
#if __has_warning("-Wordered-compare-function-pointers")
#pragma GCC diagnostic pop
#endif
#endif

/** Error message of the last failed import process */
static std::string gLastErrorString;

/** will return all registered importers. */
void GetImporterInstanceList(std::vector<BaseImporter *> &out);

/** will delete all registered importers. */
void DeleteImporterInstanceList(std::vector<BaseImporter *> &out);
} // namespace Assimp

#ifndef ASSIMP_BUILD_SINGLETHREADED
/** Global mutex to manage the access to the log-stream map */
static std::mutex gLogStreamMutex;
#endif

// ------------------------------------------------------------------------------------------------
// Reads the given file and returns its content.
const aiScene *aiImportFile(const char *pFile, unsigned int pFlags) {
    return aiImportFileEx(pFile, pFlags, nullptr);
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiImportFileEx(const char *pFile, unsigned int pFlags, aiFileIO *pFS) {
    return aiImportFileExWithProperties(pFile, pFlags, pFS, nullptr);
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiImportFileExWithProperties(const char *pFile, unsigned int pFlags,
        aiFileIO *pFS, const aiPropertyStore *props) {

    // create an Importer for this file
    Assimp::Importer *imp = new Assimp::Importer();

    // copy properties
    if (props) {
        const PropertyMap *pp = reinterpret_cast<const PropertyMap *>(props);
        ImporterPimpl *pimpl = imp->Pimpl();
        pimpl->mIntProperties = pp->ints;
        pimpl->mFloatProperties = pp->floats;
        pimpl->mStringProperties = pp->strings;
        pimpl->mMatrixProperties = pp->matrices;
    }
    // setup a custom IO system if necessary
    if (pFS) {
        imp->SetIOHandler(new CIOSystemWrapper(pFS));
    }

    // and have it read the file
    const aiScene* scene = imp->ReadFile(pFile, pFlags);

    // if succeeded, store the importer in the scene and keep it alive
    if (scene) {
        ScenePrivateData *priv = const_cast<ScenePrivateData *>(ScenePriv(scene));
        priv->mOrigImporter = imp;
    } else {
        // if failed, extract error code and destroy the import
        gLastErrorString = imp->GetErrorString();
        delete imp;
    }

    return scene;
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiImportFileFromMemory(
        const char *pBuffer,
        unsigned int pLength,
        unsigned int pFlags,
        const char *pHint) {
    return aiImportFileFromMemoryWithProperties(pBuffer, pLength, pFlags, pHint, nullptr);
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiImportFileFromMemoryWithProperties(
        const char *pBuffer,
        unsigned int pLength,
        unsigned int pFlags,
        const char *pHint,
        const aiPropertyStore *props) {

    const aiScene *scene = nullptr;

    // create an Importer for this file
    Assimp::Importer *imp = new Assimp::Importer();

    // copy properties
    if (props) {
        const PropertyMap *pp = reinterpret_cast<const PropertyMap *>(props);
        ImporterPimpl *pimpl = imp->Pimpl();
        pimpl->mIntProperties = pp->ints;
        pimpl->mFloatProperties = pp->floats;
        pimpl->mStringProperties = pp->strings;
        pimpl->mMatrixProperties = pp->matrices;
    }

    // and have it read the file from the memory buffer
    scene = imp->ReadFileFromMemory(pBuffer, pLength, pFlags, pHint);

    // if succeeded, store the importer in the scene and keep it alive
    if (scene) {
        ScenePrivateData *priv = const_cast<ScenePrivateData *>(ScenePriv(scene));
        priv->mOrigImporter = imp;
    } else {
        // if failed, extract error code and destroy the import
        gLastErrorString = imp->GetErrorString();
        delete imp;
    }
    // return imported data. If the import failed the pointer is nullptr anyways
    return scene;
}

// ------------------------------------------------------------------------------------------------
// Releases all resources associated with the given import process.
void aiReleaseImport(const aiScene *pScene) {
    if (!pScene) {
        return;
    }

    // find the importer associated with this data
    const ScenePrivateData *priv = ScenePriv(pScene);
    if (!priv || !priv->mOrigImporter) {
        delete pScene;
    } else {
        // deleting the Importer also deletes the scene
        // Note: the reason that this is not written as 'delete priv->mOrigImporter'
        // is a suspected bug in gcc 4.4+ (http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52339)
        Importer *importer = priv->mOrigImporter;
        delete importer;
    }
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiApplyPostProcessing(const aiScene *pScene,
        unsigned int pFlags) {
    const aiScene *sc = nullptr;

    // find the importer associated with this data
    const ScenePrivateData *priv = ScenePriv(pScene);
    if (!priv || !priv->mOrigImporter) {
        return nullptr;
    }

    sc = priv->mOrigImporter->ApplyPostProcessing(pFlags);

    if (!sc) {
        aiReleaseImport(pScene);
        return nullptr;
    }

    return sc;
}

// ------------------------------------------------------------------------------------------------
const aiScene *aiApplyCustomizedPostProcessing(const aiScene *scene,
        BaseProcess *process) {
    const aiScene *sc(nullptr);

    // find the importer associated with this data
    const ScenePrivateData *priv = ScenePriv(scene);
    if (nullptr == priv || nullptr == priv->mOrigImporter) {
        return nullptr;
    }

    sc = priv->mOrigImporter->ApplyCustomizedPostProcessing(process);

    if (!sc) {
        aiReleaseImport(scene);
        return nullptr;
    }

    return sc;
}

// ------------------------------------------------------------------------------------------------
// Returns the error text of the last failed import process.
const char *aiGetErrorString() {
    return gLastErrorString.c_str();
}

// -----------------------------------------------------------------------------------------------
// Return the description of a importer given its index
const aiImporterDesc *aiGetImportFormatDescription(size_t pIndex) {
    return Importer().GetImporterInfo(pIndex);
}

// -----------------------------------------------------------------------------------------------
// Return the number of importers
size_t aiGetImportFormatCount(void) {
    return Importer().GetImporterCount();
}

// ------------------------------------------------------------------------------------------------
// Returns the error text of the last failed import process.
aiBool aiIsExtensionSupported(const char *szExtension) {
    aiBool candoit = AI_FALSE;

    // FIXME: no need to create a temporary Importer instance just for that ..
    Assimp::Importer tmp;
    candoit = tmp.IsExtensionSupported(std::string(szExtension)) ? AI_TRUE : AI_FALSE;

    return candoit;
}

// ------------------------------------------------------------------------------------------------
// Get a list of all file extensions supported by ASSIMP
void aiGetExtensionList(aiString *szOut) {
    // FIXME: no need to create a temporary Importer instance just for that ..
    Assimp::Importer tmp;
    tmp.GetExtensionList(*szOut);
}

// ------------------------------------------------------------------------------------------------
// Get the memory requirements for a particular import.
void aiGetMemoryRequirements(const aiScene *pIn,
        aiMemoryInfo *in) {

    // find the importer associated with this data
    const ScenePrivateData *priv = ScenePriv(pIn);
    if (!priv || !priv->mOrigImporter) {
        return;
    }

    return priv->mOrigImporter->GetMemoryRequirements(*in);
}

// ------------------------------------------------------------------------------------------------
aiPropertyStore *aiCreatePropertyStore(void) {
    return reinterpret_cast<aiPropertyStore *>(new PropertyMap());
}

// ------------------------------------------------------------------------------------------------
void aiReleasePropertyStore(aiPropertyStore *p) {
    delete reinterpret_cast<PropertyMap *>(p);
}

// ------------------------------------------------------------------------------------------------
// Importer::SetPropertyInteger
void aiSetImportPropertyInteger(aiPropertyStore *p, const char *szName, int value) {
    PropertyMap *pp = reinterpret_cast<PropertyMap *>(p);
    SetGenericProperty<int>(pp->ints, szName, value);
}

// ------------------------------------------------------------------------------------------------
// Importer::SetPropertyFloat
void aiSetImportPropertyFloat(aiPropertyStore *p, const char *szName, ai_real value) {
    PropertyMap *pp = reinterpret_cast<PropertyMap *>(p);
    SetGenericProperty<ai_real>(pp->floats, szName, value);
}

// ------------------------------------------------------------------------------------------------
// Importer::SetPropertyString
void aiSetImportPropertyString(aiPropertyStore *p, const char *szName,
        const aiString *st) {
    if (!st) {
        return;
    }
    PropertyMap *pp = reinterpret_cast<PropertyMap *>(p);
    SetGenericProperty<std::string>(pp->strings, szName, std::string(st->C_Str()));
}

// ------------------------------------------------------------------------------------------------
// Importer::SetPropertyMatrix
void aiSetImportPropertyMatrix(aiPropertyStore *p, const char *szName,
        const aiMatrix4x4 *mat) {
    if (!mat) {
        return;
    }
    PropertyMap *pp = reinterpret_cast<PropertyMap *>(p);
    SetGenericProperty<aiMatrix4x4>(pp->matrices, szName, *mat);
}

// ------------------------------------------------------------------------------------------------
// Rotation matrix to quaternion
void aiCreateQuaternionFromMatrix(aiQuaternion *quat, const aiMatrix3x3 *mat) {
    *quat = aiQuaternion(*mat);
}

// ------------------------------------------------------------------------------------------------
// Matrix decomposition
void aiDecomposeMatrix(const aiMatrix4x4 *mat, aiVector3D *scaling,
        aiQuaternion *rotation,
        aiVector3D *position) {
    mat->Decompose(*scaling, *rotation, *position);
}

// ------------------------------------------------------------------------------------------------
// Matrix transpose
void aiTransposeMatrix3(aiMatrix3x3 *mat) {
    mat->Transpose();
}

// ------------------------------------------------------------------------------------------------
void aiTransposeMatrix4(aiMatrix4x4 *mat) {
    mat->Transpose();
}

// ------------------------------------------------------------------------------------------------
// Vector transformation
void aiTransformVecByMatrix3(aiVector3D *vec,
        const aiMatrix3x3 *mat) {
    *vec *= (*mat);
}

// ------------------------------------------------------------------------------------------------
void aiTransformVecByMatrix4(aiVector3D *vec,
        const aiMatrix4x4 *mat) {
    *vec *= (*mat);
}

// ------------------------------------------------------------------------------------------------
// Matrix multiplication
void aiMultiplyMatrix4(
        aiMatrix4x4 *dst,
        const aiMatrix4x4 *src) {
    *dst = (*dst) * (*src);
}

// ------------------------------------------------------------------------------------------------
void aiMultiplyMatrix3(
        aiMatrix3x3 *dst,
        const aiMatrix3x3 *src) {
    *dst = (*dst) * (*src);
}

// ------------------------------------------------------------------------------------------------
// Matrix identity
void aiIdentityMatrix3(
        aiMatrix3x3 *mat) {
    *mat = aiMatrix3x3();
}

// ------------------------------------------------------------------------------------------------
void aiIdentityMatrix4(
        aiMatrix4x4 *mat) {
    *mat = aiMatrix4x4();
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc *aiGetImporterDesc(const char *extension) {
    if (nullptr == extension) {
        return nullptr;
    }
    const aiImporterDesc *desc(nullptr);
    std::vector<BaseImporter *> out;
    GetImporterInstanceList(out);
    for (size_t i = 0; i < out.size(); ++i) {
        if (0 == strncmp(out[i]->GetInfo()->mFileExtensions, extension, strlen(extension))) {
            desc = out[i]->GetInfo();
            break;
        }
    }

    DeleteImporterInstanceList(out);

    return desc;
}

// ------------------------------------------------------------------------------------------------
int aiVector2AreEqual(
        const aiVector2D *a,
        const aiVector2D *b) {
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
int aiVector2AreEqualEpsilon(
        const aiVector2D *a,
        const aiVector2D *b,
        const float epsilon) {
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
void aiVector2Add(
        aiVector2D *dst,
        const aiVector2D *src) {
    *dst = *dst + *src;
}

// ------------------------------------------------------------------------------------------------
void aiVector2Subtract(
        aiVector2D *dst,
        const aiVector2D *src) {
    *dst = *dst - *src;
}

// ------------------------------------------------------------------------------------------------
void aiVector2Scale(
        aiVector2D *dst,
        const float s) {
    *dst *= s;
}

// ------------------------------------------------------------------------------------------------
void aiVector2SymMul(
        aiVector2D *dst,
        const aiVector2D *other) {
    *dst = dst->SymMul(*other);
}

// ------------------------------------------------------------------------------------------------
void aiVector2DivideByScalar(
        aiVector2D *dst,
        const float s) {
    *dst /= s;
}

// ------------------------------------------------------------------------------------------------
void aiVector2DivideByVector(
        aiVector2D *dst,
        aiVector2D *v) {
    *dst = *dst / *v;
}

// ------------------------------------------------------------------------------------------------
float aiVector2Length(
        const aiVector2D *v) {
    return v->Length();
}

// ------------------------------------------------------------------------------------------------
float aiVector2SquareLength(
        const aiVector2D *v) {
    return v->SquareLength();
}

// ------------------------------------------------------------------------------------------------
void aiVector2Negate(
        aiVector2D *dst) {
    *dst = -(*dst);
}

// ------------------------------------------------------------------------------------------------
float aiVector2DotProduct(
        const aiVector2D *a,
        const aiVector2D *b) {
    return (*a) * (*b);
}

// ------------------------------------------------------------------------------------------------
void aiVector2Normalize(
        aiVector2D *v) {
    v->Normalize();
}

// ------------------------------------------------------------------------------------------------
int aiVector3AreEqual(
        const aiVector3D *a,
        const aiVector3D *b) {
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
int aiVector3AreEqualEpsilon(
        const aiVector3D *a,
        const aiVector3D *b,
        const float epsilon) {
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
int aiVector3LessThan(
        const aiVector3D *a,
        const aiVector3D *b) {
    return *a < *b;
}

// ------------------------------------------------------------------------------------------------
void aiVector3Add(
        aiVector3D *dst,
        const aiVector3D *src) {
    *dst = *dst + *src;
}

// ------------------------------------------------------------------------------------------------
void aiVector3Subtract(
        aiVector3D *dst,
        const aiVector3D *src) {
    *dst = *dst - *src;
}

// ------------------------------------------------------------------------------------------------
void aiVector3Scale(
        aiVector3D *dst,
        const float s) {
    *dst *= s;
}

// ------------------------------------------------------------------------------------------------
void aiVector3SymMul(
        aiVector3D *dst,
        const aiVector3D *other) {
    *dst = dst->SymMul(*other);
}

// ------------------------------------------------------------------------------------------------
void aiVector3DivideByScalar(
        aiVector3D *dst, const float s) {
    *dst /= s;
}

// ------------------------------------------------------------------------------------------------
void aiVector3DivideByVector(
        aiVector3D *dst,
        aiVector3D *v) {
    *dst = *dst / *v;
}

// ------------------------------------------------------------------------------------------------
float aiVector3Length(
        const aiVector3D *v) {
    return v->Length();
}

// ------------------------------------------------------------------------------------------------
float aiVector3SquareLength(
        const aiVector3D *v) {
    return v->SquareLength();
}

// ------------------------------------------------------------------------------------------------
void aiVector3Negate(
        aiVector3D *dst) {
    *dst = -(*dst);
}

// ------------------------------------------------------------------------------------------------
float aiVector3DotProduct(
        const aiVector3D *a,
        const aiVector3D *b) {
    return (*a) * (*b);
}

// ------------------------------------------------------------------------------------------------
void aiVector3CrossProduct(
        aiVector3D *dst,
        const aiVector3D *a,
        const aiVector3D *b) {
    *dst = *a ^ *b;
}

// ------------------------------------------------------------------------------------------------
void aiVector3Normalize(
        aiVector3D *v) {
    v->Normalize();
}

// ------------------------------------------------------------------------------------------------
void aiVector3NormalizeSafe(
        aiVector3D *v) {
    v->NormalizeSafe();
}

// ------------------------------------------------------------------------------------------------
void aiVector3RotateByQuaternion(
        aiVector3D *v,
        const aiQuaternion *q) {
    *v = q->Rotate(*v);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix3FromMatrix4(
        aiMatrix3x3 *dst,
        const aiMatrix4x4 *mat) {
    *dst = aiMatrix3x3(*mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix3FromQuaternion(
        aiMatrix3x3 *mat,
        const aiQuaternion *q) {
    *mat = q->GetMatrix();
}

// ------------------------------------------------------------------------------------------------
int aiMatrix3AreEqual(
        const aiMatrix3x3 *a,
        const aiMatrix3x3 *b) {
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
int aiMatrix3AreEqualEpsilon(
        const aiMatrix3x3 *a,
        const aiMatrix3x3 *b,
        const float epsilon) {
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix3Inverse(aiMatrix3x3 *mat) {
    mat->Inverse();
}

// ------------------------------------------------------------------------------------------------
float aiMatrix3Determinant(const aiMatrix3x3 *mat) {
    return mat->Determinant();
}

// ------------------------------------------------------------------------------------------------
void aiMatrix3RotationZ(
        aiMatrix3x3 *mat,
        const float angle) {
    aiMatrix3x3::RotationZ(angle, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix3FromRotationAroundAxis(
        aiMatrix3x3 *mat,
        const aiVector3D *axis,
        const float angle) {
    aiMatrix3x3::Rotation(angle, *axis, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix3Translation(
        aiMatrix3x3 *mat,
        const aiVector2D *translation) {
    aiMatrix3x3::Translation(*translation, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix3FromTo(
        aiMatrix3x3 *mat,
        const aiVector3D *from,
        const aiVector3D *to) {
    aiMatrix3x3::FromToMatrix(*from, *to, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4FromMatrix3(
        aiMatrix4x4 *dst,
        const aiMatrix3x3 *mat) {
    *dst = aiMatrix4x4(*mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4FromScalingQuaternionPosition(
        aiMatrix4x4 *mat,
        const aiVector3D *scaling,
        const aiQuaternion *rotation,
        const aiVector3D *position) {
    *mat = aiMatrix4x4(*scaling, *rotation, *position);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4Add(
        aiMatrix4x4 *dst,
        const aiMatrix4x4 *src) {
    *dst = *dst + *src;
}

// ------------------------------------------------------------------------------------------------
int aiMatrix4AreEqual(
        const aiMatrix4x4 *a,
        const aiMatrix4x4 *b) {
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
int aiMatrix4AreEqualEpsilon(
        const aiMatrix4x4 *a,
        const aiMatrix4x4 *b,
        const float epsilon) {
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4Inverse(aiMatrix4x4 *mat) {
    mat->Inverse();
}

// ------------------------------------------------------------------------------------------------
float aiMatrix4Determinant(const aiMatrix4x4 *mat) {
    return mat->Determinant();
}

// ------------------------------------------------------------------------------------------------
int aiMatrix4IsIdentity(const aiMatrix4x4 *mat) {
    return mat->IsIdentity();
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4DecomposeIntoScalingEulerAnglesPosition(
        const aiMatrix4x4 *mat,
        aiVector3D *scaling,
        aiVector3D *rotation,
        aiVector3D *position) {
    mat->Decompose(*scaling, *rotation, *position);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4DecomposeIntoScalingAxisAnglePosition(
        const aiMatrix4x4 *mat,
        aiVector3D *scaling,
        aiVector3D *axis,
        ai_real *angle,
        aiVector3D *position) {
    mat->Decompose(*scaling, *axis, *angle, *position);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4DecomposeNoScaling(
        const aiMatrix4x4 *mat,
        aiQuaternion *rotation,
        aiVector3D *position) {
    mat->DecomposeNoScaling(*rotation, *position);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4FromEulerAngles(
        aiMatrix4x4 *mat,
        float x, float y, float z) {
    mat->FromEulerAnglesXYZ(x, y, z);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4RotationX(
        aiMatrix4x4 *mat,
        const float angle) {
    aiMatrix4x4::RotationX(angle, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4RotationY(
        aiMatrix4x4 *mat,
        const float angle) {
    aiMatrix4x4::RotationY(angle, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4RotationZ(
        aiMatrix4x4 *mat,
        const float angle) {
    aiMatrix4x4::RotationZ(angle, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4FromRotationAroundAxis(
        aiMatrix4x4 *mat,
        const aiVector3D *axis,
        const float angle) {
    aiMatrix4x4::Rotation(angle, *axis, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4Translation(
        aiMatrix4x4 *mat,
        const aiVector3D *translation) {
    aiMatrix4x4::Translation(*translation, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4Scaling(
        aiMatrix4x4 *mat,
        const aiVector3D *scaling) {
    aiMatrix4x4::Scaling(*scaling, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiMatrix4FromTo(
        aiMatrix4x4 *mat,
        const aiVector3D *from,
        const aiVector3D *to) {
    aiMatrix4x4::FromToMatrix(*from, *to, *mat);
}

// ------------------------------------------------------------------------------------------------
void aiQuaternionFromEulerAngles(
        aiQuaternion *q,
        float x, float y, float z) {
    *q = aiQuaternion(x, y, z);
}

// ------------------------------------------------------------------------------------------------
void aiQuaternionFromAxisAngle(
        aiQuaternion *q,
        const aiVector3D *axis,
        const float angle) {
    *q = aiQuaternion(*axis, angle);
}

// ------------------------------------------------------------------------------------------------
void aiQuaternionFromNormalizedQuaternion(
        aiQuaternion *q,
        const aiVector3D *normalized) {
    *q = aiQuaternion(*normalized);
}

// ------------------------------------------------------------------------------------------------
int aiQuaternionAreEqual(
        const aiQuaternion *a,
        const aiQuaternion *b) {
    return *a == *b;
}

// ------------------------------------------------------------------------------------------------
int aiQuaternionAreEqualEpsilon(
        const aiQuaternion *a,
        const aiQuaternion *b,
        const float epsilon) {
    return a->Equal(*b, epsilon);
}

// ------------------------------------------------------------------------------------------------
void aiQuaternionNormalize(
        aiQuaternion *q) {
    q->Normalize();
}

// ------------------------------------------------------------------------------------------------
void aiQuaternionConjugate(
        aiQuaternion *q) {
    q->Conjugate();
}

// ------------------------------------------------------------------------------------------------
void aiQuaternionMultiply(
        aiQuaternion *dst,
        const aiQuaternion *q) {
    *dst = (*dst) * (*q);
}

// ------------------------------------------------------------------------------------------------
void aiQuaternionInterpolate(
        aiQuaternion *dst,
        const aiQuaternion *start,
        const aiQuaternion *end,
        const float factor) {
    aiQuaternion::Interpolate(*dst, *start, *end, factor);
}


// stb_image is a lightweight image loader. It is shared by:
//  - M3D import
//  - PBRT export
// Since it's a header-only library, its implementation must be instantiated in some cpp file.
// Don't scatter this task over multiple importers/exporters. Maintain it in a central place (here!).

#ifndef STB_USE_HUNTER
#   if ASSIMP_HAS_PBRT_EXPORT
#       define ASSIMP_NEEDS_STB_IMAGE 1
#   elif ASSIMP_HAS_M3D
#       define ASSIMP_NEEDS_STB_IMAGE 1
#       define STBI_ONLY_PNG
#   endif
#endif

// Ensure all symbols are linked correctly
#if ASSIMP_NEEDS_STB_IMAGE
    // Share stb_image's PNG loader with other importers/exporters instead of bringing our own copy.
#   define STBI_ONLY_PNG
#   ifdef ASSIMP_USE_STB_IMAGE_STATIC
#       define STB_IMAGE_STATIC
#   endif
#   define STB_IMAGE_IMPLEMENTATION
#   include "Common/StbCommon.h"
#endif
