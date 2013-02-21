/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
 *
 * This file is part of Imago OCR project.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "platform_tools.h"


bool platform::checkMemoryFail(int amount)
{
	try
	{
		char* data = new char[1024 * 1024 * amount];
		if (data != NULL)
		{
			delete []data;
			return false; // no fails
		}
	}
	catch(...)
	{
	}
	return true;
}


#ifdef _WIN32 // ------------------- Windows -------------------
#include <direct.h>
#include <Windows.h>

int platform::MKDIR(const std::string& directory)
{
	return _mkdir(directory.c_str());
}

unsigned int platform::TICKS()
{
	LARGE_INTEGER lFreq, lStart;
	QueryPerformanceFrequency(&lFreq);
	QueryPerformanceCounter(&lStart);
	double msecs = ((double)lStart.QuadPart / lFreq.QuadPart * 1000.0);
	__int64 ticks = (__int64)msecs;
	return (unsigned int)ticks;
}

unsigned int platform::MEM_AVAIL()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	return static_cast<size_t>(statex.ullAvailVirtual / 1024);
}

std::string platform::getLineEndings()
{
	return "\r\n";
}

int platform::CALL(const std::string& executable, const std::string& parameters, int timelimit)
{	
    int result = 0; 

    /* - NOTE - You should check here to see if the exe even exists */ 

    /* Add a space to the beginning of the Parameters */ 
	std::string Parameters = parameters;
    if (Parameters.size() != 0) 
    { 
        if (Parameters[0] != ' ') 
        { 
            Parameters.insert(0, " "); 
        } 
    } 

    /* The first parameter needs to be the exe itself */ 
    std::string sTempStr = executable; 
    size_t iPos = sTempStr.find_last_of("\\"); 
    sTempStr.erase(0, iPos + 1); 
    Parameters = sTempStr.append(Parameters); 

     /* CreateProcessW can modify Parameters thus we allocate needed memory */ 
    char* pcharParam = new char[Parameters.size() + 1]; 
    if (pcharParam == 0) 
    { 
        return -1; // memory allocation is failed
    } 
    strcpy_s(pcharParam, Parameters.size() + 1, Parameters.c_str()); 

    /* CreateProcess API initialization */ 
    STARTUPINFO siStartupInfo; 
    PROCESS_INFORMATION piProcessInfo; 
    memset(&siStartupInfo, 0, sizeof(siStartupInfo)); 
    memset(&piProcessInfo, 0, sizeof(piProcessInfo)); 
    siStartupInfo.cb = sizeof(siStartupInfo); 

    if (CreateProcess(executable.c_str(), 
		              pcharParam, 0, 0, false, 
					  CREATE_DEFAULT_ERROR_MODE, 0, 0, 
					  &siStartupInfo, &piProcessInfo) != false) 
    { 
        DWORD dwStartTime = GetTickCount();
		if (timelimit == 0)
			timelimit = INFINITE;
        DWORD dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, timelimit); 
		
		if (WAIT_TIMEOUT == dwExitCode)
		{
			result = -2; // TIMEOUT
			TerminateProcess(piProcessInfo.hProcess, 0);
		}
		else
		{
			DWORD exitCode;
			result = GetExitCodeProcess(piProcessInfo.hProcess, &exitCode);
			result = exitCode;
		}
    } 
    else 
    { 
		result = -3; // CREATE PROCESS FAILED
    } 

    /* Free memory */ 
    delete[] pcharParam; 
    pcharParam = 0; 

    /* Release handles */ 
    CloseHandle(piProcessInfo.hProcess); 
    CloseHandle(piProcessInfo.hThread); 

    return result; 
}

#else // ------------------- POSIX -------------------

#include <sys/stat.h>
#include <errno.h>

int platform::MKDIR(const std::string& directory)
{
	return mkdir(directory.c_str(), S_IRWXU|S_IRGRP|S_IXGRP);
}

unsigned int platform::TICKS()
{
	return 0; // TODO
}

unsigned int platform::MEM_AVAIL()
{
	return 0; // TODO
}

int platform::CALL(const std::string& executable, const std::string& parameters, int timelimit)
{
	return 0; // TODO
}

std::string platform::getLineEndings()
{
	return "\n";
}

#endif

