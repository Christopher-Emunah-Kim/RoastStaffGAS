// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/K_BaseCharacter.h"

// Sets default values
AK_BaseCharacter::AK_BaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AK_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AK_BaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AK_BaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

