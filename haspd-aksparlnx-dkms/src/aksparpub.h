/* $Id: aksparpub.h,v 1.4 2007/06/11 18:15:31 lav Exp $
 *
 * This file is (c) copyright 1994-2000 by Christian Groessler,
 *              (c) copyright 2001-2007 Aladdin Knowledge Systems Ltd.
 *
 */

#ifndef __AKSPAR_H__
#define __AKSPAR_H__

#define IN_DRIVER
#include "aksport.h"

/* allowed # of ports */
#define MAX_HL_PORTS 5

/* max. number of installed ISA CPC cards */
#define MAX_ISA_CPC_CARDS 2

/* default major number */
#ifndef HL_MAJOR
#define HL_MAJOR 42
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define USER64 0
#define USER32 1

#ifndef ASP_PARALLEL
#  define ASP_PARALLEL  1
#  define ASP_SERIAL    2
#  define ASP_NEC       4
#  define ASP_ECP       8
#  define ASP_PS2       16
#  define ASP_CPQDOCK   32
#  define ASP_CPC       64
#  define ASP_CR_ACC    128
#  define ASP_WAVE      256
#  define ASP_CPC_PCI   512
#endif

#ifndef set_assysdep_ptr
#  ifndef __64BIT__
#    define set_assysdep_ptr(api,buf) (api)->OSspecific=(Long)(buf)
#    define get_assysdep_ptr(api) ((void *)((api)->OSspecific))
#  else
#    define set_assysdep_ptr(api,buf) do { (api)->OSspecific=(Long)(((aks_u64_t)(buf)) & 0xffffffffu); \
                                     (api)->OSspecificHigh=(((aks_u32_t)(((aks_u64_t)(buf))>>32)) \
                                                               & 0xffffffffu);} while (0)
#    define get_assysdep_ptr(api) ((void *)((aks_u64_t)((api)->OSspecific) | \
                              (((aks_u64_t)((api)->OSspecificHigh))<<32)))
#  endif
#endif /* #ifndef set_assysdep_ptr */

typedef struct _DEP_STUFF {
#ifdef CPC_PCI
    porttabentry*      portStruct;        /* port structure */
#endif /*#ifdef CPC_PCI*/
} DEP_STUFF, *PDEP_STUFF;


#if defined(__i386__) || defined(__x86_64__)  /* never use regparms for interface routines */
#define LNX_REGPARM0 __attribute__((regparm(0)))
#define LNX_REGPARM1 __attribute__((regparm(0)))
#else
#define LNX_REGPARM0
#define LNX_REGPARM1
#endif

/* externals from API */

extern aks_u16_t API_SERVICE(HL_API *user_api_struct) LNX_REGPARM1;
extern aks_u16_t __exported_API_version__;

#ifdef WANT_PROTOS

/* function declarations */

#ifndef MODULE
long hl_init(long kmem_start);
#else
#if LINUX_VERSION_CODE >= 0x20600
static int hl_init_module(void);
#else
int init_module(void);
#endif
#if LINUX_VERSION_CODE >= 0x20600
static void hl_cleanup_module(void);
#else
void cleanup_module(void);
#endif
#if LINUX_VERSION_CODE < 0x20600
static int init_chk_port(unsigned int port);
#endif
#endif
static aks_u16_t call_api(HL_API *);
static aks_u16_t call_api_nowildport(HL_API *);
static void init_port(aks_u16_t port);
static int chk_valid_port(aks_u16_t port);
static int hl_open(struct inode *inode, struct file *file);
#if LINUX_VERSION_CODE > 0x20164
static int hl_release(struct inode *inode,struct file *file);
#else
static void hl_release(struct inode *inode, struct file *file);
#endif
#if defined(__x86_64__) && LINUX_VERSION_CODE >= 0x2060E
static long hl_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
static long hl_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
#else
static int hl_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#endif
static int hl_parport_claim_or_block(struct _porttabentry *port, signed long *timeout);
static void hl_wakeup(struct _porttabentry *port);
static void fix_wait_queue(struct pardevice *dev);

void lnx_enter_crit(unsigned long port) LNX_REGPARM1;
void lnx_leave_crit(unsigned long port) LNX_REGPARM1;
void lnx_mydelay(unsigned int delay) LNX_REGPARM1;
unsigned long lnx_jiffies(void) LNX_REGPARM0;
unsigned long lnx_HZ(void) LNX_REGPARM0;
void lnx_us_delay(aks_u32_t) LNX_REGPARM1;
void check_sched(void) LNX_REGPARM0;

#endif /* #ifdef WANT_PROTOS */
#endif /* #ifndef __AKSPAR_H__ */
/* Local Variables: */
/* c-file-style: "cpg" */
/* c-basic-offset: 4 */
/* End: */
