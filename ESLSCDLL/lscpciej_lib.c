/*****************************************************************//**
 * @file   lscpciej_lib.c
 * @copydoc lscpciej_lib.h
 *********************************************************************/

#include "lscpciej_lib.h"

/* Validate a device handle */
static inline BOOL IsValidDevice(PWDC_DEVICE pDev, const CHAR *sFunc)
{
    if (!pDev || !(PLSCPCIEJ_DEV_CTX)(pDev->pCtx))
    {
        _snprintf_s(gsLSCPCIEJ_LastErr, sizeof(gsLSCPCIEJ_LastErr) - 1, sizeof(gsLSCPCIEJ_LastErr) - 1,
            "%s: NULL device %s\n", sFunc, !pDev ? "handle" : "context");
        ErrLog(gsLSCPCIEJ_LastErr);
        return FALSE;
    }

    return TRUE;
}

/*************************************************************
  Functions implementation
 *************************************************************/
/* -----------------------------------------------
    LSCPCIEJ and WDC libraries initialize/uninitialize
   ----------------------------------------------- */
/* Initialize the LSCPCIEJ and WDC libraries */
DWORD LSCPCIEJ_LibInit(void)
{
    DWORD dwStatus;

    /* Increase the library's reference count; initialize the library only once
     */
    if (++LibInit_count > 1)
        return WD_STATUS_SUCCESS;

#ifdef WD_DRIVER_NAME_CHANGE
    /* Set the driver name */
    if (!WD_DriverName(LSCPCIEJ_DEFAULT_DRIVER_NAME))
    {
        ErrLog("Failed to set the driver name for WDC library.\n");
        return WD_SYSTEM_INTERNAL_ERROR;
    }
#endif

    /* Set WDC library's debug options
     * (default: level=TRACE; redirect output to the Debug Monitor) */
    dwStatus = WDC_SetDebugOptions(WDC_DBG_DEFAULT, NULL);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        ErrLog("Failed to initialize debug options for WDC library.\n"
            "Error 0x%lx - %s\n", dwStatus, Stat2Str(dwStatus));
        return dwStatus;
    }

    /* Open a handle to the driver and initialize the WDC library */
    dwStatus = WDC_DriverOpen(WDC_DRV_OPEN_DEFAULT, LSCPCIEJ_DEFAULT_LICENSE_STRING);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        ErrLog("Failed to initialize the WDC library. Error 0x%lx - %s\n",
            dwStatus, Stat2Str(dwStatus));
        return dwStatus;
    }

    return WD_STATUS_SUCCESS;
}

/* Uninitialize the LSCPCIEJ and WDC libraries */
DWORD LSCPCIEJ_LibUninit(void)
{
    DWORD dwStatus;

    /* Decrease the library's reference count; uninitialize the library only
     * when there are no more open handles to the library */
    if (--LibInit_count > 0)
        return WD_STATUS_SUCCESS;

    /* Uninitialize the WDC library and close the handle to WinDriver */
    dwStatus = WDC_DriverClose();
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        ErrLog("Failed to uninit the WDC library. Error 0x%lx - %s\n",
            dwStatus, Stat2Str(dwStatus));
    }

    return dwStatus;
}

/* -----------------------------------------------
    Device open/close
   ----------------------------------------------- */
/* Open a device handle */
WDC_DEVICE_HANDLE LSCPCIEJ_DeviceOpen(const WD_PCI_CARD_INFO *pDeviceInfo)
{
    DWORD dwStatus;
    PLSCPCIEJ_DEV_CTX pDevCtx = NULL;
    WDC_DEVICE_HANDLE hDev = NULL;
    /* Validate arguments */
    if (!pDeviceInfo)
    {
        ErrLog("LSCPCIEJ_DeviceOpen: Error - NULL device information "
            "struct pointer\n");
        return NULL;
    }
    /* Allocate memory for the PCI device context */
    pDevCtx = (PLSCPCIEJ_DEV_CTX)malloc(sizeof(LSCPCIEJ_DEV_CTX));
    if (!pDevCtx)
    {
        ErrLog("Failed allocating memory for PCI device context\n");
        return NULL;
    }

    BZERO(*pDevCtx);

    /* Open a device handle */
    dwStatus = WDC_PciDeviceOpen(&hDev, pDeviceInfo, pDevCtx);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        ErrLog("Failed opening a WDC device handle. Error 0x%lx - %s\n",
            dwStatus, Stat2Str(dwStatus));
        goto Error;
    }
    /* Validate device information */
    if (!DeviceValidate((PWDC_DEVICE)hDev))
        goto Error;
    /* Return handle to the new device */
    TraceLog("LSCPCIEJ_DeviceOpen: Opened a PCI device (handle 0x%p)\n"
        "Device is %s using a Kernel PlugIn driver (%s)\n", hDev,
        (WDC_IS_KP(hDev))? "" : "not" , KP_LSCPCIEJ_DRIVER_NAME);
    return hDev;

