//
// Author:   Jonathan Blow
// Version:  1
// Date:     31 August, 2018
//
// This code is released under the MIT license, which you can find at
//
//          https://opensource.org/licenses/MIT
//
//
//
// See the comments for how to use this library just below the includes.
//


//
// NOTE(Kalinovcic): I have translated the original implementation to C,
// and made a preprocessor define that enables people to include it without
// the implementation, just as a header.
//
// I also fixed two bugs:
//   - If COM initialization for VS2017 fails, we actually properly continue
//     searching for earlier versions in the registry.
//   - For earlier versions, the code now returns the "bin\amd64" VS directory.
//     Previously it was returning the "bin" directory, which is for x86 stuff.
//
// To include the implementation, before including microsoft_craziness.h, define:
//
//   #define MICROSOFT_CRAZINESS_IMPLEMENTATION
//   #include "microsoft_craziness.h"
//


// NOTE(ActuallyaDeviloper): I fixed one real and one potential access violation.
// They were caused by incorrect size calculations around wide string allocations.
// I also added a new "is_vs_2017" variable to detect VS 2017 specifically because
// this MSVC version reports a different kind of path which can cause trouble later.
// And finally I fixed 3 compiler warnings.

#ifndef MICROSOFT_CRAZINESS_HEADER_GUARD
#define MICROSOFT_CRAZINESS_HEADER_GUARD

#include <wchar.h> // for wchar_t

typedef struct
{
    int windows_sdk_version;   // Zero if no Windows SDK found.

    wchar_t *windows_sdk_root;
    wchar_t *windows_sdk_um_library_path;
    wchar_t *windows_sdk_ucrt_library_path;

    int is_vs_2017;
    wchar_t *vs_exe_path;
    wchar_t *vs_library_path;

    char *windows_sdk_root_a;
    char *windows_sdk_um_library_path_a;
    char *windows_sdk_ucrt_library_path_a;

    char *vs_exe_path_a;
    char *vs_library_path_a;
} Find_Result;

Find_Result find_visual_studio_and_windows_sdk();
void free_resources(Find_Result *result);

#endif  // MICROSOFT_CRAZINESS_HEADER_GUARD


//
// HOW TO USE THIS CODE
//
// The purpose of this file is to find the folders that contain libraries
// you may need to link against, on Windows, if you are linking with any
// compiled C or C++ code. This will be necessary for many non-C++ programming
// language environments that want to provide compatibility.
//
// We find the place where the Visual Studio libraries live (for example,
// libvcruntime.lib), where the linker and compiler executables live
// (for example, link.exe), and where the Windows SDK libraries reside
// (kernel32.lib, libucrt.lib).
//
// We all wish you didn't have to worry about so many weird dependencies,
// but we don't really have a choice about this, sadly.
//
// I don't claim that this is the absolute best way to solve this problem,
// and so far we punt on things (if you have multiple versions of Visual Studio
// installed, we return the first one, rather than the newest). But it
// will solve the basic problem for you as simply as I know how to do it,
// and because there isn't too much code here, it's easy to modify and expand.
//
//
// Here is the API you need to know about:
//


//
// Call find_visual_studio_and_windows_sdk, look at the resulting
// paths, then call free_resources on the result.
//
// Everything else in this file is implementation details that you
// don't need to care about.
//

//
// This file was about 400 lines before we started adding these comments.
// You might think that's way too much code to do something as simple
// as finding a few library and executable paths. I agree. However,
// Microsoft's own solution to this problem, called "vswhere", is a
// mere EIGHT THOUSAND LINE PROGRAM, spread across 70 files,
// that they posted to github *unironically*.
//
// I am not making this up: https://github.com/Microsoft/vswhere
//
// Several people have therefore found the need to solve this problem
// themselves. We referred to some of these other solutions when
// figuring out what to do, most prominently ziglang's version,
// by Ryan Saunderson.
//
// I hate this kind of code. The fact that we have to do this at all
// is stupid, and the actual maneuvers we need to go through
// are just painful. If programming were like this all the time,
// I would quit.
//
// Because this is such an absurd waste of time, I felt it would be
// useful to package the code in an easily-reusable way, in the
// style of the stb libraries. We haven't gone as all-out as some
// of the stb libraries do (which compile in C with no includes, often).
// For this version you need C++ and the headers at the top of the file.
//
// We return the strings as Windows wide character strings. Aesthetically
// I don't like that (I think most sane programs are UTF-8 internally),
// but apparently, not all valid Windows file paths can even be converted
// correctly to UTF-8. So have fun with that. It felt safest and simplest
// to stay with wchar_t since all of this code is fully ensconced in
// Windows crazy-land.
//
// One other shortcut I took is that this is hardcoded to return the
// folders for x64 libraries. If you want x86 or arm, you can make
// slight edits to the code below, or, if enough people want this,
// I can work it in here.
//



