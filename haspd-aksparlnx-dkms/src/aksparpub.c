/* $Id: aksparpub.c,v 1.5 2008/10/23 05:46:58 lav Exp $
 *
 * This file is (c) copyright 1994-2001 by Christian Groessler
 *              (c) copyright 2001-2007 Aladdin Knowledge Systems Ltd.
 *
 */

#define VERSION_HIGH 1
#define VERSION_LOW 7
#define VERSION_ASC "1.7"
#define VERSION_BETA 0 /* this version is not beta */

/*#define CPC_PCI*/
/*#define HL_DEBUG*/

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 16)
#include <linux/config.h>
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
/* See http://bugs.etersoft.ru/show_bug.cgi?id=5931 */
#include <generated/autoconf.h>
#endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#endif

#if defined(CONFIG_SMP) && !defined(__SMP__)
#define __SMP__
#endif
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODVERSIONS
#endif
#if defined(CONFIG_MODVERSIONS) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
#include <linux/modversions.h>
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ioport.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 0)
#undef MOD_INC_USE_COUNT
#undef MOD_DEC_USE_COUNT
#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif

#include <linux/parport.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#if defined(__x86_64__) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)
#include <linux/ioctl32.h>
#endif
#if defined(__x86_64__) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)
#include <linux/smp_lock.h>
#endif
#ifndef LINUX
#define LINUX
#endif
#include "akstypes.h"
#include "fastapi.h"
#include "unix_ioctl.h"
#define WANT_PROTOS /* pull in prototypes from aksparpub.h */
#include "aksparpub.h"
#ifdef CPC_PCI
#include <linux/pci.h>
#include <linux/pci_ids.h>
#ifndef PCI_DEVICE_ID_AKS_CPC /* newer kernel source have all unreferenced IDs removed */
#define PCI_DEVICE_ID_AKS_CPC 0x200
#endif
#endif
#include <asm/io.h>
#include <asm/uaccess.h>

unsigned int major = HL_MAJOR;
unsigned int force = FALSE;
unsigned int loglevel = 1;
/* loglevel: 0 - nothing, only errors
 *           1 - normal
 *           2 - verbose
 *           3 - ultra verbose
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
static const char __module_depends[]
    /* __attribute_used__ - only for functions? see eterbug #2673 */
    __attribute__((section(".modinfo"))) = "depends=parport";
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 10)
MODULE_PARM(force, "i");
MODULE_PARM(major, "i");
MODULE_PARM(loglevel, "i");
#else
module_param(force, uint, 0);
MODULE_PARM_DESC(force, "force loading with i/o conflicts");
module_param(major, uint, 0);
MODULE_PARM_DESC(major, "device major number to use");
module_param(loglevel, uint, 0);
MODULE_PARM_DESC(loglevel, "logging verbosity level (0..3)");
#endif

MODULE_AUTHOR("Christian Groessler, cpg@aladdin.de");
MODULE_DESCRIPTION("Hardlock/HASP Driver for Linux (new $Revision: 1.5 $)");
MODULE_LICENSE("Copyright (c) 1985-2007 Aladdin Knowledge Systems Ltd.");

static HL_API mybuf;

static char hardlock[] = "aksparlnx";

#ifdef DECLARE_MUTEX
static DECLARE_MUTEX(hl_sem);
static DECLARE_MUTEX(hl_ioctl_sem);
#elif defined(DEFINE_SEMAPHORE) && LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
static DEFINE_SEMAPHORE(hl_sem);
static DEFINE_SEMAPHORE(hl_ioctl_sem);
#elif defined(DEFINE_SEMAPHORE) && LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
static DEFINE_SEMAPHORE(hl_sem, 1);
static DEFINE_SEMAPHORE(hl_ioctl_sem, 1);
#else
static struct semaphore hl_sem = MUTEX;
static struct semaphore hl_ioctl_sem = MUTEX;
#endif

static struct _porttabentry hl_pports[MAX_HL_PORTS] /* = { [0 ... MAX_HL_PORTS-1 ] = { 0, 0, NULL, 0 } }*/;
static unsigned int hl_numports = 0;
unsigned int isacpcaddress[MAX_ISA_CPC_CARDS] = {[0 ... MAX_ISA_CPC_CARDS - 1] = 0};
unsigned int hlportaddress[MAX_HL_PORTS] = {[0 ... MAX_HL_PORTS - 1] = 0};
static int num_isacpc, num_hlports;

/* open function */

static int hl_open(struct inode *inode, struct file *file)
{
    unsigned int minor = MINOR(inode->i_rdev);

    if (minor >= 1)
        return -ENODEV; /* minor device must be 0 */
    MOD_INC_USE_COUNT;
    return 0;
}

/* close function */

static int hl_release(struct inode *inode, struct file *file)
{
    MOD_DEC_USE_COUNT;
    return 0;
}

/* handle API request ioctl */