Error:
    if (hDev)
        LSCPCIEJ_DeviceClose(hDev);
    else
        free(pDevCtx);

    return NULL;
}

/* Close device handle */
BOOL LSCPCIEJ_DeviceClose(WDC_DEVICE_HANDLE hDev)
{
    DWORD dwStatus;
    PLSCPCIEJ_DEV_CTX pDevCtx;

    TraceLog("LSCPCIEJ_DeviceClose: Entered. Device handle 0x%p\n", hDev);

    /* Validate the device handle */
    if (!hDev)
    {
        ErrLog("LSCPCIEJ_DeviceClose: Error - NULL device handle\n");
        return FALSE;
    }

    pDevCtx = (PLSCPCIEJ_DEV_CTX)WDC_GetDevContext(hDev);

    /* Disable interrupts (if enabled) */
    if (WDC_IntIsEnabled(hDev))
    {
        dwStatus = LSCPCIEJ_IntDisable(hDev);
        if (WD_STATUS_SUCCESS != dwStatus)
        {
            ErrLog("Failed disabling interrupts. Error 0x%lx - %s\n", dwStatus,
                Stat2Str(dwStatus));
        }
    }

    /* Close the device handle */
    dwStatus = WDC_PciDeviceClose(hDev);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        ErrLog("Failed closing a WDC device handle (0x%p). Error 0x%lx - %s\n",
            hDev, dwStatus, Stat2Str(dwStatus));
    }

    /* Free PCI device context memory */
    if (pDevCtx)
        free(pDevCtx);

    return (WD_STATUS_SUCCESS == dwStatus);
}

/* Validate device information */
static BOOL DeviceValidate(const PWDC_DEVICE pDev)
{
    DWORD i, dwNumAddrSpaces = pDev->dwNumAddrSpaces;

    /* NOTE: You can modify the implementation of this function in order to
             verify that the device has the resources you expect to find. */

    /* Verify that the device has at least one active address space */
    for (i = 0; i < dwNumAddrSpaces; i++)
    {
        if (WDC_AddrSpaceIsActive(pDev, i))
            return TRUE;
    }

    /* In this sample we accept the device even if it doesn't have any
     * address spaces */
    TraceLog("Device does not have any active memory or I/O address spaces\n");
    return TRUE;
}

/* -----------------------------------------------
    Interrupts
   ----------------------------------------------- */
/* Interrupt handler routine */
static void DLLCALLCONV LSCPCIEJ_IntHandler(PVOID pData)
{
    PWDC_DEVICE pDev = (PWDC_DEVICE)pData;
    PLSCPCIEJ_DEV_CTX pDevCtx = (PLSCPCIEJ_DEV_CTX)(pDev->pCtx);
    LSCPCIEJ_INT_RESULT intResult;

    BZERO(intResult);
    intResult.dwCounter = pDev->Int.dwCounter;
    intResult.dwLost = pDev->Int.dwLost;
    intResult.waitResult = (WD_INTERRUPT_WAIT_RESULT)pDev->Int.fStopped;
    intResult.dwEnabledIntType = WDC_GET_ENABLED_INT_TYPE(pDev);
    intResult.dwLastMessage = WDC_GET_ENABLED_INT_LAST_MSG(pDev);

    /* Execute the diagnostics application's interrupt handler routine */
    pDevCtx->funcDiagIntHandler((WDC_DEVICE_HANDLE)pDev, &intResult);
}

/* Check whether a given device contains an item of the specified type */
static BOOL IsItemExists(PWDC_DEVICE pDev, ITEM_TYPE item)
{
    DWORD i, dwNumItems = pDev->cardReg.Card.dwItems;

    for (i = 0; i < dwNumItems; i++)
    {
        if ((ITEM_TYPE)(pDev->cardReg.Card.Item[i].item) == item)
            return TRUE;
    }

    return FALSE;
}

