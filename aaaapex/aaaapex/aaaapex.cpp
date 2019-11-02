#include "global.h"
#include "lock.h"

SOCKET g_Sock;
DWORD g_PID;
uint64_t g_Base;
bool g_Locked;

#define SMOOTH 2

Vector GetEntityBasePosition(uintptr_t ent)
{
	return driver::read<Vector>(g_Sock, g_PID, ent + OFFSET_ORIGIN);
}

uintptr_t GetEntityBoneArray(uintptr_t ent)
{
	return driver::read<uintptr_t>(g_Sock, g_PID, ent + OFFSET_BONES);
}

Vector GetEntityBonePosition(uintptr_t ent, uint32_t BoneId, Vector BasePosition)
{
	unsigned long long pBoneArray = GetEntityBoneArray(ent);

	Vector EntityHead = Vector();

	EntityHead.x = driver::read<float>(g_Sock, g_PID, pBoneArray + 0xCC + (BoneId * 0x30)) + BasePosition.x;
	EntityHead.y = driver::read<float>(g_Sock, g_PID, pBoneArray + 0xDC + (BoneId * 0x30)) + BasePosition.y;
	EntityHead.z = driver::read<float>(g_Sock, g_PID, pBoneArray + 0xEC + (BoneId * 0x30)) + BasePosition.z;

	return EntityHead;
}

Vector GetViewAngles(uintptr_t ent) 
{
	return driver::read<Vector>(g_Sock, g_PID, ent + OFFSET_VIEWANGLES);
}

void SetViewAngles(uintptr_t ent, Vector angles)
{
	driver::write<Vector>(g_Sock, g_PID, ent + OFFSET_VIEWANGLES, angles);
}

Vector GetCamPos(uintptr_t ent)
{
	return driver::read<Vector>(g_Sock, g_PID, ent + OFFSET_CAMERAPOS);
}

/*void dump(uintptr_t baseaddr, uintptr_t size, int readsize)
{
	FILE* pFile;
	pFile = fopen("dump.bin", "wb");
	for (uintptr_t i = baseaddr; i < (baseaddr + size); i += readsize)
	{
		uint64_t readshit = driver::read<uint64_t>(g_Sock, g_PID, i);
		fwrite(&readshit, 1, readsize, pFile);
		Console::Debug("%llx", i);
		
	}
	fclose(pFile);
}*/

uint64_t aimentity = 0;
void aimthread() 
{	
	while (true) 
	{
		if (!(GetKeyState(0x06) & 0x8000)) 
		{
			Sleep(10);
			continue;
		}
		
		while (g_Locked) Sleep(1);
		g_Locked = true;
		Sleep(20);

		uint64_t localent = driver::read<uint64_t>(g_Sock, g_PID, g_Base + OFFSET_LOCAL_ENT);
		Vector LocalCamera = GetCamPos(localent);
		Vector ViewAngles = GetViewAngles(localent);
		Vector FeetPosition = GetEntityBasePosition(aimentity);
		Vector HeadPosition = GetEntityBonePosition(aimentity, 8, FeetPosition);
		Vector CalculatedAngles = *(Vector*)(&(Math::CalcAngle(LocalCamera, HeadPosition)));

		Vector Delta = (CalculatedAngles - ViewAngles) / SMOOTH;
		Vector SmoothedAngles = ViewAngles + Delta;

		SetViewAngles(localent, SmoothedAngles);

		g_Locked = false;
	}
}

