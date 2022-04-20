// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERTP_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterPlayerController();

private:

	int32 GetCurrentLevelIndex();

protected:
	virtual void BeginPlay() override;

	void SwitchLevel();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> HUDOverlayClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	UUserWidget* HUDOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> WPauseMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	UUserWidget* PauseMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> WMainMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	UUserWidget* MainMenu;

	bool bPauseMenuVisible;

	int32 CurrentLevelIndex;

	FString Levels[6]{ "UEDPIE_0_Opening", "UEDPIE_0_Level1", "UEDPIE_0_Level2", "UEDPIE_0_Level3", "Level4", "Level5" };

	FString LevelNames[6]{ "Opening", "Level1", "Level2", "Level3", "Level4", "Level5" };

public:
	void TogglePauseMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DisplayPauseMenu();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HidePauseMenu();

	void DisplayMainMenu();

	UFUNCTION(BlueprintCallable)
	void SaveGame(float Heath, FVector Location, FRotator Rotation, bool bSetLocation);

	UFUNCTION(BlueprintCallable)
	void LoadGame();

	void LoadNextLevel();
};
