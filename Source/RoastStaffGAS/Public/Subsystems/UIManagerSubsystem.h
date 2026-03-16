// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/RSBaseWidget.h"
#include "UIManagerSubsystem.generated.h"

/**
 * UI 생명주기 및 표시 레이어를 관리하는 중앙 집중식 서브시스템
 * 
 * 두 가지 UI 레이어 지원:
 * - PERSISTENT: 게임 플레이 중 지속적으로 표시되는 UI (예: HUD, Minimap)
 * - POPUP: 필요시 표시되는 모달 UI (예: 인벤토리, 설정)
 * 
 * 주요 기능:
 * - 위젯 캐싱으로 반복 생성/소멸 비용 절약
 * - Popup UI 스택 관리로 여러 팝업의 포커스 제어
 * - 입력 모드 자동 전환 (Popup 열림 시 UI Only, 닫힘 시 Game Only)
 */
UCLASS()
class ROASTSTAFFGAS_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
private:
    // UI레이어에 따른 ZOrder계산 내부 헬퍼
    int32 CalculateZOrder(URSBaseWidget* Widget) const;
    // 입력모드 변경 내부 헬퍼(팝업 스택에 따라)
    void NotifyInputModeChange();
    // UI 닫기 내부 헬퍼
    void CloseUIInternal(URSBaseWidget* Widget);
    
public:
    /**
     * UI를 열고 뷰포트에 추가
     * - 이미 열려있으면 기존 인스턴스 반환
     * - Persistent UI는 맵에 저장
     * - Popup UI는 스택에 추가하고 이전 Top에 FocusLost 알림
     * 
     * @param TargetClass 열어야 하는 UI 위젯 클래스
     * @return 열린 UI 위젯 인스턴스 (실패 시 nullptr)
     */
    template<typename T>
    T* OpenUI(TSubclassOf<T> TargetClassFactory);
    
    /**
     * 위젯을 캐시에서 가져오거나 없으면 새로 생성
     * - 성능 최적화: 자주 열리는 UI의 생성 비용 절감
     * - 위젯 상태 유지: 닫았다 다시 열어도 이전 상태 보존
     * 
     * @param WidgetClass 가져올/생성할 위젯 클래스
     * @return 캐싱된 또는 새로 생성된 위젯 (실패 시 nullptr)
     */
    template<typename T>
    T* GetOrCreateWidget(TSubclassOf<T> WidgetClassFactory);
    
    /**
     * 특정 타입의 UI를 닫고 뷰포트에서 제거 (타입 기반)
     * - Persistent: 맵에서 제거
     * - Popup: 스택에서 제거하고 새로운 Top에 FocusGained 알림
     * - 주의: 위젯은 캐시에 유지되므로 완전히 파괴되지 않음
     */
    template<typename T>
    void CloseUI(TSubclassOf<T> targetClassFactory);
    
    /**
     * 인스턴스 기반 위젯 직접 닫기
     * @param Widget 닫고자 하는 위젯 인스턴스
     */
    void CloseUI(URSBaseWidget* Widget);
    
    /** 
     * 스택 최상위 Popup UI 닫기
     * - 스택에서 Pop하고 그 아래 팝업에 포커스 전달
     */
    void CloseTopPopupUI();
    
    /** 
     * 열려있는 모든 Popup UI 닫기
     * - 게임 오버, 씬 전환 등에 사용
     */
    void CloseAllPopupUI();
    
    /** 
     * Popup 스택의 새로운 Top에 포커스 전달
     * - 중간 팝업이 닫혔을 때 호출됨
     */
    void RefreshTopPopupUI();
    
     /** 
     * 레벨 전환시 모든 UI상태 리셋 함수(Controller)
     * - 캐싱된 UI들을 모두 초기화하여 레벨 전환시 재생성 보장.
     */
	void ResetAllUIStates();
    
    /** Popup UI가 하나라도 열려있는지 확인 */
    FORCEINLINE bool HasOpenPopupUI() const { return PopupUIStack.Num() > 0; }
	FORCEINLINE int GetPopupStackSize() const { return PopupUIStack.Num(); }
    