int main()
{
	Console::PrintTitle("aaaapex");

	Console::Info("Connecting to driver...");
	
	Console::Debug("Initializing...");
	driver::initialize();
	g_Locked = false;

	Console::Debug("Connecting...");
	g_Sock = driver::connect();
	if (g_Sock == INVALID_SOCKET)
	{
		Console::Error("Connection failed!");
		Console::WaitAndExit();
	}

	Console::Info("Getting PID of game...");

	Console::Debug("Getting PID using CreateToolhelp32Snapshot...");
	g_PID = Utils::GetPID(L"r5apex.exe");
	if (g_PID == 0) 
	{
		Console::Error("Failed to get PID! Please make sure the game is running.");
		Console::WaitAndExit();
	}
	Console::Debug("Game PID should be %u.", g_PID);

	Console::Info("Getting base address...");

	Console::Debug("Using driver to obtain base...");
	g_Base = driver::get_process_base_address(g_Sock, g_PID);
	if (g_Base == 0) 
	{
		Console::Error("Failed  to get base address!");
		Console::WaitAndExit();
	}
	Console::Debug("Base address should be %llx.", g_Base);

	Console::Info("Running threads...");
	Console::Debug("Aimbot thread...");
	std::thread taim(aimthread);

	//dump(g_Base, 3221225472, 8);

	while (!(GetKeyState(0x73) & 0x8000)) // F4
	{
		Sleep(1);
		uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

		while (g_Locked) Sleep(1);
		g_Locked = true;
		Sleep(30);
		
		uint64_t baseent = driver::read<uint64_t>(g_Sock, g_PID, entitylist);
		if (baseent == 0) 
		{
			continue;
		}

		Vector vec = { 0, 0, 0 };
		for (int i = 1; i <= 100; i++) // TODO: Check if 1 is local
		{			
			// ---------------------------------------------------------------
			// ENTITY BASE CODE	
			// ---------------------------------------------------------------
			uint64_t centity = driver::read<uint64_t>(g_Sock, g_PID, entitylist + ((uint64_t)i << 5));

			// ---------------------------------------------------------------
			// ENGINE GLOW	
			// ---------------------------------------------------------------			
			int health = driver::read<int>(g_Sock, g_PID, centity + OFFSET_HEALTH);
			if (health < 1 || health > 100)
				continue;

			int shield = driver::read<int>(g_Sock, g_PID, centity + OFFSET_SHIELD);
			int total = health + shield;
			
			float green = 0;
			float red = 0;
			float blue = 0;
			if (total > 100)
			{
				total -= 100;
				blue = (float)total / 125.f;
				green = (125.f - (float)total) / 125.f;
			}
			else 
			{
				green = (float)total / 100.f;
				red = (100.f - (float)total) / 100.f;
			}

			driver::write(g_Sock, g_PID, centity + OFFSET_GLOW_ENABLE, true);
			driver::write(g_Sock, g_PID, centity + OFFSET_GLOW_CONTEXT, 1);

			// TODO: Color by health
			driver::write(g_Sock, g_PID, centity + OFFSET_GLOW_COLORS, red);
			driver::write(g_Sock, g_PID, centity + OFFSET_GLOW_COLORS + 0x4, green);
			driver::write(g_Sock, g_PID, centity + OFFSET_GLOW_COLORS + 0x8, blue);

			for (int offset = 0x2C8; offset <= 0x2E0; offset += 0x4)
				driver::write(g_Sock, g_PID, centity + offset, FLT_MAX);

			driver::write(g_Sock, g_PID, centity + OFFSET_GLOW_RANGE, FLT_MAX);
			//driver::write(g_Sock, g_PID, centity + OFFSET_GLOW_MAGIC, 0x4D407D7E);

			// ---------------------------------------------------------------
			// AIMBOT PREPARATION
			// ---------------------------------------------------------------			
			uint64_t localent = driver::read<uint64_t>(g_Sock, g_PID, g_Base + OFFSET_LOCAL_ENT);

			if (localent == centity) 
			{
				continue;
			}

			Vector LocalCamera = GetCamPos(localent);
			Vector ViewAngles = GetViewAngles(localent);
			Vector FeetPosition = GetEntityBasePosition(centity);
			Vector HeadPosition = GetEntityBonePosition(centity, 8, FeetPosition);
			Vector CalculatedAngles = *(Vector*)(&(Math::CalcAngle(LocalCamera, HeadPosition)));
			Vector Delta = (CalculatedAngles - ViewAngles);
			
			Vector blank = { 0, 0, 0 };
			if (vec == blank) 
			{
				vec = Delta;
			}
			else 
			{
				/*float first = abs(CalculatedAngles.x) + abs(CalculatedAngles.y);
				float second = abs(vec.x) + abs(vec.y);*/
				/*float first = abs(Delta.y) + abs(Delta.x);
				float second = abs(vec.y) + abs(vec.x);*/

				float first = (abs(Delta.y) + abs(Delta.x)) / 2;
				float second = (abs(vec.y) + abs(vec.x)) / 2;

				if (first < second) 
				{
					vec = Delta;
					aimentity = centity;
				}
			}			
		}
		g_Locked = false;
	}

	Console::Debug("Deinitializing...");
	driver::deinitialize();

	std::cout << "Press any key to exit...\n";
	Console::WaitForInput();
	return 0;
}
