#include "cli.h"
#include "printf-stdarg.h"

/**
 * Power Up/Down command
 */
BaseType_t prvPowerCommand( char *pcWriteBuffer,
                             size_t xWriteBufferLen,
                             const char *pcCommandString )
{
	int8_t *pcParameter;
	BaseType_t xParameterStringLength;
	pcParameter = FreeRTOS_CLIGetParameter
	                        (
	                          /* The command string itself. */
	                          pcCommandString,
	                          /* Return the first parameter. */
	                          1,
	                          /* Store the parameter string length. */
	                          &xParameterStringLength
	                        );
	pcParameter[ xParameterStringLength ] = 0x00;
	if(pcParameter[0] == '0')  {
		// OUTPUT DISABLE ON MAIN TRACK and PROG TRACL

	}
	else {
		// OUTPUT ENABLE ON MAIN TRACK and PROG TRACL
	}

	snprintf(pcWriteBuffer, xWriteBufferLen, "//TODO: ENABLE MAIN TRACK %s\r\n\r\n", pcParameter);
	return pdFALSE;
}

static const CLI_Command_Definition_t xPowerOnCommand =
{
		"<p",
		"<p(0|1)>: Sets power on main track. 0 disable power, 1 enable power.",
		prvPowerCommand,
		1
};


void vRegisterCLICommands() {
	FreeRTOS_CLIRegisterCommand(&xPowerOnCommand);
}
