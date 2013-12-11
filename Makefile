# ---------------------------------------------------------------------------
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ---------------------------------------------------------------------------

UNAME = $(shell uname -s)

DEBUG=no
VERBOSE_WRITEUNITEXFILE_ERROR=1

.PHONY: annotator clean_annotator

ifeq ($(UNAME),Darwin)
#annotator: .make_banner make_unitex all prepare_deploy
annotator: .make_banner maven_dependencies all prepare_deploy
	install_name_tool -change "/install/lib/libuima.0.dylib" \
	"$(UIMACPP_HOME)/install/lib/libuima.dylib" \
	UnitexAnnotatorCpp.dylib
else
annotator: .make_banner maven_dependencies all prepare_deploy
endif

clean_annotator: .clean_banner clean_maven clean
	
MACOS = no
DLL_SUFFIX = dll
SYSTEM = windows
ifeq ($(UNAME),Linux)
	DLL_SUFFIX = so
	SYSTEM = linux-like	
else
	ifeq ($(UNAME),Darwin)
		DLL_SUFFIX = dylib
		SYSTEM = linux-like
		MACOS = yes
	endif
endif
ANNOTATOR_EXECUTABLE=UnitexAnnotatorCpp.$(DLL_SUFFIX)
ifneq (,$(findstring MINGW,$(UNAME)))
	MINGW = 1
	SYSTEM = mingw32
	COMMANDPREFIXDEFINED = yes
	COMMANDPREFIX = mingw32-
	BOOST_HOME = $(BOOST_ROOT)/stage
endif
ifneq (,$(findstring CYGWIN,$(UNAME)))
	SYSTEM = cygwin
endif

#############################################################################
#                                                                           #
#                         UNITEX core definitions                           #
#                                                                           #
#############################################################################

UNITEX_SRC = ./Release/gramlab-unitexjni/Unitex-C++
UNITEX_BUILD = $(UNITEX_SRC)/build
UNITEX_BIN = $(UNITEX_SRC)/bin

64BITS = no

ifeq ($(UNAME),Darwin)
64BITS = yes
SYSTEM = linux-like
endif

ifeq ($(ICU_HOME),)
	ICU_HOME=$(UIMACPP_HOME)
endif

#############################################################################
#                                                                           #
#                          UnitexAnnotatorCpp                               #
#                                                                           #
#############################################################################

APR_INCLUDE = $(UIMACPP_HOME)/include/apr

ifeq ($(JAVA_INCLUDE),)
ifeq ($(MACOS),yes)
	JAVA_INCLUDE = /System/Library/Frameworks/JavaVM.framework/Headers
else
	JAVA_INCLUDE = $(JAVA_HOME)/include \
	               -I$(JAVA_HOME)/include/linux
endif
endif

ifeq ($(BOOST_HOME),)
	BOOST_INCLUDE=/lib/include
else
	BOOST_INCLUDE=$(BOOST_HOME)/include
endif

INCLUDE_DIRS =  -I/usr/include \
          	-I$(BOOST_INCLUDE) \
		-I$(ICU_HOME)/include \
		-I./Release/gramlab-unitexjni \
          	-I$(UNITEX_SRC) \
         	-I$(UNITEX_SRC)/logger \
         	-I$(UNITEX_SRC)/UnitexLibAndJni \
         	-I$(UIMACPP_HOME)/include \
         	-I$(APR_INCLUDE) \
         	-I$(JAVA_INCLUDE)
 		   
# name of the annotator to be created
TARGET_FILE = UnitexAnnotatorCpp

# list of object files concerning Unitex commands to be linked when building the annotator 
OBJ_COMMANDS = UnitexCommand.o \
	   		   CheckDicCommand.o \
	   		   CompressCommand.o \
	   		   Fst2CheckCommand.o \
		       Grf2Fst2Command.o \
		       FlattenCommand.o \
		       NormalizeCommand.o \
		       Fst2TxtCommand.o \
		       TokenizeCommand.o \
		       ConcordCommand.o \
		       LocateCommand.o \
		       DicoCommand.o

# list of object files concerning Unitex types to be linked when building the annotator 
OBJ_TYPES = JavaLikeEnum.o UnitexTypes.o

# list of object files concerning Unitex engine and sub-engines to be linked when building the annotator 
OBJ_ENGINES =   UnitexSubengine.o \
              	DictionaryCompiler.o \
	      	GraphCompiler.o \
          	TextPreprocessor.o \
	        ProfilingLogger.o \
		TextProcessor.o \
		QualifiedString.o \
		UnitexEngine.o 

# list of object files concerning annotations to be linked when building the annotator 
OBJ_ANNOTATIONS = AnnotationWrapper.o \
	 	  UnitexDocumentParameters.o \
		  LanguageArea.o \
                  TextAreaAnnotation.o \
                  ContextAreaAnnotation.o \
                  ParagraphAnnotation.o \
                  SentenceAnnotation.o \
                  TokenAnnotation.o \
                  TransductionOutputAnnotation.o \
                  AnnotatorPerformanceAnnotation.o \
                  AutomatonLocatePerformanceAnnotation.o

