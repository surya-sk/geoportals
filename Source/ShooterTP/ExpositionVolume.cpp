// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpositionVolume.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterPlayerController.h"

// Sets default values
AExpositionVolume::AExpositionVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ExpositionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ExpositionVolume"));
	RootComponent = ExpositionVolume;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	RootComponent->SetupAttachment(Billboard);
}

// Called when the game starts or when spawned
void AExpositionVolume::BeginPlay()
{
	Super::BeginPlay();
	
	ExpositionVolume->OnComponentBeginOverlap.AddDynamic(this, &AExpositionVolume::OnOverlapBegin);
}

// Called every frame
void AExpositionVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExpositionVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto PlayerController = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PlayerController)
	{
		PlayerController->ShowExpositionText();
	}
}

