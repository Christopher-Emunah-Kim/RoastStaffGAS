// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumUITypes.generated.h"

// ============================================================================
// UI 인프라 전용 Enum 정의 (게임플레이 enum과 분리 유지)
// ============================================================================

/** UI 레이어 타입 - ZOrder 및 관리 방식 결정 */
UENUM(BlueprintType)
enum class EUILayer : uint8
{ 
	PERSISTENT  UMETA(DisplayName = "Persistent"),   // ZOrder=100  HUD 등 항상 표시
	PAGE        UMETA(DisplayName = "Page"),         // ZOrder=200  메인 콘텐츠. 동시에 1개
	POPUP       UMETA(DisplayName = "Popup"),        // ZOrder=300+ 모달. 스택당 +10
	SYSTEM      UMETA(DisplayName = "System"),       // ZOrder=500  종료확인/에러. 최상위
};

/** UI 식별자 - UIManagerSettings에서 위젯 클래스와 1:1 매핑 */
UENUM(BlueprintType)
enum class EUIID : uint8
{
	NONE           	 UMETA(DisplayName = "None"),           // 자식 위젯 전용. 직접 오픈 불가
	BACKGROUND     	 UMETA(DisplayName = "Background"),
	LOADING        	 UMETA(DisplayName = "Loading"),
	INTRO          	 UMETA(DisplayName = "Intro"),
	TITLE          	 UMETA(DisplayName = "Title"),
	OUTGAME        	 UMETA(DisplayName = "OutGame"),
	LOBBY          	 UMETA(DisplayName = "Lobby"),
	CHAR_SELECT    	 UMETA(DisplayName = "CharSelect"),
	CHAR_GRID_POPUP  UMETA(DisplayName = "CharGridPopup"),
	STAGE_SELECT     UMETA(DisplayName = "StageSelect"),
	SETTING          UMETA(DisplayName = "Setting"),
	PAUSE            UMETA(DisplayName = "Pause"),
	HUD              UMETA(DisplayName = "HUD"),
	LEVEL_UP         UMETA(DisplayName = "LevelUp"),
	WEAPON_REPLACE   UMETA(DisplayName = "WeaponReplace"),
	GAMEOVER         UMETA(DisplayName = "GameOver"),
	GAMECLEAR        UMETA(DisplayName = "GameClear"),
	EXIT             UMETA(DisplayName = "Exit"),
};

/** 레벨 전환 대상 식별자 - MapSettings에서 실제 레벨 경로와 매핑 */
UENUM(BlueprintType)
enum class ELevelName : uint8
{
	INTRO       UMETA(DisplayName = "Intro"),
	TRANSITION  UMETA(DisplayName = "Transition"),
	OUTGAME     UMETA(DisplayName = "OutGame"),
	STAGE_1     UMETA(DisplayName = "Stage1"),   // 월드 1 스테이지 레벨
	STAGE_2     UMETA(DisplayName = "Stage2"),   // 월드 2 스테이지 레벨
};