#ifdef  MICROSOFT_CRAZINESS_IMPLEMENTATION
#ifndef MICROSOFT_CRAZINESS_IMPLEMENTATION_GUARD
#define MICROSOFT_CRAZINESS_IMPLEMENTATION_GUARD


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <io.h>         // For _get_osfhandle


#define INVALID_FILE_ATTRIBUTES  ((unsigned long)-1)
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define INVALID_HANDLE_VALUE     ((void *)(intptr_t)-1)
#define HKEY_LOCAL_MACHINE       (( HKEY ) (uintptr_t)((long)0x80000002) )
#define KEY_QUERY_VALUE          (0x0001)
#define KEY_WOW64_32KEY          (0x0200)
#define KEY_ENUMERATE_SUB_KEYS   (0x0008)
#define ERROR_FILE_NOT_FOUND     2L
#define REG_SZ                   ( 1ul )
#define CP_UTF8                  65001

#ifdef __cplusplus
#define REFIID const GUID &
#else
#define REFIID const GUID *
#endif

typedef struct _GUID GUID;
typedef struct _FILETIME *LPFILETIME;
typedef struct _WIN32_FIND_DATAW WIN32_FIND_DATAW;
typedef struct _FILETIME FILETIME;
typedef union _LARGE_INTEGER LARGE_INTEGER;
typedef struct HKEY__ *HKEY;
typedef struct IUnknown IUnknown;
typedef struct ISetupInstance ISetupInstance;
typedef struct IEnumSetupInstances IEnumSetupInstances;
typedef struct ISetupConfiguration ISetupConfiguration;

typedef struct
{
    unsigned long dwLowDateTime;
    unsigned long dwHighDateTime;
} filetime_s;

typedef struct
{
    unsigned long dwFileAttributes;
    filetime_s ftCreationTime;
    filetime_s ftLastAccessTime;
    filetime_s ftLastWriteTime;
    unsigned long nFileSizeHigh;
    unsigned long nFileSizeLow;
    unsigned long dwReserved0;
    unsigned long dwReserved1;
    wchar_t  cFileName[ 260 ];
    wchar_t  cAlternateFileName[ 14 ];
} win32_find_dataw_s;

typedef struct
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} guid_s;

typedef union
{
    struct
    {
        unsigned long LowPart;
        long HighPart;
    };
    struct
    {
        unsigned long LowPart;
        long HighPart;
    } u;
    long long QuadPart;
} large_integer_s;


#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllimport) unsigned long __stdcall GetFileAttributesW(const wchar_t *lpFileName);
__declspec(dllimport) int __stdcall GetFileSizeEx(void *hFile, LARGE_INTEGER *lpFileSize);

__declspec(dllimport) int __stdcall FindNextFileW(void *hFindFile, WIN32_FIND_DATAW *lpFindFileData);
__declspec(dllimport) int __stdcall FindClose(void *hFindFile);
__declspec(dllimport) void * __stdcall FindFirstFileW(const wchar_t *lpFileName, WIN32_FIND_DATAW *lpFindFileData);

__declspec(dllimport) long __stdcall RegQueryValueExW(HKEY hKey, const wchar_t *lpValueName, unsigned long *lpReserved, unsigned long *lpType, unsigned char *lpData, unsigned long *lpcbData);
__declspec(dllimport) long __stdcall RegOpenKeyExA(HKEY hKey, const char *lpSubKey, unsigned long ulOptions, unsigned long samDesired, HKEY *phkResult);
__declspec(dllimport) long __stdcall RegCloseKey(HKEY hKey);

