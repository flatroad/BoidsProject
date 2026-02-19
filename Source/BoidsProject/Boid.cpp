// Fill out your copyright notice in the Description page of Project Settings.


#include "Boid.h"
#include "DrawDebugHelpers.h"

// Sets default values
ABoid::ABoid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 초기 속도
	Velocity = FVector::ZeroVector;
	Acceleration = FVector::ZeroVector;

}

// Called when the game starts or when spawned
void ABoid::BeginPlay()
{
	Super::BeginPlay();
	
	// 초기 속도가 설정되지 않았다면, 랜덤 방향으로 시작.
	if (Velocity.IsNearlyZero())
	{
		// 랜덤한 방향의 단위벡터 * 최대 속도의 절반으로 시작.
		Velocity = FMath::VRand() * (MaxSpeed * 0.5f);
	}
}

// Called every frame
// 기하학적 비행은 오브젝트의 앞 방향을 따른 점진적 이동에 기반한다.
// 이 이동들은 조향(로컬 X축과 Y축에 대한 회전, pitch와 yaw)과 혼합되어, 로컬 Z축의 글로벌 방향을 재설정한다.
void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO: 행동 규칙에 따른 가속도 수집.(우선순위 순으로)
	// 우선순위 1: 충돌 회피.
	// 우선순위 2: 속도(방향) 매칭.
	// 우선순위 3: 무리 중심(너무 멀어지지 않게).
	// 우선순위 4: 경계 제한 *완료
	// 우선순위 5: 목표 이동

	// 가속도 요청은 우선순위 순서대로 고려되어 누적기에 추가된다.
	FVector BoundaryForce = CalculateBoundaryForce();
	Acceleration += BoundaryForce;

	// 가속도 크기 제한
	if (Acceleration.Size() > MaxAcceleration)
	{
		Acceleration = Acceleration.GetSafeNormal() * MaxAcceleration;
	}

	// Geometric Flight 기본 이동.

	// 가속도를 속도에 적용
	Velocity += Acceleration * DeltaTime;

	// 속도 감쇠 적용 (공기 저항)
	Velocity *= Damping;

	// 속도 제한
	float CurrentSpeed = Velocity.Size();

	if (CurrentSpeed > MaxSpeed)
	{
		// 최대 속도 초과 시: 방향 유지. 크기는 최대치로
		Velocity = Velocity.GetSafeNormal() * MaxSpeed;
	}
	else if (CurrentSpeed < MinSpeed && CurrentSpeed > KINDA_SMALL_NUMBER)
	{
		// 최소 속도 미달 시: 방향 유지, 크기 최소로
		Velocity = Velocity.GetSafeNormal() * MinSpeed;
	}

	// 위치 업데이트
	FVector NewLocation = GetActorLocation() + (Velocity * DeltaTime);
	SetActorLocation(NewLocation);

	// 회전 업데이트: 속도 방향으로 Actor를 회전
	// Boid의 앞면이 항상 진행 방향으로 향하게 한다.
	if (!Velocity.IsNearlyZero())
	{
		FRotator NewRotation = Velocity.Rotation();
		SetActorRotation(NewRotation);
	}

	// 6. 가속도 리셋 (다음 프레임에서 행동들이 새로 계산됨)
	// 매 프레임 0으로 리셋 후, 다음 차례에 다시 새로 요청한다.
	Acceleration = FVector::ZeroVector;

	// 디버그 시각화.
	DrawDebugVisualization();
}

// 초기 속도 설정
// SpawnAndLockPawn에서 스폰 직후 호출하여 원하는 초기 속도를 부여할 수 있음.
void ABoid::InitializeBoid(FVector InitialVelocity)
{
	Velocity = InitialVelocity;

	// 초기 속도가 MaxSpeed를 넘지 않도록
	if (Velocity.Size() > MaxSpeed)
	{
		Velocity = Velocity.GetSafeNormal() * MaxSpeed;
	}

	UE_LOG(LogTemp, Log, TEXT("Boid 초기화: Velocity = %s"), *Velocity.ToString());
}

