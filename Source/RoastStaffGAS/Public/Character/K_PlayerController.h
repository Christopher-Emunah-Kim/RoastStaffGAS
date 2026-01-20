// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "K_PlayerController.generated.h"

class UInputMappingContext;
class AK_PlayerCharacter;
class UNet_HUD;
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
	virtual void AcknowledgePossession(class APawn* P) override;
	
private:
	void InitializePersistentUI();
	
	UFUNCTION()
	void HandleUICloseRequest(class UK_BaseWidget* RequestingWidget);
	
protected:
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);
	
public:
	UNet_HUD* GetNetHUD() const {return TempHUDUI;}
	
protected:
	UPROPERTY(EditAnywhere, Category ="AM|Input")
	TObjectPtr<UInputMappingContext> IMC;
	
	UPROPERTY(EditAnywhere, Category="AM|Respawn")
	TSubclassOf<AK_PlayerCharacter> CharacterClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "AM|UI")
	TSubclassOf<class UK_HUDWidget> HUDWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "AM|Temp")
	TSubclassOf<UNet_HUD> TempHUDClass;
	
	UPROPERTY()
	TObjectPtr< UNet_HUD> TempHUDUI;
	
};
