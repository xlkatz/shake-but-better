#include "pch.h"

namespace BO2
{

#define LODWORD(x)  (*((DWORD*)&(x)))  // low dword
#define HIDWORD(x)  (*((DWORD*)&(x)+1))
	DWORD* bdTaskResult()
	{
		DWORD* result;
		*result = 0x820F6E58;
		return result;
	}
	DWORD* bdStatsInfoPtr() {
		DWORD* result;
		*result = 0x820FB3B0;
		return result;
	}

	int bdStatsInfo(int a1)
	{
		__int64 v2; // r11

		bdTaskResult();
		LODWORD(v2) = 0;
		HIDWORD(v2);
		*(DWORD*)(a1 + 4) = 0; //leaderboard id
		*(DWORD*)a1;
		*(QWORD*)(a1 + 8) = v2;
		*(DWORD*)(a1 + 16) = 1;
		*(QWORD*)(a1 + 24) = v2;
		*(QWORD*)(a1 + 32) = v2;
		*(DWORD*)(a1 + 108) = 0;
		memset(&a1 + 40, 0, 65);
		return a1;
	}
	void leaderboard() {}

	void DrawMenuText()
	{
		options.menuMaxScroll = 0;
		options.menuOptionIndex = 0;

		SubMenuMenuOption_List.clear();
		BoolMenuOption_List.clear();
		IntMenuOption_List.clear();
		FloatMenuOption_List.clear();

		switch (options.menuPageIndex)
		{
		case MAIN:
			if (cgGame->clientNum == 0) {
				DrawToggle("Spoof Rank", &options.BoolRank);

			}
			DrawToggle("No Recoil", &options.NoRecoil);
			DrawToggle("Scoreboard (test)", &options.Scoreboard);
			DrawStringSlider("Font", &options.menuFontIndex, FontForIndex(options.menuFontIndex.current));
			break;
		case AIMBOT:
			DrawToggle("Aimbot", &options.AimbotToggle);
			DrawToggle("Aim Required", &options.AimRequired);
			break;
		case VISUALS:
			DrawToggle("Esp Box", &options.EspBoxToggle);
			DrawToggle("Esp Bones", &options.EspDrawBones);
			DrawToggle("Esp BoxFrog", &options.EspFrogChan);
			DrawToggle("Esp DrawLine", &options.EspDrawLine);
			DrawToggle("Esp Item", &options.DrawItem);
			DrawToggle("Wallhack", &options.Wallhack);
			break;
		case PLAYERS:
			for (int i = 0; i < 18; i++) {
				if (!strcmp(cgGame->clientInfo[i].name, ""))
					DrawButton("N/A");
				else
					DrawButton(va("[%i] %s [%s]", i, cgGame->clientInfo[i].name, isTeam(&cg_entitiesArray[i]) ? "^2Friendly^7" : "^1Enemy^7"));
			}
			break;

		case SETTINGS:
			DrawIntSlider("Menu X", &options.menuX, "%i");
			DrawIntSlider("Menu Y", &options.menuY, "%i");
			DrawToggle("Ip Spoof", &options.IpSpoof);
			break;
		}
	}