/* Enable interrupts */
DWORD LSCPCIEJ_IntEnable(WDC_DEVICE_HANDLE hDev, LSCPCIEJ_INT_HANDLER funcIntHandler)
{
    DWORD dwStatus;
    PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
    PLSCPCIEJ_DEV_CTX pDevCtx;

    

    TraceLog("LSCPCIEJ_IntEnable: Entered. Device handle 0x%p\n", hDev);

    /* Validate the device handle */
    if (!IsValidDevice(pDev, "LSCPCIEJ_IntEnable"))
        return WD_INVALID_PARAMETER;

    /* Verify that the device has an interrupt item */
    if (!IsItemExists(pDev, ITEM_INTERRUPT))
        return WD_OPERATION_FAILED;

    pDevCtx = (PLSCPCIEJ_DEV_CTX)(pDev->pCtx);

    /* Check whether interrupts are already enabled */
    if (WDC_IntIsEnabled(hDev))
    {
        ErrLog("Interrupts are already enabled ...\n");
        return WD_OPERATION_ALREADY_DONE;
    }

    /* Define the number of interrupt transfer commands to use */
    #define NUM_TRANS_CMDS 0
    /* NOTE: In order to correctly handle PCI interrupts, you need to
             ADD CODE HERE to set up transfer commands to read/write the
             relevant register(s) in order to correctly acknowledge the
             interrupts, as dictated by your hardware's specifications.
             When adding transfer commands, be sure to also modify the
             definition of NUM_TRANS_CMDS (above) accordingly. */

    /* Store the diag interrupt handler routine, which will be executed by
       LSCPCIEJ_IntHandler() when an interrupt is received */
    pDevCtx->funcDiagIntHandler = funcIntHandler;

    /* Enable the interrupts */
    /* NOTE: When adding read transfer commands, set the INTERRUPT_CMD_COPY flag
             in the 4th argument (dwOptions) passed to WDC_IntEnable() */
    dwStatus = WDC_IntEnable(hDev, NULL, 0, 0,
        LSCPCIEJ_IntHandler, (PVOID)pDev, WDC_IS_KP(hDev));

    if (WD_STATUS_SUCCESS != dwStatus)
    {
        ErrLog("Failed enabling interrupts. Error 0x%lx - %s\n",
            dwStatus, Stat2Str(dwStatus));

        return dwStatus;
    }
        
    /* TODO: You can add code here to write to the device in order to
             physically enable the hardware interrupts. */

    TraceLog("LSCPCIEJ_IntEnable: Interrupts enabled\n");

    return WD_STATUS_SUCCESS;
}

/* Disable interrupts */
DWORD LSCPCIEJ_IntDisable(WDC_DEVICE_HANDLE hDev)
{
    DWORD dwStatus;
    PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
    PLSCPCIEJ_DEV_CTX pDevCtx;

    TraceLog("LSCPCIEJ_IntDisable entered. Device handle 0x%p\n", hDev);

    /* Validate the device handle */
    if (!IsValidDevice(pDev, "LSCPCIEJ_IntDisable"))
        return WD_INVALID_PARAMETER;

    pDevCtx = (PLSCPCIEJ_DEV_CTX)(pDev->pCtx);

    /* Check whether interrupts are already enabled */
    if (!WDC_IntIsEnabled(hDev))
    {
        ErrLog("Interrupts are already disabled ...\n");
        return WD_OPERATION_ALREADY_DONE;
    }

    /* TODO: You can add code here to write to the device in order to
             physically disable the hardware interrupts. */

    /* Disable interrupts */
    dwStatus = WDC_IntDisable(hDev);
    if (WD_STATUS_SUCCESS != dwStatus)
    {
        ErrLog("Failed disabling interrupts. Error 0x%lx - %s\n",
            dwStatus, Stat2Str(dwStatus));
    }

    return dwStatus;
}

/* Check whether interrupts are enabled for the given device */
BOOL LSCPCIEJ_IntIsEnabled(WDC_DEVICE_HANDLE hDev)
{
    /* Validate the device handle */
    if (!IsValidDevice((PWDC_DEVICE)hDev, "LSCPCIEJ_IntIsEnabled"))
        return FALSE;

    /* Check whether interrupts are already enabled */
    return WDC_IntIsEnabled(hDev);
}