__declspec(dllimport) long __stdcall CoInitialize(void *pvReserved);
__declspec(dllimport) long __stdcall CoCreateInstance(REFIID rclsid, IUnknown *pUnkOuter, unsigned long dwClsContext, REFIID riid, void **ppv);

__declspec(dllimport) void __stdcall SysFreeString(wchar_t *bstrString);

__declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int CodePage, unsigned long dwFlags, const wchar_t *lpWideCharStr, int cchWideChar, char *lpMultiByteStr, int cbMultiByte, const char *lpDefaultChar, int *lpUsedDefaultChar);

#ifdef __cplusplus
} // extern "C"
#endif


// COM objects for the ridiculous Microsoft craziness.

typedef struct ISetupInstanceVtbl
{
    // IUnknown methods
    long          (__stdcall *QueryInterface) (ISetupInstance *This, const GUID *, void **);
    unsigned long (__stdcall *AddRef)         (ISetupInstance *This);
    unsigned long (__stdcall *Release)        (ISetupInstance *This);

    // ISetupInstance methods
    long (__stdcall *GetInstanceId)          (ISetupInstance *This, wchar_t **pbstrInstanceId);
    long (__stdcall *GetInstallDate)         (ISetupInstance *This, LPFILETIME pInstallDate);
    long (__stdcall *GetInstallationName)    (ISetupInstance *This, wchar_t **pbstrInstallationName);
    long (__stdcall *GetInstallationPath)    (ISetupInstance *This, wchar_t **pbstrInstallationPath);
    long (__stdcall *GetInstallationVersion) (ISetupInstance *This, wchar_t **pbstrInstallationVersion);
    long (__stdcall *GetDisplayName)         (ISetupInstance *This, unsigned long lcid, wchar_t **pbstrDisplayName);
    long (__stdcall *GetDescription)         (ISetupInstance *This, unsigned long lcid, wchar_t **pbstrDescription);
    long (__stdcall *ResolvePath)            (ISetupInstance *This, const wchar_t *pwszRelativePath, wchar_t **pbstrAbsolutePath);
} ISetupInstanceVtbl;

typedef struct IEnumSetupInstancesVtbl
{
    // IUnknown methods
    long          (__stdcall *QueryInterface) (IEnumSetupInstances *This, const GUID *, void **);
    unsigned long (__stdcall *AddRef)         (IEnumSetupInstances *This);
    unsigned long (__stdcall *Release)        (IEnumSetupInstances *This);

    // IEnumSetupInstances methods
    long (__stdcall *Next)  (IEnumSetupInstances *This, unsigned long celt, ISetupInstance** rgelt, unsigned long* pceltFetched);
    long (__stdcall *Skip)  (IEnumSetupInstances *This, unsigned long celt);
    long (__stdcall *Reset) (IEnumSetupInstances *This);
    long (__stdcall *Clone) (IEnumSetupInstances *This, IEnumSetupInstances** ppenum);
} IEnumSetupInstancesVtbl;

typedef struct ISetupConfigurationVtbl
{
    // IUnknown methods
    long          (__stdcall *QueryInterface) (ISetupConfiguration *This, const GUID *, void **);
    unsigned long (__stdcall *AddRef)         (ISetupConfiguration *This);
    unsigned long (__stdcall *Release)        (ISetupConfiguration *This);

    // ISetupConfiguration methods
    long (__stdcall *EnumInstances)                (ISetupConfiguration *This, IEnumSetupInstances** ppEnumInstances);
    long (__stdcall *GetInstanceForCurrentProcess) (ISetupConfiguration *This, ISetupInstance** ppInstance);
    long (__stdcall *GetInstanceForPath)           (ISetupConfiguration *This, const wchar_t *wzPath, ISetupInstance** ppInstance);
} ISetupConfigurationVtbl;


struct ISetupInstance
{
    ISetupInstanceVtbl *lpVtbl;
};
struct IEnumSetupInstances
{
    IEnumSetupInstancesVtbl *lpVtbl;
};
struct ISetupConfiguration
{
    ISetupConfigurationVtbl *lpVtbl;
};



// The beginning of the actual code that does things.


