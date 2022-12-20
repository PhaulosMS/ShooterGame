// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/ShooterAnimInstance.h"
#include "Character/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties()
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter)
	{
		// Setting Variables
		Speed = ShooterCharacter->GetVelocity().Length();
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
		ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Length() > 0.0f ? bIsAccelerating = true : bIsAccelerating = false;
		bAiming = ShooterCharacter->GetAiming();

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if (ShooterCharacter->GetVelocity().Size() > 0.0f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White,FString::Printf(TEXT("BaseAimRotation: %f"), AimRotation.Yaw));
			GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::White,FString::Printf(TEXT("MovementRotation: %f"), MovementRotation.Yaw));
			GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::White,FString::Printf(TEXT("Movementment Offset: %f"), MovementOffsetYaw));
		}
	}
}


void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	UpdateAnimationProperties();
}
