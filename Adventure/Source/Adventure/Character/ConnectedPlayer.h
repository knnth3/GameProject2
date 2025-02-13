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

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ConnectedPlayer.generated.h"

UENUM(BlueprintType)
enum class CONNECTED_PLAYER_CAMERA : uint8
{
	NONE,
	CHARACTER,
	OVERVIEW
};

UCLASS()
class ADVENTURE_API AConnectedPlayer : public APawn
{
	GENERATED_BODY()

public:
	AConnectedPlayer();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	bool GetPawnLocation(FVector& Location) const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	class AMapPawn* GetSelectedPawn() const;

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	CONNECTED_PLAYER_CAMERA GetCameraType()const;

protected:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Player Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// Camera Functions
	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SwapCameraView();

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void RotatePawnCameraUpDown(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void RotatePawnCameraLeftRight(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "MapPawn Camera")
	void ZoomPawnCameraInOut(const float& AxisValue, const float& DeltaTime);

	// Accessor Functions

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	bool GetSelectedActorLocation(FVector& Location) const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	class UStatisticsComponent* GetSelectedActorStats()const;

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	int GetSpectatingPawnID() const;
	
	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void MovePlayer(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SetPawnTargetLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void ClearPawnTargetLocation();

	UFUNCTION(BlueprintCallable, Category = "Connected Player")
	void SetSpectatingPawn(const int PawnID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Connected Player")
	void OnCameraTypeChanged(CONNECTED_PLAYER_CAMERA Type);

private:

	void SetCameraToOverview();
	void SetCameraToCharacter();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MovePlayer(const int PawnID, const FVector& Location, const FVector& Destination);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetPawnTargetLocation(const int PawnID, const FVector& Location);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ClearPawnTargetLocation();

	UPROPERTY(Replicated)
	int SpectatingPawnID;

	class AMapPawn* m_SelectedPawn;
	class AWorldGrid* m_WorldGrid;
	float m_CameraTransitionAcceleration;
	CONNECTED_PLAYER_CAMERA m_CameraType;
};
