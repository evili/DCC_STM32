#include <stdlib.h>
#include "cli.h"
#include "main.h"
#include "dcc.h"
#include "adc.h"
#include "printf-stdarg.h"

#define CLI_DEFAULT_WAIT 200U

extern osMessageQueueId_t dccMainPacketQueueHandle;
extern osMessageQueueId_t dccProgPacketQueueHandle;

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
 	snprintf(pcWriteBuffer, xWriteBufferLen, "<p1>");
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
 	snprintf(pcWriteBuffer, xWriteBufferLen, "<p0>");
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
 	snprintf(pcWriteBuffer, xWriteBufferLen,"<p%u>", pin_status);
	return pdFALSE;
}

/**
 * Throttle command
 */
BaseType_t prvThrottleCommand( char *pcWriteBuffer,
		size_t xWriteBufferLen,
		const char *pcCommandString )
{
	// Get parameters
    unsigned int reg = atol(FreeRTOS_CLIGetParameter(pcCommandString, 1, NULL));
    unsigned int cab = atol(FreeRTOS_CLIGetParameter(pcCommandString, 2, NULL));
    unsigned int spd = atol(FreeRTOS_CLIGetParameter(pcCommandString, 3, NULL));
    unsigned int dir = atol(FreeRTOS_CLIGetParameter(pcCommandString, 4, NULL));
    // Check ranges
    reg = (reg > DCC_QUEUE_LEN)   ? (DCC_QUEUE_LEN-1) : reg;
    cab = (cab > DCC_ADDRESS_MAX) ? DCC_ADDRESS_MAX   : cab;
    spd = (spd > 126)             ? 126               : spd;
    dir = (dir)                   ? 1                 : 0;
    // Register allocated?
    if (Rooster[reg].allocated == pdFALSE)
    {
    	// Set IDLE packet
    	Rooster[reg].packet = DCC_PACKET_IDLE;
    }

        // Update packet with cab, speed and direction.
    	// Critical section. The packet could be the actual pump packet.
    	taskENTER_CRITICAL();
    	//UBaseType_t water_mark = uxTaskGetStackHighWaterMark(NULL);
    	DCC_Packet_set_address(&Rooster[reg].packet, cab);
    	taskEXIT_CRITICAL();
    	taskENTER_CRITICAL();
    	DCC_Packet_set_speed(&Rooster[reg].packet, spd, dir);
    	taskEXIT_CRITICAL();
		// Register[reg] == NULL && packet != NULL ==> No Register, packet is "new"

		if(Rooster[reg].allocated == pdFALSE)
		{
			// DCC_Packet *packet = &(Rooster.packet[reg]);
			uint32_t msg = (uint32_t) &(Rooster[reg].packet);
			osStatus status = osMessageQueuePut(dccMainPacketQueueHandle,  &(msg), 0U, CLI_DEFAULT_WAIT);
			if(status == osOK) {
				Rooster[reg].allocated = pdTRUE;
			}
			else {
				snprintf(pcWriteBuffer, xWriteBufferLen, "ERROR: Packet queue problem %d\r\n\r\n", (int) status);
				return pdFALSE;
			}
		}
		// if we are here, everything is ok (i.e. Register[reg] != NULL && packet != NULL
		snprintf(pcWriteBuffer, xWriteBufferLen, "<T %d %d %d>", cab, spd, dir);
		return pdFALSE;
}

/**
 * Function command
 */
BaseType_t prvFunctionCommand( char *pcWriteBuffer,
		size_t xWriteBufferLen,
		const char *pcCommandString )
{
	DCC_Packet *packet;
    unsigned int cab   = atol(FreeRTOS_CLIGetParameter(pcCommandString, 1, NULL));
    uint8_t fbyte = atoi(FreeRTOS_CLIGetParameter(pcCommandString, 2, NULL));
    // adjust ranges
    cab = (cab > DCC_ADDRESS_MAX) ? DCC_ADDRESS_MAX   : cab;
    fbyte = (fbyte | 0x80) & 0xBF;
    packet = (DCC_Packet *) pvPortMalloc(sizeof(DCC_Packet));
    *packet = DCC_PACKET_CMD;
    packet->data[0] = fbyte;
    DCC_Packet_set_address(packet, cab);
    //uint32_t msg = (uint32_t ) packet;
    osStatus status = osMessageQueuePut(dccMainPacketQueueHandle, &packet, 0U, CLI_DEFAULT_WAIT);
	if(status != osOK) {
		vPortFree(packet);
		snprintf(pcWriteBuffer, xWriteBufferLen, "ERROR: Packet queue problem %d\r\n\r\n", (int) status);
	}
	else {
		pcWriteBuffer[0] = '\0';
	}
	return pdFALSE;
}

