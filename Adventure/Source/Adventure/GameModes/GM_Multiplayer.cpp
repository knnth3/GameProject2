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

#include "GM_Multiplayer.h"

#include "Adventure.h"
#include "GI_Adventure.h"
#include "Grid/WorldGrid.h"
#include "Widgets/HUD_MPLobby.h"
#include "PlayerStates/PS_Multiplayer.h"
#include "GameStates/GS_Multiplayer.h"
#include "PlayerControllers/PC_Multiplayer.h"
#include "DownloadManager/DownloadManager.h"

AGM_Multiplayer::AGM_Multiplayer()
{
	m_bMapHasBeenQueued = false;
	bUseSeamlessTravel = true;
	m_PlayerIndexCount = 0;
}

void AGM_Multiplayer::StartGame()
{
	UGI_Adventure* GameInstance = Cast<UGI_Adventure>(GetGameInstance());
	if (GameInstance)
	{
		UE_LOG(LogNotice, Warning, TEXT("<GameMode>: Map set and is ready for game start"));
		GameInstance->StartSession();
		LoadMap(m_CurrentMapName);
	}
}

void AGM_Multiplayer::SetMapToLoad(const FString & Name)
{
	m_CurrentMapName = Name;
}

void AGM_Multiplayer::GetMapToLoad(FString & Name)const
{
	Name = m_CurrentMapName;
}

void AGM_Multiplayer::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: New player joined!"));
	LoginConnectedPlayer(NewPlayer);
}

int AGM_Multiplayer::GeneratePlayerID()
{
	return m_PlayerIndexCount++;
}

void AGM_Multiplayer::LoginConnectedPlayer(AController * Player)
{
	APS_Multiplayer* currentPlayerState = Cast<APS_Multiplayer>(Player->PlayerState);
	AGS_Multiplayer* gameState = Cast<AGS_Multiplayer>(GameState);

	if (currentPlayerState && gameState)
	{
		std::string PlayerName = TCHAR_TO_UTF8(*currentPlayerState->GetPlayerName());

		//First to connect will be the owner
		if (m_ConnnectedPlayers.empty())
		{
			m_HostUsername = FString(PlayerName.c_str());
			UE_LOG(LogNotice, Warning, TEXT("<ServerSetup>: Host registered as %s"), *m_HostUsername);
		}

		// New player has joined
		if (m_ConnnectedPlayers.find(PlayerName) == m_ConnnectedPlayers.end())
		{
			m_ConnnectedPlayers[PlayerName] = GeneratePlayerID();
			currentPlayerState->ServerOnly_SetGameID(m_ConnnectedPlayers[PlayerName]);

			gameState->AddNewPlayer(m_ConnnectedPlayers[PlayerName], currentPlayerState->GetPlayerName());

			UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: %s has connected. Player was assigned to GameID: %i"), *FString(PlayerName.c_str()), currentPlayerState->GetGameID());
		}
		else
		{
			currentPlayerState->ServerOnly_SetGameID(m_ConnnectedPlayers[PlayerName]);
			UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: %s has reconnected."), *FString(PlayerName.c_str()));
		}
	}
}

bool AGM_Multiplayer::LoadMap(const FString& MapName)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->ServerOnly_SetMapName(MapName);
	}

	FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *MapName);
	UMapSaveFile* Save = Cast<UMapSaveFile>(UBasicFunctions::LoadSaveGameEx(path));
	if (Save)
	{
		FString CurrentLocation = Save->ActiveLocation;

		for (auto& loc : Save->Locations)
		{
			if (loc.Name == CurrentLocation)
			{
				// Create a containter to store data that will be sent over
				ULocationSave* Location = Cast<ULocationSave>(UGameplayStatics::CreateSaveGameObject(ULocationSave::StaticClass()));
				Location->LocationData = loc;

				// Pack data into a buffer
				TArray<uint8> Buffer;
				if (UBasicFunctions::ConvertSaveToBinary(Location, Buffer))
				{
					ADownloadManager::ServerOnly_SetData(Buffer);

					return true;
				}

			}
		}
	}

	return false;
}
