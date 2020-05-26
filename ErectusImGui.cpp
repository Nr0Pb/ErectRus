#include "ErectusInclude.h"

bool ImGuiContextCreated = false;
bool ImGuiD3D9Initialized = false;
bool ImGuiWin32Initialized = false;
bool ImGuiInitialized = false;

POINT PointerPosition = { 0, 0 };
ImVec2 PointerOrigin = { 0.0f, 0.0f };
bool PointerDrag = false;

DWORD64 GetPtrResult = 0;
DWORD64 GetAddressResult = 0;

bool SwapperSourceToggle = false;
bool SwapperDestinationToggle = false;

bool TransferSourceToggle = false;
bool TransferDestinationToggle = false;

int AlphaCode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int BravoCode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int CharlieCode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

char **FavoritedWeaponsArray = nullptr;

bool DragMenu()
{
	if (!GetCursorPos(&PointerPosition)) return false;

	WindowPosition[0] = PointerPosition.x - int(PointerOrigin.x);
	WindowPosition[1] = PointerPosition.y - int(PointerOrigin.y);

	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

	if (WindowPosition[0] < 0)
	{
		WindowPosition[0] = 0;
	}

	if (WindowPosition[1] < 0)
	{
		WindowPosition[1] = 0;
	}

	if (WindowPosition[0] > (ScreenX - WindowSize[0]))
	{
		WindowPosition[0] = (ScreenX - WindowSize[0]);
	}

	if (WindowPosition[1] > (ScreenY - WindowSize[1]))
	{
		WindowPosition[1] = (ScreenY - WindowSize[1]);
	}

	return MoveWindow(WindowHwnd, WindowPosition[0], WindowPosition[1], WindowSize[0], WindowSize[1], FALSE);
}

void ProcessMenu()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(float(WindowSize[0]), float(WindowSize[1])));
	ImGui::SetNextWindowCollapsed(false);

	bool AllowDrag = true;
	if (ImGui::Begin("ErectRus - Меню Процесса", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Выход"))
			{
				Close();
			}

			if (ProcessSelected)
			{
				if (ImGui::MenuItem("Настройки"))
				{
					SetOverlayMenu();
				}

				if (ImGui::MenuItem("Включить ESP"))
				{
					SetOverlayPosition(false, true);
				}
			}
			else
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				ImGui::MenuItem("Настройки");
				ImGui::MenuItem("Включить ESP");
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}

			ImGui::EndMenuBar();
		}

		ImGui::SetNextItemWidth(float(WindowSize[0]) - 16.0f);
		if (ImGui::BeginCombo("###ProcessList", ProcessList[ProcessIndex]))
		{
			if (!ProcessListUpdated)
			{
				ProcessListUpdated = true;
				if (!UpdateProcessList())
				{
					ResetProcessData(true, 1);
				}
			}

			for (int i = 0; i < ProcessListSize; i++)
			{
				if (ImGui::Selectable(ProcessList[i]))
				{
					ProcessIndex = i;
					if (ProcessIndex)
					{
						ProcessSelected = ProcessValid(ProcessIdList[ProcessIndex]);
						if (!ProcessSelected)
						{
							ResetProcessData(false, 1);
						}
					}
				}
			}

			ImGui::EndCombo();
			AllowDrag = false;
		}
		else
		{
			if (ProcessListUpdated)
			{
				ProcessListUpdated = false;
			}
		}

		ImGui::Separator();
		switch (ProcessErrorId)
		{
		case 0:
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ProcessError);
			break;
		case 1:
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), ProcessError);
			break;
		case 2:
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ProcessError);
			break;
		default:
			ImGui::Text(ProcessError);
			break;
		}

		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::Columns(2);
		ImGui::Separator();
		ImGui::Text("Горячие клавиши ESP");
		ImGui::NextColumn();
		ImGui::Text("CTRL+ENTER");
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("ImGui (D3D9) FPS");
		ImGui::NextColumn();
		ImGui::Text("%.1f", ImGui::GetIO().Framerate);
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("PID (Process Id)");
		ImGui::NextColumn();
		ImGui::Text("%lu", Pid);
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("HWND (Window)");
		ImGui::NextColumn();
		ImGui::Text("%p", Hwnd);
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("Base Address");
		ImGui::NextColumn();
		ImGui::Text("%016llX", Exe);
		ImGui::NextColumn();
		ImGui::Separator();
		ImGui::Text("HANDLE");
		ImGui::NextColumn();
		ImGui::Text("%p", Handle);
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopItemFlag();

		if (ImGui::IsMouseDragging(0) && AllowDrag)
		{
			if (!PointerDrag)
			{
				PointerOrigin = ImGui::GetMousePos();
				PointerDrag = true;
			}
		}
		else
		{
			if (PointerDrag)
			{
				PointerOrigin = { 0.0f, 0.0f };
				PointerDrag = false;
			}
		}

		if (PointerDrag)
		{
			DragMenu();
		}
	}

	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void ButtonToggle(const char *Label, bool *State)
{
	if (*State)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(224.0f, 0.0f))) *State = false;
		ImGui::PopStyleColor(3);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(224.0f, 0.0f))) *State = true;
		ImGui::PopStyleColor(3);
	}
}

void LargeButtonToggle(const char *Label, bool *State)
{
	if (*State)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(451.0f, 0.0f))) *State = false;
		ImGui::PopStyleColor(3);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(451.0f, 0.0f))) *State = true;
		ImGui::PopStyleColor(3);
	}
}

void SmallButtonToggle(const char *Label, bool *State)
{
	if (*State)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(110.0f, 0.0f))) *State = false;
		ImGui::PopStyleColor(3);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
		if (ImGui::Button(Label, ImVec2(110.0f, 0.0f))) *State = true;
		ImGui::PopStyleColor(3);
	}
}