# list of object files concerning tokenization to be linked when building the annotator 
OBJ_TOKENIZATION = UnitexTokenizer.o \
		   TextArea.o \
		   UnitexOutputOffsetConverter.o 

# list of user's object files to be linked when building the annotator
OBJS = $(OBJ_TYPES) \
	   UnitexException.o \
	   Language.o \
	   LanguageResources.o \
	   Utils.o FileUtils.o \
	   VirtualFolderCleaner.o \
	   UnitexLogInstaller.o \
	   $(OBJ_COMMANDS) \
	   $(OBJ_ENGINES) \
	   $(OBJ_ANNOTATIONS) \
	   $(OBJ_TOKENIZATION) \
	   UnitexAnnotatorCpp.o

LIBS = 

#############################################################################
#                                                                           #
#                                LINKING                                    #
#                                                                           #
#############################################################################

LIB_FOLDER = ./Release
UIMA_LIB_FOLDER = $(LIB_FOLDER)/uima-cpp/lib
ifeq ($(UNITEX_LIB_FOLDER),)
	UNITEX_LIB_FOLDER = $(LIB_FOLDER)/gramlab-unitexjni
endif
BOOST_LIB_FOLDER = $(BOOST_HOME)/lib

ICU_LINKFLAGS = -L$(ICU_HOME)/lib -licuuc -licuio -licui18n -licudata
ifeq ($(MACOS),yes)
	DYNAMIC_LINK_FLAG =
	STATIC_LINK_FLAG =
	UNITEX_LINKFLAGS = $(UNITEX_SRC)/bin/libunitex.a $(UNITEX_BUILD)/libtre/lib/libtre.a
	BOOST_LINKFLAGS = $(BOOST_HOME)/lib/libboost_filesystem-mt.a $(BOOST_HOME)/lib/libboost_system-mt.a $(BOOST_HOME)/lib/libboost_date_time-mt.a $(BOOST_HOME)/lib/libboost_thread-mt.a
else
	DYNAMIC_LINK_FLAG = -Wl,-Bdynamic
	STATIC_LINK_FLAG = -Wl,-Bstatic
	UNITEX_LINKFLAGS = -L$(UNITEX_LIB_FOLDER) -lUnitexJni
	BOOST_LINKFLAGS = $(STATIC_LINK_FLAG) -L$(BOOST_LIB_FOLDER) -lboost_date_time-mt -lboost_thread-mt -lboost_filesystem-mt -lboost_system-mt 
endif

USER_LINKFLAGS = -Wl,-z,defs -Wl,-t -lxerces-c $(ICU_LINKFLAGS) $(UNITEX_LINKFLAGS) $(BOOST_LINKFLAGS) $(DYNAMIC_LINK_FLAG) -ldl -lpthread -lrt

# Set DEBUG=1 for a debug build (if not 1 a ship build will result)
DEBUG = 0

ifeq ($(DEBUG),1)
	USER_CFLAGS = -O0 -ggdb3 -W -Wall
else
	USER_CFLAGS = -O3 -W -Wunused-parameter
endif

# Set DLL_BUILD=1 to build an annotator (shared library)
#    if not 1 an executable binary will be built
DLL_BUILD = 1

USER_CFLAGS += $(INCLUDE_DIRS) -fno-inline 

# include file with generic compiler instructions
include $(UIMACPP_HOME)/lib/base.mak

.PHONY: .make_banner .clean_banner
	
.make_banner:
	@echo "#############################################################################"
	@echo "Compiling UnitexAnnotatorCpp for $(UNAME) - DEBUG=$(DEBUG)..."
	@echo "#############################################################################"
	@echo JAVA_INCLUDE=$(JAVA_INCLUDE)
	@echo BOOST_HOME=$(BOOST_HOME)
	@echo DEBUG=$(DEBUG)

.clean_banner:
	@echo "#############################################################################"
	@echo "Cleaning UnitexAnnotatorCpp for $(UNAME)..."
	@echo "#############################################################################"
	
#############################################################################
#                                                                           #
#                           UNITEX core targets                             #
#                                                                           #
#############################################################################

#.PHONY: make_unitex clean_unitex

#make_unitex:
#	@echo "Making Unitex under $(SYSTEM)"
#	make -C $(UNITEX_BUILD) SYSTEM=$(SYSTEM) 64BITS=$(64BITS) DEBUG=$(DEBUG) STATICLIB=yes
	
#clean_unitex:
#	make -C $(UNITEX_BUILD) clean

#############################################################################
#                                                                           #
#                                 MAVEN                                     #
#                                                                           #
#############################################################################

.PHONY: clean_maven prepare_deploy maven_deploy

clean_maven:
	@echo "Clean MAVEN deployment directory: Release"
	@rm -rf Release
	
maven_dependencies:
	@echo "Getting MAVEN dependencies"
	@mvn dependency:unpack

prepare_deploy:
	@echo "Preparing for MAVEN deployment"
	@mkdir -p Release
	@echo "Copying Annotator executable into Release"
	@cp $(ANNOTATOR_EXECUTABLE) Release
	@echo "Copying dependencies under Release"
	@./copydep.sh Release
	
maven_deploy: annotator prepare_deploy
	@echo "Deploying MAVEN artifact"
	@mvn deploy
	
