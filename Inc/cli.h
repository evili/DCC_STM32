#ifndef __CLI_H__
#define __CLI_H__



#include "cmsis_os.h"
#include "FreeRTOS_CLI.h"

BaseType_t prvPowerOnCommand( char *pcWriteBuffer,
                             size_t xWriteBufferLen,
                             const char *pcCommandString );

void vRegisterCLICommands();

#endif // __CLI_H__
