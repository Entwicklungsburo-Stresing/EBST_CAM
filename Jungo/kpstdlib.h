/* Jungo Connectivity Confidential. Copyright (c) 2019 Jungo Connectivity Ltd.  https://www.jungo.com */

#ifndef _KPSTDLIB_H_
#define _KPSTDLIB_H_

#ifndef __KERNEL__
    #define __KERNEL__
#endif

#if !defined(UNIX) && defined(LINUX)
    #define UNIX
#endif

#if defined(UNIX)
    #include "windrvr.h" // for use of KDBG DWORD parameter.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Spinlocks and interlocked operations */
typedef struct _KP_SPINLOCK KP_SPINLOCK;
KP_SPINLOCK *kp_spinlock_init(void);
void kp_spinlock_wait(KP_SPINLOCK *spinlock);
void kp_spinlock_release(KP_SPINLOCK *spinlock);
void kp_spinlock_uninit(KP_SPINLOCK *spinlock);

typedef volatile int KP_INTERLOCKED;
void kp_interlocked_init(KP_INTERLOCKED *target);
void kp_interlocked_uninit(KP_INTERLOCKED *target);
int kp_interlocked_increment(KP_INTERLOCKED *target);
int kp_interlocked_decrement(KP_INTERLOCKED *target);
int kp_interlocked_add(KP_INTERLOCKED *target, int val);
int kp_interlocked_read(KP_INTERLOCKED *target);
void kp_interlocked_set(KP_INTERLOCKED *target, int val);
int kp_interlocked_exchange(KP_INTERLOCKED *target, int val);

#if defined(WINNT) || defined(WIN32)
    #if defined(_WIN64) && !defined(KERNEL_64BIT)
        #define KERNEL_64BIT
    #endif
    typedef unsigned long ULONG;
    typedef unsigned short USHORT;
    typedef unsigned char UCHAR;
    typedef long LONG;
    typedef short SHORT;
    typedef char CHAR;
    typedef ULONG DWORD;
    typedef DWORD *PDWORD;
    typedef unsigned char *PBYTE;
    typedef USHORT WORD;
    typedef void *PVOID;
    typedef char *PCHAR;
    typedef PVOID HANDLE;
    typedef ULONG BOOL;
    #ifndef WINAPI
        #define WINAPI
    #endif
#elif defined(UNIX)
    #ifndef __cdecl
        #define __cdecl
    #endif
#endif

#if defined(WINNT) || defined(WIN32)
    #define OS_needs_copy_from_user(fKernelMode) FALSE
    #define COPY_FROM_USER(dst,src,n) memcpy(dst,src,n)
    #define COPY_TO_USER(dst,src,n) memcpy(dst,src,n)
#elif defined(LINUX)
    #define OS_needs_copy_from_user(fKernelMode) (!fKernelMode)
    #define COPY_FROM_USER(dst,src,n) LINUX_copy_from_user(dst,src,n)
    #define COPY_TO_USER(dst,src,n) LINUX_copy_to_user(dst,src,n)
#endif

#define COPY_FROM_USER_OR_KERNEL(dst, src, n, fKernelMode) \
{ \
    if (OS_needs_copy_from_user(fKernelMode)) \
        COPY_FROM_USER(dst, src, n); \
    else \
        memcpy(dst, src, n); \
}

#define COPY_TO_USER_OR_KERNEL(dst, src, n, fKernelMode) \
{ \
    if (OS_needs_copy_from_user(fKernelMode)) \
        COPY_TO_USER(dst, src, n); \
    else \
        memcpy(dst, src, n); \
}

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef NULL
    #define NULL 0UL
#endif

int __cdecl KDBG(DWORD dwLevel, DWORD dwSection, const char *format, ...);

#if defined(WIN32)
    #if defined(KERNEL_64BIT)
        #include <stdarg.h>
    #else
        // Define varargs ANSI style
        #define _INTSIZEOF(n) \
            ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
        typedef char * va_list;
        #define va_start(ap,v) ( ap = (va_list)&v + _INTSIZEOF(v) )
        #define va_arg(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
        #define va_end(ap) ( ap = (va_list)0 )

    #endif

    int __cdecl _snprintf(char *buffer, unsigned long Limit, const char *format,
        ...);
    int __cdecl _vsnprintf(char *buffer, unsigned long Limit, const char
        *format, va_list Next);
#endif

char* __cdecl strcpy(char *s1, const char *s2);
void* __cdecl malloc(unsigned long size);
void __cdecl free(void *buf);

#if defined(LINUX)
    #include <linux_wrappers.h>
    #define memset LINUX_memset
    #define strncmp LINUX_strncmp
    #define strcpy LINUX_strcpy
    #define strcmp LINUX_strcmp
    #define strncpy LINUX_strncpy
    #define strcat LINUX_strcat
    #define strncat LINUX_strncat
    #define strlen LINUX_strlen
    #define memcpy LINUX_memcpy
    #define memcmp LINUX_memcmp
    #define sprintf LINUX_sprintf
    #define vsprintf LINUX_vsprintf
    #define snprintf LINUX_snprintf
    #define vsnprintf LINUX_vsnprintf
#elif defined(WINNT)
    #if !defined size_t
        #if defined(KERNEL_64BIT)
            typedef unsigned __int64 size_t;
        #else
            typedef unsigned int size_t;
        #endif
    #endif
    void* __cdecl memcpy(void *dest, const void *src, size_t count);
    void* __cdecl memset(void *dest, int c, size_t count);
#if !defined(_STRNCPY)
    char* _strncpy(char* s1, const char* s2, size_t limit);
#endif
    #define snprintf _snprintf
    #define vsnprintf _vsnprintf
    #define strncpy _strncpy
#endif

#ifdef __cplusplus
}
#endif

#endif /* _KPSTDLIB_H_ */

