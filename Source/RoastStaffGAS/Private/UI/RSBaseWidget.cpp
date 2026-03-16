// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RSBaseWidget.h"

void URSBaseWidget::OpenUI()
{
	bIsOpen = true;
	SetVisibility(ESlateVisibility::Visible);
	// 자식 클래스에서 오버라이드하여 추가 로직 구현
}

void URSBaseWidget::CloseUI()
{
	bIsOpen = false;
	SetVisibility(ESlateVisibility::Collapsed);
	// 자식 클래스에서 오버라이드하여 추가 로직 구현
}

void URSBaseWidget::RefreshUI()
{
	// 자식 클래스에서 오버라이드하여 데이터 갱신 로직 구현
}

void URSBaseWidget::OnFocusGained()
{
	// 자식 클래스에서 오버라이드 (예: 애니메이션 재생, 입력 활성화 등)
}

void URSBaseWidget::OnFocusLost()
{
	// 자식 클래스에서 오버라이드 (예: 애니메이션 정지, 입력 비활성화 등)
}