	void ServerInfo() {
		readStructs();

		DrawTextInBox("Shake BO2 Alpha", cgDC->screenWidth - cgDC->screenWidth + 5, cgDC->screenHeight - cgDC->screenHeight + 5, R_TextWidth(0, "Shake BO2 Alpha", MAXLONG, R_RegisterFont(FontForIndex(options.menuFontSize.current), 0)) * 0.65 + 26, R_TextHeight(R_RegisterFont("fonts/720/normalfont", 0)));
		DrawTextInBox(va("Host: %s", cgServer->hostName), cgDC->screenWidth - cgDC->screenWidth + 5, cgDC->screenHeight - cgDC->screenHeight + 37, R_TextWidth(0, va("Host: %s", cgServer->hostName), MAXLONG, R_RegisterFont(FontForIndex(options.menuFontSize.current), 0)) * 0.65 + 26, R_TextHeight(R_RegisterFont("fonts/720/normalfont", 0)));
		DrawTextInBox(va("Map: %s", cgServer->MapName), cgDC->screenWidth - cgDC->screenWidth + 5, cgDC->screenHeight - cgDC->screenHeight + 67, R_TextWidth(0, va("Map: %s", cgServer->MapName), MAXLONG, R_RegisterFont(FontForIndex(options.menuFontSize.current), 0)) * 0.65 + 40, R_TextHeight(R_RegisterFont("fonts/720/normalfont", 0)));
		DrawTextInBox(va("GameType: %s", cgServer->gametype), cgDC->screenWidth - cgDC->screenWidth + 5, cgDC->screenHeight - cgDC->screenHeight + 98, R_TextWidth(0, va("GameType: %s", cgServer->gametype), MAXLONG, R_RegisterFont(FontForIndex(options.menuFontSize.current), 0)) * 0.65 + 28, R_TextHeight(R_RegisterFont("fonts/720/normalfont", 0)));
		DrawTextInBox(va("Fps: %g", cgDC->FPS), cgDC->screenWidth - cgDC->screenWidth + 5, cgDC->screenHeight - cgDC->screenHeight + 5 + 125, R_TextWidth(0, va("Fps: %g", cgDC->FPS), MAXLONG, R_RegisterFont(FontForIndex(options.menuFontSize.current), 0)) * 0.65 + 14, R_TextHeight(R_RegisterFont("fonts/720/normalfont", 0)));
	}

	void DrawMenu()
	{
		DrawMenuShader();
		DrawMenuTabs();

		//Options
		DrawMenuText();
	}

	int nearestClient;
	bool playerReady;
	vec3_t anglesOut;
	const char* Tag;

	int GetNearestPlayer(int client)
	{
		nearestClient = -1;
		float nearestDistance = FLT_MAX;
		for (int i = 0; i < 18; ++i)
		{
			if (cgGame->clientNum == i)
				continue;
			if (cg_entitiesArray[i].pose.eType != ET_PLAYER)
				continue;

			if (isTeam(&cg_entitiesArray[i]))
				continue;

			if (cg_entitiesArray[i].WeaponID == 89)
				Tag = "j_ankle_ri";
			else
				Tag = "j_neck";

			float Distance = cg_entitiesArray[client].pose.Origin.GetDistance(cg_entitiesArray[i].pose.Origin);
			if (AimTarget_IsTargetVisible(0, &cg_entitiesArray[i]))
			{
				if (Distance < nearestDistance)
				{
					nearestDistance = Distance;
					nearestClient = i;
					playerReady = true;
				}
			}
		}
		return nearestClient;
	}

	void doAimbot()
	{
		if (options.AimbotToggle.state) {

			if (!Dvar_GetBool("cl_ingame"))
				return;
			if (cgGame->ps.health < 1)
				return;

			int nearestClient = GetNearestPlayer(cgGame->clientNum);
			if (playerReady && nearestClient != -1)
			{

				vec3_t Difference = AimTarget_GetTagPos(&cg_entitiesArray[nearestClient], Tag);
				vec3_t Angles = Difference - cgGame->refdef.viewOrigin;
				VecToAngels(Angles, anglesOut);
				if (nearestClient != cgGame->clientNum)
					ClientActive->viewAngle = anglesOut - ClientActive->baseAngle;

				options.NoRecoil.state = true;
			}
			playerReady = false;

		}

	}

	void NoSpread(Usercmd_t* cmd) {
		unsigned int CmdTimeSeed = cgGame->ps.commandTime;
		float minSpread, maxSpread;

		BG_seedRandWithGameTime(&CmdTimeSeed);
		G_GetSpreadForWeapon(playerstate, cg_entitiesArray->WeaponID, &minSpread, &maxSpread);

		double spread;

	}

