// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ShooterCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();

private:
	/****************
	  Input Mappings
	*****************/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Controls | Input Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Controls | Input Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Controls | Input Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Controls | Input Actions", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Controls | Input Mapping", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* BaseMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Controls | Input Mapping", meta = (AllowPrivateAccess = "true"))
	int32 BaseMappingPriority = 0;

	/****************
	Camera Components
	*****************/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta=(AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

protected:
	/****************
	Virtual Functions
	*****************/
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;





	
public:
	FORCEINLINE UCameraComponent* GetCamera() const { return FollowCamera; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	

	

};