private:
    /** 현재 열려있는 Persistent UI 맵 (클래스 -> 인스턴스) */
    UPROPERTY()
    TMap<TSubclassOf<URSBaseWidget>, URSBaseWidget*> PersistentUIMap;
    /** 
     * 열려있는 Popup UI 스택 (Last = 최상위/포커스 중)
     * 나중에 추가된 것이 위에 표시되고 먼저 입력 받음
     */
    UPROPERTY()
    TArray<URSBaseWidget*> PopupUIStack;
    /** 
     * 생성된 모든 위젯의 캐시 (클래스 -> 인스턴스)
     * 최적화 : 매번 CreateWidget 호출 방지
     * 상태 유지: UI를 닫아도 데이터 보존
     */
    UPROPERTY()
    TMap<TSubclassOf<URSBaseWidget>, URSBaseWidget*> CachedWidgets;
};


// 템플릿 함수 구현
template<typename T>
T* UUIManagerSubsystem::GetOrCreateWidget(TSubclassOf<T> WidgetClassFactory)
{
    TSubclassOf<URSBaseWidget> TargetClassFactory = WidgetClassFactory;
    
    // 캐싱된 UI가 있으면 반환(클래스 설계도로 실제 인스턴스 키 있는지 검사)
    if (CachedWidgets.Contains(TargetClassFactory))
    {
        return Cast<T>(CachedWidgets[TargetClassFactory]);
    }
    
    // 없으면 생성 후 캐시에 저장
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        return nullptr;
    }
    
    T* NewWidget = CreateWidget<T>(PC, WidgetClassFactory);
    if (NewWidget)
    {
        CachedWidgets.Add(TargetClassFactory, NewWidget);
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
    
    //캐싱된 인스턴스가 있다면 반환, 없으면 생성 후 반환
    T* Widget = GetOrCreateWidget<T>(TargetClassFactory);
    if (!Widget)
    {
        return nullptr;
    }
    
    // 이미 열려있으면 기존 인스턴스 사용
    if (Widget->IsOpen())
    {
        return Widget;
    }
    
    URSBaseWidget* BaseWidget = Widget;
    
    // Persistent 타입 UI일 때
    if (BaseWidget->UILayer == EUILayer::PERSISTENT)
    {
        TSubclassOf<URSBaseWidget> BaseClassFactory = TargetClassFactory;
        if (!PersistentUIMap.Contains(BaseClassFactory))
        {
            PersistentUIMap.Add(BaseClassFactory, BaseWidget);
        }
        
        BaseWidget->OpenUI();
        int32 ZOrder = CalculateZOrder(BaseWidget);
        BaseWidget->AddToViewport(ZOrder);
    }
    else // Popup 타입 UI일 때
    {
        // 현재의 Top에 FocusLost 알림
        if (PopupUIStack.Num() > 0)
        {
            PopupUIStack.Last()->OnFocusLost();
        }
        
        // 스택에 추가
        PopupUIStack.Add(BaseWidget);
        
        BaseWidget->OpenUI();
        int32 ZOrder = CalculateZOrder(BaseWidget);
        BaseWidget->AddToViewport(ZOrder);
        
        // 입력 모드 변경
        NotifyInputModeChange();
    }
    
    return Widget;
}

template<typename T>
void UUIManagerSubsystem::CloseUI(TSubclassOf<T> targetClassFactory)
{
	if (!targetClassFactory)
	{
		return;
	}
	
	TSubclassOf<URSBaseWidget> baseClassFactory = targetClassFactory;
	
	// 캐싱중인 UI라면 리턴
	if (!CachedWidgets.Contains(baseClassFactory))
	{
		return;
	}
	
	URSBaseWidget* widget = CachedWidgets[baseClassFactory];
	
	//실제 닫기 로직은 내부 헬퍼함수 호출
	CloseUIInternal(widget);
}