static int hl_api_ioctl(struct file *file, unsigned int cmd, unsigned long arg, int domain)
{
    aks_u8_t *data_buffer;
    unsigned long count = 0, xresult;
    unsigned short ocount;
    unsigned char *optr = NULL, *myusrbuf = NULL;

    if (cmd != HLAPI_CALL)
        return -ENOIOCTLCMD;

#ifdef HL_DEBUG
    printk(KERN_INFO "HLAPI_CALL entered\n");
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
    if (verify_area(VERIFY_WRITE, (const void *)arg, 256))
        return -EFAULT;
#else
    if (!access_ok((const void *)arg, 256))
        return -EFAULT;
#endif
    down(&hl_sem);
    if (copy_from_user(&mybuf, (const void *)arg, 0x100))
    {
        up(&hl_sem);
        return -EFAULT;
    }
    data_buffer = get_data_ptr(&mybuf);
#ifdef HL_DEBUG
    printk(KERN_INFO "%s: asdataptr = %p\n", hardlock, data_buffer);
#endif
    if ((ocount = mybuf.Bcnt))
    { /* pointer in API structure is used */
        if (domain == USER32)
            data_buffer = (aks_u8_t *)((unsigned long)data_buffer & 0xffffffff);
        optr = data_buffer;
        count = mybuf.Bcnt << 3;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
        if (verify_area(VERIFY_READ, data_buffer, count))
        {
            up(&hl_sem);
            return -EFAULT;
        }
        if (verify_area(VERIFY_WRITE, data_buffer, count))
        {
            up(&hl_sem);
            return -EFAULT;
        }
#else
        if (!access_ok(data_buffer, count))
        {
            up(&hl_sem);
            return -EFAULT;
        }
        if (!access_ok(data_buffer, count))
        {
            up(&hl_sem);
            return -EFAULT;
        }
#endif
        if (count > 0x10000)
            count = 0x10000;
        if (!(myusrbuf = vmalloc(count + 8)))
        {
            /* not enough memory to allocate buffer for user data */
            printk(KERN_INFO "%s: cannot allocate memory (myusrbuf)\n", hardlock);
            up(&hl_sem);
            return -ENOMEM;
        }
        if (copy_from_user(myusrbuf, (const void *)data_buffer, count))
        {
            up(&hl_sem);
            return -EFAULT;
        }
        mybuf.Bcnt = count >> 3;
        set_data_ptr(&mybuf, myusrbuf);
    }

    xresult = call_api(&mybuf); /* API does its work... */

    if (ocount)
    {
        if (copy_to_user(optr, (void *)myusrbuf, count))
        {
            up(&hl_sem);
            return -EFAULT;
        }
        vfree(myusrbuf);
        set_data_ptr(&mybuf, optr);
        mybuf.Bcnt = ocount;
    }

    if (copy_to_user((void *)arg, (void *)&mybuf, 0x100))
    {
        up(&hl_sem);
        return -EFAULT;
    }
    up(&hl_sem);
#ifdef HL_DEBUG
    printk(KERN_INFO "HLAPI_CALL left\n");
#endif
    return 0;
}

#if defined(__x86_64__) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)
static int hl_ioctl32(unsigned int fd, unsigned int cmd, unsigned long arg, struct file *file)
{
    return hl_api_ioctl(file, cmd, arg, USER32);
}
#endif

#if defined(__x86_64__) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
static void hl_ioctl_lock(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)
    lock_kernel();
#else
    down(&hl_ioctl_sem);
#endif
}

static void hl_ioctl_unlock(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 39)
    unlock_kernel();
#else
    up(&hl_ioctl_sem);
#endif
}

static long hl_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret;

    switch (cmd)
    {
    case HLAPI_CALL:
        hl_ioctl_lock();
        ret = hl_api_ioctl(file, cmd, arg, USER32);
        hl_ioctl_unlock();
        break;
    default:
        hl_ioctl_lock();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
        ret = hl_unlocked_ioctl(file, cmd, arg);
#else
        ret = hl_ioctl(file->f_dentry->d_inode, file, cmd, arg);
#endif
        hl_ioctl_unlock();
        break;
    }
    return ret;
}

#endif

/* ioctl function */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
static long hl_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int hl_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
    struct inode *inode = file->f_path.dentry->d_inode;
#endif
    unsigned int minor = MINOR(inode->i_rdev);
    unsigned char *portbuf;
    unsigned int i, j;
    aks_u32_t version_buffer[4];
    struct alad_portlist *plist;
    struct alad_portentry *pentry;
#ifdef __64BIT__
    struct alad_portlist64 *plist64;
    struct alad_portentry64 *pentry64;
#endif

#ifdef HL_DEBUG
    if (loglevel >= 3)
        printk(KERN_INFO "%s%d ioctl, cmd: 0x%x, arg: 0x%lx\n", hardlock, minor, cmd, arg);