	void Menu_PaintAll(int r3)
	{
		MinHook[0].Stub(r3);
		readStructs();

		if (Dvar_GetBool("cl_ingame"))
		{
			for (int i = 0; i < 18; i++)
			{
				if (!(cg_entitiesArray[i].pose.eType == ET_PLAYER && (cg_entitiesArray[i].pose.eType != ET_PLAYER_CORPSE)))
					continue;
				if (!(cg_entitiesArray[i].nextState.ClientNumber != 0))
					continue;
				if (!(cg_entitiesArray[i].nextState.Alive))
					continue;
				if (!cg_entitiesArray[i].nextState.State & (1 << 6) != 0)
					continue;

				vec2_t Pos = vec2_t();
				vec2_t head = vec2_t();
				vec3_t origin = cg_entitiesArray[i].pose.Origin;

				vec3_t headPos = AimTarget_GetTagPos(&cg_entitiesArray[i], "j_helmet");
				headPos.z += 10;
				origin.z -= 5;

				if (!WorldToScreen(0, origin, &Pos))
					continue;
				if (!WorldToScreen(0, headPos, &head))
					continue;

				float playerHeight = fabsf(head.y - Pos.y);
				float playerWidth = (fabsf(head.y - Pos.y) * 0.65f);

				if (options.EspBoxToggle.state)
					BoundingBox(Pos.x - (playerWidth / 2.f) - 6.f, head.y - 4.f, playerWidth, playerHeight, white, 1.f);
				if (options.EspDrawBones.state)
					drawBones(&cg_entitiesArray[i], Red);
				if (options.EspDrawLine.state)
					DrawLine(vec2_t(cgDC->screenWidth / 2, cgDC->screenHeight - 5), Pos, white, 1);
				if (options.EspFrogChan.state)
					drawHeart(Pos.x - (playerWidth / 2.f) - 6.f, head.y - 4.f, playerWidth, playerHeight, Red, Red);
			}
			for (int j = 0; j < 2048; j++) {
				if (!(cg_entitiesArray[j].pose.eType == ET_MISSILE))
					continue;

				if (!(cg_entitiesArray[j].nextState.Alive))
					continue;

				vec3_t origin = cg_entitiesArray[j].pose.Origin;
				vec2_t Pos = vec2_t();

				if (!WorldToScreen(0, origin, &Pos))
					continue;

				if (options.DrawItem.state)
					DrawLine(vec2_t(cgDC->screenWidth / 2, cgDC->screenHeight - 5), Pos, Red, 1);

			}
			options.menuHeight = options.menuTabHeight + (options.menuMaxScroll * (R_TextHeight(R_RegisterFont(FontForIndex(options.menuFontIndex.current), 0)) * options.menuFontSize.current)) + (options.menuBorder.current * 2) + 2;
			if (options.menuOpen)
				DrawMenu();

			*(uint32_t*)0x82259BC8 = options.NoRecoil.state ? 0x60000000 : 0x48461341;
			*(uint32_t*)0x82255E1C = options.Laser.state ? 0x2B000B01 : 0x2B0B0000;

			*(uint32_t*)0x821fc04c = options.Wallhack.state ? 0x38C0FFFF : 0x7FA6EB78;
			*(uint32_t*)0x83c56038 = options.Wallhack.state ? 0xF51F0000DF : 0x3F800000;


			ScoreBoard_Draw(1, 300, 200);
			SpoofLevel();
		}
		ServerInfo();
	}
	void Cl_WritePacket(int a) {
		MinHook[3].Stub(a);

		if (Dvar_GetBool("cl_ingame"))
			doAimbot();

	}
	int speed = 0;
	int ticks = 0;
	bool run = false;
	short PreviousButton;

