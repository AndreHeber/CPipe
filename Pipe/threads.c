#include <stdio.h>
#include <stdint.h>
#include <strsafe.h>
#include <windows.h>

#include "ringbuffer.h"
#include "pipe.h"

#define NUMBER_OF_ELEMENTS 1000

/* Integrate every element of the signal. */
static void increment(pipe_t * const pipe)
{
	uint8_t check = 1;
	while (Pipe_IsFilled(pipe))
	{
		uint32_t item = Pipe_Read(pipe);
		
		// do something
		
		Pipe_Write(pipe, item);
	}
}

DWORD WINAPI Thread1(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;

	/* Generate input */
	for (int i = 0; i < NUMBER_OF_ELEMENTS; i++)
		Pipe_Insert(pipe, 42);
	increment(pipe);

	Pipe_Write(pipe, UINT32_MAX);

	return 0;
}

DWORD WINAPI Thread2(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;

	/* Generate input */
	for (int i = 0; i < NUMBER_OF_ELEMENTS; i++)
		Pipe_Insert(pipe, 23);
	increment(pipe);

	Pipe_Write(pipe, UINT32_MAX);

	return 0;
}

DWORD WINAPI Thread3(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;
	uint32_t item = 0;
	uint8_t check = 1;
	uint8_t symbol_counter = 0;
	
	while (symbol_counter < 2)
	{
		while (Pipe_IsFilled(pipe))
		{
			item = Pipe_Read(pipe);

			if (item == UINT32_MAX)
			{
				symbol_counter++;
				break;
			}

			if (item != 42 && item != 23)
				printf("failed\n");

			Pipe_Write(pipe, item);
		}
	}

	Pipe_Write(pipe, UINT32_MAX);

	return 0;
}

DWORD WINAPI Thread4(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;
	uint32_t item = 0;
	uint8_t check = 1;
	
	while (item != UINT32_MAX)
	{
		while (Pipe_IsFilled(pipe))
		{
			item = Pipe_Read(pipe);

			if (item == UINT32_MAX)
				break;

			if (item != 42 && item != 23)
				printf("failed\n");

			Pipe_Write(pipe, item);
		}
	}

	Pipe_Write(pipe, UINT32_MAX);

	return 0;
}

DWORD WINAPI Thread5(LPVOID lpParam)
{
	pipe_t * pipe = (pipe_t *)lpParam;
	uint32_t item = 0;
	uint8_t check = 1;

	while (item != UINT32_MAX)
	{
		while (Pipe_IsFilled(pipe))
		{
			item = Pipe_Read(pipe);
			if (item == 0 || item > 102)
			{
				check = 0;
			}

			if (item == UINT32_MAX)
				break;

			Pipe_Write(pipe, item);
		}
	}

	Pipe_Write(pipe, UINT32_MAX);

	return 0;
}

void Run_Threads(pipe_t * pipe1, pipe_t * pipe2, pipe_t * pipe3, pipe_t * pipe4, pipe_t * pipe5)
{
	/* Create Threads */
	HANDLE threadHandles[5];

	threadHandles[0] = CreateThread(NULL, 0, Thread1, pipe1, 0, NULL);
	threadHandles[1] = CreateThread(NULL, 0, Thread2, pipe2, 0, NULL);
	threadHandles[2] = CreateThread(NULL, 0, Thread3, pipe3, 0, NULL);
	threadHandles[3] = CreateThread(NULL, 0, Thread4, pipe4, 0, NULL);
	threadHandles[4] = CreateThread(NULL, 0, Thread5, pipe5, 0, NULL);

	WaitForMultipleObjects(5, threadHandles, TRUE, INFINITE);

	CloseHandle(threadHandles[0]);
	CloseHandle(threadHandles[1]);
	CloseHandle(threadHandles[2]);
	CloseHandle(threadHandles[3]);
	CloseHandle(threadHandles[4]);
}

