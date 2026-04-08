// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/RSBaseWidget.h"
#include "Data/EnumUITypes.h"
#include "UIManagerSubsystem.generated.h"

/**
 * UI 생명주기 및 표시 레이어를 관리하는 중앙 집중식 서브시스템
 *
 * 4가지 UI 레이어:
 * - PERSISTENT : HUD 등 항상 표시 (ZOrder=100)
 * - PAGE       : 메인 콘텐츠. 동시에 1개. UIHistory로 BackPage() 지원 (ZOrder=200)
 * - POPUP      : 모달. 스택 중첩 가능. (ZOrder=300+)
 * - SYSTEM     : 종료확인/에러. 최상위 (ZOrder=500)
 *
 * ID 기반 API (신규): OpenUIByID / SwitchPageUI / BackPage / CloseUIByID
 * 클래스 기반 API (기존): OpenUI<T> — 하위 호환 완전 유지
 */
UCLASS()
class ROASTSTAFFGAS_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// =========================================================================
	// ID 기반 API 
	// =========================================================================
	/**
	 * EUIID로 UI를 열기.
	 * UIManagerSettings::UIClassMap / UILayerMap을 참조하여 레이어별 처리.
	 * EUIID::NONE이거나 UIClassMap에 매핑이 없으면 경고 로그 + 조기 반환.
	 */
	URSBaseWidget* OpenUIByID(EUIID ID);
	/**
	 * EUIID로 UI를 닫기.
	 * CachedWidgetsByID에서 인스턴스를 찾아 CloseUIInternal 호출.
	 */
	void CloseUIByID(EUIID ID);
	/**
	 * PAGE 레이어 전환 편의 함수.
	 * 현재 PAGE를 UIHistory에 push한 뒤 새 PAGE를 OpenUIByID로 열기.
	 * 반환값: 열린 위젯 포인터 (바인딩 등 즉시 활용 가능), 실패 시 nullptr.
	 */
	URSBaseWidget* SwitchPageUI(EUIID ID);
	
	/** ID 기반 캐시에서 위젯 조회. 없으면 nullptr */
	URSBaseWidget* GetWidgetByID(EUIID ID) const;
	
	/**
	 * UIHistory에서 이전 PAGE를 꺼내 복귀.
	 * 히스토리가 비어있으면 no-op.
	 */
	void BackPage();

	/** UIHistory 스택 초기화. 레벨 전환 전 GameInstance에서 호출. */
	void ClearUIHistory();

	// =========================================================================
	// 클래스 기반 API 
	// =========================================================================
	/**
	 * UI를 열고 뷰포트에 추가 (클래스 기반).
	 * PAGE / SYSTEM 레이어도 처리하도록 확장됨.
	 */
	template<typename T>
	T* OpenUI(TSubclassOf<T> TargetClassFactory);
	/** 위젯을 캐시에서 가져오거나 없으면 새로 생성 */
	template<typename T>
	T* GetOrCreateWidget(TSubclassOf<T> WidgetClassFactory);
	/** 특정 타입 UI 닫기 (클래스 기반) */
	template<typename T>
	void CloseUI(TSubclassOf<T> TargetClassFactory);
	/** 인스턴스 기반 직접 닫기 */
	void CloseUI(URSBaseWidget* Widget);
	/** Popup 스택 최상위 닫기 */
	void CloseTopPopupUI();
	/** 열려있는 모든 Popup 닫기 */
	void CloseAllPopupUI();
	/** Popup 스택 최상위에 포커스 전달 */
	void RefreshTopPopupUI();
	/** 레벨 전환 시 모든 UI 상태 리셋 (캐시 포함) */
	void ResetAllUIStates();
	

	FORCEINLINE bool HasOpenPopupUI()    const { return PopupUIStack.Num() > 0; }
	FORCEINLINE int  GetPopupStackSize() const { return PopupUIStack.Num(); }

	

private:
	// =========================================================================
	// 내부 헬퍼
	// =========================================================================
	/** OpenUIByID-> EUIID에 해당하는 UI레이어 체크*/
	bool CheckUILayerSetting(EUIID ID, EUILayer& OutLayer);
	/** OpenUIByID-> UI레이어에 따라 UI 처리*/
	void HandleWidgetByLayer(EUILayer Layer, URSBaseWidget* Widget);
	/** EUIID에 해당하는 위젯을 캐시에서 반환하거나 신규 생성 */
	URSBaseWidget* GetOrCreateWidgetByID(EUIID ID);
	/** 레이어별 ZOrder 계산 */
	int32 CalculateZOrder(URSBaseWidget* Widget) const;
	/** 팝업/시스템 스택 상태에 따라 입력 모드 갱신 */
	void NotifyInputModeChange();
	/** 레이어를 분기하여 실제 닫기 처리 */
	void CloseUIInternal(URSBaseWidget* Widget);
	
