/* Jungo Connectivity Confidential. Copyright (c) 2019 Jungo Connectivity Ltd.  https://www.jungo.com */

#ifndef _PCI_REGS_H_
#define _PCI_REGS_H_

/*****************************************************************************
* File - pci_regs.h - PCI configuration space and address spaces definitions *
******************************************************************************/

/* -------------------------
 * Register type definitions
 * ------------------------- */

typedef enum
{
    HEADER_TYPE_NORMAL = 0x01,
    HEADER_TYPE_BRIDGE = 0x02,
    HEADER_TYPE_CARDBUS = 0x04,
    HEADER_TYPE_NRML_BRIDGE = HEADER_TYPE_NORMAL | HEADER_TYPE_BRIDGE,
    HEADER_TYPE_NRML_CARDBUS = HEADER_TYPE_NORMAL | HEADER_TYPE_CARDBUS,
    HEADER_TYPE_BRIDGE_CARDBUS = HEADER_TYPE_BRIDGE | HEADER_TYPE_CARDBUS,
    HEADER_TYPE_ALL =
        HEADER_TYPE_NORMAL | HEADER_TYPE_BRIDGE | HEADER_TYPE_CARDBUS
} WDC_PCI_HEADER_TYPE;

/* -----------------------------------------------
    PCI configuration registers offsets
   ----------------------------------------------- */
enum {
    PCI_VID   = 0x00, /* Vendor ID */
    PCI_DID   = 0x02, /* Device ID */
    PCI_CR    = 0x04, /* Command register */
    PCI_SR    = 0x06, /* Status register */
    PCI_REV   = 0x08, /* Revision ID */
    PCI_CCR   = 0x09, /* Class code */
    PCI_CCSC  = 0x0a, /* Sub class code */
    PCI_CCBC  = 0x0b, /* Base class code */
    PCI_CLSR  = 0x0c, /* Cache line size */
    PCI_LTR   = 0x0d, /* Latency timer */
    PCI_HDR   = 0x0e, /* Header type */
    PCI_BISTR = 0x0f, /* Built-in self test */
    PCI_BAR0  = 0x10, /* Base address register */
    PCI_BAR1  = 0x14, /* Base address register */
    PCI_BAR2  = 0x18, /* Base address register */
    PCI_BAR3  = 0x1c, /* Base address register */
    PCI_BAR4  = 0x20, /* Base address register */
    PCI_BAR5  = 0x24, /* Base address register */
    PCI_CIS   = 0x28, /* CardBus CIS pointer */
    PCI_SVID  = 0x2c, /* Sub-system vendor ID */
    PCI_SDID  = 0x2e, /* Sub-system device ID */
    PCI_EROM  = 0x30, /* Expansion ROM base address */
    PCI_CAP   = 0x34, /* New capability pointer */
    PCI_ILR   = 0x3c, /* Interrupt line */
    PCI_IPR   = 0x3d, /* Interrupt pin */
    PCI_MGR   = 0x3e, /* Minimum required burst period */
    PCI_MLR   = 0x3f  /* Maximum latency - How often device must gain PCI bus
                       * access */
};

enum
{
    PCIE_CAP_ID   = 0x0,
    NEXT_CAP_PTR = 0x1,
    CAP_REG      = 0x2,
    DEV_CAPS     = 0x4,
    DEV_CTL      = 0x8,
    DEV_STS      = 0xa,
    LNK_CAPS     = 0xc,
    LNK_CTL      = 0x10,
    LNK_STS      = 0x12,
    SLOT_CAPS    = 0x14,
    SLOT_CTL     = 0x18,
    SLOT_STS     = 0x1a,
    ROOT_CAPS    = 0x1c,
    ROOT_CTL     = 0x1e,
    ROOT_STS     = 0x20,
    DEV_CAPS2    = 0x24,
    DEV_CTL2     = 0x28,
    DEV_STS2     = 0x2a,
    LNK_CAPS2    = 0x2c,
    LNK_CTL2     = 0x30,
    LNK_STS2     = 0x32,
    SLOT_CAPS2   = 0x34,
    SLOT_CTL2    = 0x38,
    SLOT_STS2    = 0x3a
};

#define PCI_HEADER_TYPE     0x0e    /* 8 bits */
#define  PCI_HEADER_TYPE_NORMAL  0
#define  PCI_HEADER_TYPE_BRIDGE  1
#define  PCI_HEADER_TYPE_CARDBUS 2

#define PCI_SR_CAP_LIST_BIT 0x00000010

/* PCI base address spaces (BARs) */
enum {
    AD_PCI_BAR0 = 0,
    AD_PCI_BAR1 = 1,
    AD_PCI_BAR2 = 2,
    AD_PCI_BAR3 = 3,
    AD_PCI_BAR4 = 4,
    AD_PCI_BAR5 = 5,
    AD_PCI_BARS = 6
};

/* PCI basic and extended capability lists last updated from-
 *     PCI Code and ID Assignment Specification Revision 1.5 */
/* Some of the following strings and macros are taken from
 * include/uapi/linux/pci_regs.h in Linux Kernel source */