#endif

    if (minor >= 1)
        return -ENODEV;

    switch (cmd)
    {

    case HL_GETVER: /* get driver version */
        version_buffer[0] = (VERSION_HIGH << 16) + VERSION_LOW;
        version_buffer[1] = (aks_u32_t)__exported_API_version__; /* high word still unused */
        version_buffer[2] = version_buffer[3] = 0;
        if (copy_to_user((void *)arg, version_buffer, 16))
            return -EFAULT;
        return 0;

    case HL_GETPORTS:
        if (!(portbuf = kmalloc(4096, GFP_KERNEL)))
        {
            /* not enough memory to allocate buffer for return data */
            printk(KERN_INFO "%s: cannot allocate 4K\n", hardlock);
            return -ENOMEM;
        }
        memset(portbuf, 0, 4096);
        plist = (struct alad_portlist *)portbuf;
        pentry = plist->list;
        for (i = 0, j = alad_portlist_size; i < hl_numports; i++)
        {
#ifdef __64BIT__
            if (hl_pports[i].port & ~0xffffffffUL)
                continue; /* port address cannot be represented to client */
#endif
            j += sizeof(struct alad_portentry);
            if (j > 4096)
                break;
            plist->num_entries++;
            pentry->port_address = hl_pports[i].port;
            pentry->flags = hl_pports[i].flags;
            pentry++;
        }
        if (copy_to_user((void *)arg, portbuf, 4096))
        {
            kfree(portbuf);
            return -EFAULT;
        }
        kfree(portbuf);
        return 0;

#ifdef __64BIT__

    case HL_GETPORTS64:
        if (!(portbuf = kmalloc(4096, GFP_KERNEL)))
        {
            /* not enough memory to allocate buffer for return data */
            printk(KERN_INFO "%s: cannot allocate 4K\n", hardlock);
            return -ENOMEM;
        }
        memset(portbuf, 0, 4096);
        plist64 = (struct alad_portlist64 *)portbuf;
        pentry64 = plist64->list;
        for (i = 0, j = alad_portlist_size64; i < hl_numports; i++)
        {
            j += sizeof(struct alad_portentry64);
            if (j > 4096)
                break;
            plist64->num_entries++;
            pentry64->port_address = hl_pports[i].port;
            pentry64->flags = hl_pports[i].flags;
            pentry64++;
        }
        if (copy_to_user((void *)arg, portbuf, 4096))
        {
            kfree(portbuf);
            return -EFAULT;
        }
        kfree(portbuf);
        return 0;

#endif /* #ifdef __64BIT__ */

    case HLAPI_CALL:
        return hl_api_ioctl(file, cmd, arg, USER64);

    default:
#ifdef HL_DEBUG
        if (loglevel >= 3)
            printk(KERN_INFO "%s%d --> invalid request! (%d)\n", hardlock, minor, cmd);
#endif
        return -ENOIOCTLCMD;
    }
}

/* driver functions */

static struct file_operations hl_fops = {
    .owner = THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
    .open = hl_open,
    .release = hl_release,
    .unlocked_ioctl = hl_unlocked_ioctl,
#else
    .ioctl = hl_ioctl,
    .open = hl_open,
    .release = hl_release,
#endif
#if defined(__x86_64__) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
    .compat_ioctl = hl_compat_ioctl,
#endif
};

/* api helper routines */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
#define resched_if_needed()        \
    do                             \
    {                              \
        if (current->need_resched) \
            schedule();            \
    } while (0)
#else
#define resched_if_needed() \
    do                      \
    {                       \
        if (need_resched()) \
            schedule();     \
    } while (0)
#endif

void LNX_REGPARM0 check_sched(void)
{
    resched_if_needed();
}

/* us delay (no upper limit guaranteed) */
#define DELAY_CMP (1000000L / HZ)
void LNX_REGPARM1 lnx_us_delay(aks_u32_t micro)
{
    unsigned long ticks;

    if (micro < DELAY_CMP / 2)
    {
        udelay(micro);
    }
    else
    {
        /* calc # of timer ticks to wait */
        ticks = (micro + DELAY_CMP / 2) / DELAY_CMP;
        if (ticks < 2)
            ticks = 2;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))
        current->state = TASK_INTERRUPTIBLE;
#else
        // Rel. commit "sched: Change task_struct::state" (Peter Zijlstra, Jun 11 2021)
        WRITE_ONCE(current->__state, TASK_INTERRUPTIBLE);
#endif
        schedule_timeout(ticks);
    }
}

char copyright_notice[] = "*******\n"
                          "*** Linux Hardlock E-Y-E / HASP driver v" VERSION_ASC
                          " (parport support"
                          " with timeout"
#ifdef CPC_PCI
                          ", CPC support"
