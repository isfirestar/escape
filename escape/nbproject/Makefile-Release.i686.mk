#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=None-Linux
CND_DLIB_EXT=so
CND_CONF=Release.i686
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/bfc7bc46/encrypt.o \
	${OBJECTDIR}/_ext/bfc7bc46/endpoint.o \
	${OBJECTDIR}/_ext/bfc7bc46/log.o \
	${OBJECTDIR}/_ext/bfc7bc46/network_handler.o \
	${OBJECTDIR}/_ext/bfc7bc46/os_util.o \
	${OBJECTDIR}/_ext/bfc7bc46/shmem.o \
	${OBJECTDIR}/_ext/bfc7bc46/swnet.o \
	${OBJECTDIR}/_ext/bfc7bc46/toolkit.o \
	${OBJECTDIR}/_ext/5c0/args.o \
	${OBJECTDIR}/_ext/5c0/entry.o \
	${OBJECTDIR}/_ext/5c0/session.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m32
CXXFLAGS=-m32

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/escape

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/escape: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/escape ${OBJECTFILES} ${LDLIBSOPTIONS} /usr/local/lib/nshost.so -lpthread -ldl -lrt -Wl,-rpath=/usr/local/lib

${OBJECTDIR}/_ext/bfc7bc46/encrypt.o: ../../libnsp/encrypt.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bfc7bc46
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bfc7bc46/encrypt.o ../../libnsp/encrypt.cpp

${OBJECTDIR}/_ext/bfc7bc46/endpoint.o: ../../libnsp/endpoint.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bfc7bc46
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bfc7bc46/endpoint.o ../../libnsp/endpoint.cpp

${OBJECTDIR}/_ext/bfc7bc46/log.o: ../../libnsp/log.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bfc7bc46
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bfc7bc46/log.o ../../libnsp/log.cpp

${OBJECTDIR}/_ext/bfc7bc46/network_handler.o: ../../libnsp/network_handler.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bfc7bc46
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bfc7bc46/network_handler.o ../../libnsp/network_handler.cpp

${OBJECTDIR}/_ext/bfc7bc46/os_util.o: ../../libnsp/os_util.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bfc7bc46
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bfc7bc46/os_util.o ../../libnsp/os_util.cpp

${OBJECTDIR}/_ext/bfc7bc46/shmem.o: ../../libnsp/shmem.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bfc7bc46
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bfc7bc46/shmem.o ../../libnsp/shmem.cpp

${OBJECTDIR}/_ext/bfc7bc46/swnet.o: ../../libnsp/swnet.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bfc7bc46
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bfc7bc46/swnet.o ../../libnsp/swnet.cpp

${OBJECTDIR}/_ext/bfc7bc46/toolkit.o: ../../libnsp/toolkit.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bfc7bc46
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bfc7bc46/toolkit.o ../../libnsp/toolkit.cpp

${OBJECTDIR}/_ext/5c0/args.o: ../args.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/args.o ../args.cpp

${OBJECTDIR}/_ext/5c0/entry.o: ../entry.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/entry.o ../entry.cpp

${OBJECTDIR}/_ext/5c0/session.o: ../session.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -I../../libnsp -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/session.o ../session.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
