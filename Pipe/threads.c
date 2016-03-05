#include <stdio.h>
#include <stdint.h>
#include <strsafe.h>
#include <windows.h>

#include "ringbuffer.h"
#include "pipe.h"

/* Integrate every element of the signal. */
static void increment(pipe_t * const p)
{
	while (Pipe_isFilled(p))
	{
		uint32_t item = Pipe_Read(p);
		item++;
		Pipe_Write(p, item);
	}
}


/* Print the signal. */
static void write_to_file1(pipe_t * const pipe)
{
	HANDLE hFile;
	char DataBuffer[128];
	DWORD dwBytesWritten = 0;

	hFile = CreateFile("thread1.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	while (1) {
		while (Pipe_isFilled(pipe))
		{
			sprintf_s(DataBuffer, 128, "%d\r\n", Pipe_Read(pipe));
			WriteFile(hFile, DataBuffer, (DWORD)strlen(DataBuffer), &dwBytesWritten, NULL);
		}
	}
}

static void write_to_file2(pipe_t * const pipe)
{
	HANDLE hFile;
	char DataBuffer[128];
	DWORD dwBytesWritten = 0;

	hFile = CreateFile("thread2.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	while (1) {
		while (Pipe_isFilled(pipe))
		{
			sprintf_s(DataBuffer, 128, "%d\r\n", Pipe_Read(pipe));
			WriteFile(hFile, DataBuffer, (DWORD)strlen(DataBuffer), &dwBytesWritten, NULL);
		}
	}
}

DWORD WINAPI incrementThread1(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;
	LARGE_INTEGER time;

	while (1)
	{
		/* Generate input */
		QueryPerformanceCounter(&time);
		Pipe_Insert(pipe, (uint32_t)time.u.LowPart);
		QueryPerformanceCounter(&time);
		Pipe_Insert(pipe, (uint32_t)time.u.LowPart);
		QueryPerformanceCounter(&time);
		Pipe_Insert(pipe, (uint32_t)time.u.LowPart);

		increment(pipe);
	}

	return 0;
}

DWORD WINAPI incrementThread2(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;
	LARGE_INTEGER time;

	while (1)
	{
		/* Generate input */
		QueryPerformanceCounter(&time);
		Pipe_Insert(pipe, (uint32_t)time.u.LowPart);
		QueryPerformanceCounter(&time);
		Pipe_Insert(pipe, (uint32_t)time.u.LowPart);
		QueryPerformanceCounter(&time);
		Pipe_Insert(pipe, (uint32_t)time.u.LowPart);

		increment(pipe);
	}

	return 0;
}

DWORD WINAPI incrementThread3(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;

	while (1)
	{
		increment(pipe);
	}

	return 0;
}

DWORD WINAPI Thread2(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;
	write_to_file2(pipe);

	return 0;
}

DWORD WINAPI Thread1(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;
	write_to_file1(pipe);

	return 0;
}

static void log(pipe_t * const source, pipe_t * const target, uint32_t element) {}

void threads(void)
{
	/* Create pipes and connect them */
	Pipe_Create(increment_pipe1, 4, 1, NULL, log);
	Pipe_Create(increment_pipe2, 4, 1, NULL, log);
	Pipe_Create(increment_pipe3, 8, 2, NULL, log);
	Pipe_Create(write_to_file1_pipe, 8, 1, NULL, log);
	Pipe_Create(write_to_file2_pipe, 8, 1, NULL, log);

	Pipe_Connect(&increment_pipe1, &increment_pipe3);
	Pipe_Connect(&increment_pipe2, &increment_pipe3);
	Pipe_Connect(&increment_pipe3, &write_to_file1_pipe);
	Pipe_Connect(&increment_pipe3, &write_to_file2_pipe);

	/* Create Threads */
	CreateThread(NULL, 0, incrementThread1, &increment_pipe1, 0, NULL);
	CreateThread(NULL, 0, incrementThread2, &increment_pipe2, 0, NULL);
	CreateThread(NULL, 0, incrementThread3, &increment_pipe3, 0, NULL);

	HANDLE thread1Handle, thread2Handle;

	thread1Handle = CreateThread(NULL, 0, Thread1, &write_to_file1_pipe, 0, NULL);
	thread2Handle = CreateThread(NULL, 0, Thread2, &write_to_file2_pipe, 0, NULL);

	HANDLE threadHandles[] = { thread1Handle, thread2Handle };
	WaitForMultipleObjects(2, threadHandles, TRUE, INFINITE);

	CloseHandle(thread1Handle);
	CloseHandle(thread2Handle);

	return 0;
}