void free_resources(Find_Result *result) {
    free(result->windows_sdk_root);
    free(result->windows_sdk_um_library_path);
    free(result->windows_sdk_ucrt_library_path);
    free(result->vs_exe_path);
    free(result->vs_library_path);

    free(result->windows_sdk_root_a);
    free(result->windows_sdk_um_library_path_a);
    free(result->windows_sdk_ucrt_library_path_a);
    free(result->vs_exe_path_a);
    free(result->vs_library_path_a);
}


typedef struct {
    int32_t best_version[4];  // For Windows 8 versions, only two of these numbers are used.
    wchar_t *best_name;
} Version_Data;

static bool os_file_exists(const wchar_t *name) {
    // @Robustness: What flags do we really want to check here?

    unsigned long attrib = GetFileAttributesW(name);
    if (attrib == INVALID_FILE_ATTRIBUTES) return false;
    if (attrib & FILE_ATTRIBUTE_DIRECTORY) return false;

    return true;
}

static wchar_t *concat(const wchar_t *a, const wchar_t *b, const wchar_t *c, const wchar_t *d) {
    // Concatenate up to 4 wide strings together. Allocated with malloc.
    // If you don't like that, use a programming language that actually
    // helps you with using custom allocators. Or just edit the code.

    size_t len_a = wcslen(a);
    size_t len_b = wcslen(b);

    size_t len_c = 0;
    if (c) len_c = wcslen(c);

    size_t len_d = 0;
    if (d) len_d = wcslen(d);

    wchar_t *result = (wchar_t *)malloc((len_a + len_b + len_c + len_d + 1) * 2);
    memcpy(result, a, len_a*2);
    memcpy(result + len_a, b, len_b*2);

    if (c) memcpy(result + len_a + len_b, c, len_c * 2);
    if (d) memcpy(result + len_a + len_b + len_c, d, len_d * 2);

    result[len_a + len_b + len_c + len_d] = 0;

    return result;
}

static wchar_t *concat2(const wchar_t *a, const wchar_t *b) {
    return concat(a, b, NULL, NULL);
}

static wchar_t *concat3(const wchar_t *a, const wchar_t *b, const wchar_t *c) {
    return concat(a, b, c, NULL);
}

static wchar_t *concat4(const wchar_t *a, const wchar_t *b, const wchar_t *c, const wchar_t *d) {
    return concat(a, b, c, d);
}

typedef void (*Visit_Proc_W)(const wchar_t *short_name, const wchar_t *full_name, Version_Data *data);
static bool visit_files_w(const wchar_t *dir_name, Version_Data *data, Visit_Proc_W proc) {

    // Visit everything in one folder (non-recursively). If it's a directory
    // that doesn't start with ".", call the visit proc on it. The visit proc
    // will see if the filename conforms to the expected versioning pattern.

    win32_find_dataw_s find_data;

    wchar_t *wildcard_name = concat2(dir_name, L"\\*");
    void *handle = FindFirstFileW(wildcard_name, (WIN32_FIND_DATAW *)&find_data);
    free(wildcard_name);

    if (handle == INVALID_HANDLE_VALUE) return false;

    while (true) {
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (find_data.cFileName[0] != '.')) {
            wchar_t *full_name = concat3(dir_name, L"\\", find_data.cFileName);
            proc(find_data.cFileName, full_name, data);
            free(full_name);
        }

        int success = FindNextFileW(handle, (WIN32_FIND_DATAW *)&find_data);
        if (!success) break;
    }

    FindClose(handle);

    return true;
}


static wchar_t *find_windows_kit_root_with_key(HKEY key, const wchar_t *version) {
    // Given a key to an already opened registry entry,
    // get the value stored under the 'version' subkey.
    // If that's not the right terminology, hey, I never do registry stuff.

    unsigned long required_length;
    long rc = RegQueryValueExW(key, version, NULL, NULL, NULL, &required_length);
    if (rc != 0) return NULL;

    unsigned long length = required_length + 2;  // The +2 is for the maybe optional zero later on. Probably we are over-allocating.
    wchar_t *value = (wchar_t *)malloc(length);
    if (!value) return NULL;

    rc = RegQueryValueExW(key, version, NULL, NULL, (unsigned char *)value, &required_length);  // We know that version is zero-terminated...
    if (rc != 0) return NULL;

    // The documentation says that if the string for some reason was not stored
    // with zero-termination, we need to manually terminate it. Sigh!!

    value[required_length / 2] = 0;

    return value;
}

