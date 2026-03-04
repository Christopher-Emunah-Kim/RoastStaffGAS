// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h" // KHS_SCREEN_INFO 사용을 위해 포함

// =============================================================================
// I. (CORE) 로깅 시스템 구성 요소
// =============================================================================
// -----------------------------------------------------------------------------
// 1. 로그 카테고리 선언
// -----------------------------------------------------------------------------
// KHS 전역 로깅 카테고리 선언
ROASTSTAFFGAS_API DECLARE_LOG_CATEGORY_EXTERN(LogKHS, Log, All);

// -----------------------------------------------------------------------------
// 2. 로그 레벨 정의 및 UE Verbosity 매핑
// -----------------------------------------------------------------------------

/** KHS 프로젝트 전용 로그 레벨 6단계 정의 */
enum class EKLogLevel : uint8
{
	Trace = 0,	// 최세밀 추적 (개발/디버그 전용임)
	Debug = 1,	// 일반 디버깅 정보 (개발/디버그 전용임)
	Info = 2,	// 중요한 상태 변화 (모든 빌드 포함됨)
	Warn = 3,	// 잠재적인 문제 (모든 빌드 포함됨)
	Error = 4,	// 기능 오류 (모든 빌드 포함됨)
	Fatal = 5	// 치명적인 오류 (모든 빌드 포함됨)
};

// -----------------------------------------------------------------------------
// 3. 빌드 환경별 컴파일 타임 최적화 (Shipping 필터링)
// -----------------------------------------------------------------------------

// Shipping 빌드 시 오버헤드 줄이려 TRACE(0), DEBUG(1) 레벨 코드는 컴파일 제외함
#if UE_BUILD_SHIPPING
  // Shipping 시 INFO (Level 2) 이상만 컴파일에 포함됨
  #define KHS_COMPILETIME_MIN_LEVEL EKLogLevel::Info
#else
  // Debug/Development 빌드 시 모든 레벨(TRACE=Level 0) 활성화함
  #define KHS_COMPILETIME_MIN_LEVEL EKLogLevel::Trace
#endif

// 현재 로그 레벨이 컴파일 타임 최소 레벨보다 높거나 같은지 확인하는 조건 매크로
#define KHS_LEVEL_ENABLED(Level) (static_cast<int32>(Level) >= static_cast<int32>(KHS_COMPILETIME_MIN_LEVEL))


// -----------------------------------------------------------------------------
// 4. 핵심 로깅 처리 매크로 (로그 출처 자동 포함)
// -----------------------------------------------------------------------------
/**
* 컴파일 타임 필터링 후 실제 UE_LOG를 호출하여 로그 출력함.
* 함수 이름, 파일 이름, 라인 번호를 자동으로 포함함.
*/
#define KHS_LOG_INTERNAL(Category, Level, Format, ...) \
	do { \
		if (KHS_LEVEL_ENABLED(Level)) \
		{ \
			switch(Level) {																					  \
                case EKLogLevel::Trace:																		  \
                    UE_LOG(Category, VeryVerbose, TEXT("[%s][%s:%d] " Format),								  \
                        TEXT(__FUNCTION__), *FPaths::GetCleanFilename(__FILE__), __LINE__, ##__VA_ARGS__);	  \
                    break;																					  \
                case EKLogLevel::Debug:																		  \
                    UE_LOG(Category, Verbose, TEXT("[%s][%s:%d] " Format),									  \
                        TEXT(__FUNCTION__), *FPaths::GetCleanFilename(__FILE__), __LINE__, ##__VA_ARGS__);    \
                    break;																					  \
                case EKLogLevel::Info:																		  \
                    UE_LOG(Category, Log, TEXT("[%s][%s:%d] " Format),										  \
                        TEXT(__FUNCTION__), *FPaths::GetCleanFilename(__FILE__), __LINE__, ##__VA_ARGS__);    \
                    break;																					  \
                case EKLogLevel::Warn:																		  \
                    UE_LOG(Category, Warning, TEXT("[%s][%s:%d] " Format),									  \
                        TEXT(__FUNCTION__), *FPaths::GetCleanFilename(__FILE__), __LINE__, ##__VA_ARGS__);    \
                    break;																					  \
                case EKLogLevel::Error:																		  \
                    UE_LOG(Category, Error, TEXT("[%s][%s:%d] " Format),									  \
                        TEXT(__FUNCTION__), *FPaths::GetCleanFilename(__FILE__), __LINE__, ##__VA_ARGS__);    \
                    break;																					  \
                case EKLogLevel::Fatal:																		  \
                    UE_LOG(Category, Fatal, TEXT("[%s][%s:%d] " Format),									  \
                        TEXT(__FUNCTION__), *FPaths::GetCleanFilename(__FILE__), __LINE__, ##__VA_ARGS__);    \
                    break;																					  \
				default:																					  \
					UE_LOG(Category, Log, TEXT("[%s][%s:%d] " Format),										  \
						TEXT(__FUNCTION__), *FPaths::GetCleanFilename(__FILE__), __LINE__, ##__VA_ARGS__);    \
				break;																						  \
            }																								  \
		} \
	} while(0)

// -----------------------------------------------------------------------------
// 5. 6대 기본 로그 레벨 매크로 (사용자 인터페이스)
// -----------------------------------------------------------------------------

// --- 전역 카테고리 (LogKHS) 사용 매크로 ---
#define KHS_TRACE(Format, ...) KHS_LOG_INTERNAL(LogKHS, EKLogLevel::Trace, Format, ##__VA_ARGS__)
#define KHS_DEBUG(Format, ...) KHS_LOG_INTERNAL(LogKHS, EKLogLevel::Debug, Format, ##__VA_ARGS__)
#define KHS_INFO(Format, ...)  KHS_LOG_INTERNAL(LogKHS, EKLogLevel::Info,  Format, ##__VA_ARGS__)
#define KHS_WARN(Format, ...)  KHS_LOG_INTERNAL(LogKHS, EKLogLevel::Warn,  Format, ##__VA_ARGS__)
#define KHS_ERROR(Format, ...) KHS_LOG_INTERNAL(LogKHS, EKLogLevel::Error, Format, ##__VA_ARGS__)
#define KHS_FATAL(Format, ...) KHS_LOG_INTERNAL(LogKHS, EKLogLevel::Fatal, Format, ##__VA_ARGS__)


// -----------------------------------------------------------------------------
// 6.  에디터 화면 출력 (WITH_EDITOR)
// -----------------------------------------------------------------------------
/**
* 에디터 빌드 시에만 화면에 디버그 메시지 띄우고, 로그에도 INFO 레벨로 동시 기록함
*/
#if WITH_EDITOR

// 에디터 모드에서만 화면에 디버그 메시지 출력
#define KHS_SCREEN_INFO(Format, ...) \
	do \
	{ \
		const FString DebugMessage = FString::Printf(Format, ##__VA_ARGS__); \
		KHS_INFO(TEXT("SCREEN: %s"), *DebugMessage); \
		if (GEngine) \
		{ \
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, DebugMessage); \
		} \
	} while(0)

#else

// 에디터가 아닌 빌드에서는 빈 매크로로 대체
#define KHS_SCREEN_INFO(Format, ...) do {   } while(0)

#endif