// 경계 제한
// 경계 벽에서 BoundaryMargin 거리 안에 들어오면, 안쪽으로 되돌리는 가속도를 생성한다.
// 벽에 가까울수록 되돌림 힘이 강해진다. (0 ~ 1 비율)
FVector ABoid::CalculateBoundaryForce() const
{
	FVector Force = FVector::ZeroVector;
	FVector Location = GetActorLocation();

	// 각 축 (X, Y, Z) 에 대해 독립적으로 경계 체크
	// 배열을 사용하여 X(0), Y(0), Z(2) 반복 처리.

	for (int Axis = 0; Axis < 3; ++Axis)
	{
		// 최소 경계에 가까운 경우
		float DistToMin = Location[Axis] - BoundaryMin[Axis];
		if (DistToMin < BoundaryMargin)
		{
			// 벽에 붙을수록 1, Margin 경계에서 0 에 가깝게.
			// FMath::Clamp로 0 ~ 1 범위 보장.
			float Ratio = FMath::Clamp(1.0f - (DistToMin / BoundaryMargin), 0.0f, 1.0f);

			// 플러스 방향으로 밀어냄 (Min 벽에서 멀어지는 방향)
			Force[Axis] += BoundaryTurnForce * Ratio;
		}
			// 최대 경계에 가까운 경우
		float DistToMax = BoundaryMax[Axis] - Location[Axis];
		if (DistToMax < BoundaryMargin)
		{
			// 벽에 붙을수록 1에 가깝게
			float Ratio = FMath::Clamp(1.0f - (DistToMax / BoundaryMargin), 0.0f, 1.0f);

			// 마이너스 방향으로 밀어냄 (Max 벽에서 멀어지는 방향)
			Force[Axis] -= BoundaryTurnForce * Ratio;
		}
	}
	return (Force);
}

// 디버그 시각화
void ABoid::DrawDebugVisualization() const
{
	if (!bShowDebug)
	{
		return;
	}

	FVector Location = GetActorLocation();
	float CurrentSpeed = Velocity.Size();

	// Boid 위치: 빨간 구
	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		DebugSphereRadius,
		8,
		FColor::Red,
		false,
		0.0f
	);

	// 2. 속도 방향: 파란 화살표
	// 속도 방향으로 파란 선으로 표시 (속력에 따라 길이 변화)
	float ArrowLength = FMath::Clamp(CurrentSpeed * 0.3f, 30.0f, 300.0f);
	DrawDebugDirectionalArrow(
		GetWorld(),
		Location,
		Location + (Velocity.GetSafeNormal() * ArrowLength),
		20.0f,
		FColor::Blue,
		false,
		0.0f
	);

	// 3. 속도 텍스트: BOid 위에 현재 속력 흰색 텍스트 표시
	FString SpeedText = FString::Printf(TEXT("%.0f cm/s"), CurrentSpeed);
	DrawDebugString(
		GetWorld(),
		Location + FVector(0.0f, 0.0f, 50.0f),
		SpeedText,
		nullptr,
		FColor::White,
		0.0f,
		true
	);

	// 4. 경계박스: 노란 와이어프레임. 1개 테스트용
	if (bShowBoundaryDebug)
	{
		// 경계의 중심점.
		FVector BoundaryCenter = (BoundaryMin + BoundaryMax) * 0.5;
		// 중심에서 각 면까지의 거리 (반)
		FVector BoundaryExtent = (BoundaryMax - BoundaryMin) * 0.5;

		DrawDebugBox(
			GetWorld(),
			BoundaryCenter,
			BoundaryExtent,
			FColor::Yellow,
			false,
			0.0f
		);

		// 내부 안전 영역 박스: 초록색
		FVector safeMin = BoundaryMin + FVector(BoundaryMargin);
		FVector safeMax = BoundaryMax + FVector(BoundaryMargin);
		FVector SafeConter = (safeMin + safeMax) * 0.5f;
		FVector SafeExtent = (safeMax - safeMin) * 0.5f;

		DrawDebugBox(
			GetWorld(),
			SafeConter,
			SafeExtent,
			FColor::Green,
			false,
			0.0f
		);
	}
}
