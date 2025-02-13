// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "W_MainMenu.h"
#include "Adventure.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "W_MainMenu_Child.h"


bool UW_MainMenu::Initialize()
{
	bool preInit = Super::Initialize();

	if (preInit)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			GameInstance = Cast<UGI_Adventure>(World->GetGameInstance());
			if (GameInstance)
			{
				CreateSubMenus();
			}
		}
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("Failed to create menu: %s"), *this->GetName());
	}

	return preInit;
}

bool UW_MainMenu::HostGame(const FHOSTGAME_SETTINGS & Settings)
{
	if (GameInstance)
	{
		return GameInstance->HostGame(Settings);
	}

	return false;
}

bool UW_MainMenu::JoinGame(const FJOINGAME_SETTINGS & Settings)
{
	if (GameInstance)
	{
		return GameInstance->JoinGame(Settings);
	}

	return false;
}

bool UW_MainMenu::LoadGameBuilder(const FGAMEBUILDER_SETTINGS & Settings)
{
	if (GameInstance)
	{
		return GameInstance->LoadGameBuilder(Settings);
	}

	return false;
}

void UW_MainMenu::CloseGame()
{
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* controller = World->GetFirstPlayerController();
		if (controller)
		{
			controller->ConsoleCommand("quit", true);
		}
	}
}

void UW_MainMenu::ActivateSubMenu(int index)
{
	if (SubMenu)
	{
		SubMenu->SetActiveWidgetIndex(index);
	}
}

void UW_MainMenu::RefreshServerList(const FSESSION_SEARCH_SETTINGS& Settings)
{
	if (GameInstance)
	{
		FSESSION_SEARCH_SETTINGS settings;
		GameInstance->FindSessions(settings);
	}
}

void UW_MainMenu::GetSessionList(TArray<FString>& Array) const
{
	if (GameInstance)
	{
		GameInstance->GetSessionList(Array);
	}
}

bool UW_MainMenu::IsSessionSearchActive()const
{
	if (GameInstance)
	{
		return GameInstance->IsSessionSearchActive();
	}
	return false;
}

void UW_MainMenu::CreateSubMenus()
{
	if (GameInstance)
	{
		if (SubMenu)
		{
			for (const auto& menuClass : SubMenuClasses)
			{
				UW_MainMenu_Child* Child = CreateWidget<UW_MainMenu_Child>(GameInstance, menuClass);
				if (Child)
				{
					Child->ConnectTo(this);
					SubMenu->AddChild(Child);
				}
				else
				{
					UE_LOG(LogNotice, Warning, TEXT("Initializing a submenu failed: Target was NULL."));
				}
			}
		}
		else
		{
			UE_LOG(LogNotice, Error, TEXT("Initializing submenus for main menu failed: Sub Menu could not be found."));
		}
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("Initializing submenus for main menu failed: GameInstance could not be found."));
	}
}
