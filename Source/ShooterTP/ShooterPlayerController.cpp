// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterTPSaveGame.h"

AShooterPlayerController::AShooterPlayerController()
{
	bPauseMenuVisible = false;
	CurrentLevelIndex = -1;
}

void AShooterPlayerController::DisplayPauseMenu()
{
	PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
	if (PauseMenu)
	{
		HUDOverlay->SetVisibility(ESlateVisibility::Hidden);
		PauseMenu->AddToViewport();
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);
		SetInputMode(FInputModeUIOnly());
		
		//UGameplayStatics::SetGamePaused(this, true);
	}
}

void AShooterPlayerController::HidePauseMenu()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = false;
		HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		//UGameplayStatics::SetGamePaused(this, false);
		PauseMenu->RemoveFromViewport();
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
			FInputModeUIOnly InputUIModeUIOnly;
			SetInputMode(InputUIModeUIOnly);
		}
	}
}

void AShooterPlayerController::SaveGame(float Heath, FVector Location, FRotator Rotation, bool bSetLocation)
{
	UShooterTPSaveGame* SaveGameInstance = Cast<UShooterTPSaveGame>(UGameplayStatics::CreateSaveGameObject(UShooterTPSaveGame::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Heath;
	SaveGameInstance->CharacterStats.Location = Location;
	SaveGameInstance->CharacterStats.Rotation = Rotation;
	SaveGameInstance->CharacterStats.bSetLocation = bSetLocation;
	SaveGameInstance->CharacterStats.LevelIndex = GetCurrentLevelIndex();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AShooterPlayerController::LoadGame()
{
	UShooterTPSaveGame* LoadGameInstance = Cast<UShooterTPSaveGame>(UGameplayStatics::CreateSaveGameObject(UShooterTPSaveGame::StaticClass()));
	LoadGameInstance = Cast<UShooterTPSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	if (LoadGameInstance)
	{
		CurrentLevelIndex = LoadGameInstance->CharacterStats.LevelIndex;
	}
	else
	{
		CurrentLevelIndex = 0;
	}
	UE_LOG(LogTemp, Warning, TEXT("LOAD_GAME: Index is %d"), CurrentLevelIndex);
	SwitchLevel();
}

void AShooterPlayerController::LoadNextLevel()
{
	CurrentLevelIndex++;
	UShooterTPSaveGame* SaveGameInstance = Cast<UShooterTPSaveGame>(UGameplayStatics::CreateSaveGameObject(UShooterTPSaveGame::StaticClass()));
	SaveGameInstance->CharacterStats.LevelIndex = CurrentLevelIndex;
	SaveGameInstance->CharacterStats.bSetLocation = false;
	UE_LOG(LogTemp, Warning, TEXT("LOAD_NEXT_LEVEL: Index is %d"), CurrentLevelIndex);

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);

	SwitchLevel();
}

void AShooterPlayerController::CloseExpositionText()
{
	if (ExpositionText)
	{
		//HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		ExpositionText->RemoveFromViewport();
		FInputModeGameOnly InputUIModeGameOnly;
		SetInputMode(InputUIModeGameOnly);
	}
}

void AShooterPlayerController::ShowExpositionText()
{
	if (WExpositionText)
	{
		ExpositionText = CreateWidget<UUserWidget>(this, WExpositionText);
		if (ExpositionText)
		{
			//HUDOverlay->SetVisibility(ESlateVisibility::Hidden);
			ExpositionText->AddToViewport();
			ExpositionText->SetVisibility(ESlateVisibility::Visible);
			FInputModeUIOnly InputModeUIOnly;
			SetInputMode(InputModeUIOnly);
		}
	}
}

void AShooterPlayerController::ShowTutorialText()
{
	if (TutorialText)
	{
		TutorialText->SetVisibility(ESlateVisibility::Visible);
	}
}

void AShooterPlayerController::HideTutorialText()
{
	if (TutorialText)
	{
		TutorialText->SetVisibility(ESlateVisibility::Hidden);
	}
}


int32 AShooterPlayerController::GetCurrentLevelIndex()
{
	FString CurrentLevelName = GetWorld()->GetMapName();
	FName CurrentLevel = (*CurrentLevelName);

	int32 size = sizeof(LevelNames) / sizeof(LevelNames[0]);
	auto itr = std::find(LevelNames, LevelNames + size, CurrentLevelName);
	return std::distance(LevelNames, itr);
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FString CurrentLevelName = GetWorld()->GetMapName();
	if (CurrentLevelName == "UEDPIE_0_MainMenu" || CurrentLevelName == "MainMenu")
	{
		DisplayMainMenu();
	}

	else
	{
		CurrentLevelIndex = GetCurrentLevelIndex();
		SetInputMode(FInputModeGameOnly());
		if (HUDOverlayClass)
		{
			HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
			if (HUDOverlay)
			{
				HUDOverlay->AddToViewport();
				HUDOverlay->SetVisibility(ESlateVisibility::Visible);
			}
		}
		if (WTutorialText)
		{
			TutorialText = CreateWidget<UUserWidget>(this, WTutorialText);
			if (TutorialText)
			{
				TutorialText->AddToViewport();
				TutorialText->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("BEGIN_PLAY: Current level name is %s and index is %d"), *CurrentLevelName, CurrentLevelIndex);
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