public:
	// =========================================================================
	// ZOrder 상수 — CalculateZOrder 내부 및 외부(PC FloatingDamage 등) 공유
	// =========================================================================
	static constexpr int32 ZOrder_PERSISTENT = 100;
	static constexpr int32 ZOrder_PAGE       = 200;
	static constexpr int32 ZOrder_POPUP_BASE = 300;
	static constexpr int32 ZOrder_POPUP_STEP =  10;
	static constexpr int32 ZOrder_SYSTEM     = 500;
	
private:
	// =========================================================================
	// 내부 상태
	// =========================================================================
	/** 현재 열려있는 PERSISTENT 위젯 (클래스 → 인스턴스) */
	UPROPERTY()
	TMap<TSubclassOf<URSBaseWidget>, URSBaseWidget*> PersistentUIMap;
	/** 현재 열려있는 POPUP 위젯 스택 (Last = 최상위/포커스 중) */
	UPROPERTY()
	TArray<URSBaseWidget*> PopupUIStack;
	/** 현재 열려있는 PAGE 위젯 스택 (실질적으로 Last 1개가 표시됨) */
	UPROPERTY()
	TArray<URSBaseWidget*> PageUIStack;
	/** 현재 열려있는 SYSTEM 위젯 스택 */
	UPROPERTY()
	TArray<URSBaseWidget*> SystemUIStack;
	/** PAGE 이동 이력 — BackPage() 복귀에 사용 */
	UPROPERTY()
	TArray<EUIID> UIHistory;
	/** 클래스 기반 캐시 (기존) */
	UPROPERTY()
	TMap<TSubclassOf<URSBaseWidget>, URSBaseWidget*> CachedWidgets;
	/** ID 기반 캐시 (신규 — OpenUIByID 전용) */
	UPROPERTY()
	TMap<uint8, URSBaseWidget*> CachedWidgetsByID;
	
};






// =============================================================================
// 템플릿 구현
// =============================================================================

template<typename T>
T* UUIManagerSubsystem::GetOrCreateWidget(TSubclassOf<T> WidgetClassFactory)
{
	TSubclassOf<URSBaseWidget> TargetClass = WidgetClassFactory;

	if (CachedWidgets.Contains(TargetClass))
	{
		return Cast<T>(CachedWidgets[TargetClass]);
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	T* NewWidget = CreateWidget<T>(PC, WidgetClassFactory);
	if (NewWidget)
	{
		CachedWidgets.Add(TargetClass, NewWidget);
	}
	return NewWidget;
}

template<typename T>
T* UUIManagerSubsystem::OpenUI(TSubclassOf<T> TargetClassFactory)
{
	if (!TargetClassFactory)
	{
		return nullptr;
	}

	T* Widget = GetOrCreateWidget<T>(TargetClassFactory);
	if (!Widget)
	{
		return nullptr;
	}

	if (Widget->IsOpen())
	{
		return Widget;
	}

	URSBaseWidget* BaseWidget = Widget;
	const EUILayer Layer = BaseWidget->UILayer;

	if (Layer == EUILayer::PERSISTENT)
	{
		TSubclassOf<URSBaseWidget> BaseClass = TargetClassFactory;
		if (!PersistentUIMap.Contains(BaseClass))
		{
			PersistentUIMap.Add(BaseClass, BaseWidget);
		}
		BaseWidget->OpenUI();
		BaseWidget->AddToViewport(CalculateZOrder(BaseWidget));
	}
	else if (Layer == EUILayer::PAGE)
	{
		// 현재 PAGE가 있으면 닫기
		if (PageUIStack.Num() > 0)
		{
			CloseUIInternal(PageUIStack.Last());
		}
		PageUIStack.Add(BaseWidget);
		BaseWidget->OpenUI();
		BaseWidget->AddToViewport(CalculateZOrder(BaseWidget));
	}
	else if (Layer == EUILayer::SYSTEM)
	{
		SystemUIStack.Add(BaseWidget);
		BaseWidget->OpenUI();
		BaseWidget->AddToViewport(CalculateZOrder(BaseWidget));
		NotifyInputModeChange();
	}
	else // POPUP
	{
		if (PopupUIStack.Num() > 0)
		{
			PopupUIStack.Last()->OnFocusLost();
		}
		PopupUIStack.Add(BaseWidget);
		BaseWidget->OpenUI();
		BaseWidget->AddToViewport(CalculateZOrder(BaseWidget));
		NotifyInputModeChange();
	}

	return Widget;
}

template<typename T>
void UUIManagerSubsystem::CloseUI(TSubclassOf<T> TargetClassFactory)
{
	if (!TargetClassFactory)
	{
		return;
	}

	TSubclassOf<URSBaseWidget> BaseClass = TargetClassFactory;
	if (!CachedWidgets.Contains(BaseClass))
	{
		return;
	}

	CloseUIInternal(CachedWidgets[BaseClass]);
}
