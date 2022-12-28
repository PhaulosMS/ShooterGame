// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AShooterCharacter::AShooterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bAiming = false;
	// Dont rotate when controller rotates, only rotates camera when controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 200.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 70.0f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraCurrentFOV = 0.0f;
	CameraDefaultFOV = 0.0;
	CameraZoomedFOV = 40.0f;
	ZoomInterpSpeed = 30.0f;

	HipTurnRate = 1.0f;
	HipLookUpRate = 1.0f;
	AimingTurnRate = 0.2f;
	AimingLookUpRate = 0.2f;

	CrosshairAimFactor = 0.0f;
	CrosshairSpreadMultiplier = 0.0f;
	CrosshairVelocityFactor = 0.0f;
	CrosshairInAirFactor = 0.0f;
	CrosshairShootingFactor = 0.0f;

	ShootTimeDuration = 0.05f;
	bFiringBullet = false;

	AutomaticFireRate = 0.1f;
	bShouldFire = true;
	bFireButtonPressed = false;
}


// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Make sure that we have a valid PlayerController.
	if (const APlayerController* const PC = Cast<APlayerController>(GetController()))
	{
		// Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// Add each mapping context, along with their priority values. Higher values outprioritize lower values.
			Subsystem->AddMappingContext(BaseMappingContext, BaseMappingPriority);
		}
	}

	if (FollowCamera)
	{
		CameraDefaultFOV = GetCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
}



// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CameraZoomInterp(DeltaTime);
	CalculateCrosshairSpread(DeltaTime);
}


// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		PlayerEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move);
		PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look);
		PlayerEnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::FireButtonPressed);
		PlayerEnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::FireButtonReleased);
		PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ACharacter::StopJumping);
		PlayerEnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Triggered, this, &AShooterCharacter::AimingButtonPressed);
		PlayerEnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Completed, this, &AShooterCharacter::AimingButtonReleased);
	}
}



void AShooterCharacter::Move(const FInputActionValue& Value)
{
	if (Controller && (Value.GetMagnitude() != 0.0))
	{
		const FVector2D MovementVector = Value.Get<FVector2D>();
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation = FRotator(0.0f, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AShooterCharacter::Look(const FInputActionValue& Value)
{
	float TurnScaleFactor;
	float LookUpScaleFactor;
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (bAiming)
	{
		TurnScaleFactor = AimingTurnRate;
		LookUpScaleFactor = AimingLookUpRate;
	}
	else
	{
		TurnScaleFactor = HipTurnRate;
		LookUpScaleFactor = HipLookUpRate;
	}
	AddControllerPitchInput(LookAxisVector.Y * LookUpScaleFactor);
	AddControllerYawInput(LookAxisVector.X * TurnScaleFactor);
}

void AShooterCharacter::CameraZoomInterp(float DeltaTime)
{
	CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, bAiming ? CameraZoomedFOV : CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	GetCamera()->SetFieldOfView(CameraCurrentFOV);
}


void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	// Crosshair Velocity calculation
	const FVector2D WalkSpeedRange = FVector2D(0.0f, 600.0f);
	const FVector2D VelocityMultiplierRange = FVector2D(0.0f, 1.0f);
	FVector Velocity = GetVelocity();
	Velocity.Z = 0;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	// Crosshair in air calculation
	CrosshairInAirFactor = GetCharacterMovement()->IsFalling() ? FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f) : FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);
	// Crosshair  bAiming calculation
	CrosshairAimFactor = bAiming ?  FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.0f) : FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);
	// Crosshair Calucation if firing a bullet
	CrosshairShootingFactor = bFiringBullet ? FMath::FInterpTo(CrosshairShootingFactor, 0.3, DeltaTime, 60.0f) : FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);

	// Applying the crosshair factors
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
 {
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
 }
 
 void AShooterCharacter::FinishCrosshairBulletFire()
 {
	bFiringBullet = false;
 }

void AShooterCharacter::Fire()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	if (const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket"))
	{
		const FTransform BarrelSocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, BarrelSocketTransform);
		}

		FVector BeamEnd;

		if (GetBeamEndLocation(BarrelSocketTransform.GetLocation(), BeamEnd))
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}
			if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(), BeamParticles, BarrelSocketTransform))
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"), HipFireMontage);
	}

	StartCrosshairBulletFire();
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Get current size of the viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}


	// Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshairs
	if (UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation,
	                                             CrosshairWorldPosition, CrosshairWorldDirection))
	{
		// If deprojection was successful?
		FHitResult ScreenTraceHit;
		const FVector Start = CrosshairWorldPosition;
		const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * 50'000.0f;
		OutBeamLocation = End; // Set beam end point to line trace end point by default


		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECC_Visibility);
		// Trace outwards from crosshairs world location

		if (ScreenTraceHit.bBlockingHit) // was there a trace hit?
		{
			OutBeamLocation = ScreenTraceHit.Location; // Sets beam endpoint to the hit location endpoint instead
		}

		// Perform a second trace this time from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart = MuzzleSocketLocation;
		const FVector WeaponTraceEnd = OutBeamLocation;

		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECC_Visibility);
		if (WeaponTraceHit.bBlockingHit) // object between barrel and beamendpoint?
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false;
}

void AShooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		Fire();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
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

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartFireTimer();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}


