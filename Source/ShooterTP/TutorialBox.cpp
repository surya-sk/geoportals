// Fill out your copyright notice in the Description page of Project Settings.


#include "TutorialBox.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATutorialBox::ATutorialBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TutorialBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TutorialBox"));
	RootComponent = TutorialBox;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	RootComponent->SetupAttachment(Billboard);
}

// Called when the game starts or when spawned
void ATutorialBox::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerController = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	TutorialBox->OnComponentBeginOverlap.AddDynamic(this, &ATutorialBox::OnOverlapBegin);
	TutorialBox->OnComponentEndOverlap.AddDynamic(this, &ATutorialBox::OnOverlapEnd);
}

// Called every frame
void ATutorialBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATutorialBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (PlayerController)
	{
		PlayerController->ShowTutorialText();
	}
}

void ATutorialBox::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(PlayerController)
	{
		PlayerController->HideTutorialText();
	}
}

