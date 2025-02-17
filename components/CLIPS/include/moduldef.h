   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 7.00  11/19/24            */
   /*                                                     */
   /*                DEFMODULE HEADER FILE                */
   /*******************************************************/

/*************************************************************/
/* Purpose: Defines basic defmodule primitive functions such */
/*   as allocating and deallocating, traversing, and finding */
/*   defmodule data structures.                              */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*            Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Changed integer type/precision.                */
/*                                                           */
/*            Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Added const qualifiers to remove C++           */
/*            deprecation warnings.                          */
/*                                                           */
/*            Converted API macros to function calls.        */
/*                                                           */
/*      6.40: Removed LOCALE definition.                     */
/*                                                           */
/*            Pragma once and other inclusion changes.       */
/*                                                           */
/*            Added support for booleans with <stdbool.h>.   */
/*                                                           */
/*            Removed use of void pointers for specific      */
/*            data structures.                               */
/*                                                           */
/*            ALLOW_ENVIRONMENT_GLOBALS no longer supported. */
/*                                                           */
/*            UDF redesign.                                  */
/*                                                           */
/*      6.42: Fixed GC bug by including garbage fact and     */
/*            instances in the GC frame.                     */
/*                                                           */
/*      7.00: Deftable construct added.                      */
/*                                                           */
/*            Construct hashing for quick lookup.            */
/*                                                           */
/*            Support for named facts.                       */
/*                                                           */
/*************************************************************/

#ifndef _H_moduldef

#pragma once

#define _H_moduldef

typedef struct defmodule Defmodule;
typedef struct portItem PortItem;
typedef struct defmoduleItemHeader DefmoduleItemHeader;
typedef struct moduleItem ModuleItem;
typedef struct constructHeader ConstructHeader;
typedef struct moduleStackItem ModuleStackItem;

typedef void *AllocateModuleFunction(Environment *);
typedef void InitModuleFunction(Environment *,void *);
typedef void FreeModuleFunction(Environment *,void *);

typedef ConstructHeader *FindConstructFunction(Environment *,const char *);
typedef ConstructHeader *GetNextConstructFunction(Environment *,ConstructHeader *);
typedef bool IsConstructDeletableFunction(ConstructHeader *);
typedef bool DeleteConstructFunction(ConstructHeader *,Environment *);
typedef void FreeConstructFunction(Environment *,ConstructHeader *);

typedef enum
  {
   DEFMODULE,
   DEFRULE,
   DEFTEMPLATE,
   DEFFACTS,
   DEFGLOBAL,
   DEFFUNCTION,
   DEFGENERIC,
   DEFMETHOD,
   DEFCLASS,
   DEFMESSAGE_HANDLER,
   DEFINSTANCES,
   DEFTABLE
  } ConstructType;

#include <stdio.h>
#include "entities.h"
#include "userdata.h"

struct constructHeader
  {
   ConstructType constructType;
   CLIPSLexeme *name;
   const char *ppForm;
   DefmoduleItemHeader *whichModule;
   unsigned long bsaveID;
   ConstructHeader *next;
   struct userData *usrData;
   Environment *env;
  };

struct itemHashTableEntry
  {
   ConstructHeader *item;
   size_t hashValue;
   struct itemHashTableEntry *next;
  };
    
struct defmoduleItemHeader
  {
   Defmodule *theModule;
   ConstructHeader *firstItem;
   ConstructHeader *lastItem;
  };
  
struct defmoduleItemHeaderHM
  {
   Defmodule *theModule;
   ConstructHeader *firstItem;
   ConstructHeader *lastItem;
   unsigned long itemCount;
   size_t hashTableSize;
   struct itemHashTableEntry **hashTable;
  };
  
typedef ConstructHeader *LookupConstructFunction(Environment *,CLIPSLexeme *);

/**********************************************************************/
/* defmodule                                                          */
/* ----------                                                         */
/* name: The name of the defmodule (stored as a reference in the      */
/*   table).                                                          */
/*                                                                    */
/* ppForm: The pretty print representation of the defmodule (used by  */
/*   the save and ppdefmodule commands).                              */
/*                                                                    */
/* itemsArray: An array of pointers to the module specific data used  */
/*   by each construct specified with the RegisterModuleItem          */
/*   function. The data pointer stored in the array is allocated by   */
/*   the allocateFunction in moduleItem data structure.               */
/*                                                                    */
/* importList: The list of items which are being imported by this     */
/*   module from other modules.                                       */
/*                                                                    */
/* next: A pointer to the next defmodule data structure.              */
/**********************************************************************/

struct defmodule
  {
   ConstructHeader header;
   DefmoduleItemHeader **itemsArray;
   PortItem *importList;
   PortItem *exportList;
   bool visitedFlag;
  };

struct portItem
  {
   CLIPSLexeme *moduleName;
   CLIPSLexeme *constructType;
   CLIPSLexeme *constructName;
   PortItem *next;
  };

#define MIHS (DefmoduleItemHeader *)

