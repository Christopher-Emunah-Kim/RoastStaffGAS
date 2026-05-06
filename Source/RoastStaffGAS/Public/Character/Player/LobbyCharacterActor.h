// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LobbyCharacterActor.generated.h"

class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyCharacterClicked, FName, CharID);

/**
 * ALobbyCharacterActor
 *
 * 로비 레벨에 배치되는 선택 가능한 캐릭터 Actor.
 * 호버 시 아웃라인(CustomDepth), 클릭 시 기립 몽타주 + 선택 델리게이트 브로드캐스트.
 * 클릭 감지: PC에서 bEnableMouseOverEvents + bEnableClickEvents 활성화 필요.
 */
UCLASS()
class ROASTSTAFFGAS_API ALobbyCharacterActor : public ACharacter
{
	GENERATED_BODY()

public:
	ALobbyCharacterActor();

	/** 캐릭터 클릭 시 브로드캐스트 — ARSOutGamePlayerController 구독 */
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyCharacterClicked OnCharacterClickedDel;

	FORCEINLINE FName   GetCharacterID()      const { return CharacterID; }
	FORCEINLINE AActor* GetCharacterCamera()  const { return CharacterCameraRef; }
	FORCEINLINE float   GetCameraBlendTime()  const { return CameraBlendTime; }

	/** PC::PlayerTick에서 수동 hover 제어 */
	void SetOutlineActive(bool bActive);

protected:
	virtual void NotifyActorOnClicked(FKey ButtonPressed) override;

private:

	/** DT_Character FK — RDS::SetSelectedCharacter / ULobbyCharInfoPanel 조회에 사용 */
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	FName CharacterID;
	/** 클릭 시 재생할 기립 몽타주 */
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	TObjectPtr<UAnimMontage> StandUpMontage;
	/** 이 캐릭터로 카메라 블렌드할 때 사용할 레벨 배치 CameraActor 참조 — 인스턴스별 할당 */
	UPROPERTY(EditInstanceOnly, Category = "Lobby")
	TObjectPtr<AActor> CharacterCameraRef;
	/** SetViewTargetWithBlend 블렌드 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	float CameraBlendTime = 0.8f;
};
