// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "K_PlayerController.generated.h"

class UInputMappingContext;
class AK_PlayerCharacter;

/**
 * 
 */
UCLASS()
class ROASTSTAFFGAS_API AK_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);
	
	UPROPERTY(EditAnywhere, Category ="AM|Input")
	TObjectPtr<UInputMappingContext> IMC;
	
	UPROPERTY(EditAnywhere, Category="AM|Respawn")
	TSubclassOf<AK_PlayerCharacter> CharacterClass;
	
	
};
