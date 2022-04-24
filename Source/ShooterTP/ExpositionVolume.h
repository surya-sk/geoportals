// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExpositionVolume.generated.h"

UCLASS()
class SHOOTERTP_API AExpositionVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExpositionVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exposition")
	class UBoxComponent* ExpositionVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
	class UBillboardComponent* Billboard;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
