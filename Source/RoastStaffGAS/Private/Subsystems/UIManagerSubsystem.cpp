// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/UIManagerSubsystem.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUIManagerSubsystem::Deinitialize()
{
	CloseAllPopupUI();
	
	//모든 캐시된 위젯 정리
	//TODO 함수형으로 전환
	for (auto& Pair : CachedWidgets)
	{
		if (Pair.Value && Pair.Value->IsInViewport())
		{
			Pair.Value->RemoveFromParent();
		}
	}
	
	CachedWidgets.Empty();
	PersistentUIMap.Empty();
	PopupUIStack.Empty();
	
	Super::Deinitialize();
}

int32 UUIManagerSubsystem::CalculateZOrder(URSBaseWidget* Widget) const
{
	if (!Widget)
	{
		return 0;
	}
    
	// Persistent 타입의 경우 ZOrder 낮게
	if (Widget->UILayer == EUILayer::PERSISTENT)
	{
		return Widget->ZOrder;
	}
	// Popup 타입의 경우 ZOrder 높게 (100 + 스택 깊이)
	else
	{
		return 100 + PopupUIStack.Num();
	}
}

void UUIManagerSubsystem::NotifyInputModeChange()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!ensureMsgf(PC, TEXT("World First PlayerController is NULL")))
	{
		return;
	}
	
	bool bHasModalPopup = PopupUIStack.ContainsByPredicate([](const URSBaseWidget* Widget)
	{
		return Widget && Widget->bIsModal;
	});
    
	if (bHasModalPopup) // Popup이 있을 때는 UI Only 
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(PopupUIStack.Last()->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
	else // Popup이 없을 때는 Game Only 
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}

void UUIManagerSubsystem::CloseUIInternal(URSBaseWidget* Widget)
{
	if (!Widget || !Widget->IsOpen())
	{
		return;
	}
    
	if (Widget->UILayer == EUILayer::PERSISTENT) //Persistent
	{
		Widget->CloseUI();
		Widget->RemoveFromParent();
        
		TSubclassOf<URSBaseWidget> WidgetClass = Widget->GetClass();
		PersistentUIMap.Remove(WidgetClass);
	}
	else // Popup
	{
		int32 Index = PopupUIStack.Find(Widget);
		if (Index != INDEX_NONE)
		{
			PopupUIStack.RemoveAt(Index);
			Widget->CloseUI();
			Widget->RemoveFromParent();
            
			RefreshTopPopupUI();
			NotifyInputModeChange();
		}
	}
}

void UUIManagerSubsystem::CloseUI(URSBaseWidget* Widget)
{
	CloseUIInternal(Widget);
}

void UUIManagerSubsystem::CloseTopPopupUI()
{
	if (PopupUIStack.Num() == 0)
	{
		return;
	}
    
	URSBaseWidget* TopWidget = PopupUIStack.Pop();
	TopWidget->CloseUI();
	TopWidget->RemoveFromParent();
    
	RefreshTopPopupUI();
	NotifyInputModeChange();
}

void UUIManagerSubsystem::CloseAllPopupUI()
{
	while (PopupUIStack.Num() > 0)
	{
		CloseTopPopupUI();
	}
}

void UUIManagerSubsystem::RefreshTopPopupUI()
{
	// 스택에 팝업이 남아있다면 새로운 Top에게 포커스 전달
	if (PopupUIStack.Num() > 0)
	{
		PopupUIStack.Last()->OnFocusGained();
	}
}

void UUIManagerSubsystem::ResetAllUIStates()
{
	//캐싱된 인스턴스들 상태 초기화
	for (auto& pair : CachedWidgets)
	{
		if (pair.Value)
		{
			if (pair.Value->IsOpen())
			{
				pair.Value->CloseUI();
			}
			
			if (pair.Value->IsInViewport())
			{
				pair.Value->RemoveFromParent();
			}
		}
	}
	
	//관련 컨테이너 비우기
	PersistentUIMap.Empty();
	PopupUIStack.Empty();
	CachedWidgets.Empty ();
	
	//입력모드 초기화
	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc)
	{
		pc->SetInputMode(FInputModeGameOnly());
		pc->SetShowMouseCursor(false);
	}
}
