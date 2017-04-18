#pragma once
/*-----------------------------*/
/* Macro definitions for ELAN. */
/*-----------------------------*/
#include <inttypes.h>                                                                                                                                                                                                         
 
#ifndef _ELAN_DEFINE_H_
#define _ELAN_DEFINE_H_

#define ELAN_CHECK_ERROR_ALLOC(ptr, str, ...) if (ptr == NULL) { char _tmpStr[1024]; sprintf_s(_tmpStr, "ERROR : memory allocation for %s\n", str); fprintf(stderr, _tmpStr, ##__VA_ARGS__); exit(-1); }

#define ELAN_MIN(a,b) (((a)<(b))?(a):(b))
#define ELAN_MAX(a,b) (((a)>(b))?(a):(b))


/* Constants definitions for ELAN. */
/*---------------------------------*/
#define ES_PI	    3.14159265358979323846
#define ES_EPS	    1e-10


/* ELAN path definition. */
/*-----------------------*/
static char ELAN_PATH[1024];
static char ELAN_COLOR_PATH[1024];
static char ELAN_ELEC_PATH[1024];
static char ELAN_ERPA_PATH[1024];
static char ELAN_TFVIZ_PATH[1024];
static char ELAN_EEG_PATH[1024];
static char ELAN_BIN_PATH[1024];
static char ELAN_SYS_PATH[1024];

/* SetElanPath : macro to set ELAN paths. */
/*----------------------------------------*/
#define SetElanPath if (getenv("ELANPATH")==NULL) \
 { \
 fprintf(stderr, "Environment variable ELANPATH not found.\n");\
 } \
else \
 { \
 strcpy(ELAN_PATH, getenv("ELANPATH"));  \
 sprintf(ELAN_COLOR_PATH, "%s/color/", ELAN_PATH);  \
 sprintf(ELAN_ELEC_PATH, "%s/elec/", ELAN_PATH);  \
 sprintf(ELAN_ERPA_PATH, "%s/erpa/", ELAN_PATH);  \
 sprintf(ELAN_TFVIZ_PATH, "%s/tfviz/", ELAN_PATH);  \
 sprintf(ELAN_EEG_PATH, "%s/eeg/", ELAN_PATH);  \
 sprintf(ELAN_BIN_PATH, "%s/bin/", ELAN_PATH); \
 sprintf(ELAN_SYS_PATH, "%s/sys/", ELAN_PATH); \
 }

 
/* Integer type definitions. */
/*---------------------------*/
typedef int8_t int8;                                                                                                                                                                                                          
typedef uint8_t uint8;                                                                                                                                                                                                        
typedef int16_t int16;                                                                                                                                                                                                        
typedef uint16_t uint16;                                                                                                                                                                                                      
typedef int32_t int32;                                                                                                                                                                                                        
typedef uint32_t uint32;                                                                                                                                                                                                      
typedef int64_t int64;                                                                                                                                                                                                        
typedef uint64_t uint64;


#else

/* External global variables. */
/*----------------------------*/
extern char ELAN_PATH[1024];
extern char ELAN_COLOR_PATH[1024];
extern char ELAN_ELEC_PATH[1024];
extern char ELAN_ERPA_PATH[1024];
extern char ELAN_TFVIZ_PATH[1024];
extern char ELAN_EEG_PATH[1024];
extern char ELAN_BIN_PATH[1024];
extern char ELAN_SYS_PATH[1024];

#endif