/* Capability lists */
#define  PCI_CAP_LIST_ID    0     /* Capability ID */
#define  PCI_CAP_ID_PM      0x01  /* Power Management */
#define  PCI_CAP_ID_AGP     0x02  /* Accelerated Graphics Port */
#define  PCI_CAP_ID_VPD     0x03  /* Vital Product Data */
#define  PCI_CAP_ID_SLOTID  0x04  /* Slot Identification */
#define  PCI_CAP_ID_MSI     0x05  /* Message Signalled Interrupts */
#define  PCI_CAP_ID_CHSWP   0x06  /* CompactPCI HotSwap */
#define  PCI_CAP_ID_PCIX    0x07  /* PCI-X */
#define  PCI_CAP_ID_HT      0x08  /* HyperTransport */
#define  PCI_CAP_ID_VNDR    0x09  /* Vendor-Specific */
#define  PCI_CAP_ID_DBG     0x0A  /* Debug port */
#define  PCI_CAP_ID_CCRC    0x0B  /* CompactPCI Central Resource Control */
#define  PCI_CAP_ID_SHPC    0x0C  /* PCI Standard Hot-Plug Controller */
#define  PCI_CAP_ID_SSVID   0x0D  /* Bridge subsystem vendor/device ID */
#define  PCI_CAP_ID_AGP3    0x0E  /* AGP Target PCI-PCI bridge */
#define  PCI_CAP_ID_SECDEV  0x0F  /* Secure Device */
#define  PCI_CAP_ID_EXP     0x10  /* PCI Express */
#define  PCI_CAP_ID_MSIX    0x11  /* MSI-X */
#define  PCI_CAP_ID_SATA    0x12  /* SATA Data/Index Conf. */
#define  PCI_CAP_ID_AF      0x13  /* PCI Advanced Features */
#define  PCI_CAP_LIST_NEXT  1     /* Next capability in the list */

/* Extended Capabilities (PCI-X 2.0 and Express) */
#define PCI_EXT_CAP_ID(header)          (header & 0x0000ffff)
#define PCI_EXT_CAP_VER(header)         ((header >> 16) & 0xf)
#define PCI_EXT_CAP_NEXT(header)        ((header >> 20) & 0xffc)

#define PCI_EXT_CAP_ID_ERR      0x0001  /* Advanced Error Reporting */
#define PCI_EXT_CAP_ID_VC       0x0002  /* Virtual Channel Capability */
#define PCI_EXT_CAP_ID_DSN      0x0003  /* Device Serial Number */
#define PCI_EXT_CAP_ID_PWR      0x0004  /* Power Budgeting */
#define PCI_EXT_CAP_ID_RCLD     0x0005  /* Root Complex Link Declaration */
#define PCI_EXT_CAP_ID_RCILC    0x0006  /* Root Complex Internal Link Control */
#define PCI_EXT_CAP_ID_RCEC     0x0007  /* Root Complex Event Collector */
#define PCI_EXT_CAP_ID_MFVC     0x0008  /* Multi-Function VC Capability */
#define PCI_EXT_CAP_ID_VC9      0x0009  /* same as _VC */
#define PCI_EXT_CAP_ID_RCRB     0x000A  /* Root Complex RB? */
#define PCI_EXT_CAP_ID_VNDR     0x000B  /* Vendor-Specific */
#define PCI_EXT_CAP_ID_CAC      0x000C  /* Config Access - obsolete */
#define PCI_EXT_CAP_ID_ACS      0x000D  /* Access Control Services */
#define PCI_EXT_CAP_ID_ARI      0x000E  /* Alternate Routing ID */
#define PCI_EXT_CAP_ID_ATS      0x000F  /* Address Translation Services */
#define PCI_EXT_CAP_ID_SRIOV    0x0010  /* Single Root I/O Virtualization */
#define PCI_EXT_CAP_ID_MRIOV    0x0011  /* Multi Root I/O Virtualization */
#define PCI_EXT_CAP_ID_MCAST    0x0012  /* Multicast */
#define PCI_EXT_CAP_ID_PRI      0x0013  /* Page Request Interface */
#define PCI_EXT_CAP_ID_AMD_XXX  0x0014  /* Reserved for AMD */
#define PCI_EXT_CAP_ID_REBAR    0x0015  /* Resizable BAR */
#define PCI_EXT_CAP_ID_DPA      0x0016  /* Dynamic Power Allocation */
#define PCI_EXT_CAP_ID_TPH      0x0017  /* TPH Requester */
#define PCI_EXT_CAP_ID_LTR      0x0018  /* Latency Tolerance Reporting */
#define PCI_EXT_CAP_ID_SECPCI   0x0019  /* Secondary PCIe Capability */
#define PCI_EXT_CAP_ID_PMUX     0x001A  /* Protocol Multiplexing */
#define PCI_EXT_CAP_ID_PASID    0x001B  /* Process Address Space ID */
#define PCI_EXT_CAP_ID_LNR      0x001C  /* LN Requester (LNR) */
#define PCI_EXT_CAP_ID_DPC      0x001D  /* Downstream Port Containment (DPC) */
#define PCI_EXT_CAP_ID_L1PMS    0x001E  /* L1 PM Substates */
#define PCI_EXT_CAP_ID_PTM      0x001F  /* Precision Time Measurement (PTM) */
#define PCI_EXT_CAP_ID_MPHY     0x0020  /* PCI Express over M-PHY (M-PCIe) */
#define PCI_EXT_CAP_ID_FRSQ     0x0021  /* FRS Queueing */
#define PCI_EXT_CAP_ID_RTR      0x0022  /* Readiness Time Reporting */

