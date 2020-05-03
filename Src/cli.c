#include "cli.h"


BaseType_t prvPowerCommand( int8_t *pcWriteBuffer,
                             size_t xWriteBufferLen,
                             const int8_t *pcCommandString )
{
	int8_t *pcParameter;
	BaseType_t xParameterStringLength;
	pcParameter1 = FreeRTOS_CLIGetParameter
	                        (
	                          /* The command string itself. */
	                          pcCommandString,
	                          /* Return the first parameter. */
	                          1,
	                          /* Store the parameter string length. */
	                          &xParameterStringLength
	                        );
	pcParameter[ xParameterStringLength ] = 0x00;
	if(pcParameter == "0")  {
		// OUTPUT DISABLE ON MAIN TRACK and PROG TRACL

	}
	else {
		// OUTPUT ENABLE ON MAIN TRACK and PROG TRACL
	}

	snprintf( pcWriteBuffer, xWriteBufferLen, "//TODO: ENABLE MAIN TRACK %srnrn", pcParameter);
	return pdFALSE;
}

