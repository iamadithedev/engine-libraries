
SET(SharedMemory_SRCS
	plugins/collisionFilterPlugin/collisionFilterPlugin.cpp
	plugins/collisionFilterPlugin/collisionFilterPlugin.h
	IKTrajectoryHelper.cpp
	IKTrajectoryHelper.h
	PhysicsClient.cpp
	PhysicsClientSharedMemory.cpp
	PhysicsClientExample.cpp
	PhysicsServerExample.cpp
	PhysicsServerExampleBullet2.cpp
	PhysicsServerSharedMemory.cpp
	PhysicsServerSharedMemory.h
	PhysicsServer.cpp
	PhysicsServer.h
	PhysicsClientC_API.cpp
	GraphicsClientExample.cpp
	GraphicsClientExample.h
	GraphicsServerExample.cpp
	GraphicsServerExample.h
	GraphicsSharedMemoryBlock.h
	GraphicsSharedMemoryCommands.h
	GraphicsSharedMemoryPublic.h
	SharedMemoryCommands.h
	SharedMemoryPublic.h
	PhysicsServer.cpp
	PosixSharedMemory.cpp
	Win32SharedMemory.cpp
	InProcessMemory.cpp
	PhysicsDirect.cpp
	PhysicsDirect.h
	PhysicsDirectC_API.cpp
	PhysicsDirectC_API.h
	PhysicsLoopBack.cpp
	PhysicsLoopBack.h
	PhysicsLoopBackC_API.cpp
	PhysicsLoopBackC_API.h
	PhysicsClientSharedMemory_C_API.cpp
	PhysicsClientSharedMemory_C_API.h
	PhysicsClientSharedMemory2_C_API.cpp
	PhysicsClientSharedMemory2_C_API.h
	PhysicsClientSharedMemory2.cpp
	PhysicsClientSharedMemory2.h
	SharedMemoryCommandProcessor.cpp
	SharedMemoryCommandProcessor.h
	PhysicsServerCommandProcessor.cpp
	PhysicsServerCommandProcessor.h
	plugins/tinyRendererPlugin/tinyRendererPlugin.cpp
	plugins/tinyRendererPlugin/TinyRendererVisualShapeConverter.cpp
SharedMemoryCommands.h
	SharedMemoryPublic.h
	b3PluginManager.cpp
	../TinyRenderer/geometry.cpp
	../TinyRenderer/model.cpp
	../TinyRenderer/tgaimage.cpp
	../TinyRenderer/our_gl.cpp
	../TinyRenderer/TinyRenderer.cpp
	../OpenGLWindow/SimpleCamera.cpp
	../OpenGLWindow/SimpleCamera.h
	../Importers/ImportURDFDemo/ConvertRigidBodies2MultiBody.h
	../Importers/ImportURDFDemo/MultiBodyCreationInterface.h
	../Importers/ImportURDFDemo/MyMultiBodyCreator.cpp
	../Importers/ImportURDFDemo/MyMultiBodyCreator.h
	../Importers/ImportURDFDemo/BulletUrdfImporter.cpp
	../Importers/ImportURDFDemo/BulletUrdfImporter.h
	../Importers/ImportURDFDemo/UrdfParser.cpp
 	../Importers/ImportURDFDemo/urdfStringSplit.cpp
	../Importers/ImportURDFDemo/UrdfParser.cpp
	../Importers/ImportURDFDemo/UrdfParser.h
	../Importers/ImportURDFDemo/URDF2Bullet.cpp
	../Importers/ImportURDFDemo/URDF2Bullet.h
	../Importers/ImportMJCFDemo/BulletMJCFImporter.cpp
	../Importers/ImportMJCFDemo/BulletMJCFImporter.h
	../Utils/b3ResourcePath.cpp
	../Utils/b3Clock.cpp
	../Utils/RobotLoggingUtil.cpp
	../Utils/RobotLoggingUtil.h
	../Utils/ChromeTraceUtil.cpp
	../Utils/ChromeTraceUtil.h
	../Importers/ImportURDFDemo/URDFImporterInterface.h
	../Importers/ImportURDFDemo/URDFJointTypes.h
	../Importers/ImportObjDemo/Wavefront2GLInstanceGraphicsShape.cpp
	../Importers/ImportObjDemo/LoadMeshFromObj.cpp
	../Importers/ImportSTLDemo/ImportSTLSetup.h
	../Importers/ImportSTLDemo/LoadMeshFromSTL.h
	../Importers/ImportColladaDemo/LoadMeshFromCollada.cpp
	../Importers/ImportColladaDemo/ColladaGraphicsInstance.h
	../ThirdPartyLibs/Wavefront/tiny_obj_loader.cpp
	../ThirdPartyLibs/tinyxml2/tinyxml2.cpp
	../Importers/ImportMeshUtility/b3ImportMeshUtility.cpp
	../MultiThreading/b3ThreadSupportInterface.cpp
	../MultiThreading/b3ThreadSupportInterface.h
)

INCLUDE_DIRECTORIES(
	${BULLET_PHYSICS_SOURCE_DIR}/src
	${BULLET_PHYSICS_SOURCE_DIR}/examples/ThirdPartyLibs
)

IF (USE_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD)
	LINK_LIBRARIES(BulletSoftBody)
ENDIF()

LINK_LIBRARIES(stb)

LINK_LIBRARIES(
 Bullet3Common BulletWorldImporter BulletFileLoader BulletInverseDynamicsUtils BulletInverseDynamics BulletDynamics BulletCollision LinearMath BussIK
)


IF (WIN32)
        ADD_EXECUTABLE(App_PhysicsServer_SharedMemory
                ${SharedMemory_SRCS}
                main.cpp
                ../MultiThreading/b3Win32ThreadSupport.cpp
	              ../MultiThreading/b3Win32ThreadSupport.h
                ${BULLET_PHYSICS_SOURCE_DIR}/build3/bullet.rc
        )
