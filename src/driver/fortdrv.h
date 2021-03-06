#ifndef FORTDRV_H
#define FORTDRV_H

#include "common/common.h"

/* WDM for Development in User Mode */
#if !defined(FORT_DRIVER)
#    include "wdm/um_wdm.h"
#    include "wdm/um_ndis.h"
#    include "wdm/um_fwpsk.h"
#    include "wdm/um_fwpmk.h"
#endif

#define fort_mem_alloc(size, tag) ExAllocatePoolWithTag(NonPagedPool, (size), (tag))
#define fort_mem_free(p, tag)     ExFreePoolWithTag((p), (tag))

#define fort_request_complete_info(irp, status, info)                                              \
    do {                                                                                           \
        (irp)->IoStatus.Status = (status);                                                         \
        (irp)->IoStatus.Information = (info);                                                      \
        IoCompleteRequest((irp), IO_NO_INCREMENT);                                                 \
    } while (0)

#define fort_request_complete(irp, status) fort_request_complete_info((irp), (status), 0)

#endif // FORTDRV_H
