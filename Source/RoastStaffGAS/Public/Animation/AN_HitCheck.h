#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_HitCheck.generated.h"

/**
 * UAN_HitCheck
 * 몽타주 HitCheck 타이밍에 ASC로 GameplayEvent를 전송하는 공용 AnimNotify.
 * 모든 캐릭터 스킬 몽타주에서 재사용 가능.
 */
UCLASS()
class ROASTSTAFFGAS_API UAN_HitCheck : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_HitCheck();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

private:
	// 몽타주별로 다른 태그 지정 가능 — 기본값: Event.Montage.HitCheck
	UPROPERTY(EditAnywhere, Category = "HitCheck")
	FGameplayTag EventTag;
};