ELSE(WIN32)
	FIND_PACKAGE(Threads)
	LINK_LIBRARIES( ${CMAKE_THREAD_LIBS_INIT} ${DL} )
	IF(APPLE)
			        ADD_EXECUTABLE(App_PhysicsServer_SharedMemory
			                ${SharedMemory_SRCS}
			                ../MultiThreading/b3PosixThreadSupport.cpp
	               			../MultiThreading/b3PosixThreadSupport.h
			                main.cpp
			        )

	ELSE(APPLE)
			        ADD_EXECUTABLE(App_PhysicsServer_SharedMemory
			                ${SharedMemory_SRCS}
			                ../MultiThreading/b3PosixThreadSupport.cpp
	               			../MultiThreading/b3PosixThreadSupport.h
			                main.cpp
			        )
        ENDIF(APPLE)
ENDIF(WIN32)




IF (INTERNAL_ADD_POSTFIX_EXECUTABLE_NAMES)
                        SET_TARGET_PROPERTIES(App_PhysicsServer_SharedMemory PROPERTIES  DEBUG_POSTFIX "_Debug")
                        SET_TARGET_PROPERTIES(App_PhysicsServer_SharedMemory PROPERTIES  MINSIZEREL_POSTFIX "_MinsizeRel")
                        SET_TARGET_PROPERTIES(App_PhysicsServer_SharedMemory PROPERTIES  RELWITHDEBINFO_POSTFIX "_RelWithDebugInfo")
ENDIF(INTERNAL_ADD_POSTFIX_EXECUTABLE_NAMES)



INCLUDE_DIRECTORIES(
	${BULLET_PHYSICS_SOURCE_DIR}/src
	${BULLET_PHYSICS_SOURCE_DIR}/examples/ThirdPartyLibs
	${BULLET_PHYSICS_SOURCE_DIR}/examples/ThirdPartyLibs/glad
)

ADD_DEFINITIONS(-DB3_USE_STANDALONE_EXAMPLE)

LINK_LIBRARIES(
	BulletSoftBody Bullet3Common BulletWorldImporter BulletInverseDynamicsUtils BulletInverseDynamics BulletDynamics BulletCollision LinearMath BussIK OpenGLWindow
)



IF (WIN32)
 				ADD_DEFINITIONS(-DGLEW_STATIC)
				LINK_LIBRARIES( ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} )

        ADD_EXECUTABLE(App_PhysicsServer_SharedMemory_GUI
                ${SharedMemory_SRCS}
								../StandaloneMain/main_opengl_single_example.cpp
								../ExampleBrowser/OpenGLGuiHelper.cpp
								../ExampleBrowser/GL_ShapeDrawer.cpp
								../ExampleBrowser/CollisionShape2TriangleMesh.cpp
								../MultiThreading/b3Win32ThreadSupport.cpp
								../MultiThreading/b3Win32ThreadSupport.h
			          ${BULLET_PHYSICS_SOURCE_DIR}/build3/bullet.rc
        )
ELSE(WIN32)
	FIND_PACKAGE(Threads)
	LINK_LIBRARIES( ${CMAKE_THREAD_LIBS_INIT} ${DL} )
			 IF(APPLE)
			        FIND_LIBRARY(COCOA NAMES Cocoa)
              MESSAGE(${COCOA})
              LINK_LIBRARIES(${COCOA} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

			        ADD_EXECUTABLE(App_PhysicsServer_SharedMemory_GUI
			                ${SharedMemory_SRCS}
	               			../StandaloneMain/main_opengl_single_example.cpp
											../ExampleBrowser/OpenGLGuiHelper.cpp
											../ExampleBrowser/GL_ShapeDrawer.cpp
											../ExampleBrowser/CollisionShape2TriangleMesh.cpp
			                ../MultiThreading/b3PosixThreadSupport.cpp
	               			../MultiThreading/b3PosixThreadSupport.h
			        )

        ELSE(APPLE)
         			ADD_DEFINITIONS("-DGLEW_INIT_OPENGL11_FUNCTIONS=1")
              ADD_DEFINITIONS("-DGLEW_STATIC")
              ADD_DEFINITIONS("-DGLEW_DYNAMIC_LOAD_ALL_GLX_FUNCTIONS=1")

			        ADD_EXECUTABLE(App_PhysicsServer_SharedMemory_GUI
			                ${SharedMemory_SRCS}
	              			../StandaloneMain/main_opengl_single_example.cpp
											../ExampleBrowser/OpenGLGuiHelper.cpp
											../ExampleBrowser/GL_ShapeDrawer.cpp
											../ExampleBrowser/CollisionShape2TriangleMesh.cpp
			                ../MultiThreading/b3PosixThreadSupport.cpp
	               			../MultiThreading/b3PosixThreadSupport.h
			        )
        ENDIF(APPLE)
ENDIF(WIN32)




IF (INTERNAL_ADD_POSTFIX_EXECUTABLE_NAMES)
                        SET_TARGET_PROPERTIES(App_PhysicsServer_SharedMemory_GUI PROPERTIES  DEBUG_POSTFIX "_Debug")
                        SET_TARGET_PROPERTIES(App_PhysicsServer_SharedMemory_GUI PROPERTIES  MINSIZEREL_POSTFIX "_MinsizeRel")
                        SET_TARGET_PROPERTIES(App_PhysicsServer_SharedMemory_GUI PROPERTIES  RELWITHDEBINFO_POSTFIX "_RelWithDebugInfo")
ENDIF(INTERNAL_ADD_POSTFIX_EXECUTABLE_NAMES)