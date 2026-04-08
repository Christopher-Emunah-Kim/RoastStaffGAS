// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/UIManagerSubsystem.h"

#include "System/LoggingSystem.h"
#include "System/UIManagerSettings.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUIManagerSubsystem::Deinitialize()
{
	CloseAllPopupUI();

	for (auto& Pair : CachedWidgets)
	{
		if (Pair.Value && Pair.Value->IsInViewport())
		{
			Pair.Value->RemoveFromParent();
		}
	}
	for (auto& Pair : CachedWidgetsByID)
	{
		if (Pair.Value && Pair.Value->IsInViewport())
		{
			Pair.Value->RemoveFromParent();
		}
	}

	CachedWidgets.Empty();
	CachedWidgetsByID.Empty();
	PersistentUIMap.Empty();
	PopupUIStack.Empty();
	PageUIStack.Empty();
	SystemUIStack.Empty();
	UIHistory.Empty();

	Super::Deinitialize();
}

// -----------------------------------------------------------------------------
// ID 기반 API
// -----------------------------------------------------------------------------


URSBaseWidget* UUIManagerSubsystem::OpenUIByID(EUIID ID)
{
	EUILayer Layer;
	if (!CheckUILayerSetting(ID, Layer))
	{
		return nullptr;
	}

	URSBaseWidget* Widget = GetOrCreateWidgetByID(ID);
	if (!ensure(Widget))
	{
		return nullptr;
	}

	if (Widget->IsOpen()) //이미 처리되서 열려있다면 그대로 반환
	{
		return Widget;
	}

	HandleWidgetByLayer(Layer, Widget);

	return Widget;
}

void UUIManagerSubsystem::CloseUIByID(EUIID ID)
{
	URSBaseWidget** Found = CachedWidgetsByID.Find(static_cast<uint8>(ID));
	
	if (!Found || !(*Found))
	{
		return;
	}
	
	CloseUIInternal(*Found);
}

URSBaseWidget* UUIManagerSubsystem::SwitchPageUI(EUIID ID)
{
	// 히스토리를 기록할 데이터가 없는 경우 즉시 다음 단계로 진행
	const UUIManagerSettings* Settings = UUIManagerSettings::Get();

	if (PageUIStack.Num() == 0 || !Settings)
	{
		return OpenUIByID(ID);
	}

	// 현재 열려있는 PAGE의 EUIID를 방향 조회하여 히스토리 저장
	URSBaseWidget* CurrentPage = PageUIStack.Last();
	for (const auto& Pair : CachedWidgetsByID)
	{
		if (Pair.Value == CurrentPage)
		{
			UIHistory.Add(static_cast<EUIID>(Pair.Key));
			break;
		}
	}

	// 현재 PAGE를 히스토리에 기록한 뒤 새 PAGE 열기 후 반환
	return OpenUIByID(ID);
}

void UUIManagerSubsystem::BackPage()
{
	if (UIHistory.Num() == 0)
	{
		// 히스토리 없음 — no-op
		return;
	}

	const EUIID PrevID = UIHistory.Pop();
	OpenUIByID(PrevID);
}

void UUIManagerSubsystem::ClearUIHistory()
{
	UIHistory.Empty();
}

// -----------------------------------------------------------------------------
// 내부 헬퍼
// -----------------------------------------------------------------------------


