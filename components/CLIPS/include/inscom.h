   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 7.00  03/02/24            */
   /*                                                     */
   /*            INSTANCE COMMAND HEADER MODULE           */
   /*******************************************************/

/*************************************************************/
/* Purpose:                                                  */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Contributing Programmer(s):                               */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*      6.23: Correction for FalseSymbol/TrueSymbol. DR0859  */
/*                                                           */
/*            Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*      6.24: Loading a binary instance file from a run-time */
/*            program caused a bus error. DR0866             */
/*                                                           */
/*            Removed LOGICAL_DEPENDENCIES compilation flag. */
/*                                                           */
/*            Converted INSTANCE_PATTERN_MATCHING to         */
/*            DEFRULE_CONSTRUCT.                             */
/*                                                           */
/*            Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*      6.30: Removed conditional code for unsupported       */
/*            compilers/operating systems (IBM_MCW,          */
/*            MAC_MCW, and IBM_TBC).                         */
/*                                                           */
/*            Changed integer type/precision.                */
/*                                                           */
/*            Changed garbage collection algorithm.          */
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
/*************************************************************/

#ifndef _H_inscom

#pragma once

#define _H_inscom

typedef enum
  {
   UIE_NO_ERROR = 0,
   UIE_NULL_POINTER_ERROR,
   UIE_COULD_NOT_DELETE_ERROR,
   UIE_DELETED_ERROR,
   UIE_RULE_NETWORK_ERROR
  } UnmakeInstanceError;

typedef enum
  {
   MIE_NO_ERROR = 0,
   MIE_NULL_POINTER_ERROR,
   MIE_PARSING_ERROR,
   MIE_COULD_NOT_CREATE_ERROR,
   MIE_RULE_NETWORK_ERROR
  } MakeInstanceError;

typedef enum
  {
   IBE_NO_ERROR = 0,
   IBE_NULL_POINTER_ERROR,
   IBE_DEFCLASS_NOT_FOUND_ERROR,
   IBE_COULD_NOT_CREATE_ERROR,
   IBE_RULE_NETWORK_ERROR
  } InstanceBuilderError;

typedef enum
  {
   IME_NO_ERROR = 0,
   IME_NULL_POINTER_ERROR,
   IME_DELETED_ERROR,
   IME_COULD_NOT_MODIFY_ERROR,
   IME_RULE_NETWORK_ERROR
  } InstanceModifierError;

#include "insfun.h"
#include "object.h"

#define INSTANCE_DATA 29

struct instanceData
  {
   Instance DummyInstance;
   Instance **InstanceTable;
   bool MaintainGarbageInstances;
   bool MkInsMsgPass;
   bool ChangesToInstances;
   struct patternEntityRecord InstanceInfo;
   Instance *InstanceList;
   unsigned long GlobalNumberOfInstances;
   Instance *CurrentInstance;
   Instance *InstanceListBottom;
   bool ObjectModDupMsgValid;
   UnmakeInstanceError unmakeInstanceError;
   MakeInstanceError makeInstanceError;
   InstanceModifierError instanceModifierError;
   InstanceBuilderError instanceBuilderError;
  };

#define InstanceData(theEnv) ((struct instanceData *) GetEnvironmentData(theEnv,INSTANCE_DATA))

   void                           SetupInstances(Environment *);
   UnmakeInstanceError            DeleteInstance(Instance *);
   UnmakeInstanceError            DeleteAllInstances(Environment *);
   UnmakeInstanceError            UnmakeInstance(Instance *);
   bool                           UnmakeInstanceCallback(Instance *,Environment *);
   UnmakeInstanceError            UnmakeAllInstances(Environment *);
#if DEBUGGING_FUNCTIONS
   void                           InstancesCommand(Environment *,UDFContext *,UDFValue *);
   void                           PPInstanceCommand(Environment *,UDFContext *,UDFValue *);
   void                           Instances(Environment *,const char *,Defmodule *,const char *,bool);
#endif
   Instance                      *MakeInstance(Environment *,const char *);
   MakeInstanceError              GetMakeInstanceError(Environment *);
   Instance                      *CreateRawInstance(Environment *,Defclass *,const char *);
   Instance                      *FindInstance(Environment *,Defmodule *,const char *,bool);
   bool                           ValidInstanceAddress(Instance *);
   GetSlotError                   DirectGetSlot(Instance *,const char *,CLIPSValue *);
   PutSlotError                   DirectPutSlot(Instance *,const char *,CLIPSValue *);
   PutSlotError                   DirectPutSlotInteger(Instance *,const char *,long long);
   PutSlotError                   DirectPutSlotFloat(Instance *,const char *,double);
   PutSlotError                   DirectPutSlotSymbol(Instance *,const char *,const char *);
   PutSlotError                   DirectPutSlotString(Instance *,const char *,const char *);
   PutSlotError                   DirectPutSlotInstanceName(Instance *,const char *,const char *);
   PutSlotError                   DirectPutSlotCLIPSInteger(Instance *,const char *,CLIPSInteger *);
   PutSlotError                   DirectPutSlotCLIPSFloat(Instance *,const char *,CLIPSFloat *);
   PutSlotError                   DirectPutSlotCLIPSLexeme(Instance *,const char *,CLIPSLexeme *);
   PutSlotError                   DirectPutSlotFact(Instance *,const char *,Fact *);
   PutSlotError                   DirectPutSlotInstance(Instance *,const char *,Instance *);
   PutSlotError                   DirectPutSlotMultifield(Instance *,const char *,Multifield *);
   PutSlotError                   DirectPutSlotCLIPSExternalAddress(Instance *,const char *,CLIPSExternalAddress *);
   const char                    *InstanceName(Instance *);
   Defclass                      *InstanceClass(Instance *);
   unsigned long                  GetGlobalNumberOfInstances(Environment *);
   Instance                      *GetNextInstance(Environment *,Instance *);
   Instance                      *GetNextInstanceInScope(Environment *,Instance *);
   Instance                      *GetNextInstanceInClass(Defclass *,Instance *);
   Instance                      *GetNextInstanceInClassAndSubclasses(Defclass **,Instance *,UDFValue *);
   void                           InstancePPForm(Instance *,StringBuilder *);
   void                           ClassCommand(Environment *,UDFContext *,UDFValue *);
   void                           DeleteInstanceCommand(Environment *,UDFContext *,UDFValue *);
   void                           UnmakeInstanceCommand(Environment *,UDFContext *,UDFValue *);
   void                           SymbolToInstanceNameFunction(Environment *,UDFContext *,UDFValue *);
   void                           InstanceNameToSymbolFunction(Environment *,UDFContext *,UDFValue *);
   void                           InstanceAddressCommand(Environment *,UDFContext *,UDFValue *);
   void                           InstanceNameCommand(Environment *,UDFContext *,UDFValue *);
   void                           InstanceAddressPCommand(Environment *,UDFContext *,UDFValue *);
   void                           InstanceNamePCommand(Environment *,UDFContext *,UDFValue *);
   void                           InstancePCommand(Environment *,UDFContext *,UDFValue *);
   void                           InstanceExistPCommand(Environment *,UDFContext *,UDFValue *);
   void                           CreateInstanceHandler(Environment *,UDFContext *,UDFValue *);

#endif /* _H_inscom */