#define GET_CAPABILITY_STR(cap_id) \
    (cap_id) == 0x00 ? "Null Capability" : \
    (cap_id) == PCI_CAP_ID_PM ? "Power Management" : \
    (cap_id) == PCI_CAP_ID_AGP ? "Accelerated Graphics Port" : \
    (cap_id) == PCI_CAP_ID_VPD ? "Vital Product Data" : \
    (cap_id) == PCI_CAP_ID_SLOTID ? "Slot Identification" : \
    (cap_id) == PCI_CAP_ID_MSI ? "Message Signalled Interrupts (MSI)" : \
    (cap_id) == PCI_CAP_ID_CHSWP ? "CompactPCI HotSwap" : \
    (cap_id) == PCI_CAP_ID_PCIX ? "PCI-X" : \
    (cap_id) == PCI_CAP_ID_HT ? "HyperTransport" : \
    (cap_id) == PCI_CAP_ID_VNDR ? "Vendor-Specific" : \
    (cap_id) == PCI_CAP_ID_DBG ? "Debug port" : \
    (cap_id) == PCI_CAP_ID_CCRC ? "CompactPCI Central Resource Control" : \
    (cap_id) == PCI_CAP_ID_SHPC ? "PCI Standard Hot-Plug Controller" : \
    (cap_id) == PCI_CAP_ID_SSVID ? "Bridge subsystem vendor/device ID" : \
    (cap_id) == PCI_CAP_ID_AGP3 ? "AGP Target PCI-PCI bridge" : \
    (cap_id) == PCI_CAP_ID_SECDEV ? "Secure Device" : \
    (cap_id) == PCI_CAP_ID_EXP ? "PCI Express" : \
    (cap_id) == PCI_CAP_ID_MSIX ? "Extended Message Signalled Interrupts (MSI-X)" : \
    (cap_id) == PCI_CAP_ID_SATA ? "SATA Data/Index Conf." : \
    (cap_id) == PCI_CAP_ID_AF ? "PCI Advanced Features" : \
    "Unknown"

#define GET_EXTENDED_CAPABILITY_STR(cap_id) \
    (cap_id) == 0x0000 ? "Null Capability" : \
    (cap_id) == PCI_EXT_CAP_ID_ERR ? "Advanced Error Reporting (AER)" : \
    (cap_id) == PCI_EXT_CAP_ID_VC ? "Virtual Channel (VC)" : /* MFVC Extended Cap struct is not present in the device */ \
    (cap_id) == PCI_EXT_CAP_ID_DSN ? "Device Serial Number" : \
    (cap_id) == PCI_EXT_CAP_ID_PWR ? "Power Budgeting" : \
    (cap_id) == PCI_EXT_CAP_ID_RCLD ? "Root Complex Link Declaration" : \
    (cap_id) == PCI_EXT_CAP_ID_RCILC ? "Root Complex Internal Link Control" : \
    (cap_id) == PCI_EXT_CAP_ID_RCEC ? "Root Complex Event Collector Endpoint Association" : \
    (cap_id) == PCI_EXT_CAP_ID_MFVC ? "Multi-Function Virtual Channel (MFVC)" : \
    (cap_id) == PCI_EXT_CAP_ID_VC9 ? "Virtual Channel (VC)" : /* MFVC Extended Cap struct is present in the device */ \
    (cap_id) == PCI_EXT_CAP_ID_RCRB ? "Root Complex Register Block (RCRB) Header" : \
    (cap_id) == PCI_EXT_CAP_ID_VNDR ? "Vendor-Specific Extended Capability (VSEC)" : \
    (cap_id) == PCI_EXT_CAP_ID_CAC ? "Configuration Access Correlation (CAC)" : \
    (cap_id) == PCI_EXT_CAP_ID_ACS ? "Access Control Services (ACS)" : \
    (cap_id) == PCI_EXT_CAP_ID_ARI ? "Alternative Routing-ID Interpretation (ARI)" : \
    (cap_id) == PCI_EXT_CAP_ID_ATS ? "Address Translation Services (ATS)" : \
    (cap_id) == PCI_EXT_CAP_ID_SRIOV ? "Single Root I/O Virtualization (SR-IOV)" : \
    (cap_id) == PCI_EXT_CAP_ID_MRIOV ? "Multi-Root I/O Virtualization (MR-IOV)" : \
    (cap_id) == PCI_EXT_CAP_ID_MCAST ? "Multicast" : \
    (cap_id) == PCI_EXT_CAP_ID_PRI ? "Page Request" : \
    (cap_id) == PCI_EXT_CAP_ID_AMD_XXX ? "Reserved for AMD" : \
    (cap_id) == PCI_EXT_CAP_ID_REBAR ? "Resizable BAR" : \
    (cap_id) == PCI_EXT_CAP_ID_DPA ? "Dynamic Power Allocation (DPA)" : \
    (cap_id) == PCI_EXT_CAP_ID_TPH ? "TLP Processing Hints (TPH)" : \
    (cap_id) == PCI_EXT_CAP_ID_LTR ? "Latency Tolerance Reporting (LTR)" : \
    (cap_id) == PCI_EXT_CAP_ID_SECPCI ? "Secondary PCI Express" : \
    (cap_id) == PCI_EXT_CAP_ID_PMUX ? "Protocol Multiplexing (PMUX)" : \
    (cap_id) == PCI_EXT_CAP_ID_PASID ? "Process Address Space ID (PASID)" : \
    (cap_id) == PCI_EXT_CAP_ID_LNR ? "LN Requester (LNR)" : \
    (cap_id) == PCI_EXT_CAP_ID_L1PMS ? "Downstream Port Containment (DPC)" : \
    (cap_id) == PCI_EXT_CAP_ID_L1PMS ? "L1 PM Substates" : \
    (cap_id) == PCI_EXT_CAP_ID_PTM ? "Precision Time Measurement (PTM)" : \
    (cap_id) == PCI_EXT_CAP_ID_MPHY ? "PCI Express over M-PHY (M-PCIe)" : \
    (cap_id) == PCI_EXT_CAP_ID_FRSQ ? "FRS Queueing" : \
    (cap_id) == PCI_EXT_CAP_ID_RTR ? "Readiness Time Reporting" : \
    "Unknown"

