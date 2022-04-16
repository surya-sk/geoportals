// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShooterTPSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FCharacterStats
{
	GENERATED_BODY();

	UPROPERTY(VisibleAnywhere, Category="SaveData")
	float Health;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FName LevelName;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FVector Location;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FRotator Rotation;

};

/**
 * 
 */
UCLASS()
class SHOOTERTP_API UShooterTPSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UShooterTPSaveGame();

	UPROPERTY(VisibleAnywhere, Category=Basic)
	FString PlayerName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;	

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FCharacterStats CharacterStats;
};
