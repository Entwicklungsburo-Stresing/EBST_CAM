#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR               3
#define VERSION_PCIE_BOARD_VERSION  16
#define VERSION_MINOR				4

#define VER_FILE_VERSION            VERSION_MAJOR, VERSION_PCIE_BOARD_VERSION, VERSION_MINOR
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
                                    "." STRINGIZE(VERSION_PCIE_BOARD_VERSION)    \
                                    "." STRINGIZE(VERSION_MINOR) \

#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_COPYRIGHT_STR           "Copyright EB Stresing (C) 2020"
#define VER_COMPANY_NAME			"EB Stresing - Berlin"