#define PCI_EXP_DEVCAP_PHANTOM_SHIFT 3
#define PCI_STATUS_DEVSEL_SHIFT      9
#define PCI_EXP_DEVCTL_READRQ_SHIFT  12
#define PCI_EXP_SLTCAP_SPLV_SHIFT    7
#define PCI_EXP_FLAGS_TYPE_SHIFT     9
#define PCI_EXP_DEVCAP_L1_SHIFT      9
#define PCI_EXP_DEVCAP_PWD_SCL_SHIFT 26
#define PCI_EXP_DEVCAP_PWR_VAL_SHIFT 18
#define PCI_EXP_DEVCTL_PAYLOAD_SHIFT 5
#define PCI_EXP_LNKCAP_MLW_SHIFT     4
#define PCI_EXP_LNKCAP_ASPMS_SHIFT   10
#define PCI_EXP_LNKCAP_L0SEL_SHIFT   12
#define PCI_EXP_LNKCAP_L1EL_SHIFT    15
#define PCI_EXP_SLTCAP_SPLS_SHIFT    15
#define PCI_EXP_SLTCAP_AIC_SHIFT     6
#define PCI_EXP_SLTCTL_PIC_SHIFT     8
#define PCI_EXP_DEVCAP2_EE_TLP_PREFIX_SUPP_SHIFT 22
#define PCI_EXP_LNKCTL2_TRANS_MARGIN_MASK_SHIFT  22

#define PCI_COMMAND     0x04    /* 16 bits */
#define  PCI_COMMAND_IO     0x1 /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY 0x2 /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER 0x4 /* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL    0x8 /* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE 0x10    /* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE 0x20   /* Enable palette snooping */
#define  PCI_COMMAND_PARITY 0x40    /* Enable parity checking */
#define  PCI_COMMAND_WAIT   0x80    /* Enable address/data stepping */
#define  PCI_COMMAND_SERR   0x100   /* Enable SERR */
#define  PCI_COMMAND_FAST_BACK  0x200   /* Enable back-to-back writes */
#define  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

#define PCI_STATUS      0x06    /* 16 bits */
#define  PCI_STATUS_INTERRUPT 0x08 /* Interrupt status */
#define  PCI_STATUS_CAP_LIST  0x10 /* Support Capability List */
#define  PCI_STATUS_66MHZ     0x20 /* Support 66 Mhz PCI 2.1 bus */
#define  PCI_STATUS_UDF       0x40 /* Support User Definable Features */

#define  PCI_STATUS_FAST_BACK     0x80    /* Accept fast-back to back */
#define  PCI_STATUS_PARITY        0x100   /* Detected parity error */
#define  PCI_STATUS_DEVSEL_MASK   0x600   /* DEVSEL timing */
#define  PCI_STATUS_DEVSEL_FAST   0x000
#define  PCI_STATUS_DEVSEL_MEDIUM 0x200
#define  PCI_STATUS_DEVSEL_SLOW   0x400
#define  PCI_STATUS_SIG_TARGET_ABORT 0x800 /* Set on target abort */
#define  PCI_STATUS_REC_TARGET_ABORT 0x1000 /* Master ack of abort */
#define  PCI_STATUS_REC_MASTER_ABORT 0x2000 /* Set on master abort */
#define  PCI_STATUS_SIG_SYSTEM_ERROR 0x4000 /* Set when we drive SERR */
#define  PCI_STATUS_DETECTED_PARITY  0x8000 /* Set on parity error */

#
/* PCI Express capability registers */

