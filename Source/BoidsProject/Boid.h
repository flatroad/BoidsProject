// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boid.generated.h"

UCLASS()
class BOIDSPROJECT_API ABoid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	//////////////////////////////////////////////////////////////////////////
	// 이동 상태
	//////////////////////////////////////////////////////////////////////////

	// 현재 속도 벡터
	// 속도(벡터량) = 진행 방향(heading) * 속력(speed).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boid|State")
	FVector Velocity;

	// 현재 프레임의 가속도 (매 프레임 리셋됨)
	// 고정된 가속도 총량이 우선순위대로 배분된다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boid|State")
	FVector Acceleration;


	//////////////////////////////////////////////////////////////////////////
	// 이동 파라미터
	//////////////////////////////////////////////////////////////////////////
	
	// 최대 속도
	// 최대 속도는 넘지 않는다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Movement")
	float MaxSpeed = 600.0f;

	// 최소 속도
	// 최소 속도보다 느려지면 안된다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Movement")
	float MinSpeed = 0.0f;

	// 최대 가속도
	// 과도한 가속도 요청을 막고 속도와 방향의 부드러운 변화를 하게 제한한다.
	// 유한한 에너지를 가진 생물체의 단순 모델.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Movement")
	float MaxAcceleration = 300.0f;

	// 속도 감쇠 계수
	// 공기저항. (단순화 시켰음 실제로 이런 수치 아님)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Movement")
	float Damping = 0.99f;
	
	//////////////////////////////////////////////////////////////////////////
	// 경계 파라미터
	//////////////////////////////////////////////////////////////////////////
	// 0,0,0 ~ 10000,10000,10000 범위 내에서만 이동.

	// 경계 최소 좌표 (월드 좌표 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Boundary")
	FVector BoundaryMin = FVector(0.0f, 0.0f, 0.0f);

	// 경계 최대 좌표 (월드 좌표 기준)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Boundary")
	FVector BoundaryMax = FVector(10000.0f, 10000.0f, 10000.0f);

	// 경계 되돌림이 시작되는 거리 (벽에서 이 거리 안에 들어오면 되돌림 시작)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Boundary")
	float BoundaryMargin = 1000.0f;

	// 경계 되돌림 힘의 강도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Boundary")
	float BoundaryTurnForce = 200.0f;

	//////////////////////////////////////////////////////////////////////////
	// 디버그용
	//////////////////////////////////////////////////////////////////////////

	// 디버그 구체 표시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Debug")
	bool bShowDebug = true;

	// 디버그 구체 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Debug")
	float DebugSphereRadius = 30.0f;

	// 경계 박스 디버그 표시 여부 (1개 테스트용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid|Debug")
	bool bShowBoundaryDebug = true;

	//////////////////////////////////////////////////////////////////////////
	// 초기화 함수
	//////////////////////////////////////////////////////////////////////////

	// 초기 속도 설정 (SpawnAndLockPawn에서 스폰 후 호출)
	UFUNCTION(BlueprintCallable, Category = "Boid")
	void InitializeBoid(FVector InitialVelocity);


private:
	//////////////////////////////////////////////////////////////////////////
	// 행동 함수. 
	//////////////////////////////////////////////////////////////////////////
	// 각 함수는 가속도 벡터를 반환한다.
	// 모든 행동은 Boid를 어느 방향으로 조향할지에 대한 독립적인 제안으로 생성.
	// 모든 제안은 가속도 요청으로 표현.


	// 경계 제한: 범위를 벗어나려 하면 안쪽으로 되돌림.
	FVector CalculateBoundaryForce() const;

	// 디버그 시각화 그리기
	void DrawDebugVisualization() const;
};
