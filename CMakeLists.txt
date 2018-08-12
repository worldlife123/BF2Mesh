#CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT(BF2Mesh)

SET(SRC_LIST main.cpp
	bf2mesh.h
	bf2mesh.cpp
	bf2conParser.h
	bf2conParser.cpp
	bf2Object.h
	bf2Object.cpp
)

ADD_EXECUTABLE(BF2Mesh ${SRC_LIST})