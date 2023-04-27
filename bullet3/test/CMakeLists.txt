IF(BUILD_BULLET3)
	SUBDIRS(  InverseDynamics SharedMemory )
ENDIF(BUILD_BULLET3)

SUBDIRS( collision BulletDynamics )