bool UUIManagerSubsystem::CheckUILayerSetting(EUIID ID, EUILayer& OutLayer)
{
	if (ID == EUIID::NONE)
	{
		KHS_WARN(TEXT("EUIID::NONE은 직접 오픈 불가"));
		return false;
	}

	const UUIManagerSettings* Settings = UUIManagerSettings::Get();
	if (!Settings)
	{
		KHS_ERROR(TEXT("UIManagerSettings를 찾을 수 없음"));
		return false;
	}

	const TSoftClassPtr<URSBaseWidget>* SoftClassPtr = Settings->UIClassMap.Find(ID);
	if (!SoftClassPtr || SoftClassPtr->IsNull())
	{
		KHS_WARN(TEXT("ID(%d)에 매핑된 위젯 클래스 없음. UIManagerSettings 확인 필요"), static_cast<uint8>(ID));
		return false;
	}

	const EUILayer* LayerPtr = Settings->UILayerMap.Find(ID);
	if (!LayerPtr)
	{
		KHS_WARN(TEXT("ID(%d)에 매핑된 레이어 없음. UIManagerSettings 확인 필요"), static_cast<uint8>(ID));
		return false;
	}
	
	OutLayer = *LayerPtr;
	return true;
}


void UUIManagerSubsystem::HandleWidgetByLayer(EUILayer Layer, URSBaseWidget* Widget)
{
	switch (Layer)
	{
	case EUILayer::PERSISTENT:
		{
			PersistentUIMap.Add(Widget->GetClass(), Widget);
			Widget->OpenUI();
			Widget->AddToViewport(CalculateZOrder(Widget));
		}
	break;
		
	case EUILayer::PAGE:
		{
			if (PageUIStack.Num() > 0)
			{
				CloseUIInternal(PageUIStack.Last());
			}
			PageUIStack.Add(Widget);
			
			Widget->OpenUI();
			Widget->AddToViewport(CalculateZOrder(Widget));
		}
		break;
		
	case EUILayer::SYSTEM:
		{
			SystemUIStack.Add(Widget);
			
			Widget->OpenUI();
			Widget->AddToViewport(CalculateZOrder(Widget));
			NotifyInputModeChange();
		}
		break;
		
	case EUILayer::POPUP:
		{
			if (PopupUIStack.Num() > 0)
			{
				PopupUIStack.Last()->OnFocusLost();
			}
			PopupUIStack.Add(Widget);
			Widget->OpenUI();
			Widget->AddToViewport(CalculateZOrder(Widget));
			NotifyInputModeChange();
		}
		break;
		
	default: //기본처리는 팝업과 동일 처리
		{
			if (PopupUIStack.Num() > 0)
			{
				PopupUIStack.Last()->OnFocusLost();
			}
			PopupUIStack.Add(Widget);
			Widget->OpenUI();
			Widget->AddToViewport(CalculateZOrder(Widget));
			NotifyInputModeChange();
		}
		break;
	}
}

