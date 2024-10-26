/* $Id: unix_ioctl.h,v 1.2 2007/06/11 18:15:32 lav Exp $
**
** Unix driver ioctl commands and io definitions
** (c) 2000,2005 by Aladdin Knowledge Systems Ltd.
**
** $Revision: 1.2 $
** $Author: lav $
** $Date: 2007/06/11 18:15:32 $
**
** $Log: unix_ioctl.h,v $
** Revision 1.2  2007/06/11 18:15:32  lav
** updated to 1.7
**
** Revision 1.4  2005/01/21 14:15:16  chris
** fix copyright line
**
** Revision 1.3  2000/09/12 15:39:57  chris
** HL_GETPORTS64: new ioctl code and parameter structs
**
** Revision 1.2  2000/07/18 11:22:15  chris
** improved portability; added HLPF_xxx flags
**
** Revision 1.1  2000/07/18 10:50:52  chris
** Initial revision
*/

#ifndef _UNIX_IOCTL_H_
#define _UNIX_IOCTL_H_

#define DEV  "/dev/Hardlock"     /* driver special file */

#define HL_GETVER     0x01         /* get version of driver    */
#define HL_GETPORTS   0x02fed112   /* get ports used by driver */
#define HL_GETPORTS64 0x02fec112   /* get ports used by driver (64bit version) */
#define HLAPI_CALL    0x10         /* API call to driver       */

/* structure returned from HL_GETPORTS call */

struct alad_portentry {
    aks_u32_t port_address;
    aks_u32_t flags;
};

struct alad_portlist {
    int num_entries;
    int reserved;
    struct alad_portentry list[1];
};

#define alad_portlist_size (sizeof(struct alad_portlist) - sizeof(struct alad_portentry))

#ifdef __64BIT__

/* structure returned from HL_GETPORTS64 call */

struct alad_portentry64 {
    aks_u64_t port_address;
    aks_u64_t flags;
};

struct alad_portlist64 {
    int num_entries;
    int reserved;
    struct alad_portentry64 list[1];
};

#define alad_portlist_size64 (sizeof(struct alad_portlist64) - sizeof(struct alad_portentry64))
#endif /* #ifdef __64BIT__ */

/* definitions for hlportflags[] and/or hl_pports.flags (bitmask, lowest 16 bits) */
/* upper/next 16bits return port type (ASP_xxx) */

#define HLPF_REQUED  1       /* port has been registered via request_region(), Linux only */
#define HLPF_PARPORT 2       /* port is under control of parport, Linux only */

#endif /* #ifndef _UNIX_IOCTL_H_ */