static void win10_best(const wchar_t *short_name, const wchar_t *full_name, Version_Data *data) {
    // Find the Windows 10 subdirectory with the highest version number.

    int i0, i1, i2, i3;
    int success = swscanf_s(short_name, L"%d.%d.%d.%d", &i0, &i1, &i2, &i3);
    if (success < 4) return;

    if (i0 < data->best_version[0]) return;
    else if (i0 == data->best_version[0]) {
        if (i1 < data->best_version[1]) return;
        else if (i1 == data->best_version[1]) {
            if (i2 < data->best_version[2]) return;
            else if (i2 == data->best_version[2]) {
                if (i3 < data->best_version[3]) return;
            }
        }
    }

    // we have to copy_string and free here because visit_files free's the full_name string
    // after we execute this function, so Win*_Data would contain an invalid pointer.
    if (data->best_name) free(data->best_name);
    data->best_name = _wcsdup(full_name);

    if (data->best_name) {
        data->best_version[0] = i0;
        data->best_version[1] = i1;
        data->best_version[2] = i2;
        data->best_version[3] = i3;
    }
}

static void win8_best(const wchar_t *short_name, const wchar_t *full_name, Version_Data *data) {
    // Find the Windows 8 subdirectory with the highest version number.

    int i0, i1;
    int success = swscanf_s(short_name, L"winv%d.%d", &i0, &i1);
    if (success < 2) return;

    if (i0 < data->best_version[0]) return;
    else if (i0 == data->best_version[0]) {
        if (i1 < data->best_version[1]) return;
    }

    // we have to copy_string and free here because visit_files free's the full_name string
    // after we execute this function, so Win*_Data would contain an invalid pointer.
    if (data->best_name) free(data->best_name);
    data->best_name = _wcsdup(full_name);

    if (data->best_name) {
        data->best_version[0] = i0;
        data->best_version[1] = i1;
    }
}

static void find_windows_kit_root(Find_Result *result) {
    // Information about the Windows 10 and Windows 8 development kits
    // is stored in the same place in the registry. We open a key
    // to that place, first checking preferntially for a Windows 10 kit,
    // then, if that's not found, a Windows 8 kit.

    HKEY main_key;

    long rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots",
							0, KEY_QUERY_VALUE | KEY_WOW64_32KEY | KEY_ENUMERATE_SUB_KEYS, &main_key);
    if (rc != 0) return;

    // Look for a Windows 10 entry.
    wchar_t *windows10_root = find_windows_kit_root_with_key(main_key, L"KitsRoot10");

    if (windows10_root) {
        wchar_t *windows10_lib = concat2(windows10_root, L"Lib");
        free(windows10_root);

        Version_Data data = {0};
        visit_files_w(windows10_lib, &data, win10_best);
        free(windows10_lib);

        if (data.best_name) {
            result->windows_sdk_version = 10;
            result->windows_sdk_root = data.best_name;
            RegCloseKey(main_key);
            return;
        }
    }

    // Look for a Windows 8 entry.
    wchar_t *windows8_root = find_windows_kit_root_with_key(main_key, L"KitsRoot81");

    if (windows8_root) {
        wchar_t *windows8_lib = concat2(windows8_root, L"Lib");
        free(windows8_root);

        Version_Data data = {0};
        visit_files_w(windows8_lib, &data, win8_best);
        free(windows8_lib);

        if (data.best_name) {
            result->windows_sdk_version = 8;
            result->windows_sdk_root = data.best_name;
            RegCloseKey(main_key);
            return;
        }
    }

    // If we get here, we failed to find anything.
    RegCloseKey(main_key);
}