#endif
                          ")"
                          "\n"
                          "*** Driver   (c) copyright 1994-2001 by Christian Groessler\n"
                          "***          (c) copyright 2001-2007 by Aladdin Knowledge Systems, Ltd.\n"
                          "*** EYE-API  (c) copyright 1993-1997 by ///FAST Software Security AG\n"
                          "***          (c) copyright 1997-2007 by Aladdin Knowledge Systems, Ltd.\n"
                          "*** HASP-API (c) copyright 2000-2007 by Aladdin Knowledge Systems, Ltd.\n"
                          "*** all rights reserved\n"
                          "*******\n";

static signed long use_timeout = 0;
unsigned int timeout = 100; /* in 10ms, 0 - no timeout */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 10)
MODULE_PARM(hlportaddress, "1-" __MODULE_STRING(MAX_HL_PORTS) "l");
MODULE_PARM(isacpcaddress, "1-" __MODULE_STRING(MAX_ISA_CPC_CARDS) "l");
MODULE_PARM(timeout, "i");
#else
module_param_array(hlportaddress, uint, &num_hlports, 0);
MODULE_PARM_DESC(hlportaddress, "addresses non-parport parallel ports (max. " __MODULE_STRING(MAX_HL_PORTS) ")");
module_param_array(isacpcaddress, uint, &num_isacpc, 0);
MODULE_PARM_DESC(isacpcaddress, "port addresses of ISA CPC card (max. " __MODULE_STRING(MAX_ISA_CPC_CARDS) ")");
module_param(timeout, uint, 0);
MODULE_PARM_DESC(timeout, "timeout in 10ms units"); /* @@@ */
#endif

static int hl_preempt(void *handle)
{
    return 1;
}

static int find_port_index(unsigned long port)
{
    int i;

    for (i = 0; i < hl_numports; i++)
        if (hl_pports[i].port == port)
            return i;
    return -1;
}

static int chk_valid_port(aks_u16_t port)
{
    if (find_port_index(port) != -1)
        return TRUE;
    return FALSE;
}

#if defined(__x86_64__) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)
static int reg_apicall;
/*static int reg_getver;*/
static int reg_getports;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
static int hl_init_module(void)
#else
int init_module(void)
#endif
{
#if defined(__x86_64__) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)
    int ret;
#endif
    unsigned int i;
    struct parport *port;
#if LINUX_VERSION_CODE >= 0x20600 || defined(CPC_PCI)
    struct resource *reghelp;
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
    unsigned int flag;
#endif
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 4, 0))
    struct pardev_cb hl_cb;
#endif

    if (sizeof(HL_API) != 0x100)
    {
        printk(KERN_INFO "%s: internal compile error: sizeof(HL_API) != 0x100\n", hardlock);
        return -EIO;
    }

    if (register_chrdev(major, hardlock, &hl_fops))
    {
        printk(KERN_INFO "%s: unable to register Aladdin driver as major device %d\n", hardlock, major);
        return -EIO;
    }

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 4, 0))
    memset(&hl_cb, 0, sizeof(hl_cb));
    hl_cb.private = &hl_pports[hl_numports];
    hl_cb.preempt = hl_preempt;
    hl_cb.wakeup = (void (*)(void *))hl_wakeup;
#endif

    /*
     * get parport ports
     */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
    for (port = parport_enumerate(); port; port = port->next)
    {
#else
    for (i = 0, port = parport_find_number(i); port && i < PARPORT_MAX; port = parport_find_number(++i))
    {
#endif

        init_waitqueue_head(&hl_pports[hl_numports].wait_q);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 4, 0))
        hl_pports[hl_numports].pdev = parport_register_dev_model(port, hardlock, &hl_cb, 0);
#else
        hl_pports[hl_numports].pdev = parport_register_device(port, hardlock, hl_preempt,
                                                              (void (*)(void *))hl_wakeup, NULL, 0,
                                                              &hl_pports[hl_numports]);
#endif

        if (!hl_pports[hl_numports].pdev)
        {
            if (loglevel)
                printk(KERN_INFO "%s: failed to register port %08lx from parport\n", hardlock, port->base);
            continue;
        }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
        printk(KERN_INFO "%s: found parport %u at %04lX\n", hardlock, i, (unsigned long)port->base);
#endif

        hl_pports[hl_numports].port = port->base;
        hl_pports[hl_numports].flags = HLPF_PARPORT | (ASP_PARALLEL << 16);
        hl_numports++;
        if (hl_numports >= MAX_HL_PORTS)
            break;
    }