uint8_t ringbuffer_valid_and_equal(ringbuffer_t * rb1, ringbuffer_t * rb2)
{
	//char DataBuffer[128];
	//DWORD dwBytesWritten;

	uint32_t element1 = 0;
	uint32_t element2 = 0;
	uint32_t element_42_counter = 0;
	uint32_t element_23_counter = 0;

	while (1)
	{
		element1 = RingBuffer_Read(rb1);
		element2 = RingBuffer_Read(rb2);

		if (element1 != element2)
			printf("Elements unequal!\n");

		if (element1 == UINT32_MAX)
			break;

		if (element1 == 42)
		{
			element_42_counter++;
			//sprintf_s(DataBuffer, 128, "%u - %u\r\n", element1, element_42_counter);
		}

		if (element1 == 23)
		{
			element_23_counter++;
			//sprintf_s(DataBuffer, 128, "%u - %u\r\n", element1, element_23_counter);
		}

		//WriteFile(h, DataBuffer, (DWORD)strlen(DataBuffer), &dwBytesWritten, NULL);
	}

	if ((element_42_counter != element_23_counter) || (element_42_counter != NUMBER_OF_ELEMENTS))
	{
		printf("error!\n");
		return 0;
	}
	return 1;
}

static void log(pipe_t * const source, pipe_t * const target, uint32_t element)
{
	/*wchar_t text[64];
	swprintf_s(text, 64, L"%s: %d\n", source->name, element);
	OutputDebugStringW(text);*/
}

void threads(uint32_t loops)
{
	/* Create pipes and connect them */
	Pipe_Create(increment_pipe1, 1, 1, NULL, log);
	Pipe_Create(increment_pipe2, 1, 1, NULL, log);
	Pipe_Create(increment_pipe3, 2, 2, NULL, log);
	Pipe_Create(increment_pipe4, 1, 1, NULL, log);
	Pipe_Create(increment_pipe5, 1, 1, NULL, log);
	Pipe_Create(check_pipe1, 1, 1, NULL, log);
	Pipe_Create(check_pipe2, 1, 1, NULL, log);

	Pipe_CreateInputBuffer(increment_pipe1, NUMBER_OF_ELEMENTS+2);
	Pipe_CreateInputBuffer(increment_pipe2, NUMBER_OF_ELEMENTS+2);

	Pipe_CreateConnection(increment_pipe1, increment_pipe3, NUMBER_OF_ELEMENTS+2);
	Pipe_CreateConnection(increment_pipe2, increment_pipe3, NUMBER_OF_ELEMENTS+2);
	Pipe_CreateConnection(increment_pipe3, increment_pipe4, NUMBER_OF_ELEMENTS * 2 + 2);
	Pipe_CreateConnection(increment_pipe3, increment_pipe5, NUMBER_OF_ELEMENTS * 2 + 2);
	Pipe_CreateConnection(increment_pipe4, check_pipe1, NUMBER_OF_ELEMENTS * 2 + 2);
	Pipe_CreateConnection(increment_pipe5, check_pipe2, NUMBER_OF_ELEMENTS * 2 + 2);

	//HANDLE hFile;
	//hFile = CreateFile("buffer.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	SYSTEMTIME local_time;
	GetLocalTime(&local_time);

	printf("\n");
	printf("Starting Thread Test\n");
	printf("at %02d/%02d/%02d %02d:%02d:%02d\n", local_time.wYear, local_time.wMonth, local_time.wDay, local_time.wHour, local_time.wMinute, local_time.wSecond);
	printf("\n");

	LARGE_INTEGER Frequency;
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);
	
	uint32_t success_counter = 0;
	for (int i = 0; i < loops; i++)
	{
		Run_Threads(&increment_pipe1, &increment_pipe2, &increment_pipe3, &increment_pipe4, &increment_pipe5);

		//if (ringbuffer_valid_and_equal(check_pipe1.input_connector->connection[0], check_pipe2.input_connector->connection[0], hFile))
		if (ringbuffer_valid_and_equal(check_pipe1.input_connector->connection[0], check_pipe2.input_connector->connection[0]))
			success_counter++;
	}
	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	int seconds = ElapsedMicroseconds.QuadPart / 1000000;

	int hours = seconds / 3600;
	int minutes = (seconds - (hours * 3600)) / 60;
	seconds = seconds - (minutes * 60);
	
	printf("Loops: %d\n", loops);
	printf("Successfull: %d\n", success_counter);
	printf("Failed: %d\n", loops - success_counter);
	printf("Count of Concurrent Data Operations: %d\n", 2 * NUMBER_OF_ELEMENTS * loops);

	printf("\n");
	GetLocalTime(&local_time);
	printf("Test finished\n");
	printf("at %02d/%02d/%02d %02d:%02d:%02d\n", local_time.wYear, local_time.wMonth, local_time.wDay, local_time.wHour, local_time.wMinute, local_time.wSecond);
	printf("Elapsed time: %02d:%02d:%02d", hours, minutes, seconds);
	printf("\n");


	//CloseHandle(hFile);
}