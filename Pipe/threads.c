#include <stdio.h>
#include <stdint.h>
#include <strsafe.h>
#include <windows.h>

#include "ringbuffer.h"
#include "hub.h"

#define NUMBER_OF_ELEMENTS 1000

/* Integrate every element of the signal. */
static void increment(hub_t * const hub)
{
	while (Hub_IsFilled(hub))
	{
		uint32_t item = Hub_Read(hub);
		
		// do something
		
		Hub_Write(hub, item);
	}
}

DWORD WINAPI Thread1(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;

	/* Generate input */
	for (int i = 0; i < NUMBER_OF_ELEMENTS; i++)
		Hub_Insert(hub, 42);
	increment(hub);

	Hub_Write(hub, UINT32_MAX);

	return 0;
}

DWORD WINAPI Thread2(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;

	/* Generate input */
	for (int i = 0; i < NUMBER_OF_ELEMENTS; i++)
		Hub_Insert(hub, 23);
	increment(hub);

	Hub_Write(hub, UINT32_MAX);

	return 0;
}

DWORD WINAPI Thread3(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;
	uint8_t symbol_counter = 0;
	
	while (symbol_counter < 2)
	{
		while (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);

			if (item == UINT32_MAX)
			{
				symbol_counter++;
				break;
			}

			if (item != 42 && item != 23)
				printf("failed\n");

			Hub_Write(hub, item);
		}
	}

	Hub_Write(hub, UINT32_MAX);

	return 0;
}

DWORD WINAPI Thread4(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;
	
	while (item != UINT32_MAX)
	{
		while (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);

			if (item == UINT32_MAX)
				break;

			if (item != 42 && item != 23)
				printf("failed\n");

			Hub_Write(hub, item);
		}
	}

	Hub_Write(hub, UINT32_MAX);

	return 0;
}

DWORD WINAPI Thread5(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;
	uint8_t check = 1;

	while (item != UINT32_MAX)
	{
		while (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);
			if (item == 0 || item > 102)
			{
				check = 0;
			}

			if (item == UINT32_MAX)
				break;

			Hub_Write(hub, item);
		}
	}

	Hub_Write(hub, UINT32_MAX);

	return 0;
}

void Run_Threads(hub_t * hub1, hub_t * hub2, hub_t * hub3, hub_t * hub4, hub_t * hub5)
{
	/* Create Threads */
	HANDLE threadHandles[5];

	threadHandles[0] = CreateThread(NULL, 0, Thread1, hub1, 0, NULL);
	threadHandles[1] = CreateThread(NULL, 0, Thread2, hub2, 0, NULL);
	threadHandles[2] = CreateThread(NULL, 0, Thread3, hub3, 0, NULL);
	threadHandles[3] = CreateThread(NULL, 0, Thread4, hub4, 0, NULL);
	threadHandles[4] = CreateThread(NULL, 0, Thread5, hub5, 0, NULL);

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

void threads(uint32_t loops)
{
	/* Create hubs and connect them */
	Hub_Create(increment_hub1, 1, 1, NULL);
	Hub_Create(increment_hub2, 1, 1, NULL);
	Hub_Create(increment_hub3, 2, 2, NULL);
	Hub_Create(increment_hub4, 1, 1, NULL);
	Hub_Create(increment_hub5, 1, 1, NULL);
	Hub_Create(check_hub1, 1, 1, NULL);
	Hub_Create(check_hub2, 1, 1, NULL);

	Hub_CreateInputBuffer(increment_hub1, NUMBER_OF_ELEMENTS+2);
	Hub_CreateInputBuffer(increment_hub2, NUMBER_OF_ELEMENTS+2);

	Hub_CreateConnection(increment_hub1, increment_hub3, NUMBER_OF_ELEMENTS+2);
	Hub_CreateConnection(increment_hub2, increment_hub3, NUMBER_OF_ELEMENTS+2);
	Hub_CreateConnection(increment_hub3, increment_hub4, NUMBER_OF_ELEMENTS * 2 + 2);
	Hub_CreateConnection(increment_hub3, increment_hub5, NUMBER_OF_ELEMENTS * 2 + 2);
	Hub_CreateConnection(increment_hub4, check_hub1, NUMBER_OF_ELEMENTS * 2 + 2);
	Hub_CreateConnection(increment_hub5, check_hub2, NUMBER_OF_ELEMENTS * 2 + 2);

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
	for (uint32_t i = 0; i < loops; i++)
	{
		Run_Threads(&increment_hub1, &increment_hub2, &increment_hub3, &increment_hub4, &increment_hub5);

		//if (ringbuffer_valid_and_equal(check_hub1.input_connector->connection[0], check_hub2.input_connector->connection[0], hFile))
		if (ringbuffer_valid_and_equal(check_hub1.input_connector->connection[0], check_hub2.input_connector->connection[0]))
			success_counter++;
	}
	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	int seconds = (int)ElapsedMicroseconds.QuadPart / 1000000;

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