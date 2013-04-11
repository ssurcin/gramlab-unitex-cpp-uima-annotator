#include "UnitexJNI.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UnitexTool.h"
#include "SyncLogger.h"

#include "AbstractFilePlugCallback.h"
#include "UserCancellingPlugCallback.h"
#include "UniLogger.h"
#include "UniRunLogger.h"

#include "UnitexLibIO.h"

#if defined(UNITEX_HAVING_PERSISTANCE_INTERFACE) && (!(defined(UNITEX_PREVENT_USING_PERSISTANCE_INTERFACE)))
#include "PersistenceInterface.h"
#endif

#include "Copyright.h"

#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif

#ifdef HAS_LOGGER_NAMESPACE
using namespace logger;
#endif

#if (!defined(SVN_REVISION))
#include "Unitex_revision.h"
#define SVN_REVISION UNITEX_REVISION
#endif

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#if ((defined(WIN32) || defined(_WIN32) || defined (_WIN64) || defined (_M_IX86)  || \
	defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__) || \
	defined(_M_X64) || defined(_M_X86) || defined(TARGET_CPU_X86) || defined(TARGET_CPU_X86_64) || \
	defined(__arm__) || defined(_ARM_) || defined(__CC_ARM) || defined(_M_ARM) || defined(_M_ARMT) || \
	defined(__LITTLE_ENDIAN__) \
	) && (!(defined(INTEL_X86_LIKE_LITTLE_ENDIAN))))
#define INTEL_X86_LIKE_LITTLE_ENDIAN 1
#endif

#ifdef INTEL_X86_LIKE_LITTLE_ENDIAN
static bool is_little_endian()
{
	// printf("INTEL_X86_LIKE_LITTLE_ENDIAN is,");
	return true;
}
#else
static bool is_little_endian()
{
	const jchar i=1;
	const char *c=(const char*)&i;
	bool little_endian = ((*c) != 0);
	// printf("is_little_endian = %s\n",little_endian ? "y":"n");
	return little_endian;
}
#endif


///////////////////////////////////////////////////////////////////////////////
// 
// Helper class jstringToCUtf to convert Java string to C-like UTF string
//
///////////////////////////////////////////////////////////////////////////////

class jstringToCUtf
{
public:
	jstringToCUtf();
	~jstringToCUtf();
	const char* initJString(JNIEnv *env,jstring jstr);
	const char* getJString() ;
	void clear();
private:
	JNIEnv*env;
	jstring jstr;
	const char*c_str;
};

jstringToCUtf::jstringToCUtf() :
env(NULL),jstr(NULL),c_str(NULL)
{
}

const char* jstringToCUtf::initJString(JNIEnv *envSet,jstring jstrSet)
{
	clear();
	env=envSet;
	jstr=jstrSet;
	c_str = env->GetStringUTFChars(jstr, 0);
	return c_str;
}

const char* jstringToCUtf::getJString()
{
	return c_str;
}

void jstringToCUtf::clear()
{
	if (c_str != NULL)
		env->ReleaseStringUTFChars(jstr, c_str);
	c_str = NULL;
}

jstringToCUtf::~jstringToCUtf()
{
	clear();
}

///////////////////////////////////////////////////////////////////////////////
//
// Helper functions to extract arguments to/from jstring
//
///////////////////////////////////////////////////////////////////////////////

static int countArgs(char** args)
{
	int count=0;

	char**tmp=args;
	while ((*tmp) != NULL)
	{
		count++;
		tmp++;
	}
	return count;
}

static void freeArgs(char**args)
{
	char** tmp=args;
	while ((*tmp) != NULL)
	{
		free(*tmp);
		tmp++;
	}
	free(args);
}


static const char* CopyStrArg(const char*lpSrc,char* lpDest,size_t dwMaxSize)
{
	int isInQuote=0;
	size_t dwDestPos = 0;

	*lpDest = '\0';

	if (lpSrc==NULL)
		return NULL;

	while ((*lpSrc) == ' ')
		lpSrc++;

	while (((*lpSrc) != '\0') && ((dwMaxSize==0) || (dwDestPos<dwMaxSize)))
	{
		if ((*lpSrc) == '"')
		{
			isInQuote = !isInQuote;
			lpSrc++;
			continue;
		}

		if (((*lpSrc) == ' ') && (!isInQuote))
		{
			while ((*lpSrc) == ' ')
				lpSrc++;
			return lpSrc;
		}

		*lpDest = *lpSrc;
		lpDest++;
		dwDestPos++;
		*lpDest = '\0';
		lpSrc++;
	}
	return lpSrc;
}


static char**argsFromStrArray(JNIEnv* jenv, jobjectArray strArray)
{
	int nSize = jenv->GetArrayLength(strArray);
	char **args=NULL;	
	if (nSize>0)
		args = (char**)malloc((nSize+1) * sizeof(char*));

	if (args != NULL)
	{
		int nbArgs = nSize;
		int i;
		for (i = 0; i < nbArgs; i++) {
			jstring jstr = (jstring)jenv->GetObjectArrayElement(strArray, i);
			const char* sz = jenv->GetStringUTFChars(jstr, NULL);

			// we can replace these line by mpszArgs[i] = strdup(sz) to not remove quote
			size_t string_alloc_size = strlen(sz)+4;
			args[i] = (char*)malloc(string_alloc_size+4);
			CopyStrArg(sz,args[i],string_alloc_size);

			jenv->ReleaseStringUTFChars(jstr, sz);
			/* explicitly releasing to assist garbage collection, though not required */
			jenv->DeleteLocalRef(jstr);
		}
		args[i] = NULL;
	}
	return args;
}


