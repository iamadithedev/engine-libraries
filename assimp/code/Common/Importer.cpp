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

/** @file  Importer.cpp
 *  @brief Implementation of the CPP-API class #Importer
 */

#include <assimp/importerdesc.h>

// ------------------------------------------------------------------------------------------------
/* Uncomment this line to prevent Assimp from catching unknown exceptions.
 *
 * Note that any Exception except DeadlyImportError may lead to
 * undefined behaviour -> loaders could remain in an unusable state and
 * further imports with the same Importer instance could fail/crash/burn ...
 */
// ------------------------------------------------------------------------------------------------
#ifndef ASSIMP_BUILD_DEBUG
#   define ASSIMP_CATCH_GLOBAL_EXCEPTIONS
#endif

// ------------------------------------------------------------------------------------------------
// Internal headers
// ------------------------------------------------------------------------------------------------
#include "Common/Importer.h"
#include "Common/BaseProcess.h"
#include "PostProcessing/ProcessHelper.h"
#include "Common/ScenePreprocessor.h"
#include "Common/ScenePrivate.h"

#include <assimp/BaseImporter.h>
#include <assimp/GenericProperty.h>
#include <assimp/MemoryIOWrapper.h>
#include <assimp/commonMetaData.h>

#include <exception>
#include <set>
#include <memory>

#include <assimp/DefaultIOStream.h>
#include <assimp/DefaultIOSystem.h>

namespace Assimp {
    // ImporterRegistry.cpp
    void GetImporterInstanceList(std::vector< BaseImporter* >& out);
	void DeleteImporterInstanceList(std::vector< BaseImporter* >& out);

    // PostStepRegistry.cpp
    void GetPostProcessingStepInstanceList(std::vector< BaseProcess* >& out);
}

using namespace Assimp;
using namespace Assimp::Intern;

// ------------------------------------------------------------------------------------------------
// Intern::AllocateFromAssimpHeap serves as abstract base class. It overrides
// new and delete (and their array counterparts) of public API classes (e.g. Logger) to
// utilize our DLL heap.
// See http://www.gotw.ca/publications/mill15.htm
// ------------------------------------------------------------------------------------------------
void* AllocateFromAssimpHeap::operator new ( size_t num_bytes)  {
    return ::operator new(num_bytes);
}

void* AllocateFromAssimpHeap::operator new ( size_t num_bytes, const std::nothrow_t& ) noexcept  {
    try {
        return AllocateFromAssimpHeap::operator new( num_bytes );
    }
    catch( ... )    {
        return nullptr;
    }
}

void AllocateFromAssimpHeap::operator delete ( void* data)  {
    return ::operator delete(data);
}

void* AllocateFromAssimpHeap::operator new[] ( size_t num_bytes)    {
    return ::operator new[](num_bytes);
}

void* AllocateFromAssimpHeap::operator new[] ( size_t num_bytes, const std::nothrow_t& ) noexcept {
    try {
        return AllocateFromAssimpHeap::operator new[]( num_bytes );
    } catch( ... )    {
        return nullptr;
    }
}

void AllocateFromAssimpHeap::operator delete[] ( void* data)    {
    return ::operator delete[](data);
}

// ------------------------------------------------------------------------------------------------
// Importer constructor.
Importer::Importer()
 : pimpl( new ImporterPimpl ) {
    pimpl->mScene = nullptr;

    // Allocate a default IO handler
    pimpl->mIOHandler = new DefaultIOSystem;
    pimpl->mIsDefaultHandler = true;
    pimpl->bExtraVerbose     = false; // disable extra verbose mode by default

    GetImporterInstanceList(pimpl->mImporter);
    GetPostProcessingStepInstanceList(pimpl->mPostProcessingSteps);

    // Allocate a SharedPostProcessInfo object and store pointers to it in all post-process steps in the list.
    pimpl->mPPShared = new SharedPostProcessInfo();
    for (auto it =  pimpl->mPostProcessingSteps.begin();
        it != pimpl->mPostProcessingSteps.end();
        ++it)   {

        (*it)->SetSharedData(pimpl->mPPShared);
    }
}

