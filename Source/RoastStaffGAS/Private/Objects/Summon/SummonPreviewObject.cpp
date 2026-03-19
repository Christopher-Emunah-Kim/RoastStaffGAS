// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Summon/SummonPreviewObject.h"

#include "Character/Player/RSPlayerController.h"

// Sets default values
ASummonPreviewObject::ASummonPreviewObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}


// Called every frame
void ASummonPreviewObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ARSPlayerController* PC = Cast<ARSPlayerController>(GetWorld()->GetFirstPlayerController());               
	if (!PC)
	{                                                                                                            
		return;                                                                                                
	}

	SetActorLocation(PC->GetCachedAimLocation());   
}