#define PCI_EXP_FLAGS   2 /* Capabilities register */
#define PCI_EXP_FLAGS_VERS  0x000f /* Capability version */
#define PCI_EXP_FLAGS_TYPE  0x00f0 /* Device/Port type */
#define  PCI_EXP_TYPE_ENDPOINT    0x0 /* Express Endpoint */
#define  PCI_EXP_TYPE_LEG_END     0x1 /* Legacy Endpoint */
#define  PCI_EXP_TYPE_ROOT_PORT   0x4 /* Root Port */
#define  PCI_EXP_TYPE_UPSTREAM    0x5 /* Upstream Port */
#define  PCI_EXP_TYPE_DOWNSTREAM  0x6 /* Downstream Port */
#define  PCI_EXP_TYPE_PCI_BRIDGE  0x7 /* PCIe to PCI/PCI-X Bridge */
#define  PCI_EXP_TYPE_PCIE_BRIDGE 0x8 /* PCI/PCI-X to PCIe Bridge */
#define  PCI_EXP_TYPE_RC_END      0x9 /* Root Complex Integrated Endpoint */
#define  PCI_EXP_TYPE_RC_EC       0xa /* Root Complex Event Collector */
#define PCI_EXP_FLAGS_SLOT  0x0100  /* Slot implemented */
#define PCI_EXP_FLAGS_IRQ   0x3e00  /* Interrupt message number */
#define PCI_EXP_DEVCAP  4/* Device capabilities */
#define  PCI_EXP_DEVCAP_PAYLOAD 0x00000007 /* Max_Payload_Size */
#define  PCI_EXP_DEVCAP_PHANTOM 0x00000018 /* Phantom functions */
#define  PCI_EXP_DEVCAP_EXT_TAG 0x00000020 /* Extended tags */
#define  PCI_EXP_DEVCAP_L0S     0x000001c0 /* L0s Acceptable Latency */
#define  PCI_EXP_DEVCAP_L1      0x00000e00 /* L1 Acceptable Latency */
#define  PCI_EXP_DEVCAP_ATN_BUT 0x00001000 /* Attention Button Present */
#define  PCI_EXP_DEVCAP_ATN_IND 0x00002000 /* Attention Indicator Present */
#define  PCI_EXP_DEVCAP_PWR_IND 0x00004000 /* Power Indicator Present */
#define  PCI_EXP_DEVCAP_RBER    0x00008000 /* Role-Based Error Reporting */
#define  PCI_EXP_DEVCAP_PWR_VAL 0x03fc0000 /* Slot Power Limit Value */
#define  PCI_EXP_DEVCAP_PWR_SCL 0x0c000000 /* Slot Power Limit Scale */
#define  PCI_EXP_DEVCAP_FLR     0x10000000 /* Function Level Reset */
#define PCI_EXP_DEVCTL      8   /* Device Control */
#define  PCI_EXP_DEVCTL_CERE     0x0001 /* Correctable Error Reporting En. */
#define  PCI_EXP_DEVCTL_NFERE    0x0002 /* Non-Fatal Error Reporting Enable */
#define  PCI_EXP_DEVCTL_FERE     0x0004 /* Fatal Error Reporting Enable */
#define  PCI_EXP_DEVCTL_URRE     0x0008 /* Unsupported Request Reporting En. */
#define  PCI_EXP_DEVCTL_RELAX_EN 0x0010 /* Enable relaxed ordering */
#define  PCI_EXP_DEVCTL_PAYLOAD  0x00e0 /* Max_Payload_Size */
#define  PCI_EXP_DEVCTL_EXT_TAG  0x0100 /* Extended Tag Field Enable */
#define  PCI_EXP_DEVCTL_PHANTOM  0x0200 /* Phantom Functions Enable */
#define  PCI_EXP_DEVCTL_AUX_PME  0x0400 /* Auxiliary Power PM Enable */
#define  PCI_EXP_DEVCTL_NOSNOOP_EN 0x0800  /* Enable No Snoop */
#define  PCI_EXP_DEVCTL_READRQ   0x7000 /* Max_Read_Request_Size */
#define  PCI_EXP_DEVCTL_READRQ_128B  0x0000 /* 128 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_256B  0x1000 /* 256 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_512B  0x2000 /* 512 Bytes */
#define  PCI_EXP_DEVCTL_READRQ_1024B 0x3000 /* 1024 Bytes */
#define  PCI_EXP_DEVCTL_BCR_FLR 0x8000  /* Bridge Configuration Retry / FLR */
#define PCI_EXP_DEVSTA      10  /* Device Status */
#define  PCI_EXP_DEVSTA_CED     0x0001  /* Correctable Error Detected */
#define  PCI_EXP_DEVSTA_NFED    0x0002  /* Non-Fatal Error Detected */
#define  PCI_EXP_DEVSTA_FED     0x0004  /* Fatal Error Detected */
#define  PCI_EXP_DEVSTA_URD     0x0008  /* Unsupported Request Detected */
#define  PCI_EXP_DEVSTA_AUXPD   0x0010  /* AUX Power Detected */
#define  PCI_EXP_DEVSTA_TRPND   0x0020  /* Transactions Pending */
#define PCI_EXP_LNKCAP      12  /* Link Capabilities */
#define  PCI_EXP_LNKCAP_SLS 0x0000000f /* Supported Link Speeds */
#define  PCI_EXP_LNKCAP_SLS_2_5GB 0x00000001 /* LNKCAP2 SLS Vector bit 0 */
#define  PCI_EXP_LNKCAP_SLS_5_0GB 0x00000002 /* LNKCAP2 SLS Vector bit 1 */
#define  PCI_EXP_LNKCAP_MLW 0x000003f0 /* Maximum Link Width */
#define  PCI_EXP_LNKCAP_ASPMS   0x00000c00 /* ASPM Support */
#define  PCI_EXP_LNKCAP_L0SEL   0x00007000 /* L0s Exit Latency */
#define  PCI_EXP_LNKCAP_L1EL    0x00038000 /* L1 Exit Latency */
#define  PCI_EXP_LNKCAP_CLKPM   0x00040000 /* Clock Power Management */
#define  PCI_EXP_LNKCAP_SDERC   0x00080000 /* Surprise Down Error Reporting
                                              Capable */
#define  PCI_EXP_LNKCAP_DLLLARC 0x00100000 /* Data Link Layer Link Active
                                              Reporting Capable */
#define  PCI_EXP_LNKCAP_LBNC    0x00200000 /* Link Bandwidth Notification
                                              Capability */
