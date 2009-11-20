/*
	Console system. Heavily influenced by Quake3's code :)
*/

#include <string>
#include <vector>
#include <sstream>

#include "console.h"
#include "d3_func.h"
#include "iofocus.h"
#include "logString.h"

extern "C" 
{
	#include "avp_menugfx.hpp"
	#include "platform.h"
	extern unsigned char DebouncedKeyboardInput[MAX_NUMBER_OF_INPUT_KEYS];
	extern int RealFrameTime;
}

#define CHAR_WIDTH	15
#define CHAR_HEIGHT	15

#define ONE_FIXED	65536

char buf[100];

typedef void (*funcPointer) (void);

struct Command 
{
	char *cmdName;
	char *cmdDescription;
	funcPointer cmdFuncPointer;
};

std::vector<Command> cmdList;
std::vector<Command>::iterator cmdIt;
std::vector<std::string>cmdArgs;

struct Console 
{
	bool isActive;
	bool isOpen;

	int xPos;
	int yPos;
	int lines;
	int lineWidth;

	int indent;

	int width;
	int height;

	int time;
	int destinationY;

	std::vector<std::string> text;
	std::string	inputLine;
};

Console console;

void Con_PrintError(const std::string &errorString)
{
	console.text.push_back(errorString);

	// write to log file for now
	LogErrorString(errorString);
}

void Con_PrintMessage(const std::string &messageString)
{
	console.text.push_back(messageString);

	// write to log file for now
	LogString(messageString);
}


void Con_AddLine(const std::string &temp)
{
	console.text.push_back(temp);
}

void Con_AddCommand(char *command, funcPointer function)
{
	for (cmdIt = cmdList.begin(); cmdIt < cmdList.end(); ++cmdIt)
	{
		if (!strcmp(command, cmdIt->cmdName))
		{
			OutputDebugString("command already exists\n");
			return;
		}
	}
	
	Command newCommand;
	
	newCommand.cmdName = command;
	newCommand.cmdDescription = "no description yet";
	newCommand.cmdFuncPointer = function;

	cmdList.push_back(newCommand);
}

void Blah()
{
	OutputDebugString("Blah was called!\n");

	// we're testing and expect 3 ints
}

void Con_Init()
{
	console.isActive = false;
	console.xPos = 0;
	console.yPos = 0;
	console.width = 640;

	console.lines = 240 / CHAR_HEIGHT;
	console.lineWidth = 640 / CHAR_WIDTH;

	console.height = console.lines * CHAR_HEIGHT;
	console.indent = CHAR_WIDTH / 2;

	console.destinationY = 0;

	sprintf(buf, "console height: %d num lines: %d\n", console.lines * CHAR_HEIGHT, console.lines);
	OutputDebugString(buf);

	Con_AddCommand("blah", Blah);
	Con_AddCommand("toggleconsole", Con_Toggle);
//	LoadConsoleFont();

	OutputDebugString("console initialised\n");
}

void Con_ProcessCommand()
{
	/* if nothing typed, don't bother doing anything */
	if (console.inputLine.length() == 0) 
		return;

	// clear the arg vector
	cmdArgs.clear();

	// find the command name first
	std::string commandName;
	Command theCommand = {0};
	std::stringstream stream(console.inputLine);

	// the command will be first word up to first space
	getline(stream, commandName, ' ');

	// see if the command actually exists first.
	for (cmdIt = cmdList.begin(); cmdIt < cmdList.end(); ++cmdIt)
	{
		if (commandName == cmdIt->cmdName)
		{
			// found it
			theCommand = *cmdIt;
			break;
		}
	}
	
	// in case we didnt find it
	if (theCommand.cmdFuncPointer == NULL)
	{
		Con_AddLine(commandName);
		Con_AddLine("Unknown command \"" + commandName + "\"");
		console.inputLine.clear();
		return;
	}

	// parse the rest of the string for arguments
	std::string currentArg;

	while (getline(stream, currentArg, ' '))
	{
		// add the current parameter to the arguments list
		cmdArgs.push_back(currentArg);
	}

	Con_AddLine(console.inputLine);

	console.inputLine.clear();

	// lets call the function now..
	theCommand.cmdFuncPointer();
}

void Con_ProcessInput()
{
	if (DebouncedKeyboardInput[KEY_BACKSPACE])
		Con_RemoveTypedChar();

	if (DebouncedKeyboardInput[KEY_CR])
		Con_ProcessCommand();
}

