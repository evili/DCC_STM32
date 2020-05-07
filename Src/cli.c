#include "cli.h"
#include "main.h"
#include "printf-stdarg.h"

/**
Command list from DccPlusPlus:

01 <t>: sets the throttle for a mobile engine decoder using 128-step speeds
02 <f>: controls mobile engine decoder functions F0-F28
03 <a>: controls stationary accessory decoders
04 <T>: controls turnouts connected to stationary accessory decoders
05 <w>: writes a configuration variable byte to an engine decoder on the main ops track
06 <b>: sets/clear a configuration variable bit in an engine decoder on the main operations track
07 <W>: writes a configuration variable byte to an engine decoder on the programming track
08 <B>: sets/clear a configuration variable bit in an engine decoder on the programming track
09 <R>: reads a configuration variable byte from an engine decoder on the programming track
10 <1>: turns on track power
11 <0>: turns off track power
12 <c>: reads current draw from main operations track
13 <s>: returns status messages, including power state, turnout states, and sketch version
 */

BaseType_t dummyCommand(char *pcWriteBuffer,
                        size_t xWriteBufferLen,
                        const char *pcCommandString) {
  snprintf(pcWriteBuffer, xWriteBufferLen, "//TODO: Not implemented: %s\r\n\r\n", pcCommandString);
  return pdFALSE;
}

/**
 * Power On command
 */  
BaseType_t prvPowerOnCommand( char *pcWriteBuffer,
                             size_t xWriteBufferLen,
                             const char *pcCommandString )
{
	 // Disable Channels, then disable counter.
	 HAL_GPIO_WritePin(ENABLE_MAIN_GPIO_Port, ENABLE_MAIN_Pin, GPIO_PIN_SET);
	 HAL_GPIO_WritePin(ENABLE_PROG_GPIO_Port, ENABLE_PROG_Pin, GPIO_PIN_SET);
 	snprintf(pcWriteBuffer, xWriteBufferLen, "1\r\n\r\n");
	return pdFALSE;
}

/**
 * Power Off command
 */
BaseType_t prvPowerOffCommand( char *pcWriteBuffer,
		size_t xWriteBufferLen,
		const char *pcCommandString )
{
	 // Disable Channels, then disable counter.
	 HAL_GPIO_WritePin(ENABLE_MAIN_GPIO_Port, ENABLE_MAIN_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(ENABLE_PROG_GPIO_Port, ENABLE_PROG_Pin, GPIO_PIN_RESET);
 	snprintf(pcWriteBuffer, xWriteBufferLen, "0\r\n\r\n");
	return pdFALSE;
}

/**
 * Power Toggle command
 */
BaseType_t prvPowerToggleCommand( char *pcWriteBuffer,
		size_t xWriteBufferLen,
		const char *pcCommandString )
{
	// Disable Channels, then disable counter.
	HAL_GPIO_TogglePin(ENABLE_MAIN_GPIO_Port, ENABLE_MAIN_Pin);
	HAL_GPIO_TogglePin(ENABLE_PROG_GPIO_Port, ENABLE_PROG_Pin);
	uint8_t pin_status = ((ENABLE_MAIN_GPIO_Port->ODR & ENABLE_MAIN_Pin) != 0X00u);
 	snprintf(pcWriteBuffer, xWriteBufferLen,"%u\r\n\r\n", pin_status);
	return pdFALSE;
}

static const CLI_Command_Definition_t xPowerOnCommand =
{
		"<1",
		"<1>:\r\n\tTurn ON track power.\r\n",
		prvPowerOnCommand,
		0
};

static const CLI_Command_Definition_t xPowerOffCommand =
{
		"<0",
		"<0>:\r\n\tTurn OFF track power.\r\n",
		prvPowerOffCommand,
		0
};

static const CLI_Command_Definition_t xPowerToggleCommand =
{
		"<+",
		"<+>:\r\n\tToggle ON/OFF track power.\r\n",
		prvPowerToggleCommand,
		0
};


static const CLI_Command_Definition_t xThrottleCommand =
{
		"<t",
		"<t M S D>:\r\n\tSets the throttle for a mobile engine decoder using 128-step speeds.\r\n",
		dummyCommand,
		0
};
static const CLI_Command_Definition_t xFunctionCommand =
{
		"<f",
		"<f M S D>:\r\n\tControls mobile engine decoder functions F0-F28.\r\n",
		dummyCommand,
		0
};
static const CLI_Command_Definition_t xAccessoryCommand =
{
		"<a",
		"<a D S>:\r\n\tControls turnouts connected to stationary accessory decoders.\r\n",
		dummyCommand,
		0
};
static const CLI_Command_Definition_t xTurnoutCommand =
{
		"<T",
		"<T D S>:\r\n\tControls turnouts connected to stationary accessory decoders.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xWriteCVMainCommand =
{
		"<w",
		"<w A C V >:\r\n\tWrites a Configuration Variable byte to an engine decoder on the main ops track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xBitCVMainCommand =
{
		"<b",
		"<b A C B V >:\r\n\tSets/Clears a Configuration Variable bit in an engine decoder on the main operations track.\r\n",
		dummyCommand,
		0
};
static const CLI_Command_Definition_t xWriteCVProgCommand =
{
		"<W",
		"<W A C V >:\r\n\tWrites a Configuration Variable byte to an engine decoder on the programming track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xBitCVProgCommand =
{
		"<B",
		"<B A C B V >:\r\n\tSets/Clears a Configuration Variable bit in an engine decoder on the programming track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xReadCVProgCommand =
{
		"<R",
		"<R A C>:\r\n\tReads a Configuration Variable byte from an engine decoder on the programming track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xReadCurrentCommand =
{
		"<c",
		"<c>:\r\n\tReads current draw from main operations track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xStatusCommand =
{
		"<s",
		"<s>:\r\n\tReturns status messages, including power state, turnout states, and sketch version.\r\n",
		dummyCommand,
		0
};

void vRegisterCLICommands() {
  FreeRTOS_CLIRegisterCommand(&xThrottleCommand);      // 01
  FreeRTOS_CLIRegisterCommand(&xFunctionCommand);      // 02
  FreeRTOS_CLIRegisterCommand(&xAccessoryCommand);     // 03
  FreeRTOS_CLIRegisterCommand(&xTurnoutCommand);       // 04
  FreeRTOS_CLIRegisterCommand(&xWriteCVMainCommand);   // 05
  FreeRTOS_CLIRegisterCommand(&xBitCVMainCommand);     // 06
  FreeRTOS_CLIRegisterCommand(&xWriteCVProgCommand);   // 07
  FreeRTOS_CLIRegisterCommand(&xBitCVProgCommand);     // 08
  FreeRTOS_CLIRegisterCommand(&xReadCVProgCommand);    // 09
  FreeRTOS_CLIRegisterCommand(&xPowerOnCommand);       // 10
  FreeRTOS_CLIRegisterCommand(&xPowerOffCommand);      // 11
  FreeRTOS_CLIRegisterCommand(&xReadCurrentCommand);   // 12
  FreeRTOS_CLIRegisterCommand(&xStatusCommand);        // 13
  // Extras:
  FreeRTOS_CLIRegisterCommand(&xPowerToggleCommand);        // 14
}
