#include "pch.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#if defined(_WIN32) || defined(WIN32)
#define WINDOWS
#include "Windows.h"
#include <tlhelp32.h>
#endif

using namespace std;
typedef LONG(NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);


/*
*	Get all process ids with the current target executable name.
*/
vector<DWORD> getPIDS(wstring target) {
	vector<DWORD> pids;
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof entry;

	if (!Process32FirstW(snap, &entry)) {
		return pids;
	}

	// Get all processes with the exe file matching the provided target.
	do {
		if (wstring(entry.szExeFile) == target) {
			pids.emplace_back(entry.th32ProcessID);
		}
	} while (Process32NextW(snap, &entry));
	return pids;
}

/*
*	Get the HANDLE for the current process ID.
*	Modified from https://stackoverflow.com/questions/11010165/how-to-suspend-resume-a-process-in-windows
*/
HANDLE getHThread(DWORD processId)
{
	HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	THREADENTRY32 threadEntry;
	threadEntry.dwSize = sizeof(THREADENTRY32);

	Thread32First(hThreadSnapshot, &threadEntry);
	do
	{
		if (threadEntry.th32OwnerProcessID == processId)
		{
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
				threadEntry.th32ThreadID);

			return hThread;
		}
	} while (Thread32Next(hThreadSnapshot, &threadEntry));

	CloseHandle(hThreadSnapshot);
	return HANDLE();
}

/*
*	Create the Private Public GTA5 server on the provided pid.
*/
void CreatePrivatePublic(DWORD pid) {
	std::cout << "Suspending GTA!" << endl;
	HANDLE gameThread = getHThread(pid);
	SuspendThread(gameThread);
	this_thread::sleep_for(chrono::seconds(8));
	ResumeThread(gameThread);
	cout << "Finished Suspending!" << endl;
}

int main()
{
#ifndef WINDOWS
	cout << "System must be windows to run this application." << endl;
	return -1;
#endif
	cout << "Fetching GTA5 process ids..." << endl;
	vector<DWORD> pids = getPIDS(L"GTA5.exe");

	// If there are no GTA5 games running
	if (pids.size() == 0) {
		cout << "GTA5 was not detected as running." << endl;
		cout << "Press Enter to Continue";
		cin.ignore();
		return -2;
	}


	// If there is exactally one copy.
	if (pids.size() == 1) {
		CreatePrivatePublic(pids[0]);
	}

	// If there is more than one copy of GTA5
	if (pids.size() > 1) {
		cout << "More than one GTA5 detected. Available pids:" << endl;
		for (unsigned int i = 0; i < pids.size(); i++) {
			cout << "[" << i << "] - PID: " << pids[i] << endl;
		}

		// Get the users choice.
		int choice;
		while (true) {
			cout << "Enter the id of the process you want the private public server on: ";
			cin >> choice;
			if (choice < 0 || choice > pids.size() - 1) {
				cout << "Invalid pid entered." << endl;
			}
			else {
				break;
			}
		}
		CreatePrivatePublic(pids[choice]);
	}

	cout << "Press Enter to Continue" << endl;
	cin.ignore();
	return 0;
}