void Con_Toggle()
{
	OutputDebugString("console toggle\n");
	console.isActive = !console.isActive;
	console.time = 250;

	if (console.height > 0) 
		console.destinationY = 0;
	else 
		console.destinationY = console.lines * CHAR_HEIGHT;

	// toggle on/off
	IOFOCUS_Set( IOFOCUS_Get() ^ IOFOCUS_NEWCONSOLE);
}

bool Con_IsActive()
{
	return console.isActive;
}

bool Con_IsOpen()
{
	return console.isOpen;
}

void Con_CheckResize()
{
/*
	if (console.width != ScreenDescriptorBlock.SDB_Width)
	{
		console.width = ScreenDescriptorBlock.SDB_Width;
	}
*/
}

void Con_AddTypedChar(char c)
{
	if (!console.isActive) 
		return;

	if (c == 0x08)
	{
//		Con_Key_Backspace(true);
		Con_RemoveTypedChar();
	}

	else if (c == 13)
	{	
//		Con_Key_Enter(true);
		Con_ProcessCommand();
	}
	else
	{
	//	sprintf(buf, "Con_AddTypedChar: %c\n", c);
	//	OutputDebugString(buf);

		console.inputLine.push_back(c);
	}
}

void Con_RemoveTypedChar()
{
	if (console.inputLine.length())
	{
		console.inputLine.resize(console.inputLine.length() - 1, 1);
	}
}

void Con_Draw()
{
	int charWidth = 0;

	/* is console moving to a new position? */
	if (console.destinationY > console.height)
	{
		console.height += static_cast<int>(RealFrameTime * 0.01f);
		if (console.height > console.destinationY)
			console.height = console.destinationY;

		console.isOpen = false;
	}
	else if (console.destinationY < console.height)
	{
		console.height -= static_cast<int>(RealFrameTime * 0.01f);
		if (console.height < console.destinationY)
			console.height = console.destinationY;

		console.isOpen = false;
	}

	if (console.destinationY == console.height)
		console.isOpen = true;

	// draw the background quad
	DrawQuad(console.xPos, console.yPos, console.width, console.height, D3DCOLOR_ARGB(255, 38, 80, 145));

	if (console.height > 0)
	{
		// draw the outline bar that runs along the bottom of the console
		DrawQuad(console.xPos, console.yPos+console.height, console.width, 2, D3DCOLOR_ARGB(255, 255, 255, 255));
	}

	int charCount = 0;
	static int alpha = ONE_FIXED;

	alpha -= static_cast<int>(RealFrameTime * 1.2f);
	if (alpha < 0) alpha = ONE_FIXED;

	/* draw input cusor */
	charWidth = RenderSmallChar('>', console.indent, console.height - CHAR_HEIGHT, ONE_FIXED, ONE_FIXED, ONE_FIXED, ONE_FIXED);

	RenderSmallChar('_', console.indent + charWidth, console.height - CHAR_HEIGHT, alpha, ONE_FIXED, ONE_FIXED, ONE_FIXED);

	int rows = console.text.size() - 1;

	int y = console.height - CHAR_HEIGHT * 2;

	int lines = console.lines;
	int xOffset = 0;

	for (int i = rows; i >= 0; i--, y -= CHAR_HEIGHT)
	{
		xOffset = 0;
		charWidth = 0;

		for(int j = 0; j < console.text[i].length(); j++)
		{
			//if ((j * CHAR_WIDTH) > console.lineWidth) break;
			charWidth = RenderSmallChar(console.text.at(i).at(j), console.indent + /*(xOffset * j)*/xOffset, y, ONE_FIXED, ONE_FIXED / 2, ONE_FIXED, ONE_FIXED);
			xOffset+=charWidth;
		}
	}

	xOffset = CHAR_WIDTH * 2;
	charWidth = 0;

	for (int j = 0; j < console.inputLine.length(); j++)
	{
		//if ((j * CHAR_WIDTH) > console.lineWidth) break;
		charWidth = RenderSmallChar(console.inputLine.at(j), console.indent + /*(xOffset * j)*/xOffset, console.height - CHAR_HEIGHT, ONE_FIXED, ONE_FIXED, ONE_FIXED, ONE_FIXED);
		xOffset+=charWidth;
	}
}