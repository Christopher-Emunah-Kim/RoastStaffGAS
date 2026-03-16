// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RSBaseWidget.generated.h"

/**
 * 위젯이 닫기를 요청할 때 발생하는 이벤트
 * PlayerController나 다른 시스템에서 구독하여 실제 닫기 처리 수행
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCloseUIRequested, URSBaseWidget*, RequestingWidget);

UENUM(BlueprintType)
enum class EUILayer : uint8
{
	PERSISTENT UMETA(DisplayName = "Persistent"),
	POPUP UMETA(DisplayName = "Popup"),
};


/**
 * 모든 UI 위젯의 베이스 클래스
 * - 열림/닫힘 상태 관리
 * - 포커스 이벤트 처리
 * - 닫기 요청 델리게이트
 */
UCLASS()
class ROASTSTAFFGAS_API URSBaseWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** UI를 활성화하고 화면에 표시 */
	virtual void OpenUI();
	/** UI를 비활성화하고 화면에서 숨김 */
	virtual void CloseUI();
	/** UI 데이터 갱신 (예: 아이템 목록, 스탯 변경 등) */
	virtual void RefreshUI();
	/** 이 UI가 팝업 스택의 최상위가 되었을 때 호출 */
	virtual void OnFocusGained();
	/** 이 UI 위에 다른 팝업이 열렸을 때 호출 */
	virtual void OnFocusLost();
	/** UI가 현재 열려있는지 확인 */
	FORCEINLINE bool IsOpen() const { return bIsOpen; }
    
	/**
	 * 위젯 닫기 요청 델리게이트
	 */
	UPROPERTY(BlueprintAssignable, Category = "UI Events")
	FOnCloseUIRequested OnCloseUIRequested;
    
protected:
	/** UI 열림/닫힘 상태 */
	bool bIsOpen = false;
    
public:
	/** UI 레이어 타입 (Persistent: 지속형, Popup: 팝업형) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	EUILayer UILayer = EUILayer::PERSISTENT;
    
	/** 뷰포트 내 표시 순서 (높을수록 위에 렌더링) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	int32 ZOrder = 0;

	/** 모달 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	bool bIsModal = false;

};