URSBaseWidget* UUIManagerSubsystem::GetOrCreateWidgetByID(EUIID ID)
{
	const uint8 Key = static_cast<uint8>(ID);

	if (CachedWidgetsByID.Contains(Key))
	{
		return CachedWidgetsByID[Key];
	}

	const UUIManagerSettings* Settings = UUIManagerSettings::Get();
	if (!Settings)
	{
		return nullptr;
	}

	const TSoftClassPtr<URSBaseWidget>* SoftClassPtr = Settings->UIClassMap.Find(ID);
	if (!SoftClassPtr || SoftClassPtr->IsNull())
	{
		return nullptr;
	}

	// Soft 참조를 동기 로드 — 에디터에서 BP가 할당된 후에만 호출되므로 이미 로드됨을 가정
	TSubclassOf<URSBaseWidget> WidgetClass = SoftClassPtr->LoadSynchronous();
	if (!WidgetClass)
	{
		KHS_ERROR(TEXT("ID(%d) 위젯 클래스 로드 실패"), static_cast<uint8>(ID));
		return nullptr;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	URSBaseWidget* NewWidget = CreateWidget<URSBaseWidget>(PC, WidgetClass);
	if (NewWidget)
	{
		CachedWidgetsByID.Add(Key, NewWidget);
	}
	return NewWidget;
}

int32 UUIManagerSubsystem::CalculateZOrder(URSBaseWidget* Widget) const
{
	if (!Widget)
	{
		return 0;
	}

	switch (Widget->UILayer)
	{
	case EUILayer::PERSISTENT:
		return ZOrder_PERSISTENT;
	case EUILayer::PAGE:
		return ZOrder_PAGE;
	case EUILayer::POPUP:
		// POPUP은 스택 깊이에 따라 ZOrder_POPUP_STEP씩 증가
		return ZOrder_POPUP_BASE + (PopupUIStack.Num() * ZOrder_POPUP_STEP);
	case EUILayer::SYSTEM:
		return ZOrder_SYSTEM;
	default:
		return Widget->ZOrder;
	}
}

void UUIManagerSubsystem::NotifyInputModeChange()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!ensureMsgf(PC, TEXT("UIManagerSubsystem::NotifyInputModeChange — PlayerController NULL")))
	{
		return;
	}

	// SYSTEM > POPUP 순서로 UIOnly 여부 판단
	const bool bHasSystemUI = SystemUIStack.Num() > 0;
	const bool bHasModalPopup = PopupUIStack.ContainsByPredicate([](const URSBaseWidget* W)
	{
		return W && W->bIsModal;
	});

	if (bHasSystemUI || bHasModalPopup)
	{
		URSBaseWidget* FocusTarget = bHasSystemUI ? SystemUIStack.Last() : PopupUIStack.Last();
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(FocusTarget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
	else
	{
		FInputModeGameOnly GameOnlyMode;
		GameOnlyMode.SetConsumeCaptureMouseDown(false);
		PC->SetInputMode(GameOnlyMode);
		PC->SetShowMouseCursor(true); // 에임 커서 유지
	}
}

void UUIManagerSubsystem::CloseUIInternal(URSBaseWidget* Widget)
{
	if (!Widget || !Widget->IsOpen())
	{
		return;
	}

	Widget->CloseUI();
	Widget->RemoveFromParent();

	switch (Widget->UILayer)
	{
	case EUILayer::PERSISTENT:
		PersistentUIMap.Remove(Widget->GetClass());
		break;

	case EUILayer::PAGE:
		PageUIStack.Remove(Widget);
		break;

	case EUILayer::SYSTEM:
		SystemUIStack.Remove(Widget);
		NotifyInputModeChange();
		break;

	default: // POPUP
		PopupUIStack.Remove(Widget);
		RefreshTopPopupUI();
		NotifyInputModeChange();
		break;
	}
}

// -----------------------------------------------------------------------------
// 기존 API (하위 호환)
// -----------------------------------------------------------------------------

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
	if (PopupUIStack.Num() > 0)
	{
		PopupUIStack.Last()->OnFocusGained();
	}
}

void UUIManagerSubsystem::ResetAllUIStates()
{
	// 캐시된 모든 위젯 뷰포트에서 제거
	for (auto& Pair : CachedWidgets)
	{
		if (Pair.Value)
		{
			if (Pair.Value->IsOpen())
			{
				Pair.Value->CloseUI();
			}
			
			if (Pair.Value->IsInViewport())
			{
				Pair.Value->RemoveFromParent();
			}
		}
	}
	
	for (auto& Pair : CachedWidgetsByID)
	{
		if (Pair.Value)
		{
			if (Pair.Value->IsOpen())
			{
				Pair.Value->CloseUI();
			}
			
			if (Pair.Value->IsInViewport())
			{
				Pair.Value->RemoveFromParent();
			}
		}
	}

	PersistentUIMap.Empty();
	PopupUIStack.Empty();
	PageUIStack.Empty();
	SystemUIStack.Empty();
	CachedWidgets.Empty();
	CachedWidgetsByID.Empty();
	ClearUIHistory();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}

URSBaseWidget* UUIManagerSubsystem::GetWidgetByID(EUIID ID) const
{
	const URSBaseWidget* const* Found = CachedWidgetsByID.Find(static_cast<uint8>(ID));
	
	return Found ? const_cast<URSBaseWidget*>(*Found) : nullptr;
}
