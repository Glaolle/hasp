/*  $Id: akstypes.h,v 1.2 2007/06/11 18:15:32 lav Exp $
**
**  Generic type defines
**  (c) 1999-2006 by Aladdin Knowledge Systems Ltd.
**
**  $Revision: 1.2 $
**  $Date: 2007/06/11 18:15:32 $
**  $Author: lav $
**
**  The types are:
**  --------------
**  aks_u8_t  - 8bit  value, unsigned
**  aks_s8_t  - 8bit  value, signed
**  aks_u16_t - 16bit value, unsigned
**  aks_s16_t - 16bit value, signed
**  aks_u32_t - 32bit value, unsigned
**  aks_s32_t - 32bit value, signed
**  aks_u64_t - 64bit value, unsigned   (not all compilers support this)
**  aks_s64_t - 64bit value, signed     (not all compilers support this)
**  aks_ptr_t - a pointer is guaranteed to fit into this     |
**                                                           |
**                                               if not, _AKS_NO_REAL_64TYPE_ gets defined
**  $Log: akstypes.h,v $
**  Revision 1.2  2007/06/11 18:15:32  lav
**  updated to 1.7
**
**  Revision 1.1.1.1  2006/03/31 09:24:54  lav
**  initial import
**
**  Revision 1.27  2004/11/25 16:15:15  chris
**  detect linux x86_64; obsolete NEED_OLD_TYPES
**
**  Revision 1.26  2004/05/07 08:07:01  martin
**  Added support for 8 bit controllers & compilers like Motorola & Cosmic.
**
**  Revision 1.25  2004/02/26 17:46:51  chris
**  fix 64bit types on Borland C
**
**  Revision 1.24  2003/12/09 10:35:25  chris
**  don't define aks_bool_t diffenently on c++
**
**  Revision 1.23  2003/07/24 20:21:02  chris
**  Solaris fixes
**
**  Revision 1.22  2002/09/16 10:19:30  chris
**  support for 64bit types on Metrowerks compiler
**
**  Revision 1.21  2002/06/07 14:04:30  chris
**  Win64 fix
**
**  Revision 1.20  2002/04/16 11:30:22  chris
**  prevent redefinition of __32BIT__
**
**  Revision 1.19  2002/03/15 15:58:40  chris
**  Win64 changes
**
**  Revision 1.18  2001/04/04 14:50:52  chris
**  define __32BIT__ only if it's not defined yet (__POWERPC__)
**
**  Revision 1.17  2001/04/04 08:15:03  Werner
**  added __POWERPC__ type
**
**  Revision 1.16  2001/03/19 13:37:20  Andi
**  Defined aks_size_t, which holds machine word size.
**
**  Revision 1.15  2001/01/24 11:50:03  chris
**  set _AKS_NO_REAL_64TYPE_ if aks_u64_t is defined as array
**
**  Revision 1.14  2000/12/21 13:24:59  Werner
**  added aks_bool_t
**
**  Revision 1.13  2000/11/09 13:25:24  chris
**  changed TRUE define to be the value it normally is
**
**  Revision 1.12  2000/11/08 15:57:04  werner
**  added TRUE and FALSE definitions
**
**  Revision 1.11  2000/10/11 12:40:55  chris
**  proper aks_x64_t for SunPro C
**
**  Revision 1.10  2000/07/14 16:47:54  chris
**  detect linux ia64
**
**  Revision 1.9  2000/04/13 10:19:11  chris
**  renamed aks_ptrsz_t to aks_ptr_t
**
**  Revision 1.8  2000/04/11 12:59:07  chris
**  adaptations for Win64
**
**  Revision 1.7  2000/03/03 13:10:15  chris
**  workaround for old (v7) Watcom compiler
**
**  Revision 1.6  1999/11/25 12:24:23  chris
**  Watcom C v11 also supports 64bit type
**
**  Revision 1.5  1999/11/22 21:47:16  chris
**  don't define __32BIT__ and __64BIT__ if they are defined already
**
**  Revision 1.4  1999/11/17 00:15:40  chris
**  aks_ptrsz_t
**
**  Revision 1.3  1999/11/03 12:38:55  Werner
**
**  Revision 1.2  1999/10/25 14:26:37  chris
**  make aks_u32_t unsigned long under Win32 for now :-(
**
**  Revision 1.1  1999/10/25 12:03:34  chris
**  Initial revision
*/

#ifndef _AKSTYPES_H_
#define _AKSTYPES_H_

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

/* some common default word sizes */

#if (defined(__6808__)) && !defined(__8BIT__)
#define __8BIT__
#endif
#if (defined(WIN95) || defined(SCO) || defined(DJGPP)) && !defined(__32BIT__)
#define __32BIT__
#endif
#if (defined(WINNT) || defined(WIN32)) && !defined(_WIN64) && !defined(__32BIT__)
#define __32BIT__
#endif
#if (defined(WIN64) || defined(_WIN64))
#ifndef __64BIT__
#define __64BIT__
#endif
#ifndef WIN64
#define WIN64
#endif
#endif
#if defined(SOLARIS)
#  if !defined(__32BIT__) && !defined(__64BIT__)
#    if defined(__arch64__) || defined(__sparcv9)
#      define __64BIT__
#    else
#      define __32BIT__
#    endif
#  endif
#endif
#if defined(LINUX)
#  if defined(__alpha__) && !defined(__64BIT__)
#    define __64BIT__
#    define __LP64__
#  endif
#  if defined(__ia64__) && !defined(__64BIT__)
#    define __64BIT__
#  endif
#  if defined(__x86_64__) && !defined(__64BIT__)
#    define __64BIT__
#  endif
#  if defined(__i386__) && !defined(__32BIT__)
#    define __32BIT__
#  endif
#endif
#if defined (__POWERPC__)
#  if !defined(__ppc64__) && !defined(__32BIT__)
#    define __32BIT__
#  endif
#  if defined(__ppc64__) && !defined(__64BIT__)
#    define __64BIT__
#  endif
#endif