#define  PCI_EXP_LNKCAP_PN  0xff000000 /* Port Number */
#define PCI_EXP_LNKCTL      16 /* Link Control */
#define  PCI_EXP_LNKCTL_ASPMC     0x0003 /* ASPM Control */
#define  PCI_EXP_LNKCTL_ASPM_L0S  0x0001 /* L0s Enable */
#define  PCI_EXP_LNKCTL_ASPM_L1   0x0002 /* L1 Enable */
#define  PCI_EXP_LNKCTL_RCB       0x0008 /* Read Completion Boundary */
#define  PCI_EXP_LNKCTL_LD        0x0010 /* Link Disable */
#define  PCI_EXP_LNKCTL_RL        0x0020 /* Retrain Link */
#define  PCI_EXP_LNKCTL_CCC       0x0040 /* Common Clock Configuration */
#define  PCI_EXP_LNKCTL_ES        0x0080 /* Extended Synch */
#define  PCI_EXP_LNKCTL_CLKREQ_EN 0x0100 /* Enable clkreq */
#define  PCI_EXP_LNKCTL_HAWD      0x0200 /* Hardware Autonomous Width Disable*/
#define  PCI_EXP_LNKCTL_LBMIE     0x0400 /* Link Bandwidth Management Interrupt
                                           Enable */
#define  PCI_EXP_LNKCTL_LABIE   0x0800 /* Link Autonomous Bandwidth Interrupt
                                           Enable */
#define PCI_EXP_LNKSTA      18 /* Link Status */
#define  PCI_EXP_LNKSTA_CLS 0x000f /* Current Link Speed */
#define  PCI_EXP_LNKSTA_CLS_2_5GB 0x0001 /* Current Link Speed 2.5GT/s */
#define  PCI_EXP_LNKSTA_CLS_5_0GB 0x0002 /* Current Link Speed 5.0GT/s */
#define  PCI_EXP_LNKSTA_CLS_8_0GB 0x0003 /* Current Link Speed 8.0GT/s */
#define  PCI_EXP_LNKSTA_NLW     0x03f0 /* Negotiated Link Width */
#define  PCI_EXP_LNKSTA_NLW_X1  0x0010  /* Current Link Width x1 */
#define  PCI_EXP_LNKSTA_NLW_X2  0x0020  /* Current Link Width x2 */
#define  PCI_EXP_LNKSTA_NLW_X4  0x0040  /* Current Link Width x4 */
#define  PCI_EXP_LNKSTA_NLW_X8  0x0080  /* Current Link Width x8 */
#define  PCI_EXP_LNKSTA_NLW_SHIFT 4 /* start of NLW mask in link status */
#define  PCI_EXP_LNKSTA_LT      0x0800  /* Link Training */
#define  PCI_EXP_LNKSTA_SLC     0x1000  /* Slot Clock Configuration */
#define  PCI_EXP_LNKSTA_DLLLA   0x2000  /* Data Link Layer Link Active */
#define  PCI_EXP_LNKSTA_LBMS    0x4000  /* Link Bandwidth Management Status */
#define  PCI_EXP_LNKSTA_LABS    0x8000  /* Link Autonomous Bandwidth Status */
#define PCI_CAP_EXP_ENDPOINT_SIZEOF_V1  20  /* v1 endpoints end here */
#define PCI_EXP_SLTCAP      20  /* Slot Capabilities */
#define  PCI_EXP_SLTCAP_ABP 0x00000001 /* Attention Button Present */
#define  PCI_EXP_SLTCAP_PCP 0x00000002 /* Power Controller Present */
#define  PCI_EXP_SLTCAP_MRLSP   0x00000004 /* MRL Sensor Present */
#define  PCI_EXP_SLTCAP_AIP 0x00000008 /* Attention Indicator Present */
#define  PCI_EXP_SLTCAP_PIP 0x00000010 /* Power Indicator Present */
#define  PCI_EXP_SLTCAP_HPS 0x00000020 /* Hot-Plug Surprise */
#define  PCI_EXP_SLTCAP_HPC 0x00000040 /* Hot-Plug Capable */
#define  PCI_EXP_SLTCAP_SPLV    0x00007f80 /* Slot Power Limit Value */
#define  PCI_EXP_SLTCAP_SPLS    0x00018000 /* Slot Power Limit Scale */
#define  PCI_EXP_SLTCAP_EIP 0x00020000 /* Electromechanical Interlock Present */
#define  PCI_EXP_SLTCAP_NCCS    0x00040000 /* No Command Completed Support */
#define  PCI_EXP_SLTCAP_PSN 0xfff80000 /* Physical Slot Number */
#define PCI_EXP_SLTCTL      24 /* Slot Control */
#define  PCI_EXP_SLTCTL_ABPE    0x0001  /* Attention Button Pressed Enable */
#define  PCI_EXP_SLTCTL_PFDE    0x0002  /* Power Fault Detected Enable */
#define  PCI_EXP_SLTCTL_MRLSCE  0x0004  /* MRL Sensor Changed Enable */
#define  PCI_EXP_SLTCTL_PDCE    0x0008  /* Presence Detect Changed Enable */
#define  PCI_EXP_SLTCTL_CCIE    0x0010  /* Command Completed Interrupt Enable */
#define  PCI_EXP_SLTCTL_HPIE    0x0020  /* Hot-Plug Interrupt Enable */
#define  PCI_EXP_SLTCTL_AIC 0x00c0  /* Attention Indicator Control */
#define  PCI_EXP_SLTCTL_ATTN_IND_ON    0x0040 /* Attention Indicator on */
#define  PCI_EXP_SLTCTL_ATTN_IND_BLINK 0x0080 /* Attention Indicator blinking */
#define  PCI_EXP_SLTCTL_ATTN_IND_OFF   0x00c0 /* Attention Indicator off */
#define  PCI_EXP_SLTCTL_PIC 0x0300  /* Power Indicator Control */
#define  PCI_EXP_SLTCTL_PWR_IND_ON     0x0100 /* Power Indicator on */
#define  PCI_EXP_SLTCTL_PWR_IND_BLINK  0x0200 /* Power Indicator blinking */
#define  PCI_EXP_SLTCTL_PWR_IND_OFF    0x0300 /* Power Indicator off */
#define  PCI_EXP_SLTCTL_PCC 0x0400  /* Power Controller Control */
#define  PCI_EXP_SLTCTL_PWR_ON         0x0000 /* Power On */
#define  PCI_EXP_SLTCTL_PWR_OFF        0x0400 /* Power Off */
#define  PCI_EXP_SLTCTL_EIC 0x0800  /* Electromechanical Interlock Control */
#define  PCI_EXP_SLTCTL_DLLSCE  0x1000  /* Data Link Layer State Changed
                                           Enable */
