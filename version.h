#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR_ESCAM         3  //Zaehlt nur alle paar jahre hoch, z.B. windows 20 mit 128 bit
#define VERSION_PCIE_BOARD_VERSION  26 //PCI Karten Version
#define VERSION_MINOR_ESCAM         2

#define VER_FILE_VERSION            VERSION_MAJOR_ESCAM, VERSION_PCIE_BOARD_VERSION, VERSION_MINOR_ESCAM
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR_ESCAM)        \
                                    "." STRINGIZE(VERSION_PCIE_BOARD_VERSION)    \
                                    "." STRINGIZE(VERSION_MINOR_ESCAM) \

#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_COPYRIGHT_STR           "Copyright EB Stresing (C) 2022"
#define VER_COMPANY_NAME            "EB Stresing - Berlin"