/**
 *  Status Command: minimal response to make other soft recognize us as true DCC++ Station
 */
BaseType_t prvStatusCommand( char *pcWriteBuffer,
		size_t xWriteBufferLen,
		const char *pcCommandString )
{
	static BaseType_t phase = 0;
	static BaseType_t reg = 0;
	BaseType_t status = pdFALSE;
	uint8_t pwr, spd, dir;
	uint16_t cab;
	switch(phase) {
	case 0:
		pwr = ((ENABLE_MAIN_GPIO_Port->ODR & ENABLE_MAIN_Pin) != 0X00u);
		snprintf(pcWriteBuffer, xWriteBufferLen,"<p%u>", pwr);
		status = pdTRUE;
		phase = 1;
		break;
	case 1:
		if(Rooster[reg].allocated == pdTRUE)
		{
			cab = DCC_Packet_get_address(Rooster[reg].packet);
			DCC_Packet_get_speed(Rooster[reg].packet, &spd, &dir);
			snprintf(pcWriteBuffer, xWriteBufferLen, "<T %d %d %d>",
					cab, spd, dir);
		}
		else
		{
			// snprintf(pcWriteBuffer, xWriteBufferLen, "");
			pcWriteBuffer[0] = '\0';
		}
		if(reg<DCC_QUEUE_LEN) {
			reg++;
		}
		else
		{
			reg = 0;
			phase = 2;
		}
		status = pdTRUE;
		break;
	case 2:
		snprintf(pcWriteBuffer, xWriteBufferLen, DCCPP_STATION);
		phase = 3;
		status = pdTRUE;
		break;
	case 3:
		snprintf(pcWriteBuffer, xWriteBufferLen, "<N SERIAL>");
		phase = 4;
		status = pdTRUE;
		break;
	default:
		phase = 0;
		pcWriteBuffer[0] = '\0';
		status = pdFALSE;
		break;
	}
	return status;
}

/**
 * ReadCurrentCommand
 */
BaseType_t prvReadCurrentCommand( char *pcWriteBuffer,
		size_t xWriteBufferLen,
		const char *pcCommandString )
{
	// TODO: Read the current draw.
	snprintf(pcWriteBuffer, xWriteBufferLen, "<a %d>", ADC_Value.senseA);
	return pdFALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
///// CLI definitions
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
		"<t REGISTER CAB SPEED DIRECTION>:\r\n\tSets the throttle for a mobile engine decoder using 128-step speeds.\r\n",
		prvThrottleCommand,
		4
};

static const CLI_Command_Definition_t xFunctionCommand =
{
		"<f",
		"<f CAB BYTE1 [BYTE2]>:\r\n\tControls mobile engine decoder functions F0-F28.\r\n",
		prvFunctionCommand,
		2
};

static const CLI_Command_Definition_t xAccessoryCommand =
{
		"<a",
		"<a ADDRESS SUBADDRESS ACTIVATE>:\r\n\tControls turnouts connected to stationary accessory decoders.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xTurnoutCommand =
{
		"<T",
		"<T ID THROW>:\r\n\tControls turnouts connected to stationary accessory decoders.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xWriteCVMainCommand =
{
		"<w",
		"<w CAB CV VALUE>:\r\n\tWrites a Configuration Variable byte to an engine decoder on the main ops track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xBitCVMainCommand =
{
		"<b",
		"<b CAB CV BIT VALUE>:\r\n\tSets/Clears a Configuration Variable bit in an engine decoder on the main operations track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xWriteCVProgCommand =
{
		"<W",
		"<W CAB CV VALUE CALLBACKNUM CALLBACKSUB>:\r\n\tWrites a Configuration Variable byte to an engine decoder on the programming track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xBitCVProgCommand =
{
		"<B",
		"<B CAB CV BIT VALUE CALLBACKNUM CALLBACKSUB>:\r\n\tSets/Clears a Configuration Variable bit in an engine decoder on the programming track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xReadCVProgCommand =
{
		"<R",
		"<R W CV VALUE CALLBACKNUM CALLBACKSUB>:\r\n\tReads a Configuration Variable byte from an engine decoder on the programming track.\r\n",
		dummyCommand,
		0
};

static const CLI_Command_Definition_t xReadCurrentCommand =
{
		"<c",
		"<c>:\r\n\tReads current draw from main operations track.\r\n",
		prvReadCurrentCommand,
		0
};

static const CLI_Command_Definition_t xStatusCommand =
{
		"<s",
		"<s>:\r\n\tReturns status messages, including power state, turnout states, and sketch version.\r\n",
		prvStatusCommand,
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