#define PCI_EXP_SLTSTA      26 /* Slot Status */
#define  PCI_EXP_SLTSTA_ABP     0x0001  /* Attention Button Pressed */
#define  PCI_EXP_SLTSTA_PFD     0x0002  /* Power Fault Detected */
#define  PCI_EXP_SLTSTA_MRLSC   0x0004  /* MRL Sensor Changed */
#define  PCI_EXP_SLTSTA_PDC     0x0008  /* Presence Detect Changed */
#define  PCI_EXP_SLTSTA_CC      0x0010  /* Command Completed */
#define  PCI_EXP_SLTSTA_MRLSS   0x0020  /* MRL Sensor State */
#define  PCI_EXP_SLTSTA_PDS     0x0040  /* Presence Detect State */
#define  PCI_EXP_SLTSTA_EIS     0x0080  /* Electromechanical Interlock Status */
#define  PCI_EXP_SLTSTA_DLLSC   0x0100  /* Data Link Layer State Changed */
#define PCI_EXP_RTCTL       28  /* Root Control */
#define  PCI_EXP_RTCTL_SECEE    0x0001  /* System Error on Correctable Error */
#define  PCI_EXP_RTCTL_SENFEE   0x0002  /* System Error on Non-Fatal Error */
#define  PCI_EXP_RTCTL_SEFEE    0x0004  /* System Error on Fatal Error */
#define  PCI_EXP_RTCTL_PMEIE    0x0008  /* PME Interrupt Enable */
#define  PCI_EXP_RTCTL_CRSSVE   0x0010  /* CRS Software Visibility Enable */
#define PCI_EXP_RTCAP       30  /* Root Capabilities */
#define  PCI_EXP_RTCAP_CRSVIS   0x0001  /* CRS Software Visibility capability */
#define PCI_EXP_RTSTA       32  /* Root Status */
#define PCI_EXP_RTSTA_PME       0x00010000 /* PME status */
#define PCI_EXP_RTSTA_PENDING   0x00020000 /* PME pending */

#define PCI_EXP_DEVCAP2     36  /* Device Capabilities 2 */
#define PCI_EXP_DEVCAP2_RANGE_A 0x1 /* Completion Timeout Range A */
#define PCI_EXP_DEVCAP2_RANGE_B 0x2 /* Completion Timeout Range B */
#define PCI_EXP_DEVCAP2_RANGE_C 0x4 /* Completion Timeout Range C */
#define PCI_EXP_DEVCAP2_RANGE_D 0x8 /* Completion Timeout Range D */
#define PCI_EXP_DEVCAP2_COMP_TO_RANGES_SUPP 0xF
                           /* Completion Timeout Ranges Supported */
#define PCI_EXP_DEVCAP2_COMP_TO_DIS_SUPP    0x000010
                                    /* Completion Timeout Disable Supported */
#define  PCI_EXP_DEVCAP2_ARI        0x00000020 /* Alternative Routing-ID */
#define  PCI_EXP_DEVCAP2_ATOMIC_ROUTE   0x00000040 /* Atomic Op routing */
#define PCI_EXP_DEVCAP2_ATOMIC_COMP32      0x000080
                                    /* 32-bit AtomicOp Completer Supported */
#define PCI_EXP_DEVCAP2_ATOMIC_COMP64   0x00000100 /* Atomic 64-bit compare */
#define PCI_EXP_DEVCAP2_128_CAS_COMP_SUPP   0x000200
                                    /* 128-bit CAS Completer Supported */
#define PCI_EXP_DEVCAP2_NO_RO_ENABLED_PR    0x000400
                                    /* No RO Enabled PR-PR Passing */
#define  PCI_EXP_DEVCAP2_LTR        0x00000800 /* Latency tolerance reporting */
#define PCI_EXP_DEVCAP2_TPH_COMP_SUPP       0x001000
                                    /* TPH Completer Supported */
#define PCI_EXP_DEVCAP2_EXT_TPH_COMP_SUPP   0x002000
                                    /* Extended TPH Completer Supported */
#define  PCI_EXP_DEVCAP2_OBFF_MASK  0x000c0000 /* OBFF support mechanism */
#define  PCI_EXP_DEVCAP2_OBFF_MSG   0x00040000 /* New message signaling */
#define  PCI_EXP_DEVCAP2_OBFF_WAKE  0x00080000 /* Re-use WAKE# for OBFF */
#define PCI_EXP_DEVCAP2_EXT_FMT_FIELD_SUPP  0x100000
                                    /* Extended Fmt Field Supported */
#define PCI_EXP_DEVCAP2_EE_TLP_PREFIX_SUPP  0x200000
                                    /* End-End TLP Prefix Supported */