/* -----------------------------------------------
    Plug-and-play and power management events
   ----------------------------------------------- */
/* Plug-and-play or power management event handler routine */
static void LSCPCIEJ_EventHandler(WD_EVENT *pEvent, PVOID pData)
{
    PWDC_DEVICE pDev = (PWDC_DEVICE)pData;
    PLSCPCIEJ_DEV_CTX pDevCtx = (PLSCPCIEJ_DEV_CTX)(pDev->pCtx);

    TraceLog("LSCPCIEJ_EventHandler entered, pData 0x%p, dwAction 0x%lx\n", pData,
        pEvent->dwAction);

    /* Execute the diagnostics application's event handler function */
    pDevCtx->funcDiagEventHandler((WDC_DEVICE_HANDLE)pDev, pEvent->dwAction);
}

/* Register a plug-and-play or power management event */
DWORD LSCPCIEJ_EventRegister(WDC_DEVICE_HANDLE hDev,
    LSCPCIEJ_EVENT_HANDLER funcEventHandler)
{
    DWORD dwStatus;
    PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
    PLSCPCIEJ_DEV_CTX pDevCtx;
    DWORD dwActions = WD_ACTIONS_ALL;
    /* TODO: Modify the above to set up the plug-and-play/power management
             events for which you wish to receive notifications.
             dwActions can be set to any combination of the WD_EVENT_ACTION
             flags defined in windrvr.h. */

    TraceLog("LSCPCIEJ_EventRegister entered. Device handle 0x%p\n", hDev);

    /* Validate the device handle */
    if (!IsValidDevice(pDev, "LSCPCIEJ_EventRegister"))
        return WD_INVALID_PARAMETER;

    pDevCtx = (PLSCPCIEJ_DEV_CTX)(pDev->pCtx);

    /* Check whether the event is already registered */
    if (WDC_EventIsRegistered(hDev))
    {
        ErrLog("Events are already registered ...\n");
        return WD_OPERATION_ALREADY_DONE;
    }

    /* Store the diag event handler routine to be executed from
     * LSCPCIEJ_EventHandler() upon an event */
    pDevCtx->funcDiagEventHandler = funcEventHandler;

    /* Register the event */
    dwStatus = WDC_EventRegister(hDev, dwActions, LSCPCIEJ_EventHandler, hDev,
        WDC_IS_KP(hDev));

    if (WD_STATUS_SUCCESS != dwStatus)
    {
        ErrLog("Failed to register events. Error 0x%lx - %s\n",
            dwStatus, Stat2Str(dwStatus));
        return dwStatus;
    }

    TraceLog("Events registered\n");
    return WD_STATUS_SUCCESS;
}

/* Unregister a plug-and-play or power management event */
DWORD LSCPCIEJ_EventUnregister(WDC_DEVICE_HANDLE hDev)
{
    DWORD dwStatus;

    TraceLog("LSCPCIEJ_EventUnregister entered. Device handle 0x%p\n", hDev);

    /* Validate the device handle */
    if (!IsValidDevice((PWDC_DEVICE)hDev, "LSCPCIEJ_EventUnregister"))
        return WD_INVALID_PARAMETER;

    /* Check whether the event is currently registered */
    if (!WDC_EventIsRegistered(hDev))
    {
        ErrLog("Cannot unregister events - no events currently "
            "registered ...\n");
        return WD_OPERATION_ALREADY_DONE;
    }

    /* Unregister the event */
    dwStatus = WDC_EventUnregister(hDev);

    if (WD_STATUS_SUCCESS != dwStatus)
    {
        ErrLog("Failed to unregister events. Error 0x%lx - %s\n", dwStatus,
            Stat2Str(dwStatus));
    }

    return dwStatus;
}

/* Check whether a given plug-and-play or power management event is registered
 */
BOOL LSCPCIEJ_EventIsRegistered(WDC_DEVICE_HANDLE hDev)
{
    /* Validate the device handle */
    if (!IsValidDevice((PWDC_DEVICE)hDev, "LSCPCIEJ_EventIsRegistered"))
        return FALSE;

    /* Check whether the event is registered */
    return WDC_EventIsRegistered(hDev);
}

/* -----------------------------------------------
    Address spaces information
   ----------------------------------------------- */
