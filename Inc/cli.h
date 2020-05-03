#ifndef __CLI_H__
#define __CLI_H__

#include "FreeRTOS_CLI.h"

BaseType_t prvPowerCommand( int8_t *pcWriteBuffer,
                             size_t xWriteBufferLen,
                             const int8_t *pcCommandString );


static const CLI_Command_Definition_t xPowerOnCommand =
{
		"<p",
		"<p(0|1)>: Sets power on main track. 0 disable power, 1 enable power. "
		prvPowerCommand,
		1
};

#endif // __CLI_H__