static char**argsFromJString(JNIEnv* jenv, jstring jstr)
{
	jstringToCUtf jtcu;

	const char*cmdLine=jtcu.initJString(jenv,jstr);
	size_t len_cmd_line=strlen(cmdLine);

	char* work_buf = (char*)malloc(len_cmd_line+0x10);
	if (work_buf==NULL)
		return 0;

	const char*lpParcLine;
	int nb_args_found=0;

	lpParcLine = cmdLine;
	while ((*lpParcLine) != '\0')
	{
		*work_buf=0;
		lpParcLine = CopyStrArg(lpParcLine, work_buf, len_cmd_line+8);
		nb_args_found ++;
	}

	char **args=NULL;
	if (nb_args_found>0)
		args = (char**)malloc((nb_args_found+1) * sizeof(char*));
	else
		args = NULL;

	if (args == NULL)
	{
		free(work_buf);
		return NULL;
	}

	lpParcLine = cmdLine;
	int i=0;
	while ((*lpParcLine) != '\0')
	{
		*work_buf=0;
		lpParcLine = CopyStrArg(lpParcLine, work_buf, len_cmd_line+8);
		args[i] = strdup(work_buf);
		i++;
	}
	args[i] = NULL;

	free(work_buf);
	return args;
}

///////////////////////////////////////////////////////////////////////////////
//
// Implementation of UnitexJNI
//
///////////////////////////////////////////////////////////////////////////////

/*
* Class:     org_gramlab_kwaga_unitex_uima_unitex_UnitexJNI
* Method:    UnitexTool
* Signature: ([Ljava/lang/String;)I
*/
JNIEXPORT jint JNICALL Java_org_gramlab_kwaga_unitex_1uima_unitex_UnitexJNI_UnitexTool(JNIEnv* jenv, jclass, jobjectArray strArray)
{
	char** args = argsFromStrArray(jenv, strArray);
	jint retValue = (jint)main_UnitexTool_C(countArgs(args), args);
	freeArgs(args);
	return retValue;
}

/*
* Class:     org_gramlab_kwaga_unitex_uima_unitex_UnitexJNI
* Method:    setStdOutFile
* Signature: (Ljava/lang/String;Z)Z
*/
JNIEXPORT jboolean JNICALL Java_org_gramlab_kwaga_unitex_1uima_unitex_UnitexJNI_setStdOutFile(JNIEnv* jenv, jclass, jstring jDest, jboolean jbTrashOutput)
{
	enum stdwrite_kind swk = stdwrite_kind_out;
	jboolean res = (SetStdWriteCB(swk, jbTrashOutput, NULL, NULL) == 1);
	return res;
}

static struct UniLoggerSpace* p_ule = NULL;

/*
* Class:     org_gramlab_kwaga_unitex_uima_unitex_UnitexJNI
* Method:    installLogger
* Signature: (Ljava/lang/String;Z)J
*/
JNIEXPORT jlong JNICALL Java_org_gramlab_kwaga_unitex_1uima_unitex_UnitexJNI_installLogger(JNIEnv* env, jclass, jstring foldername, jboolean store_file_out_content)
{
	jstringToCUtf jstc_foldername;
	jstc_foldername.initJString(env,foldername);

	if (p_ule)
		return JNI_FALSE;

	p_ule= (struct UniLoggerSpace *)malloc(sizeof(struct UniLoggerSpace));
	if (!p_ule)
		return JNI_FALSE;
	memset(p_ule,0,sizeof(struct UniLoggerSpace));

	p_ule->size_of_struct = sizeof(struct UniLoggerSpace);
	p_ule->privateUnloggerData = NULL;
	p_ule->szPathLog = strdup(jstc_foldername.getJString());
	p_ule->szNameLog = NULL;
	p_ule->store_file_out_content = store_file_out_content;
	p_ule->store_list_file_out_content = 1;
	p_ule->store_file_in_content = 1;
	p_ule->store_list_file_in_content = 1;
	p_ule->store_std_out_content = 0;
	p_ule->store_std_err_content = 0;
	p_ule->auto_increment_logfilename = 1;

	if (0 != AddActivityLogger(p_ule))
		return JNI_TRUE;

	free(p_ule);
	return JNI_FALSE;
}

/*
* Class:     org_gramlab_kwaga_unitex_uima_unitex_UnitexJNI
* Method:    getUnitexRevision
* Signature: ()I
*/
JNIEXPORT jint JNICALL Java_org_gramlab_kwaga_unitex_1uima_unitex_UnitexJNI_getUnitexRevision(JNIEnv* jenv, jclass)
{
#ifdef SVN_REVISION
	return (jint)SVN_REVISION;
#else
	return (jint)-1;
#endif

}
