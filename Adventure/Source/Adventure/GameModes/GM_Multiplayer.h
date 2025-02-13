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

#pragma once

#include <map>
#include <string>
#include "basics.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GM_Multiplayer.generated.h"

/**
 *
 */
UCLASS()
class ADVENTURE_API AGM_Multiplayer : public AGameModeBase
{
	GENERATED_BODY()

	public:

	AGM_Multiplayer();

	// Sets game session to in progress and loads the map
	void StartGame();

	// Set map to load
	UFUNCTION(BlueprintCallable, Category = "Lobby Gamemode")
	void SetMapToLoad(const FString& Name);

	// Get map to load
	UFUNCTION(BlueprintCallable, Category = "Lobby Gamemode")
	void GetMapToLoad(FString& Name)const;

protected:

	// Function called when a player has successfully logged in
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:

	// Generates a playerID to be able to refrence separate instances
	int GeneratePlayerID();

	// Handles login attempt (ensure the function remains fast and simple or client will hang)
	void LoginConnectedPlayer(AController * Player);

	bool LoadMap(const FString & MapName);

	bool m_bMapHasBeenQueued;
	int m_PlayerIndexCount;
	FString m_CurrentMapName;
	FString m_HostUsername;
	FGridCoordinate m_GridDimensions;
	std::map<std::string, int> m_ConnnectedPlayers;
};
