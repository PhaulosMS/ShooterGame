// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseItem.generated.h"

class UBoxComponent;
class UWidgetComponent;

UCLASS()
class SHOOTER_API ABaseItem : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseItem();
protected:
	/****************
	Virtual Functions
	*****************/
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* CollisionBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* PickupWidget;
	
public:
	

};