void OverlayMenu()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(float(WindowSize[0]), float(WindowSize[1])));
	ImGui::SetNextWindowCollapsed(false);

	bool AllowDrag = true;
	if (ImGui::Begin("ErectRus - Настройки", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Выход"))
			{
				Close();
			}

			if (ImGui::MenuItem("Настройки процесса"))
			{
				SetProcessMenu();
			}

			if (ImGui::MenuItem("Включить ESP"))
			{
				if (!SetOverlayPosition(false, true))
				{
					SetProcessMenu();
				}
			}

			ImGui::EndMenuBar();
		}

		if (ImGui::BeginTabBar("###OverlayMenuTabBar", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("ESP###ESPTab"))
			{
				if (ImGui::CollapsingHeader("Настройка ESP игроков"))
				{
					ButtonToggle("Включить ESP игроков", &PlayerSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###PlayerSettingsEnabledDistance", &PlayerSettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&PlayerSettings.EnabledDistance, 0, 3000);

					ButtonToggle("Показывать живых", &PlayerSettings.DrawAlive);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###PlayerSettingsAliveColor", PlayerSettings.AliveColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.AliveColor);

					ButtonToggle("Показывать упавших", &PlayerSettings.DrawDowned);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###PlayerSettingsDownedColor", PlayerSettings.DownedColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.DownedColor);

					ButtonToggle("Показывать мертвых", &PlayerSettings.DrawDead);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###PlayerSettingsDeadColor", PlayerSettings.DeadColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.DeadColor);

					ButtonToggle("Показывать неизвестных", &PlayerSettings.DrawUnknown);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###PlayerSettingsUnknownColor", PlayerSettings.UnknownColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.UnknownColor);

					ButtonToggle("Показывать активных", &PlayerSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###PlayerSettingsEnabledAlpha", &PlayerSettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&PlayerSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивных", &PlayerSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###PlayerSettingsDisabledAlpha", &PlayerSettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&PlayerSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать названных", &PlayerSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянных", &PlayerSettings.DrawUnnamed);

					ButtonToggle("Показыть имя", &PlayerSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &PlayerSettings.ShowDistance);

					ButtonToggle("Показать жизнь", &PlayerSettings.ShowHealth);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать жизнь мертвых", &PlayerSettings.ShowDeadHealth);

					ButtonToggle("Тень под текстом", &PlayerSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &PlayerSettings.TextCentered);
				}

				if (ImGui::CollapsingHeader("Настройки ESP NPC"))
				{
					ButtonToggle("Включить ESP", &NpcSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###NpcSettingsEnabledDistance", &NpcSettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&NpcSettings.EnabledDistance, 0, 3000);

					ButtonToggle("Показывать живых", &NpcSettings.DrawAlive);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###NpcSettingsAliveColor", NpcSettings.AliveColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(NpcSettings.AliveColor);

					ButtonToggle("Показывать упавших", &NpcSettings.DrawDowned);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###NpcSettingsDownedColor", NpcSettings.DownedColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(NpcSettings.DownedColor);

					ButtonToggle("Показывать мертвых", &NpcSettings.DrawDead);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###NpcSettingsDeadColor", NpcSettings.DeadColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(NpcSettings.DeadColor);

					ButtonToggle("Показывать неизвестных", &NpcSettings.DrawUnknown);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###NpcSettingsUnknownColor", NpcSettings.UnknownColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(NpcSettings.UnknownColor);

					ButtonToggle("Показывать активных", &NpcSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###NpcSettingsEnabledAlpha", &NpcSettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&NpcSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивных", &NpcSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###NpcSettingsDisabledAlpha", &NpcSettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&NpcSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать названных", &NpcSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянных", &NpcSettings.DrawUnnamed);

					ButtonToggle("Показать имя", &NpcSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &NpcSettings.ShowDistance);

					ButtonToggle("Показать жизнь", &NpcSettings.ShowHealth);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать жизнь метвых", &NpcSettings.ShowDeadHealth);

					ButtonToggle("Тень под текстом", &NpcSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &NpcSettings.TextCentered);

					ButtonToggle("Всегда показывать живых 1* NPCs", &CustomLegendarySettings.OverrideLivingOneStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###LivingOneStarColor", CustomLegendarySettings.LivingOneStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.LivingOneStarColor);

					ButtonToggle("Всегда показывать мертвых 1* NPCs", &CustomLegendarySettings.OverrideDeadOneStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###DeadOneStarColor", CustomLegendarySettings.DeadOneStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.DeadOneStarColor);

					ButtonToggle("Всегда показывать живых 2* NPCs", &CustomLegendarySettings.OverrideLivingTwoStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###LivingTwoStarColor", CustomLegendarySettings.LivingTwoStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.LivingTwoStarColor);

					ButtonToggle("Всегда показывать мертвых 2* NPCs", &CustomLegendarySettings.OverrideDeadTwoStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###DeadTwoStarColor", CustomLegendarySettings.DeadTwoStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.DeadTwoStarColor);

					ButtonToggle("Всегда показывать живых 3* NPCs", &CustomLegendarySettings.OverrideLivingThreeStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###LivingThreeStarColor", CustomLegendarySettings.LivingThreeStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.LivingThreeStarColor);

					ButtonToggle("Всегда показывать мертвых 3* NPCs", &CustomLegendarySettings.OverrideDeadThreeStar);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###DeadThreeStarColor", CustomLegendarySettings.DeadThreeStarColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(CustomLegendarySettings.DeadThreeStarColor);

					LargeButtonToggle("Скрыть NPCs Поселенцев", &CustomExtraNPCSettings.HideSettlerFaction);
					LargeButtonToggle("Скрыть NPCs Рейдеров", &CustomExtraNPCSettings.HideCraterRaiderFaction);
					LargeButtonToggle("Скрыть NPCs Консерваторов", &CustomExtraNPCSettings.HideDieHardFaction);
					LargeButtonToggle("Скрыть NPCs Секретной службы", &CustomExtraNPCSettings.HideSecretServiceFaction);

					LargeButtonToggle("NPC Blacklist Enabled", &CustomExtraNPCSettings.UseNPCBlacklist);
					if (ImGui::CollapsingHeader("NPC Blacklist"))
					{
						for (int i = 0; i < 64; i++)
						{
							char NPCBlacklistEnabledText[sizeof("Черный список NPC: 63")];
							char NPCBlacklistLabelText[sizeof("###NPCBlacklist63")];
							sprintf_s(NPCBlacklistEnabledText, "Черный список NPC: %d", i);
							sprintf_s(NPCBlacklistLabelText, "###NPCBlacklist%d", i);
							ButtonToggle(NPCBlacklistEnabledText, &CustomExtraNPCSettings.NPCBlacklistEnabled[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", CustomExtraNPCSettings.NPCBlacklist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(NPCBlacklistLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &CustomExtraNPCSettings.NPCBlacklist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Настройки ESP хранилища"))
				{
					ButtonToggle("Включить ESP храницища", &ContainerSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ContainerSettingsEnabledDistance", &ContainerSettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&ContainerSettings.EnabledDistance, 0, 3000);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::ColorEdit3("###ContainerSettingsColor", ContainerSettings.Color);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(ContainerSettings.Color);

					ButtonToggle("Показывать активные", &ContainerSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###ContainerSettingsEnabledAlpha", &ContainerSettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&ContainerSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивные", &ContainerSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###ContainerSettingsDisabledAlpha", &ContainerSettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&ContainerSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать названные", &ContainerSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянные", &ContainerSettings.DrawUnnamed);

					ButtonToggle("Показать имя", &ContainerSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &ContainerSettings.ShowDistance);

					ButtonToggle("Текст под текстом", &ContainerSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &ContainerSettings.TextCentered);

					if (ImGui::CollapsingHeader("Белый список хранилищ"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Слот белого списка: 31")];
							char WhitelistText[sizeof("###ContainerWhitelist31")];
							sprintf_s(WhitelistedText, "Слот белого списка: %d", i);
							sprintf_s(WhitelistText, "###ContainerWhitelist%d", i);
							ButtonToggle(WhitelistedText, &ContainerSettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", ContainerSettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &ContainerSettings.Whitelist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Настройки ESP хлама"))
				{
					ButtonToggle("Включить ESP хлама", &JunkSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###JunkSettingsEnabledDistance", &JunkSettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&JunkSettings.EnabledDistance, 0, 3000);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::ColorEdit3("###JunkSettingsColor", JunkSettings.Color);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(JunkSettings.Color);

					ButtonToggle("Показывать активные", &JunkSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###JunkSettingsEnabledAlpha", &JunkSettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&JunkSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивных", &JunkSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###JunkSettingsDisabledAlpha", &JunkSettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&JunkSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Паказывать названные", &JunkSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянные", &JunkSettings.DrawUnnamed);

					ButtonToggle("Показать имя", &JunkSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &JunkSettings.ShowDistance);

					ButtonToggle("Тень под текстом", &JunkSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &JunkSettings.TextCentered);

					if (ImGui::CollapsingHeader("Белый список хлама"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Слот белого списка: 31")];
							char WhitelistText[sizeof("###JunkWhitelist31")];
							sprintf_s(WhitelistedText, "Слот белого списка: %d", i);
							sprintf_s(WhitelistText, "###JunkWhitelist%d", i);
							ButtonToggle(WhitelistedText, &JunkSettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", JunkSettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &JunkSettings.Whitelist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Настройки ESP схем/записок"))
				{
					ButtonToggle("Включить ESP схем", &PlanSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###PlanSettingsEnabledDistance", &PlanSettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&PlanSettings.EnabledDistance, 0, 3000);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::ColorEdit3("###PlanSettingsColor", PlanSettings.Color);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlanSettings.Color);

					ButtonToggle("Показывать активные", &PlanSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###PlanSettingsEnabledAlpha", &PlanSettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&PlanSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивные", &PlanSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###PlanSettingsDisabledAlpha", &PlanSettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&PlanSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать изученные", &CustomKnownRecipeSettings.KnownRecipesEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать неизвестные", &CustomKnownRecipeSettings.UnknownRecipesEnabled);

					ButtonToggle("Показывать названные", &PlanSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянные", &PlanSettings.DrawUnnamed);

					ButtonToggle("Показать имя", &PlanSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &PlanSettings.ShowDistance);

					ButtonToggle("Тень под текстом", &PlanSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &PlanSettings.TextCentered);
					if (ImGui::CollapsingHeader("Белый список схем"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Слот белого списка: 31")];
							char WhitelistText[sizeof("###PlanWhitelist31")];
							sprintf_s(WhitelistedText, "Слот белого списка: %d", i);
							sprintf_s(WhitelistText, "###PlanWhitelist%d", i);
							ButtonToggle(WhitelistedText, &PlanSettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", PlanSettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &PlanSettings.Whitelist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Настройки ESP журналов"))
				{
					ButtonToggle("Включить ESP журналы", &MagazineSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###MagazineSettingsEnabledDistance", &MagazineSettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&MagazineSettings.EnabledDistance, 0, 3000);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::ColorEdit3("###MagazineSettingsColor", MagazineSettings.Color);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(MagazineSettings.Color);

					ButtonToggle("Показывать активных", &MagazineSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###MagazineSettingsEnabledAlpha", &MagazineSettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&MagazineSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивных", &MagazineSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###MagazineSettingsDisabledAlpha", &MagazineSettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&MagazineSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать названные", &MagazineSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянные", &MagazineSettings.DrawUnnamed);

					ButtonToggle("Показать имя", &MagazineSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &MagazineSettings.ShowDistance);

					ButtonToggle("Тень под текстом", &MagazineSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &MagazineSettings.TextCentered);

					if (ImGui::CollapsingHeader("Белый список журналов"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Слот белого списка: 31")];
							char WhitelistText[sizeof("###MagazineWhitelist31")];
							sprintf_s(WhitelistedText, "Слот белого списка: %d", i);
							sprintf_s(WhitelistText, "###MagazineWhitelist%d", i);
							ButtonToggle(WhitelistedText, &MagazineSettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", MagazineSettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &MagazineSettings.Whitelist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Настройки ESP пупсов"))
				{
					ButtonToggle("Включить ESP пупсов", &BobbleheadSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###BobbleheadSettingsEnabledDistance", &BobbleheadSettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&BobbleheadSettings.EnabledDistance, 0, 3000);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::ColorEdit3("###BobbleheadSettingsColor", BobbleheadSettings.Color);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(BobbleheadSettings.Color);

					ButtonToggle("Показывать активных", &BobbleheadSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###BobbleheadSettingsEnabledAlpha", &BobbleheadSettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&BobbleheadSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивных", &BobbleheadSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###BobbleheadSettingsDisabledAlpha", &BobbleheadSettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&BobbleheadSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать названных", &BobbleheadSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянных", &BobbleheadSettings.DrawUnnamed);

					ButtonToggle("Показать имя", &BobbleheadSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &BobbleheadSettings.ShowDistance);

					ButtonToggle("Тень под текстом", &BobbleheadSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &BobbleheadSettings.TextCentered);

					if (ImGui::CollapsingHeader("Белый список пупсов"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Слот белого списка: 31")];
							char WhitelistText[sizeof("###BobbleheadWhitelist31")];
							sprintf_s(WhitelistedText, "Слот белого списка: %d", i);
							sprintf_s(WhitelistText, "###BobbleheadWhitelist%d", i);
							ButtonToggle(WhitelistedText, &BobbleheadSettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", BobbleheadSettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &BobbleheadSettings.Whitelist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Настройки ESP предметов"))
				{
					ButtonToggle("Включить ESP предметов", &ItemSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemSettingsEnabledDistance", &ItemSettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&ItemSettings.EnabledDistance, 0, 3000);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::ColorEdit3("###ItemSettingsColor", ItemSettings.Color);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(ItemSettings.Color);

					ButtonToggle("Показывать активных", &ItemSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###ItemSettingsEnabledAlpha", &ItemSettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&ItemSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивных", &ItemSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###ItemSettingsDisabledAlpha", &ItemSettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&ItemSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать названные", &ItemSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянные", &ItemSettings.DrawUnnamed);

					ButtonToggle("Показать имя", &ItemSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &ItemSettings.ShowDistance);

					ButtonToggle("Тень под текстом", &ItemSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &ItemSettings.TextCentered);

					if (ImGui::CollapsingHeader("Белый список предметов"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Слот белого списка: 31")];
							char WhitelistText[sizeof("###ItemWhitelist31")];
							sprintf_s(WhitelistedText, "Слот белого списка: %d", i);
							sprintf_s(WhitelistText, "###ItemWhitelist%d", i);
							ButtonToggle(WhitelistedText, &ItemSettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", ItemSettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &ItemSettings.Whitelist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Настройки ESP флоры"))
				{
					ButtonToggle("Включить ESP флоры", &FloraSettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###FloraSettingsEnabledDistance", &FloraSettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&FloraSettings.EnabledDistance, 0, 3000);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::ColorEdit3("###FloraSettingsColor", FloraSettings.Color);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(FloraSettings.Color);

					ButtonToggle("Показывать активную", &FloraSettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###FloraSettingsEnabledAlpha", &FloraSettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&FloraSettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивную", &FloraSettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###FloraSettingsDisabledAlpha", &FloraSettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&FloraSettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать названную", &FloraSettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянную", &FloraSettings.DrawUnnamed);

					LargeButtonToggle("Показывать флору Алого флюса", &CustomFluxSettings.CrimsonFluxEnabled);
					LargeButtonToggle("Показывать флору Кобальтого флюса", &CustomFluxSettings.CobaltFluxEnabled);
					LargeButtonToggle("Показывать флору Уранового флюса", &CustomFluxSettings.YellowcakeFluxEnabled);
					LargeButtonToggle("Показывать флору Флуоресцентного флюса", &CustomFluxSettings.FluorescentFluxEnabled);
					LargeButtonToggle("Показывать флору Фиолетового флюса", &CustomFluxSettings.VioletFluxEnabled);

					ButtonToggle("Показать имя", &FloraSettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &FloraSettings.ShowDistance);

					ButtonToggle("Тень под текстом", &FloraSettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &FloraSettings.TextCentered);

					if (ImGui::CollapsingHeader("Белый список флоры"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Слот белого списка: 31")];
							char WhitelistText[sizeof("###FloraWhitelist31")];
							sprintf_s(WhitelistedText, "Слот белого списка: %d", i);
							sprintf_s(WhitelistText, "###FloraWhitelist%d", i);
							ButtonToggle(WhitelistedText, &FloraSettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", FloraSettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &FloraSettings.Whitelist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Настройки ESP объектов"))
				{
					ButtonToggle("Включить ESP объектов", &EntitySettings.Enabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###EntitySettingsEnabledDistance", &EntitySettings.EnabledDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&EntitySettings.EnabledDistance, 0, 3000);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::ColorEdit3("###EntitySettingsColor", EntitySettings.Color);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(EntitySettings.Color);

					ButtonToggle("Показывать активные", &EntitySettings.DrawEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###EntitySettingsEnabledAlpha", &EntitySettings.EnabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&EntitySettings.EnabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать неактивные", &EntitySettings.DrawDisabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###EntitySettingsDisabledAlpha", &EntitySettings.DisabledAlpha, 0.0f, 1.0f, "Прозрачность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&EntitySettings.DisabledAlpha, 0.0f, 1.0f);

					ButtonToggle("Показывать названные", &EntitySettings.DrawNamed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать безымянные", &EntitySettings.DrawUnnamed);

					ButtonToggle("Показать имя", &EntitySettings.ShowName);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать дистанцию", &EntitySettings.ShowDistance);

					ButtonToggle("Тень под текстом", &EntitySettings.TextShadowed);
					ImGui::SameLine(235.0f);
					ButtonToggle("Текст по центру", &EntitySettings.TextCentered);

					if (ImGui::CollapsingHeader("Белый список объектов"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Слот белого списка: 31")];
							char WhitelistText[sizeof("###EntityWhitelist31")];
							sprintf_s(WhitelistedText, "Слот белого списка: %d", i);
							sprintf_s(WhitelistText, "###EntityWhitelist%d", i);
							ButtonToggle(WhitelistedText, &EntitySettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", EntitySettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &EntitySettings.Whitelist[i]);
						}
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Добыча###LootTab"))
			{
				if (ImGui::CollapsingHeader("Сбор добычи"))
				{
					if (CheckScrapList())
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
						if (ImGui::Button("Собрать добычу (CTRL+E)###LootSelectedScrapEnabled", ImVec2(224.0f, 0.0f)))
						{
							LootScrap();
						}
						ImGui::PopStyleColor(3);
					}
					else
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
						ImGui::Button("Собрать добычу (CTRL+E)###LootSelectedScrapDisabled", ImVec2(224.0f, 0.0f));
						ImGui::PopStyleColor(3);
						ImGui::PopItemFlag();
					}

					ImGui::SameLine(235.0f);
					ButtonToggle("Собирать по грячей клавише", &CustomScrapLooterSettings.ScrapKeybindEnabled);

					LargeButtonToggle("Собирать с ESP хлама (Используются настройки ESP хлама)", &CustomScrapLooterSettings.ScrapOverrideEnabled);

					ButtonToggle("Включить автоматический сбор###ScrapAutomaticLootingEnabled", &CustomScrapLooterSettings.ScrapAutomaticLootingEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показывать статус автоматического сбора###ScrapAutomaticStatus", &CustomScrapLooterSettings.ScrapAutomaticStatus);

					ImGui::SetNextItemWidth(224.0f);
					char ScrapAutomaticSpeedMinText[sizeof("Скорость (Мин): 60 (960 мс)")];
					sprintf_s(ScrapAutomaticSpeedMinText, "Скорость (Мин): %d (%d мс)", CustomScrapLooterSettings.ScrapAutomaticSpeedMin, CustomScrapLooterSettings.ScrapAutomaticSpeedMin * 16);
					ImGui::SliderInt("###ScrapAutomaticSpeedMin", &CustomScrapLooterSettings.ScrapAutomaticSpeedMin, 10, 60, ScrapAutomaticSpeedMinText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomScrapLooterSettings.ScrapAutomaticSpeedMax < CustomScrapLooterSettings.ScrapAutomaticSpeedMin)
						{
							CustomScrapLooterSettings.ScrapAutomaticSpeedMax = CustomScrapLooterSettings.ScrapAutomaticSpeedMin;
						}
					}
					ValidateInt(&CustomScrapLooterSettings.ScrapAutomaticSpeedMin, 10, 60);
					ValidateInt(&CustomScrapLooterSettings.ScrapAutomaticSpeedMax, 10, 60);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					char ScrapAutomaticSpeedMaxText[sizeof("Скорость (Макс): 60 (960 мс)")];
					sprintf_s(ScrapAutomaticSpeedMaxText, "Скорость (Макс): %d (%d мс)", CustomScrapLooterSettings.ScrapAutomaticSpeedMax, CustomScrapLooterSettings.ScrapAutomaticSpeedMax * 16);
					ImGui::SliderInt("###ScrapAutomaticSpeedMax", &CustomScrapLooterSettings.ScrapAutomaticSpeedMax, 10, 60, ScrapAutomaticSpeedMaxText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomScrapLooterSettings.ScrapAutomaticSpeedMax < CustomScrapLooterSettings.ScrapAutomaticSpeedMin)
						{
							CustomScrapLooterSettings.ScrapAutomaticSpeedMin = CustomScrapLooterSettings.ScrapAutomaticSpeedMax;
						}
					}
					ValidateInt(&CustomScrapLooterSettings.ScrapAutomaticSpeedMin, 10, 60);
					ValidateInt(&CustomScrapLooterSettings.ScrapAutomaticSpeedMax, 10, 60);
					if (CustomScrapLooterSettings.ScrapAutomaticSpeedMax < CustomScrapLooterSettings.ScrapAutomaticSpeedMin)
					{
						CustomScrapLooterSettings.ScrapAutomaticSpeedMax = CustomScrapLooterSettings.ScrapAutomaticSpeedMin;
					}

					ImGui::SetNextItemWidth(451.0f);
					ImGui::SliderInt("###ScrapLooterDistance", &CustomScrapLooterSettings.ScrapLooterDistance, 0, 3000, "Дистанция сбора хлама: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomScrapLooterSettings.ScrapLooterDistance, 0, 3000);

					for (int i = 0; i < 40; i++)
					{
						ButtonToggle(CustomScrapLooterSettings.ScrapNameList[i], &CustomScrapLooterSettings.ScrapEnabledList[i]);
						ImGui::SameLine(235.0f);
						char LabelText[sizeof("###ScrapReadOnly39")];
						sprintf_s(LabelText, "###ScrapReadOnly%d", i);
						char FormidText[sizeof("00000000")];
						sprintf_s(FormidText, "%08lX", CustomScrapLooterSettings.ScrapFormidList[i]);
						ImGui::SetNextItemWidth(224.0f);
						ImGui::InputText(LabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_ReadOnly);
						if (ImGui::IsItemActive()) AllowDrag = false;
					}
				}

				if (ImGui::CollapsingHeader("Сборщик предметов"))
				{
					if (CheckItemLooterSettings())
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
						if (ImGui::Button("Собрать предметы (CTRL+R)###LootSelectedItemsEnabled", ImVec2(224.0f, 0.0f)))
						{
							LootItems();
						}
						ImGui::PopStyleColor(3);
					}
					else
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
						ImGui::Button("Собрать предметы (CTRL+R)###LootSelectedItemsDisabled", ImVec2(224.0f, 0.0f));
						ImGui::PopStyleColor(3);
						ImGui::PopItemFlag();
					}

					ImGui::SameLine(235.0f);
					ButtonToggle("Собрать предметы по клавише", &CustomItemLooterSettings.ItemKeybindEnabled);

					ButtonToggle("Включить автоматический сбор###ItemAutomaticLootingEnabled", &CustomItemLooterSettings.ItemAutomaticLootingEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Показать статус автоматического сбора###ItemAutomaticStatus", &CustomItemLooterSettings.ItemAutomaticStatus);

					ImGui::SetNextItemWidth(224.0f);
					char ItemAutomaticSpeedMinText[sizeof("Скорость (Мин): 60 (960 мс)")];
					sprintf_s(ItemAutomaticSpeedMinText, "Скорость (Мин): %d (%d мс)", CustomItemLooterSettings.ItemAutomaticSpeedMin, CustomItemLooterSettings.ItemAutomaticSpeedMin * 16);
					ImGui::SliderInt("###ItemAutomaticSpeedMin", &CustomItemLooterSettings.ItemAutomaticSpeedMin, 10, 60, ItemAutomaticSpeedMinText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomItemLooterSettings.ItemAutomaticSpeedMax < CustomItemLooterSettings.ItemAutomaticSpeedMin)
						{
							CustomItemLooterSettings.ItemAutomaticSpeedMax = CustomItemLooterSettings.ItemAutomaticSpeedMin;
						}
					}
					ValidateInt(&CustomItemLooterSettings.ItemAutomaticSpeedMin, 10, 60);
					ValidateInt(&CustomItemLooterSettings.ItemAutomaticSpeedMax, 10, 60);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					char ItemAutomaticSpeedMaxText[sizeof("Скорость (Макс): 60 (960 мс)")];
					sprintf_s(ItemAutomaticSpeedMaxText, "Скорость (Макс): %d (%d мс)", CustomItemLooterSettings.ItemAutomaticSpeedMax, CustomItemLooterSettings.ItemAutomaticSpeedMax * 16);
					ImGui::SliderInt("###ItemAutomaticSpeedMax", &CustomItemLooterSettings.ItemAutomaticSpeedMax, 10, 60, ItemAutomaticSpeedMaxText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomItemLooterSettings.ItemAutomaticSpeedMax < CustomItemLooterSettings.ItemAutomaticSpeedMin)
						{
							CustomItemLooterSettings.ItemAutomaticSpeedMin = CustomItemLooterSettings.ItemAutomaticSpeedMax;
						}
					}
					ValidateInt(&CustomItemLooterSettings.ItemAutomaticSpeedMin, 10, 60);
					ValidateInt(&CustomItemLooterSettings.ItemAutomaticSpeedMax, 10, 60);
					if (CustomItemLooterSettings.ItemAutomaticSpeedMax < CustomItemLooterSettings.ItemAutomaticSpeedMin)
					{
						CustomItemLooterSettings.ItemAutomaticSpeedMax = CustomItemLooterSettings.ItemAutomaticSpeedMin;
					}
					ButtonToggle("Включить оружие###ItemLooterWeaponsEnabled", &CustomItemLooterSettings.ItemLooterWeaponsEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterWeaponsDistance", &CustomItemLooterSettings.ItemLooterWeaponsDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterWeaponsDistance, 0, 3000);

					ButtonToggle("Включить броню###ItemLooterArmorEnabled", &CustomItemLooterSettings.ItemLooterArmorEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterArmorDistance", &CustomItemLooterSettings.ItemLooterArmorDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterArmorDistance, 0, 3000);

					ButtonToggle("Включить боеприпасы###ItemLooterAmmoEnabled", &CustomItemLooterSettings.ItemLooterAmmoEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterAmmoDistance", &CustomItemLooterSettings.ItemLooterAmmoDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterAmmoDistance, 0, 3000);

					ButtonToggle("Включить моды###ItemLooterModsEnabled", &CustomItemLooterSettings.ItemLooterModsEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterModsDistance", &CustomItemLooterSettings.ItemLooterModsDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterModsDistance, 0, 3000);

					ButtonToggle("Включить торговцев###ItemLooterMagazinesEnabled", &CustomItemLooterSettings.ItemLooterMagazinesEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterMagazinesDistance", &CustomItemLooterSettings.ItemLooterMagazinesDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterMagazinesDistance, 0, 3000);

					ButtonToggle("Включить пупсы###ItemLooterBobbleheadsEnabled", &CustomItemLooterSettings.ItemLooterBobbleheadsEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterBobbleheadsDistance", &CustomItemLooterSettings.ItemLooterBobbleheadsDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterBobbleheadsDistance, 0, 3000);

					ButtonToggle("Включить помощь###ItemLooterAidEnabled", &CustomItemLooterSettings.ItemLooterAidEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterAidDistance", &CustomItemLooterSettings.ItemLooterAidDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterAidDistance, 0, 3000);

					ButtonToggle("Включить изученные схемы###ItemLooterKnownPlansEnabled", &CustomItemLooterSettings.ItemLooterKnownPlansEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterKnownPlansDistance", &CustomItemLooterSettings.ItemLooterKnownPlansDistance, 0, 3000, "Distance: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterKnownPlansDistance, 0, 3000);

					ButtonToggle("Включить неизвестные схемы###ItemLooterUnknownPlansEnabled", &CustomItemLooterSettings.ItemLooterUnknownPlansEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterUnknownPlansDistance", &CustomItemLooterSettings.ItemLooterUnknownPlansDistance, 0, 3000, "Distance: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterUnknownPlansDistance, 0, 3000);

					ButtonToggle("Включить разное###ItemLooterMiscEnabled", &CustomItemLooterSettings.ItemLooterMiscEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterMiscDistance", &CustomItemLooterSettings.ItemLooterMiscDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterMiscDistance, 0, 3000);

					ButtonToggle("Включить другое###ItemLooterUnlistedEnabled", &CustomItemLooterSettings.ItemLooterUnlistedEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterUnlistedDistance", &CustomItemLooterSettings.ItemLooterUnlistedDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterUnlistedDistance, 0, 3000);

					ButtonToggle("Включить список Formid предметов###ItemLooterListEnabled", &CustomItemLooterSettings.ItemLooterListEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###ItemLooterListDistance", &CustomItemLooterSettings.ItemLooterListDistance, 0, 3000, "Дистанция: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomItemLooterSettings.ItemLooterListDistance, 0, 3000);

					LargeButtonToggle("Включить черный список сбора предметов###ItemLooterBlacklistToggle", &CustomItemLooterSettings.ItemLooterBlacklistToggle);

					if (ImGui::CollapsingHeader("Список сбора Formid предметов"))
					{
						for (int i = 0; i < 100; i++)
						{
							char ItemLooterEnabledText[sizeof("Слот предмета: 99")];
							char ItemLooterLabelText[sizeof("###ItemLooterList99")];
							sprintf_s(ItemLooterEnabledText, "Слот предмета: %d", i);
							sprintf_s(ItemLooterLabelText, "###ItemLooterList%d", i);
							ButtonToggle(ItemLooterEnabledText, &CustomItemLooterSettings.ItemLooterEnabledList[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", CustomItemLooterSettings.ItemLooterFormidList[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(ItemLooterLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &CustomItemLooterSettings.ItemLooterFormidList[i]);
						}
					}

					if (ImGui::CollapsingHeader("Черный список сбора пребметов"))
					{
						for (int i = 0; i < 64; i++)
						{
							char ItemLooterBlacklistEnabledText[sizeof("Слот предмета : 63")];
							char ItemLooterBlacklistLabelText[sizeof("###ItemLooterBlacklist63")];
							sprintf_s(ItemLooterBlacklistEnabledText, "Слот предмета : %d", i);
							sprintf_s(ItemLooterBlacklistLabelText, "###ItemLooterBlacklist%d", i);
							ButtonToggle(ItemLooterBlacklistEnabledText, &CustomItemLooterSettings.ItemLooterBlacklistEnabled[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", CustomItemLooterSettings.ItemLooterBlacklist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(ItemLooterBlacklistLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &CustomItemLooterSettings.ItemLooterBlacklist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Сбор с NPC (Лимит дистанции 76м)"))
				{
					LargeButtonToggle("Автоматический сбор с NPC (CTRL+<)###NPCLooterEnabled", &NPCLooterSettings.EntityLooterEnabled);

					LargeButtonToggle("Показывать статус сбора с NPCs###NPCLooterStatusEnabled", &NPCLooterSettings.EntityLooterStatusEnabled);

					ButtonToggle("Включить все оружие###NPCLooterAllWeaponsEnabled", &NPCLooterSettings.EntityLooterAllWeaponsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить всю броню###NPCLooterAllArmorEnabled", &NPCLooterSettings.EntityLooterAllArmorEnabled);

					ButtonToggle("Оружие с 1*###NPCLooterOneStarWeaponsEnabled", &NPCLooterSettings.EntityLooterOneStarWeaponsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Броня с 1*###NPCLooterOneStarArmorEnabled", &NPCLooterSettings.EntityLooterOneStarArmorEnabled);

					ButtonToggle("Оружие с 2*###NPCLooterTwoStarWeaponsEnabled", &NPCLooterSettings.EntityLooterTwoStarWeaponsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Броня с 2*###NPCLooterTwoStarArmorEnabled", &NPCLooterSettings.EntityLooterTwoStarArmorEnabled);

					ButtonToggle("Оружие с 3*###NPCLooterThreeStarWeaponsEnabled", &NPCLooterSettings.EntityLooterThreeStarWeaponsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Броня с 3*###NPCLooterThreeStarArmorEnabled", &NPCLooterSettings.EntityLooterThreeStarArmorEnabled);

					ButtonToggle("Включить патроны###NPCLooterAmmoEnabled", &NPCLooterSettings.EntityLooterAmmoEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить модули###NPCLooterModsEnabled", &NPCLooterSettings.EntityLooterModsEnabled);

					ButtonToggle("Включить крышки###NPCLooterCapsEnabled", &NPCLooterSettings.EntityLooterCapsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить хлам###NPCLooterJunkEnabled", &NPCLooterSettings.EntityLooterJunkEnabled);

					ButtonToggle("Включить помощь###NPCLooterAidEnabled", &NPCLooterSettings.EntityLooterAidEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить сокровища###NPCLooterTreasureMapsEnabled", &NPCLooterSettings.EntityLooterTreasureMapsEnabled);

					ButtonToggle("Включить изученные схемы###NPCLooterKnownPlansEnabled", &NPCLooterSettings.EntityLooterKnownPlansEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить неизвесные схемы###NPCLooterUnknownPlansEnabled", &NPCLooterSettings.EntityLooterUnknownPlansEnabled);

					ButtonToggle("Включить разное###NPCLooterMiscEnabled", &NPCLooterSettings.EntityLooterMiscEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить другое###NPCLooterUnlistedEnabled", &NPCLooterSettings.EntityLooterUnlistedEnabled);

					LargeButtonToggle("Включить список Formid NPC###NPCLooterListEnabled", &NPCLooterSettings.EntityLooterListEnabled);

					LargeButtonToggle("Включить черный список с NPC###NPCLooterBlacklistToggle", &NPCLooterSettings.EntityLooterBlacklistToggle);

					if (ImGui::CollapsingHeader("Список сбора Formid NPC"))
					{
						for (int i = 0; i < 100; i++)
						{
							char NPCLooterEnabledText[sizeof("Слот сбора: 99")];
							char NPCLooterLabelText[sizeof("###NPCLooterList99")];
							sprintf_s(NPCLooterEnabledText, "Слот сбора: %d", i);
							sprintf_s(NPCLooterLabelText, "###NPCLooterList%d", i);
							ButtonToggle(NPCLooterEnabledText, &NPCLooterSettings.EntityLooterEnabledList[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", NPCLooterSettings.EntityLooterFormidList[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(NPCLooterLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &NPCLooterSettings.EntityLooterFormidList[i]);
						}
					}

					if (ImGui::CollapsingHeader("Черный список сбора с NPC"))
					{
						for (int i = 0; i < 64; i++)
						{
							char NPCLooterBlacklistEnabledText[sizeof("Слот сбора: 63")];
							char NPCLooterBlacklistLabelText[sizeof("###NPCLooterBlacklist63")];
							sprintf_s(NPCLooterBlacklistEnabledText, "Слот сбора: %d", i);
							sprintf_s(NPCLooterBlacklistLabelText, "###NPCLooterBlacklist%d", i);
							ButtonToggle(NPCLooterBlacklistEnabledText, &NPCLooterSettings.EntityLooterBlacklistEnabled[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", NPCLooterSettings.EntityLooterBlacklist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(NPCLooterBlacklistLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &NPCLooterSettings.EntityLooterBlacklist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Сборщик хранилищ (Лимит дистанции 6м)"))
				{
					LargeButtonToggle("Автоматический сбор с хранилищ (CTRL+>)###ContainerLooterEnabled", &ContainerLooterSettings.EntityLooterEnabled);

					LargeButtonToggle("Показывать статус сбора с хранилищ###ContainerLooterStatusEnabled", &ContainerLooterSettings.EntityLooterStatusEnabled);

					ButtonToggle("Включить все оружие###ContainerLooterAllWeaponsEnabled", &ContainerLooterSettings.EntityLooterAllWeaponsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить всю броню###ContainerLooterAllArmorEnabled", &ContainerLooterSettings.EntityLooterAllArmorEnabled);

					ButtonToggle("Оружие с 1*###ContainerLooterOneStarWeaponsEnabled", &ContainerLooterSettings.EntityLooterOneStarWeaponsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Броня с 1*###ContainerLooterOneStarArmorEnabled", &ContainerLooterSettings.EntityLooterOneStarArmorEnabled);

					ButtonToggle("Оружие с 2*###ContainerLooterTwoStarWeaponsEnabled", &ContainerLooterSettings.EntityLooterTwoStarWeaponsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Броня с 2*###ContainerLooterTwoStarArmorEnabled", &ContainerLooterSettings.EntityLooterTwoStarArmorEnabled);

					ButtonToggle("Оружие с 3*###ContainerLooterThreeStarWeaponsEnabled", &ContainerLooterSettings.EntityLooterThreeStarWeaponsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Броня с 3*###ContainerLooterThreeStarArmorEnabled", &ContainerLooterSettings.EntityLooterThreeStarArmorEnabled);

					ButtonToggle("Включить патроны###ContainerLooterAmmoEnabled", &ContainerLooterSettings.EntityLooterAmmoEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить модули###ContainerLooterModsEnabled", &ContainerLooterSettings.EntityLooterModsEnabled);

					ButtonToggle("Включить крышки###ContainerLooterCapsEnabled", &ContainerLooterSettings.EntityLooterCapsEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить хлам###ContainerLooterJunkEnabled", &ContainerLooterSettings.EntityLooterJunkEnabled);

					ButtonToggle("Включить помощь###ContainerLooterAidEnabled", &ContainerLooterSettings.EntityLooterAidEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить сокровища###ContainerLooterTreasureMapsEnabled", &ContainerLooterSettings.EntityLooterTreasureMapsEnabled);

					ButtonToggle("Включить изучанные схемы###ContainerLooterKnownPlansEnabled", &ContainerLooterSettings.EntityLooterKnownPlansEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить неизвестные схемы###ContainerLooterUnknownPlansEnabled", &ContainerLooterSettings.EntityLooterUnknownPlansEnabled);

					ButtonToggle("Включить разное###ContainerLooterMiscEnabled", &ContainerLooterSettings.EntityLooterMiscEnabled);
					ImGui::SameLine(235.0f);
					ButtonToggle("Включить другое###ContainerLooterUnlistedEnabled", &ContainerLooterSettings.EntityLooterUnlistedEnabled);

					LargeButtonToggle("Включить список сбора Formid хранилищ###ContainerLooterListEnabled", &ContainerLooterSettings.EntityLooterListEnabled);

					LargeButtonToggle("Включить черный список сбора хранилищ###ContainerLooterBlacklistToggle", &ContainerLooterSettings.EntityLooterBlacklistToggle);

					if (ImGui::CollapsingHeader("Список сбора Formid хранилищ"))
					{
						for (int i = 0; i < 100; i++)
						{
							char ContainerLooterEnabledText[sizeof("Слот сбора: 99")];
							char ContainerLooterLabelText[sizeof("###ContainerLooterList99")];
							sprintf_s(ContainerLooterEnabledText, "Слот сбора: %d", i);
							sprintf_s(ContainerLooterLabelText, "###ContainerLooterList%d", i);
							ButtonToggle(ContainerLooterEnabledText, &ContainerLooterSettings.EntityLooterEnabledList[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", ContainerLooterSettings.EntityLooterFormidList[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(ContainerLooterLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &ContainerLooterSettings.EntityLooterFormidList[i]);
						}
					}

					if (ImGui::CollapsingHeader("Черный список сбора хранилищ"))
					{
						for (int i = 0; i < 64; i++)
						{
							char ContainerLooterBlacklistEnabledText[sizeof("Слот сбора: 63")];
							char ContainerLooterBlacklistLabelText[sizeof("###ContainerLooterBlacklist63")];
							sprintf_s(ContainerLooterBlacklistEnabledText, "Слот сбора: %d", i);
							sprintf_s(ContainerLooterBlacklistLabelText, "###ContainerLooterBlacklist%d", i);
							ButtonToggle(ContainerLooterBlacklistEnabledText, &ContainerLooterSettings.EntityLooterBlacklistEnabled[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", ContainerLooterSettings.EntityLooterBlacklist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(ContainerLooterBlacklistLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &ContainerLooterSettings.EntityLooterBlacklist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Сбор флоры (Лимит дистанции 6м)"))
				{
					LargeButtonToggle("Автоматический сбор флоры (CTRL+P)###HarvesterEnabled", &CustomHarvesterSettings.HarvesterEnabled);

					LargeButtonToggle("Показывать статус сбора флоры###HarvesterStatusEnabled", &CustomHarvesterSettings.HarvesterStatusEnabled);

					LargeButtonToggle("Собирать с ESP флоры (Используются настройки ESP флоры)", &CustomHarvesterSettings.HarvesterOverrideEnabled);

					for (int i = 0; i < 69; i++)
					{
						ButtonToggle(CustomHarvesterSettings.HarvesterNameList[i], &CustomHarvesterSettings.HarvesterEnabledList[i]);
						ImGui::SameLine(235.0f);
						char LabelText[sizeof("###HarvesterReadOnly68")];
						sprintf_s(LabelText, "###HarvesterReadOnly%d", i);
						char FormidText[sizeof("00000000")];
						sprintf_s(FormidText, "%08lX", CustomHarvesterSettings.HarvesterFormidList[i]);
						ImGui::SetNextItemWidth(224.0f);
						ImGui::InputText(LabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_ReadOnly);
						if (ImGui::IsItemActive()) AllowDrag = false;
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Бой###CombatTab"))
			{
				if (ImGui::CollapsingHeader("Редактор оружия"))
				{
					ButtonToggle("Без отдачи", &CustomWeaponSettings.NoRecoil);
					ImGui::SameLine(235.0f);
					ButtonToggle("Бесконечные патроны", &CustomWeaponSettings.InfiniteAmmo);

					ButtonToggle("Без разброса", &CustomWeaponSettings.NoSpread);
					ImGui::SameLine(235.0f);
					ButtonToggle("Быстрая перезарядка", &CustomWeaponSettings.InstantReload);

					ButtonToggle("Без раскачки", &CustomWeaponSettings.NoSway);
					ImGui::SameLine(235.0f);
					ButtonToggle("Флаг автоматического###WeaponAutomatic", &CustomWeaponSettings.Automaticflag);

					ButtonToggle("Емкость###WeaponCapacityEnabled", &CustomWeaponSettings.CapacityEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###WeaponCapacity", &CustomWeaponSettings.Capacity, 0, 999, "Емкость: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomWeaponSettings.Capacity, 0, 999);

					ButtonToggle("Скорострельность###WeaponSpeedEnabled", &CustomWeaponSettings.SpeedEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###WeaponSpeed", &CustomWeaponSettings.Speed, 0.0f, 100.0f, "Скорость: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomWeaponSettings.Speed, 0.0f, 100.0f);

					ButtonToggle("Дальность###WeaponReachEnabled", &CustomWeaponSettings.ReachEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###WeaponReach", &CustomWeaponSettings.Reach, 0.0f, 999.0f, "Дальность: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomWeaponSettings.Reach, 0.0f, 999.0f);
				}

				if (ImGui::CollapsingHeader("Настройки наведения"))
				{
					ButtonToggle("Захват игроков (T)", &CustomTargetSettings.LockPlayers);
					ImGui::SameLine(235.0f);
					ButtonToggle("Захват NPC(T)", &CustomTargetSettings.LockNPCs);

					ButtonToggle("Перенаправить урон (Игроки)", &CustomTargetSettings.IndirectPlayers);
					ImGui::SameLine(235.0f);
					ButtonToggle("Перенаправить урон (NPCs)", &CustomTargetSettings.IndirectNPCs);

					ButtonToggle("Отправить урон (Игроки)", &CustomTargetSettings.DirectPlayers);
					ImGui::SameLine(235.0f);
					ButtonToggle("Отправить урон (NPCs)", &CustomTargetSettings.DirectNPCs);

					SmallButtonToggle("Живые###TargetLiving", &CustomTargetSettings.TargetLiving);
					ImGui::SameLine(122.0f);
					SmallButtonToggle("Упавшие###TargetDowned", &CustomTargetSettings.TargetDowned);
					ImGui::SameLine(235.0f);
					SmallButtonToggle("Мертвые###TargetDead", &CustomTargetSettings.TargetDead);
					ImGui::SameLine(349.0f);
					SmallButtonToggle("Неизвестно###TargetUnknown", &CustomTargetSettings.TargetUnknown);

					ButtonToggle("Игнорировать дистанцию отрисовки###IgnoreRenderDistance", &CustomTargetSettings.IgnoreRenderDistance);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###TargetLockingFOV", &CustomTargetSettings.LockingFOV, 5.0f, 40.0f, "FOV захвата: %.2f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomTargetSettings.LockingFOV, 5.0f, 40.0f);

					ButtonToggle("Игнарировать важных NPCs###IgnoreEssentialNPCs", &CustomTargetSettings.IgnoreEssentialNPCs);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::ColorEdit3("###TargetLockingColor", CustomTargetSettings.LockingColor);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateRGB(PlayerSettings.UnknownColor);

					ButtonToggle("Автоматический перезахват###TargetLockingRetargeting", &CustomTargetSettings.Retargeting);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					char TargetLockingCooldownText[sizeof("Задержка: 120 (1920 мс)")];
					sprintf_s(TargetLockingCooldownText, "Задержка: %d (%d ms)", CustomTargetSettings.Cooldown, CustomTargetSettings.Cooldown * 16);
					ImGui::SliderInt("###TargetLockingCooldown", &CustomTargetSettings.Cooldown, 0, 120, TargetLockingCooldownText);
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomTargetSettings.Cooldown, 0, 120);

					ImGui::SetNextItemWidth(224.0f);
					char SendDamageMinText[sizeof("Отправить урон (Мин): 60 (960 мс)")];
					sprintf_s(SendDamageMinText, "Отправить урон (Мин): %d (%d мс)", CustomTargetSettings.SendDamageMin, CustomTargetSettings.SendDamageMin * 16);
					ImGui::SliderInt("###SendDamageMin", &CustomTargetSettings.SendDamageMin, 1, 60, SendDamageMinText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomTargetSettings.SendDamageMax < CustomTargetSettings.SendDamageMin)
						{
							CustomTargetSettings.SendDamageMax = CustomTargetSettings.SendDamageMin;
						}
					}
					ValidateInt(&CustomTargetSettings.SendDamageMin, 1, 60);
					ValidateInt(&CustomTargetSettings.SendDamageMax, 1, 60);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					char SendDamageMaxText[sizeof("Отправить урон (Макс): 60 (960 мс)")];
					sprintf_s(SendDamageMaxText, "Отправить урон (Макс): %d (%d мс)", CustomTargetSettings.SendDamageMax, CustomTargetSettings.SendDamageMax * 16);
					ImGui::SliderInt("###SendDamageMax", &CustomTargetSettings.SendDamageMax, 1, 60, SendDamageMaxText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomTargetSettings.SendDamageMax < CustomTargetSettings.SendDamageMin)
						{
							CustomTargetSettings.SendDamageMin = CustomTargetSettings.SendDamageMax;
						}
					}
					ValidateInt(&CustomTargetSettings.SendDamageMin, 1, 60);
					ValidateInt(&CustomTargetSettings.SendDamageMax, 1, 60);
					if (CustomTargetSettings.SendDamageMax < CustomTargetSettings.SendDamageMin)
					{
						CustomTargetSettings.SendDamageMax = CustomTargetSettings.SendDamageMin;
					}

					char* FavoritedWeaponsPreview = nullptr;
					if (CustomTargetSettings.FavoriteIndex < 12)
					{
						FavoritedWeaponsPreview = GetFavoritedWeaponText(BYTE(CustomTargetSettings.FavoriteIndex));
						if (FavoritedWeaponsPreview == nullptr)
						{
							FavoritedWeaponsPreview = new char[sizeof("[?] Недопустимый избранный предмет")];
							sprintf_s(FavoritedWeaponsPreview, sizeof("[?] Недопустимый избранный предмет"), "[%c] Недопустимый избранный предмет", GetFavoriteSlot(BYTE(CustomTargetSettings.FavoriteIndex)));
						}
					}
					else
					{
						FavoritedWeaponsPreview = new char[sizeof("[?] Оружие не выбрано")];
						sprintf_s(FavoritedWeaponsPreview, sizeof("[?] Оружие не выбрано"), "[?] Оружие не выбрано");
					}

					ImGui::SetNextItemWidth(451.0f);
					if (ImGui::BeginCombo("###BeginTempCombo", FavoritedWeaponsPreview))
					{
						FavoritedWeaponsArray = GetFavoritedWeapons();
						if (FavoritedWeaponsArray == nullptr)
						{
							FavoritedWeaponsArray = new char* [13];
							FavoritedWeaponsArray[0] = new char[sizeof("[?] Оружие не выбрано")];
							sprintf_s(FavoritedWeaponsArray[0], sizeof("[?] Оружие не выбрано"), "[?] Оружие не выбрано");
							for (int i = 1; i < 13; i++)
							{
								FavoritedWeaponsArray[i] = new char[sizeof("[?] Недопустимый избранный предмет")];
								sprintf_s(FavoritedWeaponsArray[i], sizeof("[?] Недопустимый избранный предмет"), "[%c] Недопустимый избранный предмет", GetFavoriteSlot(BYTE(i - 1)));
							}
						}

						for (int i = 0; i < 13; i++)
						{
							if (ImGui::Selectable(FavoritedWeaponsArray[i]))
							{
								if (i)
								{
									CustomTargetSettings.FavoriteIndex = i - 1;
								}
								else
								{
									CustomTargetSettings.FavoriteIndex = 12;
								}
							}
						}

						ImGui::EndCombo();
						AllowDrag = false;
					}

					if (FavoritedWeaponsPreview != nullptr)
					{
						delete[]FavoritedWeaponsPreview;
						FavoritedWeaponsPreview = nullptr;
					}

					if (FavoritedWeaponsArray != nullptr)
					{
						for (int i = 0; i < 13; i++)
						{
							if (FavoritedWeaponsArray[i] != nullptr)
							{
								delete[]FavoritedWeaponsArray[i];
								FavoritedWeaponsArray[i] = nullptr;
							}
						}

						delete[]FavoritedWeaponsArray;
						FavoritedWeaponsArray = nullptr;
					}

					ValidateInt(&CustomTargetSettings.FavoriteIndex, 0, 12);
				}

				if (ImGui::CollapsingHeader("Настройки ближнего боя"))
				{
					LargeButtonToggle("Включить ближний бой (U)", &CustomMeleeSettings.MeleeEnabled);

					ImGui::SetNextItemWidth(224.0f);
					char MeleeSpeedMinText[sizeof("Скорость ближнего боя (Мин): 60 (960 мс)")];
					sprintf_s(MeleeSpeedMinText, "Скорость ближнего боя (Мин): %d (%d мс)", CustomMeleeSettings.MeleeSpeedMin, CustomMeleeSettings.MeleeSpeedMin * 16);
					ImGui::SliderInt("###MeleeSpeedMin", &CustomMeleeSettings.MeleeSpeedMin, 1, 60, MeleeSpeedMinText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomMeleeSettings.MeleeSpeedMax < CustomMeleeSettings.MeleeSpeedMin)
						{
							CustomMeleeSettings.MeleeSpeedMax = CustomMeleeSettings.MeleeSpeedMin;
						}
					}
					ValidateInt(&CustomMeleeSettings.MeleeSpeedMin, 1, 60);
					ValidateInt(&CustomMeleeSettings.MeleeSpeedMax, 1, 60);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					char MeleeSpeedMaxText[sizeof("Скорость ближнего боя (Макс): 60 (960 мс)")];
					sprintf_s(MeleeSpeedMaxText, "Скорость ближнего боя (Макс): %d (%d мс)", CustomMeleeSettings.MeleeSpeedMax, CustomMeleeSettings.MeleeSpeedMax * 16);
					ImGui::SliderInt("###MeleeSpeedMax", &CustomMeleeSettings.MeleeSpeedMax, 1, 60, MeleeSpeedMaxText);
					if (ImGui::IsItemActive())
					{
						AllowDrag = false;
						if (CustomMeleeSettings.MeleeSpeedMax < CustomMeleeSettings.MeleeSpeedMin)
						{
							CustomMeleeSettings.MeleeSpeedMin = CustomMeleeSettings.MeleeSpeedMax;
						}
					}
					ValidateInt(&CustomMeleeSettings.MeleeSpeedMin, 1, 60);
					ValidateInt(&CustomMeleeSettings.MeleeSpeedMax, 1, 60);
					if (CustomMeleeSettings.MeleeSpeedMax < CustomMeleeSettings.MeleeSpeedMin)
					{
						CustomMeleeSettings.MeleeSpeedMax = CustomMeleeSettings.MeleeSpeedMin;
					}
				}

				if (ImGui::CollapsingHeader("Притягивание"))
				{
					LargeButtonToggle("Притягивание игроков (CTRL+B)", &CustomOpkSettings.PlayersEnabled);
					LargeButtonToggle("Притягивание NPCs (CTRL+N)", &CustomOpkSettings.NpcsEnabled);
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Игрок###PlayerTab"))
			{
				if (ImGui::CollapsingHeader("Настройки игрока"))
				{
					LargeButtonToggle("Обман позиции (CTRL+L)##LocalPlayerPositionSpoofingEnabled", &CustomLocalPlayerSettings.PositionSpoofingEnabled);
					ButtonToggle("Установить высоту позиции###LocalPlayerDrawPositionSpoofingEnabled", &CustomLocalPlayerSettings.DrawPositionSpoofingEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerPositionSpoofingHeight", &CustomLocalPlayerSettings.PositionSpoofingHeight, -524287, 524287, "Высота позиции: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.PositionSpoofingHeight, -524287, 524287);

					ButtonToggle("Полет (CTRL+Y)###NoclipEnabled", &CustomLocalPlayerSettings.NoclipEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderFloat("###NoclipSpeed", &CustomLocalPlayerSettings.NoclipSpeed, 0.0f, 2.0f, "Скорость: %.5f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomLocalPlayerSettings.NoclipSpeed, 0.0f, 2.0f);

					ButtonToggle("Статус клиента", &CustomLocalPlayerSettings.ClientState);
					ImGui::SameLine(235.0f);
					ButtonToggle("Автоматический статус клиента", &CustomLocalPlayerSettings.AutomaticClientState);

					LargeButtonToggle("Заморозить очки действия###LocalPlayerFreezeApEnabled", &CustomLocalPlayerSettings.FreezeApEnabled);

					ButtonToggle("Очки действий###LocalPlayerAPEnabled", &CustomLocalPlayerSettings.ActionPointsEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerAP", &CustomLocalPlayerSettings.ActionPoints, 0, 99999, "Очки действий: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.ActionPoints, 0, 99999);

					ButtonToggle("Сила###LocalPlayerStrengthEnabled", &CustomLocalPlayerSettings.StrengthEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerStrength", &CustomLocalPlayerSettings.Strength, 0, 99999, "Сила: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Strength, 0, 99999);

					ButtonToggle("Восприятие###LocalPlayerPerceptionEnabled", &CustomLocalPlayerSettings.PerceptionEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerPerception", &CustomLocalPlayerSettings.Perception, 0, 99999, "Восприятие: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Perception, 0, 99999);

					ButtonToggle("Выносливость###LocalPlayerEnduranceEnabled", &CustomLocalPlayerSettings.EnduranceEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerEndurance", &CustomLocalPlayerSettings.Endurance, 0, 99999, "Выносливость: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Endurance, 0, 99999);

					ButtonToggle("Харизма###LocalPlayerCharismaEnabled", &CustomLocalPlayerSettings.CharismaEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerCharisma", &CustomLocalPlayerSettings.Charisma, 0, 99999, "Харизма: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Charisma, 0, 99999);

					ButtonToggle("Интилект###LocalPlayerIntelligenceEnabled", &CustomLocalPlayerSettings.IntelligenceEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerIntelligence", &CustomLocalPlayerSettings.Intelligence, 0, 99999, "Интилект: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Intelligence, 0, 99999);

					ButtonToggle("Ловкость###LocalPlayerAgilityEnabled", &CustomLocalPlayerSettings.AgilityEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerAgility", &CustomLocalPlayerSettings.Agility, 0, 99999, "Ловкость: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Agility, 0, 99999);

					ButtonToggle("Удача###LocalPlayerLuckEnabled", &CustomLocalPlayerSettings.LuckEnabled);
					ImGui::SameLine(235.0f);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::SliderInt("###LocalPlayerLuck", &CustomLocalPlayerSettings.Luck, 0, 99999, "Удача: %d");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateInt(&CustomLocalPlayerSettings.Luck, 0, 99999);
				}

				if (ImGui::CollapsingHeader("Настройки персонажа"))
				{
					LargeButtonToggle("Редактировать внешний вид персонажа###ChargenEditingEnabled", &CustomChargenSettings.ChargenEditingEnabled);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::SliderFloat("###ChargenThin", &CustomChargenSettings.Thin, 0.0f, 1.0f, "Вид пексонажа (Худой): %f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomChargenSettings.Thin, 0.0f, 1.0f);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::SliderFloat("###ChargenMuscular", &CustomChargenSettings.Muscular, 0.0f, 1.0f, "Вид пексонажа (Сильный): %f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomChargenSettings.Muscular, 0.0f, 1.0f);

					ImGui::SetNextItemWidth(451.0f);
					ImGui::SliderFloat("###ChargenLarge", &CustomChargenSettings.Large, 0.0f, 1.0f, "Вид пексонажа (Большой): %f");
					if (ImGui::IsItemActive()) AllowDrag = false;
					ValidateFloat(&CustomChargenSettings.Large, 0.0f, 1.0f);
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Инструменты###UtilityTab"))
			{
				if (ImGui::CollapsingHeader("Инструменты"))
				{
					ButtonToggle("Показать данные локального игрока", &CustomUtilitySettings.DebugPlayer);
					ImGui::SameLine(235.0f);
					ButtonToggle("Режим отладки ESP", &CustomUtilitySettings.DebugEsp);

					if (CustomUtilitySettings.PtrFormid)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
						if (ImGui::Button("Получить точку###GetPointerEnabled", ImVec2(224.0f, 0.0f)))
						{
							GetPtrResult = GetPtr(CustomUtilitySettings.PtrFormid);
						}
						ImGui::PopStyleColor(3);
					}
					else
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
						ImGui::Button("Получить точку###GetPointerDisabled", ImVec2(224.0f, 0.0f));
						ImGui::PopStyleColor(3);
						ImGui::PopItemFlag();
					}

					ImGui::SameLine(235.0f);
					char PtrFormidText[sizeof("00000000")];
					sprintf_s(PtrFormidText, "%08lX", CustomUtilitySettings.PtrFormid);
					ImGui::SetNextItemWidth(80.0f);
					if (ImGui::InputText("###PtrFormidText", PtrFormidText, sizeof(PtrFormidText), ImGuiInputTextFlags_CharsHexadecimal))
					{
						GetPtrResult = 0;
					}
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(PtrFormidText, "%08lX", &CustomUtilitySettings.PtrFormid);

					ImGui::SameLine(318.0f);
					char PtrPointerText[sizeof("0000000000000000")];
					sprintf_s(PtrPointerText, "%016llX", GetPtrResult);
					ImGui::SetNextItemWidth(141.0f);
					ImGui::InputText("###PtrPointerText", PtrPointerText, sizeof(PtrPointerText), ImGuiInputTextFlags_ReadOnly);
					if (ImGui::IsItemActive()) AllowDrag = false;

					if (CustomUtilitySettings.AddressFormid)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
						if (ImGui::Button("Получить адрес###GetAddressEnabled", ImVec2(224.0f, 0.0f)))
						{
							GetAddressResult = GetAddress(CustomUtilitySettings.AddressFormid);
						}
						ImGui::PopStyleColor(3);
					}
					else
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
						ImGui::Button("Получить адрес###GetAddressDisabled", ImVec2(224.0f, 0.0f));
						ImGui::PopStyleColor(3);
						ImGui::PopItemFlag();
					}

					ImGui::SameLine(235.0f);
					char AddressFormidText[sizeof("00000000")];
					sprintf_s(AddressFormidText, "%08lX", CustomUtilitySettings.AddressFormid);
					ImGui::SetNextItemWidth(80.0f);
					if (ImGui::InputText("###AddressFormidText", AddressFormidText, sizeof(AddressFormidText), ImGuiInputTextFlags_CharsHexadecimal))
					{
						GetAddressResult = 0;
					}
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(AddressFormidText, "%08lX", &CustomUtilitySettings.AddressFormid);

					ImGui::SameLine(318.0f);
					char AddressPointerText[sizeof("0000000000000000")];
					sprintf_s(AddressPointerText, "%016llX", GetAddressResult);
					ImGui::SetNextItemWidth(141.0f);
					ImGui::InputText("###AddressPointerText", AddressPointerText, sizeof(AddressPointerText), ImGuiInputTextFlags_ReadOnly);
					if (ImGui::IsItemActive()) AllowDrag = false;
				}

				if (ImGui::CollapsingHeader("Редактор ссылок"))
				{
					ButtonToggle("Источник Formid###SwapperSourceFormidToggle", &SwapperSourceToggle);
					ImGui::SameLine(235.0f);
					char SourceFormidText[sizeof("00000000")];
					sprintf_s(SourceFormidText, "%08lX", CustomSwapperSettings.SourceFormid);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::InputText("###SwapperSourceFormidText", SourceFormidText, sizeof(SourceFormidText), ImGuiInputTextFlags_CharsHexadecimal);
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(SourceFormidText, "%08lX", &CustomSwapperSettings.SourceFormid);

					ButtonToggle("Цель Formid###SwapperDestinationFormidToggle", &SwapperDestinationToggle);
					ImGui::SameLine(235.0f);
					char DestinationFormidText[sizeof("00000000")];
					sprintf_s(DestinationFormidText, "%08lX", CustomSwapperSettings.DestinationFormid);
					ImGui::SetNextItemWidth(224.0f);
					ImGui::InputText("###SwapperDestinationFormidText", DestinationFormidText, sizeof(DestinationFormidText), ImGuiInputTextFlags_CharsHexadecimal);
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(DestinationFormidText, "%08lX", &CustomSwapperSettings.DestinationFormid);

					if (SwapperSourceToggle && CustomSwapperSettings.SourceFormid && SwapperDestinationToggle && CustomSwapperSettings.DestinationFormid)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
						if (ImGui::Button("Редактировать ссылку (Перезаписать цель)###EditReferenceEnabled", ImVec2(451.0f, 0.0f)))
						{
							if (ReferenceSwap(&CustomSwapperSettings.SourceFormid, &CustomSwapperSettings.DestinationFormid))
							{
								CustomSwapperSettings.DestinationFormid = CustomSwapperSettings.SourceFormid;
								SwapperSourceToggle = false;
								SwapperDestinationToggle = false;
							}
						}
						ImGui::PopStyleColor(3);
					}
					else
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
						ImGui::Button("Редактировать ссылку (Перезаписать цель)###EditReferenceDisabled", ImVec2(451.0f, 0.0f));
						ImGui::PopStyleColor(3);
						ImGui::PopItemFlag();
					}
				}

				if (ImGui::CollapsingHeader("Передача предметов"))
				{
					SmallButtonToggle("Источник###TransferSourceFormidToggle", &TransferSourceToggle);

					ImGui::SameLine(122.0f);
					char SourceFormidText[sizeof("00000000")];
					sprintf_s(SourceFormidText, "%08lX", CustomTransferSettings.SourceFormid);
					ImGui::SetNextItemWidth(110.0f);
					ImGui::InputText("###TransferSourceFormidText", SourceFormidText, sizeof(SourceFormidText), ImGuiInputTextFlags_CharsHexadecimal);
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(SourceFormidText, "%08lX", &CustomTransferSettings.SourceFormid);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
					ImGui::SameLine(235.0f);
					if (ImGui::Button("Игрок###TransferSourceLocalPlayer", ImVec2(110.0f, 0.0f)))
					{
						CustomTransferSettings.SourceFormid = GetLocalPlayerFormid();
					}
					ImGui::SameLine(349.0f);
					if (ImGui::Button("Сундук###TransferSourceSTASH", ImVec2(110.0f, 0.0f)))
					{
						CustomTransferSettings.SourceFormid = GetStashFormid();
					}
					ImGui::PopStyleColor(3);

					SmallButtonToggle("Цель###TransferDestinationFormidToggle", &TransferDestinationToggle);
					ImGui::SameLine(122.0f);
					char DestinationFormidText[sizeof("00000000")];
					sprintf_s(DestinationFormidText, "%08lX", CustomTransferSettings.DestinationFormid);
					ImGui::SetNextItemWidth(110.0f);
					ImGui::InputText("###TransferDestinationFormidText", DestinationFormidText, sizeof(DestinationFormidText), ImGuiInputTextFlags_CharsHexadecimal);
					if (ImGui::IsItemActive()) AllowDrag = false;
					sscanf_s(DestinationFormidText, "%08lX", &CustomTransferSettings.DestinationFormid);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
					ImGui::SameLine(235.0f);
					if (ImGui::Button("Игрок###TransferDestinationLocalPlayer", ImVec2(110.0f, 0.0f)))
					{
						CustomTransferSettings.DestinationFormid = GetLocalPlayerFormid();
					}
					ImGui::SameLine(349.0f);
					if (ImGui::Button("Сундук###TransferDestinationSTASH", ImVec2(110.0f, 0.0f)))
					{
						CustomTransferSettings.DestinationFormid = GetStashFormid();
					}
					ImGui::PopStyleColor(3);

					bool AllowTransfer = false;

					if (TransferSourceToggle && CustomTransferSettings.SourceFormid && TransferDestinationToggle && CustomTransferSettings.DestinationFormid)
					{
						if (CustomTransferSettings.UseWhitelist)
						{
							AllowTransfer = CheckItemTransferList();
						}
						else
						{
							AllowTransfer = true;
						}
					}

					if (AllowTransfer)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
						if (ImGui::Button("Передать предметы###TransferItemsEnabled", ImVec2(451.0f, 0.0f)))
						{
							TransferItems(CustomTransferSettings.SourceFormid, CustomTransferSettings.DestinationFormid);
						}
						ImGui::PopStyleColor(3);
					}
					else
					{
						ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
						ImGui::Button("Передать предметы###TransferItemsDisabled", ImVec2(451.0f, 0.0f));
						ImGui::PopStyleColor(3);
						ImGui::PopItemFlag();
					}

					LargeButtonToggle("Использовать белый список", &CustomTransferSettings.UseWhitelist);
					LargeButtonToggle("Использовать черный список", &CustomTransferSettings.UseBlacklist);

					if (ImGui::CollapsingHeader("Настройки белого списка"))
					{
						for (int i = 0; i < 32; i++)
						{
							char WhitelistedText[sizeof("Слот передачи: 31")];
							char WhitelistText[sizeof("###ItemTransferWhitelist31")];
							sprintf_s(WhitelistedText, "Слот передачи: %d", i);
							sprintf_s(WhitelistText, "###ItemTransferWhitelist%d", i);
							ButtonToggle(WhitelistedText, &CustomTransferSettings.Whitelisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", CustomTransferSettings.Whitelist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(WhitelistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &CustomTransferSettings.Whitelist[i]);
						}
					}

					if (ImGui::CollapsingHeader("Настройки черного списка"))
					{
						for (int i = 0; i < 32; i++)
						{
							char BlacklistedText[sizeof("Слот передачи: 31")];
							char BlacklistText[sizeof("###ItemTransferBlacklist31")];
							sprintf_s(BlacklistedText, "Слот передачи: %d", i);
							sprintf_s(BlacklistText, "###ItemTransferBlacklist%d", i);
							ButtonToggle(BlacklistedText, &CustomTransferSettings.Blacklisted[i]);
							ImGui::SameLine(235.0f);
							char FormidText[sizeof("00000000")];
							sprintf_s(FormidText, "%08lX", CustomTransferSettings.Blacklist[i]);
							ImGui::SetNextItemWidth(224.0f);
							ImGui::InputText(BlacklistText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
							if (ImGui::IsItemActive()) AllowDrag = false;
							sscanf_s(FormidText, "%08lX", &CustomTransferSettings.Blacklist[i]);
						}
					}
				}

				if (ImGui::CollapsingHeader("Ядерные коды"))
				{
					ButtonToggle("Автоматически получать коды", &CustomNukeCodeSettings.AutomaticNukeCodes);
					ImGui::SameLine(235.0f);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
					if (ImGui::Button("Получить ядерные коды", ImVec2(224.0f, 0.0f)))
					{
						GetNukeCode(0x000921AE, AlphaCode);
						GetNukeCode(0x00092213, BravoCode);
						GetNukeCode(0x00092214, CharlieCode);
					}
					ImGui::PopStyleColor(3);

					ButtonToggle("Показывать коды Альфа", &CustomNukeCodeSettings.DrawCodeAlpha);
					ImGui::SameLine(255.0f);
					ImGui::Text("%d %d %d %d %d %d %d %d - Альфа", AlphaCode[0], AlphaCode[1], AlphaCode[2], AlphaCode[3], AlphaCode[4], AlphaCode[5], AlphaCode[6], AlphaCode[7]);

					ButtonToggle("Показывать коды Браво", &CustomNukeCodeSettings.DrawCodeBravo);
					ImGui::SameLine(255.0f);
					ImGui::Text("%d %d %d %d %d %d %d %d - Браво", BravoCode[0], BravoCode[1], BravoCode[2], BravoCode[3], BravoCode[4], BravoCode[5], BravoCode[6], BravoCode[7]);

					ButtonToggle("Показывать коды Чарли", &CustomNukeCodeSettings.DrawCodeCharlie);
					ImGui::SameLine(255.0f);
					ImGui::Text("%d %d %d %d %d %d %d %d - Чарли", CharlieCode[0], CharlieCode[1], CharlieCode[2], CharlieCode[3], CharlieCode[4], CharlieCode[5], CharlieCode[6], CharlieCode[7]);
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Телепорт###TeleporterTab"))
			{
				for (int i = 0; i < 16; i++)
				{
					char TeleportHeaderText[sizeof("Слот телепорта: 15")];
					sprintf_s(TeleportHeaderText, "Слот телепорта: %d", i);
					if (ImGui::CollapsingHeader(TeleportHeaderText))
					{
						ImGui::SetNextItemWidth(110.0f);
						char TeleportDestinationTextX[sizeof("###TeleportDestinationX15")];
						sprintf_s(TeleportDestinationTextX, "###TeleportDestinationX%d", i);
						ImGui::InputFloat(TeleportDestinationTextX, &CustomTeleportSettings.TeleportEntryData[i].Destination[0]);
						if (ImGui::IsItemActive()) AllowDrag = false;
						ImGui::SameLine(122.0f);
						ImGui::SetNextItemWidth(110.0f);
						char TeleportDestinationTextY[sizeof("###TeleportDestinationY15")];
						sprintf_s(TeleportDestinationTextY, "###TeleportDestinationY%d", i);
						ImGui::InputFloat(TeleportDestinationTextY, &CustomTeleportSettings.TeleportEntryData[i].Destination[1]);
						if (ImGui::IsItemActive()) AllowDrag = false;
						ImGui::SameLine(235.0f);
						ImGui::SetNextItemWidth(110.0f);
						char TeleportDestinationTextZ[sizeof("###TeleportDestinationZ15")];
						sprintf_s(TeleportDestinationTextZ, "###TeleportDestinationZ%d", i);
						ImGui::InputFloat(TeleportDestinationTextZ, &CustomTeleportSettings.TeleportEntryData[i].Destination[2]);
						if (ImGui::IsItemActive()) AllowDrag = false;
						ImGui::SameLine(349.0f);
						ImGui::SetNextItemWidth(110.0f);
						char TeleportDestinationTextW[sizeof("###TeleportDestinationW15")];
						sprintf_s(TeleportDestinationTextW, "###TeleportDestinationW%d", i);
						ImGui::InputFloat(TeleportDestinationTextW, &CustomTeleportSettings.TeleportEntryData[i].Destination[3]);
						if (ImGui::IsItemActive()) AllowDrag = false;

						char FormidLabelText[sizeof("###TeleportCellFormid15")];
						sprintf_s(FormidLabelText, "###TeleportCellFormid%d", i);
						char FormidText[sizeof("00000000")];
						sprintf_s(FormidText, "%08lX", CustomTeleportSettings.TeleportEntryData[i].CellFormid);
						ImGui::SetNextItemWidth(110.0f);
						ImGui::InputText(FormidLabelText, FormidText, sizeof(FormidText), ImGuiInputTextFlags_CharsHexadecimal);
						if (ImGui::IsItemActive()) AllowDrag = false;
						sscanf_s(FormidText, "%08lX", &CustomTeleportSettings.TeleportEntryData[i].CellFormid);
						ImGui::SameLine(122.0f);
						char TeleportDestinationEnabledText[sizeof("Установить###TeleportDestinationEnabled15")];
						sprintf_s(TeleportDestinationEnabledText, "Установить###TeleportDestinationEnabled%d", i);
						char TeleportDestinationDisabledText[sizeof("Установить###TeleportDestinationDisabled15")];
						sprintf_s(TeleportDestinationDisabledText, "Установить###TeleportDestinationDisabled%d", i);
						if (!CustomTeleportSettings.TeleportEntryData[i].DisableSaving)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
							if (ImGui::Button(TeleportDestinationEnabledText, ImVec2(110.0f, 0.0f))) GetTeleportPosition(i);
							ImGui::PopStyleColor(3);
						}
						else
						{
							ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
							ImGui::Button(TeleportDestinationDisabledText, ImVec2(110.0f, 0.0f));
							ImGui::PopStyleColor(3);
							ImGui::PopItemFlag();
						}
						ImGui::SameLine(235.0f);
						char DisableSavingText[sizeof("Блок###DisableSaving15")];
						sprintf_s(DisableSavingText, "Блок###DisableSaving%d", i);
						SmallButtonToggle(DisableSavingText, &CustomTeleportSettings.TeleportEntryData[i].DisableSaving);
						ImGui::SameLine(349.0f);
						char TeleportRequestEnabledText[sizeof("Телепорт###TeleportRequestEnabled15")];
						sprintf_s(TeleportRequestEnabledText, "Телепорт###TeleportRequestEnabled%d", i);
						char TeleportRequestDisabledText[sizeof("Телепорт###TeleportRequestDisabled15")];
						sprintf_s(TeleportRequestDisabledText, "Телепорт###TeleportRequestDisabled%d", i);
						if (CustomTeleportSettings.TeleportEntryData[i].CellFormid)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.0f, 0.4f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 1.0f, 0.0f, 0.5f));
							if (ImGui::Button(TeleportRequestEnabledText, ImVec2(110.0f, 0.0f))) RequestTeleport(i);
							ImGui::PopStyleColor(3);
						}
						else
						{
							ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
							ImGui::Button(TeleportRequestDisabledText, ImVec2(110.0f, 0.0f));
							ImGui::PopStyleColor(3);
							ImGui::PopItemFlag();
						}
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("BitMsgWriter###BitMsgWriterTab"))
			{
				LargeButtonToggle("Включить отправку сообщений", &AllowMessages);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		if (ImGui::GetActiveID() == ImGui::GetWindowScrollbarID(ImGui::GetCurrentWindow(), ImGuiAxis_Y))
		{
			AllowDrag = false;
		}

		if (ImGui::IsMouseDragging(0) && AllowDrag)
		{
			if (!PointerDrag)
			{
				PointerOrigin = ImGui::GetMousePos();
				PointerDrag = true;
			}
		}
		else
		{
			if (PointerDrag)
			{
				PointerOrigin = { 0.0f, 0.0f };
				PointerDrag = false;
			}
		}

		if (PointerDrag)
		{
			DragMenu();
		}
	}

	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

bool ImGuiInitialize()
{
	ImGui::CreateContext();
	ImGuiContextCreated = true;

	if (!ImGui_ImplWin32_Init(WindowHwnd)) return false;
	else ImGuiWin32Initialized = true;

	if (!ImGui_ImplDX9_Init(D3D9Device)) return false;
	else ImGuiD3D9Initialized = true;

	ImFontConfig font_config;
	font_config.OversampleH = 1; //or 2 is the same
	font_config.OversampleV = 1;
	font_config.PixelSnapH = 1;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x044F, // Cyrillic
		0,
	};
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 14.0f, &font_config, ranges);

	return true;
}

void ImGuiCleanup()
{
	if (ImGuiD3D9Initialized)
	{
		ImGui_ImplDX9_Shutdown();
		ImGuiD3D9Initialized = false;
	}

	if (ImGuiWin32Initialized)
	{
		ImGui_ImplWin32_Shutdown();
		ImGuiWin32Initialized = false;
	}

	if (ImGuiContextCreated)
	{
		ImGui::DestroyContext();
		ImGuiContextCreated = false;
	}
}