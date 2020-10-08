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
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/b0c5a78b/CExplode.o \
	${OBJECTDIR}/_ext/b0c5a78b/Crc32.o \
	${OBJECTDIR}/_ext/5c0/CMindsetConfig.o \
	${OBJECTDIR}/_ext/5c0/Network.o \
	${OBJECTDIR}/_ext/5c0/Neuron.o \
	${OBJECTDIR}/_ext/5c0/Text.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lexpat

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nnsolvetest

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nnsolvetest: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nnsolvetest ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/b0c5a78b/CExplode.o: ../../common/CExplode.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b0c5a78b
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../common/expat -I../../common -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b0c5a78b/CExplode.o ../../common/CExplode.cpp

${OBJECTDIR}/_ext/b0c5a78b/Crc32.o: ../../common/Crc32.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/b0c5a78b
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../common/expat -I../../common -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b0c5a78b/Crc32.o ../../common/Crc32.cpp

${OBJECTDIR}/_ext/5c0/CMindsetConfig.o: ../CMindsetConfig.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../common/expat -I../../common -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/CMindsetConfig.o ../CMindsetConfig.cpp

${OBJECTDIR}/_ext/5c0/Network.o: ../Network.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../common/expat -I../../common -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/Network.o ../Network.cpp

${OBJECTDIR}/_ext/5c0/Neuron.o: ../Neuron.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../common/expat -I../../common -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/Neuron.o ../Neuron.cpp

${OBJECTDIR}/_ext/5c0/Text.o: ../Text.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/5c0
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../common/expat -I../../common -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/5c0/Text.o ../Text.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../../common/expat -I../../common -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/nnsolvetest

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