static bool find_visual_studio_2017_by_fighting_through_microsoft_craziness(Find_Result *result) {
    long rc = CoInitialize(NULL);
    // "Subsequent valid calls return false." So ignore false.
    // if rc != S_OK  return false;

    guid_s my_uid                   = {0x42843719, 0xDB4C, 0x46C2, {0x8E, 0x7C, 0x64, 0xF1, 0x81, 0x6E, 0xFD, 0x5B}};
    guid_s CLSID_SetupConfiguration = {0x177F0C4A, 0x1CD3, 0x4DE7, {0xA3, 0x2C, 0x71, 0xDB, 0xBB, 0x9F, 0xA3, 0x6D}};

    ISetupConfiguration *config = NULL;

    // NOTE(Kalinovcic): This is so stupid... These functions take references, so the code is different for C and C++......
#ifdef __cplusplus
    long hr = CoCreateInstance((REFIID)(CLSID_SetupConfiguration), NULL, 0x1/*CLSCTX_INPROC_SERVER*/, (REFIID)my_uid, (void **)&config);
#else
    long hr = CoCreateInstance((REFIID)&CLSID_SetupConfiguration, NULL, 0x1/*CLSCTX_INPROC_SERVER*/, (REFIID)&my_uid, (void **)&config);
#endif

    if (hr != 0)  return false;

    IEnumSetupInstances *instances = NULL;
    hr = config->lpVtbl->EnumInstances(config, &instances);
    config->lpVtbl->Release(config);
    if (hr != 0)     return false;
    if (!instances)  return false;

    bool found_visual_studio_2017 = false;
    while (1) {
        unsigned long found = 0;
        ISetupInstance *instance = NULL;
        long hr = instances->lpVtbl->Next(instances, 1, &instance, &found);
        if (hr != 0) break;

        wchar_t *bstr_inst_path;
        hr = instance->lpVtbl->GetInstallationPath(instance, &bstr_inst_path);
        instance->lpVtbl->Release(instance);
        if (hr != 0)  continue;

        wchar_t *tools_filename = concat2(bstr_inst_path, L"\\VC\\Auxiliary\\Build\\Microsoft.VCToolsVersion.default.txt");
        SysFreeString(bstr_inst_path);

        FILE *f;
        errno_t open_result = _wfopen_s(&f, tools_filename, L"rt");
        free(tools_filename);
        if (open_result != 0) continue;
        if (!f) continue;

        large_integer_s tools_file_size;
        void *file_handle = (void *)_get_osfhandle(_fileno(f));
        int success = GetFileSizeEx(file_handle, (LARGE_INTEGER *)&tools_file_size);
        if (!success) {
            fclose(f);
            continue;
        }

        size_t version_length = (size_t)(tools_file_size.QuadPart + 1);  // Warning: This multiplication by 2 presumes there is no variable-length encoding in the wchars (wacky characters in the file could betray this expectation).
        wchar_t *version = (wchar_t *)malloc(version_length * 2);

        wchar_t *read_result = fgetws(version, (int)version_length, f);
        fclose(f);
        if (!read_result) continue;

        wchar_t *version_tail = wcschr(version, '\n');
        if (version_tail)  *version_tail = 0;  // Stomp the data, because nobody cares about it.

        wchar_t *library_path = concat4(bstr_inst_path, L"\\VC\\Tools\\MSVC\\", version, L"\\lib\\x64");
        wchar_t *library_file = concat2(library_path, L"\\vcruntime.lib");  // @Speed: Could have library_path point to this string, with a smaller count, to save on memory flailing!

        if (os_file_exists(library_file)) {
            wchar_t *link_exe_path = concat4(bstr_inst_path, L"\\VC\\Tools\\MSVC\\", version, L"\\bin\\Hostx64\\x64");
            free(version);

            result->vs_exe_path = link_exe_path;
            result->vs_library_path = library_path;
            result->is_vs_2017 = true;
            found_visual_studio_2017 = true;
            break;
        }

        free(version);

        /*
           Ryan Saunderson said:
           "Clang uses the 'SetupInstance->GetInstallationVersion' / ISetupHelper->ParseVersion to find the newest version
           and then reads the tools file to define the tools path - which is definitely better than what i did."

           So... @Incomplete: Should probably pick the newest version...
        */
    }

    instances->lpVtbl->Release(instances);
    return found_visual_studio_2017;
}

