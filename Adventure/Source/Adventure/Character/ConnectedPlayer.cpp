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

#include "ConnectedPlayer.h"

#include "../PlayerControllers/PC_Multiplayer.h"
#include "Grid/WorldGrid.h"
#include "MapPawn.h"
#include "Adventure.h"
#include "GameModes/GM_Multiplayer.h"
#include "GameStates/GS_Multiplayer.h"
#include "PlayerStates/PS_Multiplayer.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StatisticsComponent.h"
#include "Components/InteractionInterfaceComponent.h"

// Sets default values
AConnectedPlayer::AConnectedPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_SelectedPawn = nullptr;
	SpectatingPawnID = 0;
	m_CameraTransitionAcceleration = 5500.0f;
	m_CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a static mesh component
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Focus"));
	RootComponent = Scene;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(Scene);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 10.0f;
	CameraBoom->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation

	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

// Called every frame
void AConnectedPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AConnectedPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//sets variables for replicaton over a network
void AConnectedPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AConnectedPlayer, SpectatingPawnID);
}

bool AConnectedPlayer::GetPawnLocation(FVector & Location) const
{
	if (m_SelectedPawn)
	{
		Location = m_SelectedPawn->GetActorLocation();
		return true;
	}
	return false;
}

AMapPawn * AConnectedPlayer::GetSelectedPawn() const
{
	return m_SelectedPawn;
}

void AConnectedPlayer::SwapCameraView()
{
	switch (m_CameraType)
	{
	case CONNECTED_PLAYER_CAMERA::CHARACTER:
		SetCameraToOverview();
		break;
	case CONNECTED_PLAYER_CAMERA::OVERVIEW:
		SetCameraToCharacter();
		break;
	case CONNECTED_PLAYER_CAMERA::NONE:
	default:
		UE_LOG(LogNotice, Warning, TEXT("Connected player camera type is invalid"));
		break;
	}
}

void AConnectedPlayer::RotatePawnCameraUpDown(const float & AxisValue, const float & DeltaTime)
{
	if (m_SelectedPawn)
	{
		m_SelectedPawn->RotateCameraPitch(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::RotatePawnCameraLeftRight(const float & AxisValue, const float & DeltaTime)
{
	if (m_SelectedPawn)
	{
		m_SelectedPawn->RotateCameraYaw(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::ZoomPawnCameraInOut(const float & AxisValue, const float & DeltaTime)
{
	if (m_SelectedPawn)
	{
		m_SelectedPawn->ZoomCamera(AxisValue, DeltaTime);
	}
}

CONNECTED_PLAYER_CAMERA AConnectedPlayer::GetCameraType() const
{
	return m_CameraType;
}

bool AConnectedPlayer::GetSelectedActorLocation(FVector& Location) const
{
	if (m_SelectedPawn)
	{
		Location = m_SelectedPawn->GetActorLocation();
		return true;
	}

	return false;
}

UStatisticsComponent* AConnectedPlayer::GetSelectedActorStats() const
{
	if (m_SelectedPawn)
	{
		return m_SelectedPawn->GetStats();
	}

	return nullptr;
}

int AConnectedPlayer::GetSpectatingPawnID() const
{
	return SpectatingPawnID;
}

void AConnectedPlayer::MovePlayer(const FVector & Location)
{
	if (m_SelectedPawn)
	{
		Server_MovePlayer(m_SelectedPawn->GetPawnID(), m_SelectedPawn->GetActorLocation(), Location);
	}
}

void AConnectedPlayer::SetPawnTargetLocation(const FVector & Location)
{
	Server_SetPawnTargetLocation(0, Location);
}

void AConnectedPlayer::ClearPawnTargetLocation()
{
	Server_ClearPawnTargetLocation();
}

void AConnectedPlayer::SetSpectatingPawn(const int PawnID)
{
	if (PawnID == -1)
	{
		SetCameraToOverview();
		m_SelectedPawn = nullptr;
	}
	else
	{
		for (TActorIterator<AMapPawn> MapPawnIter(GetWorld()); MapPawnIter; ++MapPawnIter)
		{
			if (MapPawnIter->GetPawnID() == PawnID)
			{
				m_SelectedPawn = *MapPawnIter;
				SetCameraToCharacter();
			}
		}
	}
}

void AConnectedPlayer::SetCameraToOverview()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController && m_SelectedPawn)
	{
		// Calculate Time
		float Time = 0.8f;

		FVector newLocation = m_SelectedPawn->GetActorLocation();
		SetActorLocation(FVector(newLocation.X, newLocation.Y, GetActorLocation().Z));

		PlayerController->SetViewTargetWithBlend(this, Time, VTBlend_Cubic, 0.0f, true);
		m_CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;
		OnCameraTypeChanged(m_CameraType);
	}
}

void AConnectedPlayer::SetCameraToCharacter()
{
	if (m_SelectedPawn)
	{
		m_SelectedPawn->SetFocusToPawn(0.8f);
		m_CameraType = CONNECTED_PLAYER_CAMERA::CHARACTER;
		OnCameraTypeChanged(m_CameraType);
	}

}

//////////////////////// Private Server Functions ////////////////////////

void AConnectedPlayer::Server_MovePlayer_Implementation(const int PawnID, const FVector& Location, const FVector& Destination)
{
	APS_Multiplayer* state = Cast<APS_Multiplayer>(GetPlayerState());
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		AMapPawn* pawn = WorldGrid->ServerOnly_GetPawn(Location, PawnID);
		if (pawn && state)
		{
			UE_LOG(LogNotice, Warning, TEXT("<ConnectedPlayer%i>: Attempting to move pawn with owner ID: %i"), state->GetGameID(), pawn->GetOwnerID());
			if (state->GetGameID() == 0 || (state->GetGameID() == pawn->GetOwnerID()))
			{
				pawn->ServerOnly_SetDestination(Destination);
			}
		}
	}
}

bool AConnectedPlayer::Server_MovePlayer_Validate(const int PawnID, const FVector& Location, const FVector& Destination)
{
	return true;
}

void AConnectedPlayer::Server_SetPawnTargetLocation_Implementation(const int PawnID, const FVector& Location)
{
}

bool AConnectedPlayer::Server_SetPawnTargetLocation_Validate(const int PawnID, const FVector& Location)
{
	return true;
}

void AConnectedPlayer::Server_ClearPawnTargetLocation_Implementation()
{
}

bool AConnectedPlayer::Server_ClearPawnTargetLocation_Validate()
{
	return true;
}