#define PCI_EXP_DEVCAP2_MAX_EE_TLP_PREFIXES 0xC00000
                                    /* Max End-End TLP Prefixes */

#define PCI_EXP_DEVCTL2     40  /* Device Control 2 */
#define PCI_EXP_DEVCTL2_COMP_TIMEOUT_DISABLE 0x0010
                                    /* End-End TLP Prefix Blocking */
#define  PCI_EXP_DEVCTL2_COMP_TIMEOUT   0x000f  /* Completion Timeout Value */
#define  PCI_EXP_DEVCTL2_ARI        0x0020  /* Alternative Routing-ID */
#define PCI_EXP_DEVCTL2_ATOMIC_REQ  0x0040  /* Set Atomic requests */
#define PCI_EXP_DEVCTL2_ATOMIC_EGRESS_BLOCK 0x0080 /* Block atomic egress */
#define  PCI_EXP_DEVCTL2_IDO_REQ_EN 0x0100  /* Allow IDO for requests */
#define  PCI_EXP_DEVCTL2_IDO_CMP_EN 0x0200  /* Allow IDO for completions */
#define  PCI_EXP_DEVCTL2_LTR_EN     0x0400  /* Enable LTR mechanism */
#define  PCI_EXP_DEVCTL2_OBFF_MSGA_EN   0x2000  /* Enable OBFF Message type A */
#define  PCI_EXP_DEVCTL2_OBFF_MSGB_EN   0x4000  /* Enable OBFF Message type B */
#define  PCI_EXP_DEVCTL2_OBFF_WAKE_EN   0x6000  /* OBFF using WAKE# signaling */
#define  PCI_EXP_DEVCTL2_EE_TLP_PREFIX_BLOCK  0x8000
                                    /* End-End TLP Prefix Blocking */
#define PCI_EXP_DEVSTA2                 42  /* Device Status 2 */
#define PCI_CAP_EXP_ENDPOINT_SIZEOF_V2  44  /* v2 endpoints end here */
#define PCI_EXP_LNKCAP2                 44  /* Link Capabilities 2 */
#define  PCI_EXP_LNKCAP2_SLS_2_5GB  0x00000002 /* Supported Speed 2.5GT/s */
#define  PCI_EXP_LNKCAP2_SLS_5_0GB  0x00000004 /* Supported Speed 5.0GT/s */
#define  PCI_EXP_LNKCAP2_SLS_8_0GB  0x00000008 /* Supported Speed 8.0GT/s */
#define  PCI_EXP_LNKCAP2_CROSSLINK  0x00000100 /* Crosslink supported */
#define PCI_EXP_LNKCTL2                 48  /* Link Control 2 */
#define  PCI_EXP_LNKCTL2_LNK_SPEED_2_5       0x00000001 /*Link Speed 2.5 GT/s */
#define  PCI_EXP_LNKCTL2_LNK_SPEED_5_0       0x00000002 /*Link Speed 5 GT/s */
#define  PCI_EXP_LNKCTL2_LNK_SPEED_8_0       0x00000003 /*Link Speed 8 GT/s */
#define  PCI_EXP_LNKCTL2_TRGT_LNK_SPEED_MASK 0x0000000f
                                                     /*Target Lnk Speed Mask*/
#define  PCI_EXP_LNKCTL2_ENTER_COMP          0x00000010 /*Enter Compliance */
#define  PCI_EXP_LNKCTL2_HW_AUTO_SPEED_DIS   0x00000020
                                         /*Hardware Autonomous Speed Disable */
#define  PCI_EXP_LNKCTL2_SELECTABLE_DEEMPH   0x00000040
                                                    /*Selectable De-emphasis */
#define  PCI_EXP_LNKCTL2_TRANS_MARGIN_MASK   0x00000380 /*Transmit Margin */
#define  PCI_EXP_LNKCTL2_ENTER_MOD_COMP      0x00000400
                                                 /*Enter Modified Compliance */
#define  PCI_EXP_LNKCTL2_COMP_SOS            0x00000800 /*Compliance SOS */
#define  PCI_EXP_LNKCTL2_DEEMPH_LVL_POLL     0x00001000
                                                 /*De-emphasis level polling */
#define  PCI_EXP_LNKCTL2_TRANS_PRESENT_POLL  0x0000f000
                                             /*Transmitter Preset in polling */
#define PCI_EXP_LNKSTA2 50/* Link Status 2 */
#define  PCI_EXP_LNKSTA2_CDL        0x00000001
                         /*Current De-emphasis level (at 5 GT/s speed only) */
#define  PCI_EXP_LNKSTA2_EQUALIZ_COMP 0x00000002 /* Equalization Complete */
#define  PCI_EXP_LNKSTA2_EQUALIZ_PH1 0x00000004 /*Equalization Ph.1 Successful*/
#define  PCI_EXP_LNKSTA2_EQUALIZ_PH2 0x00000008 /*Equalization Ph.2 Successful*/
#define  PCI_EXP_LNKSTA2_EQUALIZ_PH3 0x00000010 /*Equalization Ph.3 Successful*/
#define  PCI_EXP_LNKSTA2_LINE_EQ_REQ 0x00000020 /* Link Equalization Request */
#define PCI_EXP_SLTCAP2 52  /* Slot Capabilities 2 */
#define PCI_EXP_SLTCTL2 56  /* Slot Control 2 */
#define PCI_EXP_SLTSTA2 58  /* Slot Status 2 */

#endif /* _PCI_REGS_H_ */

