/*
 * Copyright 2005 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 */
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "jni.h"
#include "jni_util.h"

#include "sun_tools_attach_WindowsAttachProvider.h"

/*
 * Class:     sun_tools_attach_WindowsAttachProvider
 * Method:    tempPath
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_sun_tools_attach_WindowsAttachProvider_tempPath(JNIEnv *env, jclass cls)
{
    char buf[256];
    DWORD bufLen, actualLen;
    jstring result = NULL;

    bufLen = sizeof(buf) / sizeof(char);
    actualLen = GetTempPath(bufLen, buf);
    if (actualLen > 0) {
        char* bufP = buf;
        if (actualLen > bufLen) {
            actualLen += sizeof(char);
            bufP = (char*)malloc(actualLen * sizeof(char));
            actualLen = GetTempPath(actualLen, bufP);
        }
        if (actualLen > 0) {
            result = JNU_NewStringPlatform(env, bufP);
        }
        if (bufP != buf) {
            free((void*)bufP);
        }
    }
    return result;
}

/*
 * Class:     sun_tools_attach_WindowsAttachProvider
 * Method:    volumeFlags
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL
Java_sun_tools_attach_WindowsAttachProvider_volumeFlags(JNIEnv *env, jclass cls, jstring str)
{
    jboolean isCopy;
    const char* volume;
    DWORD result = 0;

    volume = JNU_GetStringPlatformChars(env, str, &isCopy);
    if (volume != NULL) {
        DWORD componentLen, flags;
        BOOL res = GetVolumeInformation(volume,
                                        NULL,
                                        0,
                                        NULL,
                                        &componentLen,
                                        &flags,
                                        NULL,
                                        0);
       if (res != 0) {
           result = flags;
       }
       if (isCopy) {
            JNU_ReleaseStringPlatformChars(env, str, volume);
       }
    }
    return result;
}


/*
 * Process status helper library functions
 */
static BOOL  (WINAPI *_EnumProcesses)     (DWORD *, DWORD, DWORD *);
static BOOL  (WINAPI *_EnumProcessModules)(HANDLE, HMODULE *, DWORD, LPDWORD);
static DWORD (WINAPI *_GetModuleBaseName) (HANDLE, HMODULE, LPTSTR, DWORD);


/*
 * Class:     sun_tools_attach_WindowsAttachProvider
 * Method:    initializeProcessStatusHelper
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_tools_attach_WindowsAttachProvider_initializeProcessStatusHelper(JNIEnv *env, jclass cls)
{
    HINSTANCE psapi = LoadLibrary("PSAPI.DLL") ;
    if (psapi != NULL) {
        _EnumProcesses = (BOOL(WINAPI *)(DWORD *, DWORD, DWORD *))
            GetProcAddress(psapi, "EnumProcesses") ;
        _EnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *, DWORD, LPDWORD))
            GetProcAddress(psapi, "EnumProcessModules");
        _GetModuleBaseName = (DWORD(WINAPI *)(HANDLE, HMODULE, LPTSTR, DWORD))
            GetProcAddress(psapi, "GetModuleBaseNameA");
    }

    if ((_EnumProcesses == NULL) ||
        (_EnumProcessModules == NULL) ||
        (_GetModuleBaseName == NULL))
    {
        JNU_ThrowInternalError(env, "Unable to initialize process status helper library");
    }
}


/*
 * Class:     sun_tools_attach_WindowsAttachProvider
 * Method:    enumProcesses
 * Signature: ([JI)I
 */
JNIEXPORT jint JNICALL
Java_sun_tools_attach_WindowsAttachProvider_enumProcesses(JNIEnv *env, jclass cls,
                                                          jintArray arr, jint max)
{
    DWORD size, bytesReturned;
    DWORD* ptr;
    jint result = 0;

    size = max * sizeof(DWORD);
    ptr = (DWORD*)malloc(size);
    if (ptr != NULL) {
        BOOL res = (*_EnumProcesses)(ptr, size, &bytesReturned);
        if (res != 0) {
            result = (jint)(bytesReturned / sizeof(DWORD));
            (*env)->SetIntArrayRegion(env, arr, 0, (jsize)result, (jint*)ptr);
        }
        free((void*)ptr);
    }
    return result;
}

/*
 * Class:     sun_tools_attach_WindowsAttachProvider
 * Method:    isLibraryLoadedByProcess
 * Signature: (I[Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_tools_attach_WindowsAttachProvider_isLibraryLoadedByProcess(JNIEnv *env, jclass cls,
                                                                     jstring str, jint processId)
{
    HANDLE hProcess;
    jboolean isCopy;
    const char* lib;
    DWORD size, bytesReturned;
    HMODULE* ptr;
    jboolean result = JNI_FALSE;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                           PROCESS_VM_READ,
                           FALSE, (DWORD)processId);
    if (hProcess == NULL) {
        return JNI_FALSE;
    }
    lib = JNU_GetStringPlatformChars(env, str, &isCopy);
    if (lib == NULL) {
        CloseHandle(hProcess);
        return JNI_FALSE;
    }

    /*
     * Enumerate the modules that the process has opened and see if we have a
     * match.
     */
    size = 1024 * sizeof(HMODULE);
    ptr = (HMODULE*)malloc(size);
    if (ptr != NULL) {
        BOOL res = (*_EnumProcessModules)(hProcess, ptr, size, &bytesReturned);
        if (res != 0) {
            int count = bytesReturned / sizeof(HMODULE);
            int i = 0;
            while (i < count) {
                char base[256];
                BOOL res = (*_GetModuleBaseName)(hProcess, ptr[i], base, sizeof(base));
                if (res != 0) {
                    if (strcmp(base, lib) == 0) {
                      result = JNI_TRUE;
                      break;
                    }
                }
                i++;
            }
        }
        free((void*)ptr);
    }
    if (isCopy) {
        JNU_ReleaseStringPlatformChars(env, str, lib);
    }
    CloseHandle(hProcess);

    return result;
}