static void find_visual_studio_by_fighting_through_microsoft_craziness(Find_Result *result) {
    // The name of this procedure is kind of cryptic. Its purpose is
    // to fight through Microsoft craziness. The things that the fine
    // Visual Studio team want you to do, JUST TO FIND A SINGLE FOLDER
    // THAT EVERYONE NEEDS TO FIND, are ridiculous garbage.

    // For earlier versions of Visual Studio, you'd find this information in the registry,
    // similarly to the Windows Kits above. But no, now it's the future, so to ask the
    // question "Where is the Visual Studio folder?" you have to do a bunch of COM object
    // instantiation, enumeration, and querying. (For extra bonus points, try doing this in
    // a new, underdeveloped programming language where you don't have COM routines up
    // and running yet. So fun.)
    //
    // If all this COM object instantiation, enumeration, and querying doesn't give us
    // a useful result, we drop back to the registry-checking method.

    bool found_visual_studio_2017 = find_visual_studio_2017_by_fighting_through_microsoft_craziness(result);
    if (found_visual_studio_2017)  return;


    // If we get here, we didn't find Visual Studio 2017. Try earlier versions.

    HKEY vs7_key;
    long rc = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7", 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &vs7_key);
    if (rc != 0)  return;

    // Hardcoded search for 4 prior Visual Studio versions. Is there something better to do here?
    const wchar_t *versions[] = { L"14.0", L"12.0", L"11.0", L"10.0" };
    const int NUM_VERSIONS = sizeof(versions) / sizeof(versions[0]);

    for (int i = 0; i < NUM_VERSIONS; i++) {
        const wchar_t *v = versions[i];

        unsigned long dw_type;
        unsigned long cb_data;

        long rc = RegQueryValueExW(vs7_key, v, NULL, &dw_type, NULL, &cb_data);
        if ((rc == ERROR_FILE_NOT_FOUND) || (dw_type != REG_SZ)) {
            continue;
        }

        wchar_t *buffer = (wchar_t *)malloc(cb_data);
        if (!buffer)  return;

        rc = RegQueryValueExW(vs7_key, v, NULL, NULL, (unsigned char *)buffer, &cb_data);
        if (rc != 0)  continue;

        // @Robustness: Do the zero-termination thing suggested in the RegQueryValue docs?

        wchar_t *lib_path = concat2(buffer, L"VC\\Lib\\amd64");

        // Check to see whether a vcruntime.lib actually exists here.
        wchar_t *vcruntime_filename = concat2(lib_path, L"\\vcruntime.lib");
        bool vcruntime_exists = os_file_exists(vcruntime_filename);
        free(vcruntime_filename);

        if (vcruntime_exists) {
            result->vs_exe_path     = concat2(buffer, L"VC\\bin\\amd64");
            result->vs_library_path = lib_path;

            free(buffer);
            RegCloseKey(vs7_key);
            return;
        }

        free(lib_path);
        free(buffer);
    }

    RegCloseKey(vs7_key);

    // If we get here, we failed to find anything.
}


static char *wide_to_string(const wchar_t *wstr) {
    if (!wstr) {
        return NULL;
    }

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (size <= 0) {
        return NULL;
    }

    char *result = (char *)malloc(size);
    if (!result) {
        return NULL;
    }

    if (WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result, size, NULL, NULL) == 0) {
        free(result);
        return NULL;
    }

    return result;
}


Find_Result find_visual_studio_and_windows_sdk() {
    Find_Result result = {0};

    find_windows_kit_root(&result);

    if (result.windows_sdk_root) {
        result.windows_sdk_um_library_path   = concat2(result.windows_sdk_root, L"\\um\\x64");
        result.windows_sdk_ucrt_library_path = concat2(result.windows_sdk_root, L"\\ucrt\\x64");
    }

    find_visual_studio_by_fighting_through_microsoft_craziness(&result);

    result.windows_sdk_root_a              = wide_to_string(result.windows_sdk_root);
    result.windows_sdk_um_library_path_a   = wide_to_string(result.windows_sdk_um_library_path);
    result.windows_sdk_ucrt_library_path_a = wide_to_string(result.windows_sdk_ucrt_library_path);
    result.vs_exe_path_a                   = wide_to_string(result.vs_exe_path);
    result.vs_library_path_a               = wide_to_string(result.vs_library_path);

    return result;
}


#endif  // MICROSOFT_CRAZINESS_IMPLEMENTATION_GUARD
#endif  // MICROSOFT_CRAZINESS_IMPLEMENTATION
