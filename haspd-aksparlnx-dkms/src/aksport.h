/* "Copyright (c) 1985-2007 Aladdin Knowledge Systems Ltd." */

typedef struct _porttabentry {
    unsigned long CPC_BAR[4];
    unsigned long CPC_BAR_LEN[4];
    struct pci_dev *pcidev;
    unsigned long port;
    unsigned long len;
    struct pardevice *pdev;
    wait_queue_head_t wait_q;
    unsigned int flags;
} porttabentry;
