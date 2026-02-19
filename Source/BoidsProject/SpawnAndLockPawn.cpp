// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnAndLockPawn.h"
#include "Boid.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASpawnAndLockPawn::ASpawnAndLockPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Root Component 설정
	USceneComponent* RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	// Camera 설정
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->bUsePawnControlRotation = true;

	// Movement Component
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->MaxSpeed = MovementSpeed;

	// Preview Cube 설정
	PreviewCubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewCube"));
	PreviewCubeMesh->SetupAttachment(RootComponent);
	PreviewCubeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewCubeMesh->SetVisibility(false);

	// 큐브 메시 로드 (기본 큐브 사용)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		PreviewCubeMesh->SetStaticMesh(CubeMeshAsset.Object);
		PreviewCubeMesh->SetWorldScale3D(FVector(0.5f));
	}
}

// Called when the game starts or when spawned
void ASpawnAndLockPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// Enhanced Input 설정
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}

		// 마우스 커서 숨기기
		PlayerController->bShowMouseCursor = false;
		PlayerController->bEnableClickEvents = false;
		PlayerController->bEnableMouseOverEvents = false;

		// 마우스를 뷰포트에 고정
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
	}	

	// Preview Cude 머티리얼 설정
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (DynMaterial)
		{
			DynMaterial->SetVectorParameterValue(FName("Color"), FLinearColor(0.0f, 0.5f, 1.0f, 0.3f));
			PreviewCubeMesh->SetMaterial(0, DynMaterial);
		}
	}
}

// Called every frame
void ASpawnAndLockPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 레이캐스트 중이면 매 프레임 업데이트
	if (bIsRaycasting)
	{
		UpdateRaycast();
	}

}

// Called to bind functionality to input
void ASpawnAndLockPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Move
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpawnAndLockPawn::Move);

		// Look
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpawnAndLockPawn::Look);

		// Vertical Move
		EnhancedInputComponent->BindAction(MoveVerticalAction, ETriggerEvent::Triggered, this, &ASpawnAndLockPawn::MoveVertical);

		// Spawn Boid (누름 & 해제)
		EnhancedInputComponent->BindAction(SpawnBoidAction, ETriggerEvent::Started, this, &ASpawnAndLockPawn::OnSpawnBoidPressed);
		EnhancedInputComponent->BindAction(SpawnBoidAction, ETriggerEvent::Completed, this, &ASpawnAndLockPawn::OnSpawnBoidReleased);
	}

}

void ASpawnAndLockPawn::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Forward/Backward
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);

		// Right/Left
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ASpawnAndLockPawn::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Yaw (좌우 회전)
		AddControllerYawInput(LookAxisVector.X * LookSensitivity);

		// Pitch (상하 회전)
		AddControllerPitchInput(-1 * LookAxisVector.Y * LookSensitivity);
	}
}

void ASpawnAndLockPawn::MoveVertical(const FInputActionValue& Value)
{
	const float VerticalValue = Value.Get<float>();

	if (Controller != nullptr)
	{
		// Up/Down (World Z축 기준)
		AddMovementInput(FVector::UpVector, VerticalValue);
	}
}

void ASpawnAndLockPawn::OnSpawnBoidPressed()
{
	bIsRaycasting = true;
	PreviewCubeMesh->SetVisibility(true);
}


void ASpawnAndLockPawn::OnSpawnBoidReleased()
{
	bIsRaycasting = false;
	PreviewCubeMesh->SetVisibility(false);

	// 실제 Boid 스폰
	if (!CurrentHitLocation.IsZero())
	{
		SpawnBoidAtLocation(CurrentHitLocation);
	}

	// 위치 초기화.
	CurrentHitLocation = FVector::ZeroVector;
}

void ASpawnAndLockPawn::UpdateRaycast()
{
	if (!CameraComponent)
	{
		return;
	}

	FVector Start = CameraComponent->GetComponentLocation();
	FVector ForwardVector = CameraComponent->GetForwardVector();
	FVector End = Start + (ForwardVector * RaycastDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// 레이캐스트 실행
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	FVector SpawnLocation;

	if (bHit)
	{
		// 히트한 경우 : 표면에서 약간 떨어진 곳에 배치
		// 노멀 벡터 방향으로 오프셋 적용
		SpawnLocation = HitResult.Location + (HitResult.Normal * SpawnOffsetDistance);

		// 디버그 그리기
		DrawDebugSphere(GetWorld(), HitResult.Location, 15.0f, 12, FColor::Red, false, 0.0f);
	}
	else
	{
		// 히트 안 한 경우: 카메라 앞 일정 거리에 배치
		SpawnLocation = Start + (ForwardVector * DefaultSpawnDistance);
	}

	// 현재 히트 위치 저장.
	CurrentHitLocation = SpawnLocation;

	// 프리뷰 큐브 위치 업데이트.
	PreviewCubeMesh->SetWorldLocation(SpawnLocation);

	// 프리뷰 큐브가 보이도록 유지.
	if (!PreviewCubeMesh->IsVisible())
	{
		PreviewCubeMesh->SetVisibility(true);
	}

	DrawDebugSphere(GetWorld(), HitResult.Location, 50.0f, 12, FColor::Cyan, false, 0.0f);
}

void ASpawnAndLockPawn::SpawnBoidAtLocation(const FVector& Location)
{
	if (!BoidClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BoidClass가 설정되지 않았습니다."));
		return;
	}

	// Boid 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(BoidClass, Location, FRotator::ZeroRotator, SpawnParams);

	if (SpawnedActor)
	{
		// Actor를 ABoid로 캐스팅하여 초기 속도 설정.
		// Cast<ABoid>: 안전한 타입 변환 체크, 실패하면 nullptr 반환.
		// 카메라가 바라보는 방향으로 초기 속도를 부여한다.
		ABoid* SpawnedBoid = Cast<ABoid>(SpawnedActor);
		if (SpawnedBoid)
		{
			// 카메라의 Foward 방햐야 * 초기 속도
			FVector InitalVelocity = CameraComponent->GetForwardVector() * BoidInitialSpeed;
			SpawnedBoid->InitializeBoid(InitalVelocity);
		}
		UE_LOG(LogTemp, Log, TEXT("Boid 스폰 성공: %s"), *Location.ToString());
	}
}
