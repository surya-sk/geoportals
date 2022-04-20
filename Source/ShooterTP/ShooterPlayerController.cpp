// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterTPSaveGame.h"

AShooterPlayerController::AShooterPlayerController()
{
	bPauseMenuVisible = false;
	CurrentLevelIndex = 0;
}

void AShooterPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputUIModeGameAndUI;
		SetInputMode(InputUIModeGameAndUI);
		UGameplayStatics::SetGamePaused(this, true);
	}
}

void AShooterPlayerController::HidePauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = false;
		UGameplayStatics::SetGamePaused(this, false);
		PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		FInputModeGameOnly InputUIModeGameOnly;
		SetInputMode(InputUIModeGameOnly);

	}
}

void AShooterPlayerController::TogglePauseMenu()
{
	if (!bPauseMenuVisible)
	{
		DisplayPauseMenu();
	}
	else
	{
		HidePauseMenu();
	}
}

void AShooterPlayerController::DisplayMainMenu()
{
	if (WMainMenu)
	{
		MainMenu = CreateWidget<UUserWidget>(this, WMainMenu);
		if (MainMenu)
		{
			MainMenu->AddToViewport();
			MainMenu->SetVisibility(ESlateVisibility::Visible);
			FInputModeUIOnly InputUIModeGameAndUI;
			SetInputMode(InputUIModeGameAndUI);
		}
	}
}

void AShooterPlayerController::SaveGame(float Heath, FVector Location, FRotator Rotation, bool bSetLocation)
{
	UShooterTPSaveGame* SaveGameInstance = Cast<UShooterTPSaveGame>(UGameplayStatics::CreateSaveGameObject(UShooterTPSaveGame::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Heath;
	SaveGameInstance->CharacterStats.Location = Location;
	SaveGameInstance->CharacterStats.Rotation = Rotation;
	SaveGameInstance->CharacterStats.LevelIndex = GetCurrentLevelIndex();
	SaveGameInstance->CharacterStats.bSetLocation = bSetLocation;

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AShooterPlayerController::LoadGame()
{
	UShooterTPSaveGame* LoadGameInstance = Cast<UShooterTPSaveGame>(UGameplayStatics::CreateSaveGameObject(UShooterTPSaveGame::StaticClass()));
	LoadGameInstance = Cast<UShooterTPSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	CurrentLevelIndex = LoadGameInstance->CharacterStats.LevelIndex;
	UE_LOG(LogTemp, Warning, TEXT("%d"), CurrentLevelIndex);
	SwitchLevel();
}


int32 AShooterPlayerController::GetCurrentLevelIndex()
{
	FString CurrentLevelName = GetWorld()->GetMapName();
	FName CurrentLevel = (*CurrentLevelName);

	int32 size = sizeof(Levels) / sizeof(Levels[0]);
	auto itr = std::find(Levels, Levels + size, CurrentLevelName);
	return std::distance(Levels, itr);
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FString CurrentLevelName = GetWorld()->GetMapName();
	if (CurrentLevelName == "UEDPIE_0_MainMenu")
	{
		DisplayMainMenu();
	}

	else
	{
		if (HUDOverlayClass)
		{
			HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
			if (HUDOverlay)
			{
				HUDOverlay->AddToViewport();
				HUDOverlay->SetVisibility(ESlateVisibility::Visible);
			}
		}

		if (WPauseMenu)
		{
			PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
			if (PauseMenu)
			{
				PauseMenu->AddToViewport();
				PauseMenu->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void AShooterPlayerController::SwitchLevel()
{
	FString LevelName = LevelNames[CurrentLevelIndex];
	UE_LOG(LogTemp, Warning, TEXT("%s"), *LevelName);
	FName Level = (*LevelName);
	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::OpenLevel(World, Level);
	}
}