#ifdef CPC_PCI
    /*
     * get PCI CPC ports
     */
    if (hl_numports < MAX_HL_PORTS)
    {
        unsigned long io, eio;
        struct pci_dev *pcidev = NULL;
        int err;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
        while ((pcidev = pci_get_device(PCI_VENDOR_ID_AKS, PCI_DEVICE_ID_AKS_CPC, pcidev)))
        {
#else
        while ((pcidev = pci_find_device(PCI_VENDOR_ID_AKS, PCI_DEVICE_ID_AKS_CPC, pcidev)))
        {
#endif
            if ((err = pci_enable_device(pcidev)))
            {
                printk(KERN_INFO "%s: cannot enable PCI CPC: %d\n", hardlock, err);
                continue;
            }
            hl_pports[hl_numports].pcidev = pcidev;

            io = pci_resource_start(pcidev, 0);
            eio = pci_resource_end(pcidev, 0);
            hl_pports[hl_numports].CPC_BAR[0] = io;
            hl_pports[hl_numports].CPC_BAR_LEN[0] = eio - io + 1;
            if (loglevel >= 3)
                printk(KERN_INFO "%s: CPC IO port at %04lX,%04lX\n", hardlock, io, eio);

            io = pci_resource_start(pcidev, 1);
            eio = pci_resource_end(pcidev, 1);
            hl_pports[hl_numports].CPC_BAR[1] = io;
            hl_pports[hl_numports].CPC_BAR_LEN[1] = eio - io + 1;

            io = pci_resource_start(pcidev, 2);
            eio = pci_resource_end(pcidev, 2);
            hl_pports[hl_numports].CPC_BAR[2] = io;
            hl_pports[hl_numports].CPC_BAR_LEN[2] = eio - io + 1;

            if (loglevel >= 3)
            {
                printk(KERN_INFO "%s: pcidev = %p\n", hardlock, pcidev);
                for (i = 0; i < 3; i++)
                {
                    printk(KERN_INFO "%s: BAR[%d].start = 0x%lx, BAR[%d].len = 0x%lx\n",
                           hardlock, i,
                           hl_pports[hl_numports].CPC_BAR[i], i,
                           hl_pports[hl_numports].CPC_BAR_LEN[i]);
                }
            }

            /* register BAR 0 area */
            reghelp = request_region(hl_pports[hl_numports].CPC_BAR[0],
                                     hl_pports[hl_numports].CPC_BAR_LEN[0],
                                     hardlock); /* try to allocate the region */
            if (!reghelp)
            {
                printk(KERN_INFO "%s: cannot allocate io area (%lx,%lx) - ignoring card\n",
                       hardlock,
                       hl_pports[hl_numports].CPC_BAR[0],
                       hl_pports[hl_numports].CPC_BAR_LEN[0]);
                continue;
            }

            /* register BAR 1 area */
            reghelp = request_region(hl_pports[hl_numports].CPC_BAR[1],
                                     hl_pports[hl_numports].CPC_BAR_LEN[1],
                                     hardlock); /* try to allocate the region */
            if (!reghelp)
            {
                printk(KERN_INFO "%s: cannot allocate io area (%lx,%lx) - ignoring card\n",
                       hardlock,
                       hl_pports[hl_numports].CPC_BAR[1],
                       hl_pports[hl_numports].CPC_BAR_LEN[1]);
                continue;
            }

            /* register BAR 2 area */
            reghelp = request_region(hl_pports[hl_numports].CPC_BAR[2],
                                     hl_pports[hl_numports].CPC_BAR_LEN[2],
                                     hardlock); /* try to allocate the region */
            if (!reghelp)
            {
                printk(KERN_INFO "%s: cannot allocate io area (%lx,%lx) - ignoring card\n",
                       hardlock,
                       hl_pports[hl_numports].CPC_BAR[2],
                       hl_pports[hl_numports].CPC_BAR_LEN[2]);
                continue;
            }

            hl_pports[hl_numports].port = hl_pports[hl_numports].CPC_BAR[0];
            hl_pports[hl_numports].len = hl_pports[hl_numports].CPC_BAR_LEN[0];
            hl_pports[hl_numports].flags = HLPF_REQUED | ((ASP_PARALLEL | ASP_CPC_PCI) << 16);
            hl_numports++;
            if (hl_numports >= MAX_HL_PORTS)
                break;
        }
    }
#endif

    /*
     * get user supplied ports
     */
    if (hl_numports < MAX_HL_PORTS)
    {
        for (i = 0; i < MAX_HL_PORTS; i++)
        {
            if (hlportaddress[i])
            { /* user specified a port address */
                if (find_port_index(hlportaddress[i]) != -1)
                    continue; /* already used */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
                reghelp = request_region(hlportaddress[i], 3, hardlock); /* try to allocate the region */
                if (reghelp)
                {
                    init_port(hlportaddress[i]); /* initialize port (needed for HL power) */
                    hl_pports[hl_numports].len = 3;
                    hl_pports[hl_numports].flags = HLPF_REQUED | (ASP_PARALLEL << 16);
                }
                else if (!force)
                {
                    printk(KERN_INFO "%s: cannot install: port 0x%04X already in use by another driver.\n"
                           /*KERN_INFO "%s: You may set `force' to override.\n"*/,
                           hardlock, hlportaddress[i] /*, hardlock*/);
                    cleanup_module();
                    return -EIO;
                }
                hl_pports[hl_numports].port = hlportaddress[i];
                hl_numports++;
                if (hl_numports >= MAX_HL_PORTS)
                    break;
#else
                flag = FALSE;
                if (init_chk_port(hlportaddress[i]) || (flag = TRUE, force))
                {
                    if (!flag)
                    { /* 'force' override used, do not register this port */
                        request_region(hlportaddress[i], 3, hardlock);
                        init_port(hlportaddress[i]); /* initialize port (needed for HL power) */
                        hl_pports[hl_numports].len = 3;
                        hl_pports[hl_numports].flags = HLPF_REQUED | (ASP_PARALLEL << 16);
                    }
                    hl_pports[hl_numports].port = hlportaddress[i];
                    hl_numports++;
                    if (hl_numports >= MAX_HL_PORTS)
                        break;
                }
                else
                {
                    printk(KERN_INFO "%s: cannot install: port 0x%04X already in use by another driver.\n" KERN_INFO "%s: You may set `force' to override.\n", hardlock, hlportaddress[i], hardlock);
                    cleanup_module();
                    return -EIO;
                }
#endif
            }
        }
    }

    /*
     * get user supplied ISA CPC ports
     */
    if (hl_numports < MAX_HL_PORTS)
    {
        for (i = 0; i < MAX_ISA_CPC_CARDS; i++)
        {
            if (isacpcaddress[i])
            { /* user specified a port address */
                if (find_port_index(isacpcaddress[i]) != -1)
                    continue; /* already used */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
                reghelp = request_region(isacpcaddress[i], 3, hardlock); /* try to allocate the region */
                if (reghelp)
                {
                    init_port(isacpcaddress[i]); /* initialize port (needed for HL power) */
                    hl_pports[hl_numports].len = 3;
                    hl_pports[hl_numports].flags = HLPF_REQUED | ((ASP_PARALLEL | ASP_CPC) << 16);
                }
                else if (!force)
                {
                    printk(KERN_INFO "%s: cannot install: port 0x%04X already in use by another driver.\n"
                           /*KERN_INFO "%s: You may set `force' to override.\n"*/,
                           hardlock, isacpcaddress[i] /*, hardlock*/);
                    cleanup_module();
                    return -EIO;
                }
                hl_pports[hl_numports].port = isacpcaddress[i];
                hl_numports++;
                if (hl_numports >= MAX_HL_PORTS)
                    break;
#else
                flag = FALSE;
                if (init_chk_port(isacpcaddress[i]) || (flag = TRUE, force))
                {
                    if (!flag)
                    { /* 'force' override used, do not register this port */
                        request_region(isacpcaddress[i], 3, hardlock);
                        init_port(isacpcaddress[i]); /* initialize port (needed for HL power) */
                        hl_pports[hl_numports].len = 3;
                        hl_pports[hl_numports].flags = HLPF_REQUED | ((ASP_PARALLEL | ASP_CPC) << 16);
                    }
                    hl_pports[hl_numports].port = isacpcaddress[i];
                    hl_numports++;
                    if (hl_numports >= MAX_HL_PORTS)
                        break;
                }
                else
                {
                    printk(KERN_INFO "%s: cannot install: port 0x%04X already in use by another driver.\n" KERN_INFO "%s: You may set `force' to override.\n", hardlock, isacpcaddress[i], hardlock);
                    cleanup_module();
                    return -EIO;
                }
#endif
            }
        }
    }

    if (!hl_numports)
    {
        printk(KERN_INFO "%s: fatal: no ports found, refusing to install...\n", hardlock);
        cleanup_module();
        return -EIO;
    }
    else
    {
        if (loglevel >= 2)
            printk(KERN_INFO "%s: using %d ports\n", hardlock, hl_numports);
    }
    use_timeout = timeout * HZ / 100; /* 10 ms entries */

#if defined(__x86_64__) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)
    ret = register_ioctl32_conversion(HLAPI_CALL, hl_ioctl32);
    if (ret)
    {
        printk(KERN_INFO "%s: cannot register ioctl32 0x%x\n", hardlock, HLAPI_CALL);
    }
    else
        reg_apicall = 1;

#if 0
    ret = register_ioctl32_conversion(HL_GETVER, NULL);
    if (ret) {
        printk(KERN_INFO "%s: cannot register ioctl32 0x%x\n", hardlock, HL_GETVER);
    }
    else
        reg_getver = 1;
#endif

    ret = register_ioctl32_conversion(HL_GETPORTS, NULL);
    if (ret)
    {
        printk(KERN_INFO "%s: cannot register ioctl32 0x%x\n", hardlock, HL_GETPORTS);
    }
    else
        reg_getports = 1;
#endif

    printk(KERN_INFO "%s: EYE/HASP driver v%01d.%02d%s/API v%01d.%02d/major %02d loaded (pp"
                     "i"
                     ")\n",
           hardlock, VERSION_HIGH, VERSION_LOW, VERSION_BETA ? "beta" : "",
           __exported_API_version__ >> 8, __exported_API_version__ & 0xff, major);
    if (loglevel >= 2)
    {
        printk(KERN_INFO "%s: resulting port table: (%d entries)\n", hardlock, hl_numports);
        for (i = 0; i < hl_numports; i++)
        {
            printk(KERN_INFO "%s: port %04lX -- %s port\n", hardlock, hl_pports[i].port,
                   hl_pports[i].flags & HLPF_REQUED ? "own" : "parport");
        }
    }

    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
static void hl_cleanup_module(void)
#else
void cleanup_module(void)
#endif
{
    int i;

    for (i = 0; i < hl_numports; i++)
    {
        if (hl_pports[i].flags & HLPF_PARPORT)
        {
            parport_unregister_device(hl_pports[i].pdev);
            continue;
        }
        if (hl_pports[i].flags & HLPF_REQUED)
        {
            release_region(hl_pports[i].port, hl_pports[i].len);
#ifdef CPC_PCI
            if (hl_pports[i].flags & (ASP_CPC_PCI << 16))
            {
                release_region(hl_pports[i].CPC_BAR[1], hl_pports[i].CPC_BAR_LEN[1]);
                release_region(hl_pports[i].CPC_BAR[2], hl_pports[i].CPC_BAR_LEN[2]);
                pci_disable_device(hl_pports[i].pcidev);
            }
#endif
        }
    }

#if defined(__x86_64__) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)
    if (reg_apicall)
        unregister_ioctl32_conversion(HLAPI_CALL);
#if 0
    if (reg_getver)
        unregister_ioctl32_conversion(HL_GETVER);
#endif
    if (reg_getports)
        unregister_ioctl32_conversion(HL_GETPORTS);
#endif

    printk(KERN_INFO "%s: EYE/HASP driver unloaded\n", hardlock);
    unregister_chrdev(major, hardlock);
}

/* initialize parallel port */

static void init_port(aks_u16_t port)
{
    outb_p(0x0c, port + 2); /* set init + select lines */
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
static int init_chk_port(unsigned int port) /* returns TRUE if port is OK, else FALSE */
{
    if (check_region(port, 3) < 0)
        return FALSE;
    return TRUE;
}
#endif

/*
 * Handle an API call
 */
static aks_u16_t call_api(HL_API *api_struc)
{
    int i;
    aks_u16_t oldport, oldrem;

    if (api_struc->Port)
        return call_api_nowildport(api_struc);
    if (api_struc->Function != API_INIT && api_struc->Function != 300)
        return api_struc->Status = INVALID_PARAM;

    oldport = api_struc->Port;
    oldrem = api_struc->Remote;
    if (!(oldrem & LOCAL_DEVICE))
        return api_struc->Status = NO_ACCESS;
    api_struc->Remote = LOCAL_DEVICE;

    for (i = 0; i < hl_numports; i++)
    {
        api_struc->Port = (aks_u16_t)hl_pports[i].port;
        if (call_api_nowildport(api_struc) == STATUS_OK)
            return STATUS_OK;
    }

    api_struc->Port = oldport;
    api_struc->Remote = oldrem;
    return api_struc->Status = NO_DONGLE;
}

static void fix_wait_queue(struct pardevice *dev)
{
    unsigned long flags;

#ifdef HL_DEBUG
    if (loglevel >= 3)
        printk(KERN_DEBUG "%s: fix_wait_queue() called\n", hardlock);
#endif

    if (dev->waiting & 1)
    {
        dev->waiting = 0;

#ifdef HL_DEBUG
        if (loglevel >= 3)
            printk(KERN_DEBUG "%s: fix_wait_queue() does its work\n", hardlock);
#endif

        spin_lock_irqsave(&dev->port->waitlist_lock, flags);
        if (dev->waitprev)
            dev->waitprev->waitnext = dev->waitnext;
        else
            dev->port->waithead = dev->waitnext;
        if (dev->waitnext)
            dev->waitnext->waitprev = dev->waitprev;
        else
            dev->port->waittail = dev->waitprev;
        spin_unlock_irqrestore(&dev->port->waitlist_lock, flags);
        dev->waitprev = NULL;
        dev->waitnext = NULL;
    }
}

static aks_u16_t call_api_nowildport(HL_API *api_struc)
{
    aks_u16_t oldportflags, retval;
#ifdef CPC_PCI
    DEP_STUFF dep_stuff = {NULL};
#endif
    int pi; /* port index */
    int res;
    signed long timeout = use_timeout;

    if (api_struc->PortFlags & ASP_SERIAL) /* HL-Twin not supported */
        return api_struc->Status = NO_DONGLE;

    if (!chk_valid_port(api_struc->Port))
    {
        if (loglevel)
            printk(KERN_INFO "%s: pid %d tried access on invalid port 0x%04x\n",
                   hardlock, current->pid, api_struc->Port);
        return api_struc->Status = NO_DONGLE;
    }

    oldportflags = api_struc->PortFlags;

    pi = find_port_index(api_struc->Port);
    if (pi != -1)
    {
#ifdef CPC_PCI
        dep_stuff.portStruct = &hl_pports[pi];
#endif

        if (hl_pports[pi].flags & HLPF_PARPORT)
        { /* do nothing on own ports */
            res = hl_parport_claim_or_block(&hl_pports[pi], &timeout);
            if (signal_pending(current))
            {
                if (loglevel >= 2)
                    printk(KERN_INFO "%s: signal pending, aborting wait for port %04lX\n",
                           hardlock, hl_pports[pi].port);
                fix_wait_queue(hl_pports[pi].pdev);
                return api_struc->Status = PORT_BUSY;
            }
            if (use_timeout && !timeout)
            {
                if (loglevel >= 2)
                    printk(KERN_INFO "%s: timed out waiting for port %04lX\n",
                           hardlock, hl_pports[pi].port);
                fix_wait_queue(hl_pports[pi].pdev);
                return api_struc->Status = PORT_BUSY;
            }
            if (res < 0)
            {
                if (loglevel >= 2)
                    printk(KERN_INFO "%s: parport_claim returned %d\n",
                           hardlock, res);
                fix_wait_queue(hl_pports[pi].pdev);
                return api_struc->Status = 5; /* catchall special error code */
            }
#ifdef HL_DEBUG
            if (loglevel >= 2)
                printk(KERN_INFO "%s: parport_claim success: res = %d\n",
                       hardlock, res);
#endif
            hl_pports[pi].pdev->waiting = 0;
        }
        else
            pi = -1;
#ifdef HL_DEBUG
        if (loglevel >= 3)
            printk(KERN_DEBUG "%s: port found, index = %d\n", hardlock, pi);
#endif
    }
#ifdef HL_DEBUG
    else if (loglevel >= 2)
        printk(KERN_DEBUG "%s: cannot find port again! (0x%04lX)\n",
               hardlock, (unsigned long)api_struc->Port);
#endif

#ifdef CPC_PCI
    set_assysdep_ptr(api_struc, &dep_stuff);
#endif
    init_port(api_struc->Port); /* 05-Feb-2001: then it works on Alpha internal port */
    retval = API_SERVICE(api_struc);

    if (pi != -1)
        parport_release(hl_pports[pi].pdev);

    api_struc->PortFlags = oldportflags;
    return retval;
}

/*
 * callback routine when port is available for us
 */
static void hl_wakeup(struct _porttabentry *port)
{
    if (waitqueue_active(&port->wait_q))
    {
        parport_claim(port->pdev);
        wake_up_interruptible(&port->wait_q);
    }
}

/*
 * acquire port from parport system
 */
static int hl_parport_claim_or_block(struct _porttabentry *port, signed long *timeout)
{
    int r;
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 15, 0)
    DEFINE_WAIT(wait);
#endif

    r = parport_claim(port->pdev);
    if (r == -EAGAIN)
    {
        if (*timeout)
        {
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 15, 0)
            prepare_to_wait(&port->wait_q, &wait, TASK_INTERRUPTIBLE);
            *timeout = schedule_timeout(*timeout);
            finish_wait(&port->wait_q, &wait);
#else
            *timeout = interruptible_sleep_on_timeout(&port->wait_q, *timeout);
#endif
        }
        else
        {
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 15, 0)
            prepare_to_wait(&port->wait_q, &wait, TASK_UNINTERRUPTIBLE);
            schedule();
            finish_wait(&port->wait_q, &wait);
#else
            interruptible_sleep_on(&port->wait_q);
#endif
        }
        return 0;
    }
    return r;
}

/*
 * API callback to get current jiffies count
 */
unsigned long LNX_REGPARM0 lnx_jiffies(void)
{
    return jiffies;
}

/*
 * API callback to get HZ value
 */
unsigned long LNX_REGPARM0 lnx_HZ(void)
{
    return HZ;
}

/*
 * MyDelay API callback (called in critical section)
 */
void LNX_REGPARM1 lnx_mydelay(unsigned int delay)
{
    resched_if_needed();
}

/*
 * ENTER_CRITICAL API callback
 */
void LNX_REGPARM1 lnx_enter_crit(unsigned long port)
{
    resched_if_needed();
}

/*
 * LEAVE_CRITICAL API callback
 */
void LNX_REGPARM0 lnx_leave_crit(unsigned long port)
{
    resched_if_needed();
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
module_init(hl_init_module);
module_exit(hl_cleanup_module);
#endif

/******** end of file ********/
/* Local Variables: */
/* c-file-style: "cpg" */
/* c-basic-offset: 4 */
/* End: */
