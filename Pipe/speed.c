#include <stdio.h>
#include <stdint.h>
#include <strsafe.h>
#include <windows.h>

#include "ringbuffer.h"
#include "hub.h"

uint32_t counter;

DWORD WINAPI Speed_Thread1(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;

	/* Generate input */
	for (uint32_t i = 0; i < counter; i++)
	{
		while(Hub_IsFull(hub)) {}
		Hub_Write(hub, i);
	}
	while (Hub_IsFull(hub)) {}
	Hub_Write(hub, UINT32_MAX);

	return 0;
}

DWORD WINAPI Speed_Thread2(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;

	while (item != UINT32_MAX)
	{
		if (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);
			//printf("%d\n", item);
			while (Hub_IsFull(hub)) {}
			Hub_Write(hub, item);
		}
	}

	return 0;
}

DWORD WINAPI Speed_Thread3(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;

	while (item != UINT32_MAX)
	{
		if (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);
			while (Hub_IsFull(hub)) {}
			Hub_Write(hub, item);
		}
	}

	return 0;
}

DWORD WINAPI Speed_Thread4(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;

	while (item != UINT32_MAX)
	{
		if (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);
			while (Hub_IsFull(hub)) {}
			Hub_Write(hub, item);
		}
	}

	return 0;
}

DWORD WINAPI Speed_Thread5(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;

	while (item != UINT32_MAX)
	{
		if (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);
			while (Hub_IsFull(hub)) {}
			Hub_Write(hub, item);
		}
	}

	return 0;
}

DWORD WINAPI Speed_Thread6(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;

	while (item != UINT32_MAX)
	{
		if (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);
			while (Hub_IsFull(hub)) {}
			Hub_Write(hub, item);
		}
	}

	return 0;
}

DWORD WINAPI Speed_Thread7(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;

	while (item != UINT32_MAX)
	{
		if (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);
			while (Hub_IsFull(hub)) {}
			Hub_Write(hub, item);
		}
	}

	return 0;
}

DWORD WINAPI Speed_Thread8(LPVOID lpParam)
{
	hub_t * hub = (hub_t *)lpParam;
	uint32_t item = 0;

	while (item != UINT32_MAX)
	{
		if (Hub_IsFilled(hub))
		{
			item = Hub_Read(hub);
		}
	}

	return 0;
}

void speed(uint32_t items_count)
{
	counter = items_count;

	/* Create hubs and connect them */
	Hub_Create(hub1, 1, 1, NULL);
	Hub_Create(hub2, 1, 1, NULL);
	Hub_Create(hub3, 1, 1, NULL);
	Hub_Create(hub4, 1, 1, NULL);
	Hub_Create(hub5, 1, 1, NULL);
	Hub_Create(hub6, 1, 1, NULL);
	Hub_Create(hub7, 1, 1, NULL);
	Hub_Create(hub8, 1, 1, NULL);

#define RINGBUFFER_SIZE 1024
	Hub_CreateConnection(hub1, hub2, RINGBUFFER_SIZE);
	Hub_CreateConnection(hub2, hub3, RINGBUFFER_SIZE);
	Hub_CreateConnection(hub3, hub4, RINGBUFFER_SIZE);
	Hub_CreateConnection(hub4, hub5, RINGBUFFER_SIZE);
	Hub_CreateConnection(hub5, hub6, RINGBUFFER_SIZE);
	Hub_CreateConnection(hub6, hub7, RINGBUFFER_SIZE);
	Hub_CreateConnection(hub7, hub8, RINGBUFFER_SIZE);

	SYSTEMTIME local_time;
	GetLocalTime(&local_time);

	printf("\n");
	printf("Starting Speed Test\n");
	printf("at %02d/%02d/%02d %02d:%02d:%02d\n", local_time.wYear, local_time.wMonth, local_time.wDay, local_time.wHour, local_time.wMinute, local_time.wSecond);
	printf("\n");

	LARGE_INTEGER Frequency;
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);


	/* Create Threads */
	HANDLE threadHandles[8];

	threadHandles[0] = CreateThread(NULL, 0, Speed_Thread1, &hub1, 0, NULL);
	threadHandles[1] = CreateThread(NULL, 0, Speed_Thread2, &hub2, 0, NULL);
	threadHandles[2] = CreateThread(NULL, 0, Speed_Thread3, &hub3, 0, NULL);
	threadHandles[3] = CreateThread(NULL, 0, Speed_Thread4, &hub4, 0, NULL);
	threadHandles[4] = CreateThread(NULL, 0, Speed_Thread5, &hub5, 0, NULL);
	threadHandles[5] = CreateThread(NULL, 0, Speed_Thread6, &hub6, 0, NULL);
	threadHandles[6] = CreateThread(NULL, 0, Speed_Thread7, &hub7, 0, NULL);
	threadHandles[7] = CreateThread(NULL, 0, Speed_Thread8, &hub8, 0, NULL);

	WaitForMultipleObjects(8, threadHandles, TRUE, INFINITE);

	CloseHandle(threadHandles[0]);
	CloseHandle(threadHandles[1]);
	CloseHandle(threadHandles[2]);
	CloseHandle(threadHandles[3]);
	CloseHandle(threadHandles[4]);
	CloseHandle(threadHandles[5]);
	CloseHandle(threadHandles[6]);
	CloseHandle(threadHandles[7]);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	int seconds = (int)ElapsedMicroseconds.QuadPart / 1000000;

	int hours = seconds / 3600;
	int minutes = (seconds - (hours * 3600)) / 60;
	seconds = seconds - (minutes * 60);

	printf("Count of Concurrent Data Operations: %d\n", items_count * 14);

	printf("\n");
	GetLocalTime(&local_time);
	printf("Test finished\n");
	printf("at %02d/%02d/%02d %02d:%02d:%02d\n", local_time.wYear, local_time.wMonth, local_time.wDay, local_time.wHour, local_time.wMinute, local_time.wSecond);
	printf("Elapsed time: %02d:%02d:%02d", hours, minutes, seconds);
	printf("\n");
}