/**********************************************************************/
/* moduleItem                                                         */
/* ----------                                                         */
/* name: The name of the construct which can be placed in a module.   */
/*   For example, "defrule".                                          */
/*                                                                    */
/* allocateFunction: Used to allocate a data structure containing all */
/*   pertinent information related to a specific construct for a      */
/*   given module. For example, the deffacts construct stores a       */
/*   pointer to the first and last deffacts for each each module.     */
/*                                                                    */
/* freeFunction: Used to deallocate a data structure allocated by     */
/*   the allocateFunction. In addition, the freeFunction deletes      */
/*   all constructs of the specified type in the given module.        */
/*                                                                    */
/* bloadModuleReference: Used during a binary load to establish a     */
/*   link between the defmodule data structure and the data structure */
/*   containing all pertinent module information for a specific       */
/*   construct.                                                       */
/*                                                                    */
/* findFunction: Used to determine if a specified construct is in a   */
/*   specific module. The name is the specific construct is passed as */
/*   a string and the function returns a pointer to the specified     */
/*   construct if it exists.                                          */
/*                                                                    */
/* exportable: If true, then the specified construct type can be      */
/*   exported (and hence imported). If false, it can't be exported.   */
/*                                                                    */
/* next: A pointer to the next moduleItem data structure.             */
/**********************************************************************/

struct moduleItem
  {
   const char *name;
   unsigned moduleIndex;
   void *(*allocateFunction)(Environment *);
   void  (*initFunction)(Environment *,void *);
   void  (*freeFunction)(Environment *,void *);
   void *(*bloadModuleReference)(Environment *,unsigned long);
   void  (*constructsToCModuleReference)(Environment *,FILE *,unsigned long,unsigned int,unsigned int);
   FindConstructFunction *findFunction;
   ModuleItem *next;
  };

struct moduleStackItem
  {
   bool changeFlag;
   Defmodule *theModule;
   ModuleStackItem *next;
  };

#define DEFMODULE_DATA 4

struct defmoduleData
  {
   struct moduleItem *LastModuleItem;
   struct voidCallFunctionItem *AfterModuleChangeFunctions;
   ModuleStackItem *ModuleStack;
   bool CallModuleChangeFunctions;
   Defmodule *ListOfDefmodules;
   Defmodule *CurrentModule;
   Defmodule *LastDefmodule;
   unsigned NumberOfModuleItems;
   struct moduleItem *ListOfModuleItems;
   long ModuleChangeIndex;
   bool MainModuleRedefinable;
#if (! RUN_TIME) && (! BLOAD_ONLY)
   struct portConstructItem *ListOfPortConstructItems;
   unsigned short NumberOfDefmodules;
   struct voidCallFunctionItem *AfterModuleDefinedFunctions;
#endif
#if CONSTRUCT_COMPILER && (! RUN_TIME)
   struct CodeGeneratorItem *DefmoduleCodeItem;
#endif
#if (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE) && (! RUN_TIME)
   unsigned long BNumberOfDefmodules;
   unsigned long NumberOfPortItems;
   struct portItem *PortItemArray;
   Defmodule *DefmoduleArray;
#endif
   struct defmoduleItemHeaderHM hashMap;
  };

#define DefmoduleData(theEnv) ((struct defmoduleData *) GetEnvironmentData(theEnv,DEFMODULE_DATA))

   void                           InitializeDefmodules(Environment *);
   Defmodule                     *FindDefmodule(Environment *,const char *);
   const char                    *DefmoduleName(Defmodule *);
   const char                    *DefmodulePPForm(Defmodule *);
   Defmodule                     *GetNextDefmodule(Environment *,Defmodule *);
   void                           RemoveAllDefmodules(Environment *,void *);
   int                            AllocateModuleStorage(void);
   unsigned                       RegisterModuleItem(Environment *,const char *,
                                                     AllocateModuleFunction *,
                                                     InitModuleFunction *,
                                                     FreeModuleFunction *,
                                                     void *(*)(Environment *,unsigned long),
                                                     void (*)(Environment *,FILE *,unsigned long,unsigned int,unsigned int),
                                                     FindConstructFunction *);
   void                          *GetModuleItem(Environment *,Defmodule *,unsigned);
   void                           SetModuleItem(Environment *,Defmodule *,unsigned,void *);
   Defmodule                     *GetCurrentModule(Environment *);
   Defmodule                     *SetCurrentModule(Environment *,Defmodule *);
   void                           GetCurrentModuleCommand(Environment *,UDFContext *,UDFValue *);
   void                           SetCurrentModuleCommand(Environment *,UDFContext *,UDFValue *);
   unsigned                       GetNumberOfModuleItems(Environment *);
   void                           CreateMainModule(Environment *,void *);
   void                           SetListOfDefmodules(Environment *,Defmodule *);
   struct moduleItem             *GetListOfModuleItems(Environment *);
   struct moduleItem             *FindModuleItem(Environment *,const char *);
   void                           SaveCurrentModule(Environment *);
   void                           RestoreCurrentModule(Environment *);
   void                           AddAfterModuleChangeFunction(Environment *,const char *,VoidCallFunction *,int,void *);
   void                           IllegalModuleSpecifierMessage(Environment *);
   void                           AllocateDefmoduleGlobals(Environment *);
   unsigned short                 GetNumberOfDefmodules(Environment *);
   void                           RemoveConstructFromHashMap(Environment *,ConstructHeader *,struct defmoduleItemHeader *);
   void                           ClearDefmoduleHashMap(Environment *,struct defmoduleItemHeaderHM *);
   void                           AddConstructToHashMap(Environment *theEnv,ConstructHeader *,struct defmoduleItemHeader *);
   Defmodule                     *LookupDefmodule(Environment *,CLIPSLexeme *);
   void                           AssignHashMapSize(Environment *,struct defmoduleItemHeaderHM *,size_t);

#endif /* _H_moduldef */