/* Get number of address spaces */
DWORD LSCPCIEJ_GetNumAddrSpaces(WDC_DEVICE_HANDLE hDev)
{
    PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;

    /* Validate the device handle */
    if (!IsValidDevice(pDev, "LSCPCIEJ_GetNumAddrSpaces"))
        return 0;

    /* Return the number of address spaces for the device */
    return pDev->dwNumAddrSpaces;
}

/* Get address space information */
BOOL LSCPCIEJ_GetAddrSpaceInfo(WDC_DEVICE_HANDLE hDev,
    LSCPCIEJ_ADDR_SPACE_INFO *pAddrSpaceInfo)
{
    PWDC_DEVICE pDev = (PWDC_DEVICE)hDev;
    WDC_ADDR_DESC *pAddrDesc;
    DWORD dwAddrSpace;
    BOOL fIsMemory;

    if (!IsValidDevice(pDev, "LSCPCIEJ_GetAddrSpaceInfo"))
        return FALSE;

#if defined(DEBUG)
    if (!pAddrSpaceInfo)
    {
        ErrLog("LSCPCIEJ_GetAddrSpaceInfo: Error - NULL address space information pointer\n");
        return FALSE;
    }
#endif

    dwAddrSpace = pAddrSpaceInfo->dwAddrSpace;

    if (dwAddrSpace > pDev->dwNumAddrSpaces - 1)
    {
        ErrLog("LSCPCIEJ_GetAddrSpaceInfo: Error - Address space %ld is "
            "out of range (0 - %ld)\n", dwAddrSpace, pDev->dwNumAddrSpaces - 1);
        return FALSE;
    }

    pAddrDesc = &pDev->pAddrDesc[dwAddrSpace];

    fIsMemory = WDC_ADDR_IS_MEM(pAddrDesc);

    _snprintf_s(pAddrSpaceInfo->sName, MAX_NAME - 1, MAX_NAME - 1, "BAR %ld", dwAddrSpace);
    _snprintf_s(pAddrSpaceInfo->sType, MAX_TYPE - 1, MAX_TYPE - 1, fIsMemory ? "Memory" : "I/O");

    if (WDC_AddrSpaceIsActive(pDev, dwAddrSpace))
    {
        WD_ITEMS *pItem = &pDev->cardReg.Card.Item[pAddrDesc->dwItemIndex];
        PHYS_ADDR pAddr = fIsMemory ? pItem->I.Mem.pPhysicalAddr :
            pItem->I.IO.pAddr;

        _snprintf_s(pAddrSpaceInfo->sDesc, MAX_DESC - 1, MAX_DESC - 1,
            "0x%0*"PRI64"X - 0x%0*"PRI64"X (0x%"PRI64"x bytes)",
            (int)WDC_SIZE_64 * 2, pAddr,
            (int)WDC_SIZE_64 * 2, pAddr + pAddrDesc->qwBytes - 1,
            pAddrDesc->qwBytes);
    }
    else
    {
        _snprintf_s(pAddrSpaceInfo->sDesc, MAX_DESC - 1, MAX_DESC - 1, "Inactive address space");
    }

    /* TODO: You can modify the code above to set a different address space
     * name/description. */

    return TRUE;
}


/* -----------------------------------------------
    Debugging and error handling
   ----------------------------------------------- */
/* Log a debug error message */
void ErrLog(const CHAR *sFormat, ...)
{
    va_list argp;

    va_start(argp, sFormat);
    _vsnprintf_s(gsLSCPCIEJ_LastErr, sizeof(gsLSCPCIEJ_LastErr) - 1, sizeof(gsLSCPCIEJ_LastErr) - 1, sFormat, argp);
#ifdef _DEBUG
        WDC_Err("LSCPCIEJ lib: %s", gsLSCPCIEJ_LastErr);
#endif
    va_end(argp);
}

/* Log a debug trace message */
static void TraceLog(const CHAR *sFormat, ...)
{
#ifdef _DEBUG
    CHAR sMsg[256];
    va_list argp;

    va_start(argp, sFormat);
    _vsnprintf_s(sMsg, sizeof(sMsg) - 1, sizeof(sMsg) - 1, sFormat, argp);
    WDC_Trace("LSCPCIEJ lib: %s", sMsg);
    va_end(argp);
#else
    (void)sFormat;
#endif
}

/* Get last error */
const char *LSCPCIEJ_GetLastErr(void)
{
    return gsLSCPCIEJ_LastErr;
}

