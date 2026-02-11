// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "SpawnAndLockPawn.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class UFloatingPawnMovement;
class UStaticMeshComponent;

UCLASS()
class BOIDSPROJECT_API ASpawnAndLockPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASpawnAndLockPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UFloatingPawnMovement* MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	UStaticMeshComponent* PreviewCubeMesh;

	// Enhanced Input

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputAction* MoveVerticalAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = "Input")
	UInputAction* SpawnBoidAction;

	// Input Functions

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void MoveVertical(const FInputActionValue& Value);
	void OnSpawnBoidPressed();
	void OnSpawnBoidReleased();

	// Movement Settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MovementSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float LookSensitivity = 1.0f;

	// Spawn Settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float RaycastDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnoffSetDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float DefaultSpawnDistance = 2000.0f;

	// Boid 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AActor> BoidClass;

private:
	// 레이캐스트 중인지 여부
	bool bIsRaycasting = false;

	// 현재 하트 위치
	FVector CurrentHitLocation;

	// 레이캐스트 및 프리뷰 업데이트
	void UpdateRaycast();

	// 실제 Boid 스폰
	void SpawnBoidAtLocation(const FVector& Location);
};
