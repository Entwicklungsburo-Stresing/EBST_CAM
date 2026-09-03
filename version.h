/*****************************************************************//**
 * @file		version.h
 * @brief		Current version of this library
 * @author		Gerhard Stresing
 * @copyright	Copyright Entwicklungsbuero Stresing. This software is released under the LGPL-3.0.
 *********************************************************************/

#pragma once

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VERSION_MAJOR_ESCAM         4
#define VERSION_MINOR_ESCAM         22
#define VERSION_PATCH_ESCAM         1

#define VER_FILE_VERSION            VERSION_MAJOR_ESCAM, VERSION_MINOR_ESCAM, VERSION_PATCH_ESCAM
#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR_ESCAM)        \
                                    "." STRINGIZE(VERSION_MINOR_ESCAM)    \
                                    "." STRINGIZE(VERSION_PATCH_ESCAM) \

#define VER_PRODUCT_VERSION         VER_FILE_VERSION
#define VER_PRODUCT_VERSION_STR     VER_FILE_VERSION_STR
#define VER_COPYRIGHT_STR           "Copyright Entwicklungsbuero Stresing (C) 2025"
#define VER_COMPANY_NAME            "Entwicklungsbuero Stresing - Berlin"