// ------------------------------------------------------------------------------------------------
// Destructor of Importer
Importer::~Importer() {
    // Delete all import plugins
	DeleteImporterInstanceList(pimpl->mImporter);

    // Delete all post-processing plug-ins
    for( unsigned int a = 0; a < pimpl->mPostProcessingSteps.size(); ++a ) {
        delete pimpl->mPostProcessingSteps[a];
    }

    // Delete the assigned IO and progress handler
    delete pimpl->mIOHandler;

    // Kill imported scene. Destructor's should do that recursively
    delete pimpl->mScene;

    // Delete shared post-processing data
    delete pimpl->mPPShared;

    // and finally the pimpl itself
    delete pimpl;
}

// ------------------------------------------------------------------------------------------------
// Register a custom post-processing step
aiReturn Importer::RegisterPPStep(BaseProcess* pImp) {
    pimpl->mPostProcessingSteps.push_back(pImp);

    return AI_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// Register a custom loader plugin
aiReturn Importer::RegisterLoader(BaseImporter* pImp) {

    // --------------------------------------------------------------------
    // Check whether we would have two loaders for the same file extension
    // This is absolutely OK, but we should warn the developer of the new
    // loader that his code will probably never be called if the first
    // loader is a bit too lazy in his file checking.
    // --------------------------------------------------------------------
    std::set<std::string> st;
    std::string baked;
    pImp->GetExtensionList(st);

    for(auto it = st.begin(); it != st.end(); ++it) {
        baked += *it;
    }

    // add the loader
    pimpl->mImporter.push_back(pImp);

    return AI_SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// Unregister a custom loader plugin
aiReturn Importer::UnregisterLoader(BaseImporter* pImp) {
    if(!pImp) {
        // unregistering a nullptr importer is no problem for us ... really!
        return AI_SUCCESS;
    }

    auto it = std::find(pimpl->mImporter.begin(),
        pimpl->mImporter.end(),pImp);

    if (it != pimpl->mImporter.end())   {
        pimpl->mImporter.erase(it);
        return AI_SUCCESS;
    }

    return AI_FAILURE;
}

// ------------------------------------------------------------------------------------------------
// Unregister a custom loader plugin
aiReturn Importer::UnregisterPPStep(BaseProcess* pImp) {
    if(!pImp) {
        // unregistering a nullptr ppstep is no problem for us ... really!
        return AI_SUCCESS;
    }

    auto it = std::find(pimpl->mPostProcessingSteps.begin(),
        pimpl->mPostProcessingSteps.end(),pImp);

    if (it != pimpl->mPostProcessingSteps.end())    {
        pimpl->mPostProcessingSteps.erase(it);
        return AI_SUCCESS;
    }

    return AI_FAILURE;
}

// ------------------------------------------------------------------------------------------------
// Supplies a custom IO handler to the importer to open and access files.
void Importer::SetIOHandler( IOSystem* pIOHandler) {
    // If the new handler is zero, allocate a default IO implementation.
    if (!pIOHandler) {
        // Release pointer in the possession of the caller
        pimpl->mIOHandler = new DefaultIOSystem();
        pimpl->mIsDefaultHandler = true;
    } else if (pimpl->mIOHandler != pIOHandler) { // Otherwise register the custom handler
        delete pimpl->mIOHandler;
        pimpl->mIOHandler = pIOHandler;
        pimpl->mIsDefaultHandler = false;
    }
}

// ------------------------------------------------------------------------------------------------
// Get the currently set IO handler
IOSystem* Importer::GetIOHandler() const {
    return pimpl->mIOHandler;
}

// ------------------------------------------------------------------------------------------------
// Check whether a custom IO handler is currently set
bool Importer::IsDefaultIOHandler() const {
    return pimpl->mIsDefaultHandler;
}

// ------------------------------------------------------------------------------------------------
// Validate post process step flags
bool _ValidateFlags(unsigned int pFlags) {
    if (pFlags & aiProcess_GenSmoothNormals && pFlags & aiProcess_GenNormals)   {
        return false;
    }
    if (pFlags & aiProcess_OptimizeGraph && pFlags & aiProcess_PreTransformVertices)    {
        return false;
    }
    return true;
}

// ------------------------------------------------------------------------------------------------
// Free the current scene
void Importer::FreeScene( ) {
    delete pimpl->mScene;
    pimpl->mScene = nullptr;

    pimpl->mException = std::exception_ptr();
}

const std::exception_ptr& Importer::GetException() const {
    // Must remain valid as long as ReadFile() or FreeFile() are not called
    return pimpl->mException;
}

// ------------------------------------------------------------------------------------------------
// Enable extra-verbose mode
void Importer::SetExtraVerbose(bool bDo) {
    pimpl->bExtraVerbose = bDo;
}

// ------------------------------------------------------------------------------------------------
// Get the current scene
const aiScene* Importer::GetScene() const {
    return pimpl->mScene;
}

// ------------------------------------------------------------------------------------------------
// Orphan the current scene and return it.
aiScene* Importer::GetOrphanedScene() {
    aiScene* s = pimpl->mScene;

    pimpl->mScene = nullptr;
    pimpl->mException = std::exception_ptr();

    return s;
}

// ------------------------------------------------------------------------------------------------
// Validate post-processing flags
bool Importer::ValidateFlags(unsigned int pFlags) const {

    // run basic checks for mutually exclusive flags
    if(!_ValidateFlags(pFlags)) {
        return false;
    }

    // Now iterate through all bits which are set in the flags and check whether we find at least
    // one pp plugin which handles it.
    for (unsigned int mask = 1; mask < (1u << (sizeof(unsigned int)*8-1));mask <<= 1) {

        if (pFlags & mask) {

            bool have = false;
            for( unsigned int a = 0; a < pimpl->mPostProcessingSteps.size(); a++)   {
                if (pimpl->mPostProcessingSteps[a]-> IsActive(mask) ) {

                    have = true;
                    break;
                }
            }
            if (!have) {
                return false;
            }
        }
    }

    return true;
}

// ------------------------------------------------------------------------------------------------
const aiScene* Importer::ReadFileFromMemory(const void* pBuffer, size_t pLength, unsigned int pFlags, const char* pHint ) {

    IOSystem* io = pimpl->mIOHandler;

    if (pHint == nullptr) {
        pHint = "";
    }
    if (!pBuffer || !pLength || strlen(pHint) > MaxLenHint ) {
        return nullptr;
    }
    // prevent deletion of the previous IOHandler
    pimpl->mIOHandler = nullptr;

    SetIOHandler(new MemoryIOSystem((const uint8_t*)pBuffer,pLength,io));

    // read the file and recover the previous IOSystem
    static const size_t BufSize(Importer::MaxLenHint + 28);
    char fbuff[BufSize];
    ai_snprintf(fbuff, BufSize, "%s.%s",AI_MEMORYIO_MAGIC_FILENAME,pHint);

    ReadFile(fbuff,pFlags);
    SetIOHandler(io);

    return pimpl->mScene;
}

// ------------------------------------------------------------------------------------------------
// Reads the given file and returns its contents if successful.
const aiScene* Importer::ReadFile( const char* _pFile, unsigned int pFlags) {
    const std::string pFile(_pFile);

    // ----------------------------------------------------------------------
    // Put a large try block around everything to catch all std::exception's
    // that might be thrown by STL containers or by new().
    // ImportErrorException's are throw by ourselves and caught elsewhere.
    //-----------------------------------------------------------------------

#ifdef ASSIMP_CATCH_GLOBAL_EXCEPTIONS
    try
#endif // ! ASSIMP_CATCH_GLOBAL_EXCEPTIONS
    {
        // Check whether this Importer instance has already loaded
        // a scene. In this case we need to delete the old one
        if (pimpl->mScene)  {
            FreeScene();
        }

        // First check if the file is accessible at all
        if( !pimpl->mIOHandler->Exists( pFile)) {

            return nullptr;
        }

        // Find an worker class which can handle the file extension.
        // Multiple importers may be able to handle the same extension (.xml!); gather them all.
        SetPropertyInteger("importerIndex", -1);
        struct ImporterAndIndex {
            BaseImporter * importer;
            unsigned int   index;
        };
        std::vector<ImporterAndIndex> possibleImporters;
        for (unsigned int a = 0; a < pimpl->mImporter.size(); a++)  {

            // Every importer has a list of supported extensions.
            std::set<std::string> extensions;
            pimpl->mImporter[a]->GetExtensionList(extensions);

            // CAUTION: Do not just search for the extension!
            // GetExtension() returns the part after the *last* dot, but some extensions have dots
            // inside them, e.g. ogre.mesh.xml. Compare the entire end of the string.
            for (auto it = extensions.cbegin(); it != extensions.cend(); ++it) {

                // Yay for C++<20 not having std::string::ends_with()
                std::string extension = "." + *it;
                if (extension.length() <= pFile.length()) {
                    // Possible optimization: Fetch the lowercase filename!
                    if (0 == ASSIMP_stricmp(pFile.c_str() + pFile.length() - extension.length(), extension.c_str())) {
                        ImporterAndIndex candidate = { pimpl->mImporter[a], a };
                        possibleImporters.push_back(candidate);
                        break;
                    }
                }

            }

        }

        // If just one importer supports this extension, pick it and close the case.
        BaseImporter* imp = nullptr;
        if (1 == possibleImporters.size()) {
            imp = possibleImporters[0].importer;
            SetPropertyInteger("importerIndex", possibleImporters[0].index);
        }
        // If multiple importers claim this file extension, ask them to look at the actual file data to decide.
        // This can happen e.g. with XML (COLLADA vs. Irrlicht).
        else {
            for (std::vector<ImporterAndIndex>::const_iterator it = possibleImporters.begin(); it < possibleImporters.end(); ++it) {
                BaseImporter & importer = *it->importer;

                if (importer.CanRead( pFile, pimpl->mIOHandler, true)) {
                    imp = &importer;
                    SetPropertyInteger("importerIndex", it->index);
                    break;
                }

            }

        }

        if (!imp)   {
            // not so bad yet ... try format auto detection.
            for( unsigned int a = 0; a < pimpl->mImporter.size(); a++)  {
                if( pimpl->mImporter[a]->CanRead( pFile, pimpl->mIOHandler, true)) {
                    imp = pimpl->mImporter[a];
                    SetPropertyInteger("importerIndex", a);
                    break;
                }
            }
            // Put a proper error message if no suitable importer was found
            if( !imp)   {
                return nullptr;
            }
        }

        // Get file size for progress handler
        IOStream * fileIO = pimpl->mIOHandler->Open( pFile );
        uint32_t fileSize = 0;
        if (fileIO)
        {
            fileSize = static_cast<uint32_t>(fileIO->FileSize());
            pimpl->mIOHandler->Close( fileIO );
        }

        // Dispatch the reading to the worker class for this format
        const aiImporterDesc *desc( imp->GetInfo() );
        std::string ext( "unknown" );
        if ( nullptr != desc ) {
            ext = desc->mName;
        }

        pimpl->mScene = imp->ReadFile( this, pFile, pimpl->mIOHandler);

        SetPropertyString("sourceFilePath", pFile);

        // If successful, apply all active post processing steps to the imported data
        if( pimpl->mScene)  {
            if (!pimpl->mScene->mMetaData || !pimpl->mScene->mMetaData->HasKey(AI_METADATA_SOURCE_FORMAT)) {
                if (!pimpl->mScene->mMetaData) {
                    pimpl->mScene->mMetaData = new aiMetadata;
                }
                pimpl->mScene->mMetaData->Add(AI_METADATA_SOURCE_FORMAT, aiString(ext));
            }

            ScenePreprocessor pre(pimpl->mScene);
            pre.ProcessScene();

            // Ensure that the validation process won't be called twice
            ApplyPostProcessing(pFlags);
        }
        // if failed, extract the error string
        else if( !pimpl->mScene) {
            pimpl->mException = imp->GetException();
        }

        // clear any data allocated by post-process steps
        pimpl->mPPShared->Clean();
    }
#ifdef ASSIMP_CATCH_GLOBAL_EXCEPTIONS
    catch (std::exception &) {
        delete pimpl->mScene; pimpl->mScene = nullptr;
    }
#endif // ! ASSIMP_CATCH_GLOBAL_EXCEPTIONS

    return pimpl->mScene;
}


// ------------------------------------------------------------------------------------------------
// Apply post-processing to the currently bound scene
const aiScene* Importer::ApplyPostProcessing(unsigned int pFlags) {

    // Return immediately if no scene is active
    if (!pimpl->mScene) {
        return nullptr;
    }

    // If no flags are given, return the current scene with no further action
    if (!pFlags) {
        return pimpl->mScene;
    }

    for( unsigned int a = 0; a < pimpl->mPostProcessingSteps.size(); a++)   {
        BaseProcess* process = pimpl->mPostProcessingSteps[a];

        if( process->IsActive( pFlags)) {
            process->ExecuteOnScene ( this );
        }
        if( !pimpl->mScene) {
            break;
        }
    }

    // update private scene flags
    if( pimpl->mScene ) {
      ScenePriv(pimpl->mScene)->mPPStepsApplied |= pFlags;
    }

    // clear any data allocated by post-process steps
    pimpl->mPPShared->Clean();

    return pimpl->mScene;
}

// ------------------------------------------------------------------------------------------------
const aiScene* Importer::ApplyCustomizedPostProcessing( BaseProcess *rootProcess ) {
    // Return immediately if no scene is active
    if ( nullptr == pimpl->mScene ) {
        return nullptr;
    }

    // If no flags are given, return the current scene with no further action
    if (nullptr == rootProcess) {
        return pimpl->mScene;
    }

    rootProcess->ExecuteOnScene( this );

    // clear any data allocated by post-process steps
    pimpl->mPPShared->Clean();

    return pimpl->mScene;
}

// ------------------------------------------------------------------------------------------------
// Helper function to check whether an extension is supported by ASSIMP
bool Importer::IsExtensionSupported(const char* szExtension) const {
    return nullptr != GetImporter(szExtension);
}

// ------------------------------------------------------------------------------------------------
size_t Importer::GetImporterCount() const {
    return pimpl->mImporter.size();
}

// ------------------------------------------------------------------------------------------------
const aiImporterDesc* Importer::GetImporterInfo(size_t index) const {
    if (index >= pimpl->mImporter.size()) {
        return nullptr;
    }
    return pimpl->mImporter[index]->GetInfo();
}


// ------------------------------------------------------------------------------------------------
BaseImporter* Importer::GetImporter (size_t index) const {
    if (index >= pimpl->mImporter.size()) {
        return nullptr;
    }
    return pimpl->mImporter[index];
}

// ------------------------------------------------------------------------------------------------
// Find a loader plugin for a given file extension
BaseImporter* Importer::GetImporter (const char* szExtension) const {
    return GetImporter(GetImporterIndex(szExtension));
}

// ------------------------------------------------------------------------------------------------
// Find a loader plugin for a given file extension
size_t Importer::GetImporterIndex (const char* szExtension) const {
    // skip over wild-card and dot characters at string head --
    for ( ; *szExtension == '*' || *szExtension == '.'; ++szExtension );

    std::string ext(szExtension);
    if (ext.empty()) {
        return static_cast<size_t>(-1);
    }
    ext = ai_tolower(ext);
    std::set<std::string> str;
    for (std::vector<BaseImporter*>::const_iterator i =  pimpl->mImporter.begin();i != pimpl->mImporter.end();++i)  {
        str.clear();

        (*i)->GetExtensionList(str);
        for (std::set<std::string>::const_iterator it = str.begin(); it != str.end(); ++it) {
            if (ext == *it) {
                return std::distance(static_cast< std::vector<BaseImporter*>::const_iterator >(pimpl->mImporter.begin()), i);
            }
        }
    }
    return static_cast<size_t>(-1);
}

// ------------------------------------------------------------------------------------------------
// Helper function to build a list of all file extensions supported by ASSIMP
void Importer::GetExtensionList(aiString& szOut) const {
    std::set<std::string> str;
    for (std::vector<BaseImporter*>::const_iterator i =  pimpl->mImporter.begin();i != pimpl->mImporter.end();++i)  {
        (*i)->GetExtensionList(str);
    }

	// List can be empty
	if( !str.empty() ) {
		for (std::set<std::string>::const_iterator it = str.begin();; ) {
			szOut.Append("*.");
			szOut.Append((*it).c_str());

			if (++it == str.end()) {
				break;
			}
			szOut.Append(";");
		}
	}
}

// ------------------------------------------------------------------------------------------------
// Set a configuration property
bool Importer::SetPropertyInteger(const char* szName, int iValue) {
    bool existing;

    existing = SetGenericProperty<int>(pimpl->mIntProperties, szName,iValue);

    return existing;
}

// ------------------------------------------------------------------------------------------------
// Set a configuration property
bool Importer::SetPropertyFloat(const char* szName, ai_real iValue) {
    bool existing;

        existing = SetGenericProperty<ai_real>(pimpl->mFloatProperties, szName,iValue);

    return existing;
}

// ------------------------------------------------------------------------------------------------
// Set a configuration property
bool Importer::SetPropertyString(const char* szName, const std::string& value) {
    bool existing;

        existing = SetGenericProperty<std::string>(pimpl->mStringProperties, szName,value);

    return existing;
}

// ------------------------------------------------------------------------------------------------
// Set a configuration property
bool Importer::SetPropertyMatrix(const char* szName, const aiMatrix4x4& value) {
    bool existing;

        existing = SetGenericProperty<aiMatrix4x4>(pimpl->mMatrixProperties, szName,value);

    return existing;
}

// ------------------------------------------------------------------------------------------------
// Set a configuration property
bool Importer::SetPropertyPointer(const char* szName, void* value) {
    bool existing;

        existing = SetGenericProperty<void*>(pimpl->mPointerProperties, szName,value);

    return existing;
}

// ------------------------------------------------------------------------------------------------
// Get a configuration property
int Importer::GetPropertyInteger(const char* szName, int iErrorReturn /*= 0xffffffff*/) const {
    return GetGenericProperty<int>(pimpl->mIntProperties,szName,iErrorReturn);
}

// ------------------------------------------------------------------------------------------------
// Get a configuration property
ai_real Importer::GetPropertyFloat(const char* szName, ai_real iErrorReturn /*= 10e10*/) const {
    return GetGenericProperty<ai_real>(pimpl->mFloatProperties,szName,iErrorReturn);
}

// ------------------------------------------------------------------------------------------------
// Get a configuration property
std::string Importer::GetPropertyString(const char* szName, const std::string& iErrorReturn /*= ""*/) const {
    return GetGenericProperty<std::string>(pimpl->mStringProperties,szName,iErrorReturn);
}

// ------------------------------------------------------------------------------------------------
// Get a configuration property
aiMatrix4x4 Importer::GetPropertyMatrix(const char* szName, const aiMatrix4x4& iErrorReturn /*= aiMatrix4x4()*/) const {
    return GetGenericProperty<aiMatrix4x4>(pimpl->mMatrixProperties,szName,iErrorReturn);
}

// ------------------------------------------------------------------------------------------------
// Get a configuration property
void* Importer::GetPropertyPointer(const char* szName, void* iErrorReturn /*= nullptr*/) const {
    return GetGenericProperty<void*>(pimpl->mPointerProperties,szName,iErrorReturn);
}

// ------------------------------------------------------------------------------------------------
// Get the memory requirements of a single node
inline
void AddNodeWeight(unsigned int& iScene,const aiNode* pcNode) {
    if ( nullptr == pcNode ) {
        return;
    }
    iScene += sizeof(aiNode);
    iScene += sizeof(unsigned int) * pcNode->mNumMeshes;
    iScene += sizeof(void*) * pcNode->mNumChildren;

    for (unsigned int i = 0; i < pcNode->mNumChildren;++i) {
        AddNodeWeight(iScene,pcNode->mChildren[i]);
    }
}

// ------------------------------------------------------------------------------------------------
// Get the memory requirements of the scene
void Importer::GetMemoryRequirements(aiMemoryInfo& in) const {
    in = aiMemoryInfo();
    aiScene* mScene = pimpl->mScene;

    // return if we have no scene loaded
    if (!mScene)
        return;

    in.total = sizeof(aiScene);

    // add all meshes
    for (unsigned int i = 0; i < mScene->mNumMeshes;++i) {
        in.meshes += sizeof(aiMesh);
        if (mScene->mMeshes[i]->HasPositions()) {
            in.meshes += sizeof(aiVector3D) * mScene->mMeshes[i]->mNumVertices;
        }

        if (mScene->mMeshes[i]->HasNormals()) {
            in.meshes += sizeof(aiVector3D) * mScene->mMeshes[i]->mNumVertices;
        }

        if (mScene->mMeshes[i]->HasTangentsAndBitangents()) {
            in.meshes += sizeof(aiVector3D) * mScene->mMeshes[i]->mNumVertices * 2;
        }

        for (unsigned int a = 0; a < AI_MAX_NUMBER_OF_TEXTURECOORDS;++a) {
            if (mScene->mMeshes[i]->HasTextureCoords(a)) {
                in.meshes += sizeof(aiVector3D) * mScene->mMeshes[i]->mNumVertices;
            } else {
                break;
            }
        }
        if (mScene->mMeshes[i]->HasBones()) {
            in.meshes += sizeof(void*) * mScene->mMeshes[i]->mNumBones;
            for (unsigned int p = 0; p < mScene->mMeshes[i]->mNumBones;++p) {
                in.meshes += sizeof(aiBone);
                in.meshes += mScene->mMeshes[i]->mBones[p]->mNumWeights * sizeof(aiVertexWeight);
            }
        }
        in.meshes += (sizeof(aiFace) + 3 * sizeof(unsigned int))*mScene->mMeshes[i]->mNumFaces;
    }
    in.total += in.meshes;

    // add all embedded textures
    for (unsigned int i = 0; i < mScene->mNumTextures;++i) {
        const aiTexture* pc = mScene->mTextures[i];
        in.textures += sizeof(aiTexture);
        if (pc->mHeight) {
            in.textures += 4 * pc->mHeight * pc->mWidth;
        } else {
            in.textures += pc->mWidth;
        }
    }
    in.total += in.textures;

    // add all animations
    for (unsigned int i = 0; i < mScene->mNumAnimations;++i) {
        const aiAnimation* pc = mScene->mAnimations[i];
        in.animations += sizeof(aiAnimation);

        // add all bone anims
        for (unsigned int a = 0; a < pc->mNumChannels; ++a) {
            const aiNodeAnim* pc2 = pc->mChannels[a];
            in.animations += sizeof(aiNodeAnim);
            in.animations += pc2->mNumPositionKeys * sizeof(aiVectorKey);
            in.animations += pc2->mNumScalingKeys * sizeof(aiVectorKey);
            in.animations += pc2->mNumRotationKeys * sizeof(aiQuatKey);
        }
    }
    in.total += in.animations;

    // add all nodes
    AddNodeWeight(in.nodes,mScene->mRootNode);
    in.total += in.nodes;

    // add all materials
    for (unsigned int i = 0; i < mScene->mNumMaterials;++i) {
        const aiMaterial* pc = mScene->mMaterials[i];
        in.materials += sizeof(aiMaterial);
        in.materials += pc->mNumAllocated * sizeof(void*);

        for (unsigned int a = 0; a < pc->mNumProperties;++a) {
            in.materials += pc->mProperties[a]->mDataLength;
        }
    }

    in.total += in.materials;
}
