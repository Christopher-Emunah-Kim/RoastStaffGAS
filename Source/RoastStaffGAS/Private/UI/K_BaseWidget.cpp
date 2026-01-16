// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/K_BaseWidget.h"

void UK_BaseWidget::OpenUI()
{
	bIsOpen = true;
	SetVisibility(ESlateVisibility::Visible);
	// 자식 클래스에서 오버라이드하여 추가 로직 구현
}

void UK_BaseWidget::CloseUI()
{
	bIsOpen = false;
	SetVisibility(ESlateVisibility::Hidden);
	// 자식 클래스에서 오버라이드하여 추가 로직 구현
}

void UK_BaseWidget::RefreshUI()
{
	// 자식 클래스에서 오버라이드하여 데이터 갱신 로직 구현
}

void UK_BaseWidget::OnFocusGained()
{
	// 자식 클래스에서 오버라이드 (예: 애니메이션 재생, 입력 활성화 등)
}

void UK_BaseWidget::OnFocusLost()
{
	// 자식 클래스에서 오버라이드 (예: 애니메이션 정지, 입력 비활성화 등)
}
