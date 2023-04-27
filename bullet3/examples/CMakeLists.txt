SUBDIRS( HelloWorld BasicDemo)
IF(BUILD_BULLET3)
	SUBDIRS( ExampleBrowser SharedMemory ThirdPartyLibs/Gwen ThirdPartyLibs/BussIK OpenGLWindow TwoJoint )
ENDIF()
