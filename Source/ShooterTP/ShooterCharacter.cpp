// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ShooterTP.h"
#include "BulletHitInterface.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ShooterPlayerController.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	bAiming = false;

	CameraDefaultFOV = 0.f;
	CameraZoomedFOV = 60.f;
	CameraCurrentFOV = 0.f;
	ZoomInterpSpeed = 20.f;

	NormalTurnRate = 90.f;
	NormalLookUpRate = 90.f;
	AimingTurnRate = 20.f;
	AimingLookUpRate = 20.f;
	MouseNormalTurnRate = 1.f;
	MouseNormalLookUpRate = 1.f;
	MouseAimingTurnRate = 0.2f;
	MouseAimingLookUpRate = 0.2f;

	AutomaticFireFrequency = 0.3f;
	bShouldFire = true;
	bFireButtonPressed = false;

	NineMMDefaultAmount = 85;
	ARDefaultAmount = 120;

	bCrouching = false;

	BaseMovementSpeed = 650.f;
	CrouchMovementSpeed = 300.f;

	StandingCapsuleHalfHeight = 88.f;
	CrouchingCapsuleHalfHeight = 44.f;

	Health = 100.f;
	MaxHealth = 100.f;

	CombatState = ECombatState::ECS_Unoccupied;
	RunSpeed = 1000.f;

	bPauseButtonPressed = false;

	Under60RegenRate = 0.02f;
	Over60RegenRate = 0.01f;

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pull in towards the character if there is collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; // camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // rotate arm based on controller
	CameraBoom->SocketOffset = FVector(0.f, -50.f, 40.f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false; // camera does not rotate relative to arm, the boom does

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	Damage = 20.f;
	HeadshotDamage = 50.f;
}

float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die();
		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("CharacterDead"), true);
		}
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (FollowCamera)
	{
		CameraDefaultFOV = FollowCamera->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	InitAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value !=0.0f))
	{
		// find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction { FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); 
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor;
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseNormalTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor;
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseNormalLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	const USkeletalMeshSocket* PistolSocket = GetMesh()->GetSocketByName("PistolSocket");
	if (PistolSocket != nullptr)
	{
		const FTransform SocketTransform = PistolSocket->GetSocketTransform(GetMesh());

		if (PistolFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PistolFlash, SocketTransform);
		}

		FHitResult BeamHitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);
		if (bBeamEnd)
		{
			// Does hit actor implement BulletHitInterface??
			if (BeamHitResult.Actor.IsValid())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.Actor.Get());
				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult);
				}
				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.Actor.Get());
				if (HitEnemy)
				{
					float DamageToInflict;
					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						DamageToInflict = HeadshotDamage;
					}
					else
					{
						DamageToInflict = Damage;
					}
					UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(), DamageToInflict, GetController(), this, UDamageType::StaticClass());
				}
			}
			// Play default particles
			else
			{
				if (ImpactParticles)
				{
					// Spawn impact particles after updating beam end point
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
				}
				if (BeamParticles)
				{
					UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
					if (Beam)
					{
						Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
					}
				}
			}
		}
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;
	// Spawn trace point and particles from the crosshair
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshairs
	FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); 
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPos;
	FVector CrosshairWorldDir;

	// Get World position and location of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPos, CrosshairWorldDir);
	if (bScreenToWorld) // Deprojection successful
	{
		FHitResult ScreenTraceHit;
		const FVector Start = FVector(CrosshairWorldPos);
		const FVector End = FVector(CrosshairWorldPos + CrosshairWorldDir * 50'000.f);

		// First trace hit, check for objects between crosshair position on the screen to the end point
		OutBeamLocation = End;
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (ScreenTraceHit.bBlockingHit)
		{
			OutBeamLocation = ScreenTraceHit.Location;
		}

		// Second trace hit, check for objects between the gun barell and the beam end point from the first trace
		const FVector WeaponTraceStart = FVector(MuzzleSocketLocation);
		const FVector WeaponTraceEnd = OutBeamLocation;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
		if (!OutHitResult.bBlockingHit) // Object between barell and beam end point
		{
			OutHitResult.Location = OutBeamLocation;
			return false;
		}
		return true;
	}
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	if (CombatState != ECombatState::ECS_Stunned)
	{
		bAiming = true;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetCameraFOV(DeltaTime);

	SetLookRates();
	
	InterpCapsuleHalfHeight(DeltaTime);

	RegenerateHealth();
}

/// <summary>
/// Change look sensitvity based on aiming state
/// </summary>
void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = NormalTurnRate;
		BaseLookUpRate = NormalTurnRate;
	}
}

void AShooterCharacter::FireButtonPressed()
{
	if (!bAiming)
	{
		bShootingWithoutAiming = true;
		bAiming = true;
	}
	bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
	if (bShootingWithoutAiming)
	{
		bAiming = false;
		bShootingWithoutAiming = false;
	}
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireFrequency);
	}
}

void AShooterCharacter::AutoFireReset()
{
	bShouldFire = true;
	if (bFireButtonPressed)
	{
		StartFireTimer();
	}
}

void AShooterCharacter::InitAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, NineMMDefaultAmount);
	AmmoMap.Add(EAmmoType::EAT_AR, ARDefaultAmount);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight;
	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}
	const float InterpHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f);

	const float DeltaCapsuleHalfHeight = InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight(); // Negative when crouching
	const FVector MeshOffset = FVector(0.f, 0.f, -DeltaCapsuleHalfHeight);
	GetMesh()->AddLocalOffset(MeshOffset);
		;
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::PauseButtonPressed()
{
	bPauseButtonPressed = true;
	auto PlayerController = GetWorld()->GetFirstPlayerController<AShooterPlayerController>();
	if (PlayerController)
	{
		PlayerController->TogglePauseMenu();
	}
}

void AShooterCharacter::PauseButtonReleased()
{
	bPauseButtonPressed = false;
}

void AShooterCharacter::SetCameraFOV(float DeltaTime)
{
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	FollowCamera->SetFieldOfView(CameraCurrentFOV);
}

EPhysicalSurface AShooterCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start{ GetActorLocation() };
	const FVector End = Start + FVector(0.f, 0.f, -400.f);
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);
	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

void AShooterCharacter::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		DisableInput(PC);
	}
}

void AShooterCharacter::SprintButtonPressed()
{
	if (!bAiming)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
}

void AShooterCharacter::SprintButtonReleased()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AShooterCharacter::RegenerateHealth()
{
	if (Health < MaxHealth)
	{
		if (Health < 100.f)
		{
			Health += Under60RegenRate;
		}
		else
		{
			Health += Over60RegenRate;
		}
	}
}



// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimButton", EInputEvent::IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AShooterCharacter::SprintButtonPressed);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AShooterCharacter::SprintButtonReleased);
	PlayerInputComponent->BindAction("Pause", EInputEvent::IE_Pressed, this, &AShooterCharacter::PauseButtonPressed);
	PlayerInputComponent->BindAction("Pause", EInputEvent::IE_Pressed, this, &AShooterCharacter::PauseButtonReleased);
}

void AShooterCharacter::Stun()
{
	if (Health <= 0.f) return;
	CombatState = ECombatState::ECS_Stunned;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

void AShooterCharacter::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(*CurrentLevel);
		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