#if !defined(__8BIT__) && !defined(__16BIT__) && !defined(__32BIT__) && !defined(__64BIT__)
#error "Word size unknown! (need __8BIT__ or __16BIT__ or __32BIT__ or __64BIT__)"
#endif

#ifdef NEED_OLD_TYPES
# undef WORD
# undef BYTE
# undef LONG
  typedef unsigned short WORD;
  typedef unsigned char  BYTE;
  typedef long LONG;
#if defined(_MSC_VER)
#pragma message ("*** using obsolete NEED_OLD_TYPES!!")
#endif
#if defined(__GNUC__)
#warning *** using obsolete NEED_OLD_TYPES!!
#endif
#endif

/* new types  BEGIN ------- */

/* common for all widths */
typedef unsigned char  aks_u8_t;
typedef   signed char  aks_s8_t;
typedef unsigned short aks_u16_t;
typedef   signed short aks_s16_t;
#ifndef WIN64
typedef unsigned long  aks_ptr_t;    /* a pointer will always fit into a long */
#else
typedef unsigned __int64 aks_ptr_t;
#endif

#ifdef __8BIT__
typedef unsigned char  aks_size_t;
typedef unsigned long  aks_u32_t;
typedef   signed long  aks_s32_t;
typedef unsigned long  aks_u64_t[2];
typedef   signed long  aks_s64_t[2];
#define _AKS_NO_REAL_64TYPE_  /* set marker to handle aks_?64_t differently in the code */
#if defined(__16BIT__) || defined(__32BIT__) || defined(__64BIT__)
#error too many size defines!
#endif
#endif /* #ifdef __8BIT__ */

#ifdef __16BIT__
typedef unsigned short aks_size_t;
typedef unsigned long  aks_u32_t;
typedef   signed long  aks_s32_t;
typedef unsigned long  aks_u64_t[2];
typedef   signed long  aks_s64_t[2];
#define _AKS_NO_REAL_64TYPE_  /* set marker to handle aks_?64_t differently in the code */
#if defined(__8BIT__) || defined(__32BIT__) || defined(__64BIT__)
#error too many size defines!
#endif
#endif /* #ifdef __16BIT__ */

#if defined(__32BIT__) || defined(__64BIT__)
#if defined(WIN32) || defined(WINNT) || defined(WIN95) /* DWORD kack */
typedef unsigned long  aks_u32_t;
typedef   signed long  aks_s32_t;
#else
typedef unsigned int   aks_u32_t;
typedef   signed int   aks_s32_t;
#endif
#endif /* #if defined(__32BIT__) || defined(__64BIT__) */

#ifdef __32BIT__
typedef unsigned int   aks_size_t;
#if defined(__GNUC__) || (defined(__SUNPRO_C) && __STDC__ != 1) \
    || (defined(__MWERKS__) && (defined(__POWERPC__) || defined(__INTEL__)))
typedef unsigned long long aks_u64_t;
typedef   signed long long aks_s64_t;
#else
#if (defined(_MSC_VER) && (_MSC_VER >= 900)) || (defined(__WATCOMC__) && (__WATCOMC__+1 >= 1100+1)) \
    || (defined(__BORLANDC__) && (__BORLANDC__+1 >= 0x550+1))
typedef unsigned __int64 aks_u64_t;
typedef   signed __int64 aks_s64_t;
#else
typedef unsigned long  aks_u64_t[2];
typedef   signed long  aks_s64_t[2];
#define _AKS_NO_REAL_64TYPE_  /* set marker to handle aks_?64_t differently in the code */
#endif /* not defined _MSC_VER and not defined __GNUC__ and not defined __WATCOMC__ and not defined __BORLANDC__ */
#endif /* not defined __GNUC__ and ( not defined __SUNPRO_C && __STDC__ != 1 ) */
#if defined(__8BIT__) || defined(__16BIT__) || defined(__64BIT__)
#error too many size defines!
#endif
#endif /* #ifdef __32BIT__ */

#ifdef __64BIT__
#ifndef WIN64
typedef unsigned long  aks_size_t;
typedef unsigned long  aks_u64_t;
typedef   signed long  aks_s64_t;
#else
typedef unsigned __int64 aks_size_t;
typedef unsigned __int64 aks_u64_t;
typedef   signed __int64 aks_s64_t;
#endif /* defined WIN64 */
#if defined(__8BIT__) || defined(__16BIT__) || defined(__32BIT__)
#error too many size defines!
#endif
#endif /* defined __64BIT__ */

typedef int aks_bool_t;

/* new variable types  END --------- */

#endif /* #ifndef _AKSTYPES_H_ */
