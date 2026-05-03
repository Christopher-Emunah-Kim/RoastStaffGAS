#include "Animation/AN_HitCheck.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Tags/RSGameplayTags.h"
#include "System/LoggingSystem.h"

UAN_HitCheck::UAN_HitCheck()
{
	EventTag = RSTags::Event_Montage_HitCheck;
}

void UAN_HitCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		KHS_WARN("AN_HitCheck — Owner가 없습니다.");
		return;
	}

	if (!EventTag.IsValid())
	{
		KHS_WARN("AN_HitCheck — EventTag가 유효하지 않습니다. Owner: %s", *Owner->GetName());
		return;
	}

	FGameplayEventData Payload;
	Payload.Instigator = Owner;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);
}

FString UAN_HitCheck::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("HitCheck [%s]"), *EventTag.ToString());
}