	DWORD XamInputGetState(int userIndex, int flags, PXINPUT_STATE pState)
	{
		run = false;

		DWORD result = XInputGetStateEx(userIndex, flags, pState);

		if (pState->Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
			run = true;

		pState->Gamepad.wButtons = 0;

		if (run)
			pState->Gamepad.wButtons = XINPUT_GAMEPAD_LEFT_THUMB;
		if (userIndex != 0)
			return XInputGetStateEx(userIndex, flags, pState);

		XInputGetState(0, &Buttons);

		if (PreviousButton != Buttons.Gamepad.wButtons)
		{
			PreviousButton = Buttons.Gamepad.wButtons;

			if (Buttons.Gamepad.bLeftTrigger > 100 && KeyIsDown(Buttons, XINPUT_GAMEPAD_RIGHT_THUMB))
			{
				options.menuOpen = !options.menuOpen;
			}

			if (!options.menuOpen)
				return XInputGetStateEx(userIndex, flags, pState);


			if (KeyIsDown(Buttons, XINPUT_GAMEPAD_DPAD_DOWN))
			{
				if (options.menuScroll < options.menuMaxScroll)
					options.menuScroll++;
				if (options.menuScroll == options.menuMaxScroll)
					options.menuScroll = 0;
			}

			if (KeyIsDown(Buttons, XINPUT_GAMEPAD_DPAD_UP))
			{
				if (options.menuScroll > -1)
					options.menuScroll--;
				if (options.menuScroll == -1)
					options.menuScroll = options.menuMaxScroll - 1;
			}
			if (KeyIsDown(Buttons, XINPUT_GAMEPAD_RIGHT_SHOULDER))
			{
				if (options.menuPageIndex < 5)
					options.menuPageIndex++;
				if (options.menuPageIndex > 4)
					options.menuPageIndex = 0;
			}

			if (KeyIsDown(Buttons, XINPUT_GAMEPAD_LEFT_SHOULDER))
			{

				if (options.menuPageIndex > -1)
					options.menuPageIndex--;
				if (options.menuPageIndex == -1)
					options.menuPageIndex = 4;
			}


			if (KeyIsDown(Buttons, XINPUT_GAMEPAD_X))
			{
				ToggleBool_List(options.menuScroll);
				ResetInt_List(options.menuScroll);
				ResetFloat_List(options.menuScroll);
				SwitchToSubMenu_List(options.menuScroll, options.menuPageIndex, options.isInSubMenu, options.menuScroll, options.previousPageIndex, options.previousScroll);
			}

			if (KeyIsDown(Buttons, XINPUT_GAMEPAD_B))
			{
				options.menuOpen = !options.menuOpen;
			}

			if (KeyIsDown(Buttons, XINPUT_GAMEPAD_DPAD_RIGHT))
			{
				AddInt_List(options.menuScroll);
				AddFloat_List(options.menuScroll);
			}

			if (KeyIsDown(Buttons, XINPUT_GAMEPAD_DPAD_LEFT))
			{
				SubInt_List(options.menuScroll);
				SubFloat_List(options.menuScroll);
			}

			if (!KeyIsDown(Buttons, XINPUT_GAMEPAD_DPAD_RIGHT))
			{
				speed = 0;
				ticks = 0;
			}

			if (!KeyIsDown(Buttons, XINPUT_GAMEPAD_DPAD_LEFT))
			{
				speed = 0;
				ticks = 0;
			}

			if (!options.menuOpen)
				return XInputGetStateEx(userIndex, flags, pState);
		}

		if (!options.menuOpen)
			return XInputGetStateEx(userIndex, flags, pState);

		if (KeyIsDown(Buttons, XINPUT_GAMEPAD_DPAD_RIGHT))
		{
			speed++;

			if (ticks < 50)
			{
				ticks += speed;
				return result;
			}

			AddInt_List(options.menuScroll);
			AddFloat_List(options.menuScroll);

			ticks = 0;
		}

		if (KeyIsDown(Buttons, XINPUT_GAMEPAD_DPAD_LEFT))
		{
			speed++;

			if (ticks < 50)
			{
				ticks += speed;
				return result;
			}

			SubInt_List(options.menuScroll);
			SubFloat_List(options.menuScroll);

			ticks = 0;
		}

		if (!options.menuOpen)
			return XInputGetStateEx(userIndex, flags, pState);

		return result;
	}
}
