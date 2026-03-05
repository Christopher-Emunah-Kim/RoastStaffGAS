// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Data/DataTableStructs.h"
#include "Data/RuntimeDataStructs.h"
#include "System/LoggingSystem.h"
#include "GameDataSubsystem.generated.h"

/**
 * 게임 정적 데이터 관리 서브시스템 (GDS)
 *
 * - 게임 인스턴스 생성 시 자동 초기화. 씬 전환과 무관하게 유지.
 * - 모든 DT 로드/캐싱/조회의 단일 창구.
 * - 반환 방식: bool + OutParam. 조회 실패는 false 반환 + Warning 로그.
 * - 반환 실패 이후 처리는 요청 시스템의 책임.
 */
UCLASS()
class ROASTSTAFFGAS_API UGameDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // -------------------------------------------------------------------------
    // 초기화 상태 확인
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "MY|GDS")
    bool IsDataReady() const { return bIsDataReady; }
    
    // -------------------------------------------------------------------------
    // 캐릭터 스탯 커브 테이블 조회
    // -------------------------------------------------------------------------
    bool GetLevelCurveValue(FName CurveName, int32 Level, float& OutValue) const;

    
    // -------------------------------------------------------------------------
    // 무기 조회
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Weapon")
    bool GetWeaponData(FName WeaponID, FWeaponStaticData& OutData) const;

    /** WeaponLevel 기준 무기 ID 목록 반환. 레벨업 시 무기 풀 구성에 사용 */
    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Weapon")
    TArray<FName> GetWeaponIDsByLevel(int32 WeaponLevel) const;

    // -------------------------------------------------------------------------
    // 스킬 조회
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Skill")
    bool GetSkillData(FName SkillID, FSkillStaticData& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Skill")
    bool GetSkillEffectData(FName SkillEffectID, FSkillEffectData& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Skill")
    bool GetSkillSpawnData(FName SkillEffectID, FSkillSpawnData& OutData) const;

    // FlightData 템플릿 조회
    template<typename T>
    bool GetFlightData(FName SkillEffectID, T& OutData) const;

    // -------------------------------------------------------------------------
    // 복합 조회
    // -------------------------------------------------------------------------
    /**
     * WeaponID 하나로 DT_Weapon → DT_Skill → DT_SkillEffect를 체인 조회하여
     * FWeaponEquipData를 조합해 반환한다.
     * 중간 조회가 하나라도 실패하면 false를 반환하고 OutData는 변경하지 않는다.
     */
    bool GetWeaponEquipData(FName WeaponID, FWeaponEquipData& OutData) const;

private:
    // -------------------------------------------------------------------------
    // 초기화 헬퍼
    // -------------------------------------------------------------------------
    void LoadDataTables();
    void CacheAllData();
    void BuildSecondaryIndex();

    // -------------------------------------------------------------------------
    // 템플릿 헬퍼 3종 (기존 GDS 패턴 유지)
    // -------------------------------------------------------------------------
    template<typename T>
    void LoadDataTable(TSoftObjectPtr<UDataTable>& SoftPtr, UDataTable*& OutTable, const FString& TableName);

    template<typename T>
    void CacheDataTable(UDataTable* DataTable, TMap<FName, T>& OutCache, FName T::* KeyField, const FString& TableName);

    template<typename T>
    bool GetCachedData(const TMap<FName, T>& Cache, FName ID, T& OutData, const TCHAR* DataName) const;

    // FlightData 캐시 선택 특수화
    template<typename T>
    const TMap<FName, T>& GetFlightCache() const;

private:
    // -------------------------------------------------------------------------
    // 초기화 상태
    // -------------------------------------------------------------------------
    bool bIsDataReady = false;

    // -------------------------------------------------------------------------
    // 로드된 DataTable 포인터 (UPROPERTY로 GC 방지)
    // -------------------------------------------------------------------------
    UPROPERTY()
    UCurveTable* LoadedCurveTable = nullptr;
    UPROPERTY()
    UDataTable* LoadedWeaponTable = nullptr;
    UPROPERTY()
    UDataTable* LoadedSkillTable = nullptr;
    UPROPERTY()
    UDataTable* LoadedSkillEffectTable = nullptr;
    UPROPERTY()
    UDataTable* LoadedSpawnTable = nullptr;
    UPROPERTY()
    UDataTable* LoadedFlightArcTable = nullptr;
    UPROPERTY()
    UDataTable* LoadedFlightPierceTable = nullptr;
    UPROPERTY()
    UDataTable* LoadedFlightExplodeTable = nullptr;

    // -------------------------------------------------------------------------
    // 캐시 (ID → 구조체)
    // -------------------------------------------------------------------------
    UPROPERTY()
    TMap<FName, FWeaponStaticData> WeaponCache;
    UPROPERTY()
    TMap<FName, FSkillStaticData> SkillCache;
    UPROPERTY()
    TMap<FName, FSkillEffectData> SkillEffectCache;
    UPROPERTY()
    TMap<FName, FSkillSpawnData> SpawnCache;
    UPROPERTY()
    TMap<FName, FFlightArcData> FlightArcCache;
    UPROPERTY()
    TMap<FName, FFlightPierceData> FlightPierceCache;
    UPROPERTY()
    TMap<FName, FFlightExplodeData> FlightExplodeCache;

    // -------------------------------------------------------------------------
    // 보조 인덱스
    // -------------------------------------------------------------------------
    /** WeaponLevel → WeaponID 목록. GetWeaponIDsByLevel() 전용 */
    TMap<int32, TArray<FName>> WeaponByLevelIndex;
    
};


// =============================================================================
// 템플릿 구현부 (헤더에 위치해야 컴파일 가능)
// =============================================================================

template<typename T>
void UGameDataSubsystem::LoadDataTable(TSoftObjectPtr<UDataTable>& SoftPtr, UDataTable*& OutTable, const FString& TableName)
{
    if (SoftPtr.IsNull())
    {
        KHS_WARN(TEXT("%s 경로가 비어있습니다."), *TableName);
        OutTable = nullptr;
        return;
    }

    OutTable = SoftPtr.LoadSynchronous();

    if (OutTable)
    {
        KHS_INFO(TEXT("%s 로드 완료 (%d 행)"), *TableName, OutTable->GetRowNames().Num());
    }
    else
    {
        KHS_WARN(TEXT("%s 로드 실패"), *TableName);
    }
}

template<typename T>
void UGameDataSubsystem::CacheDataTable(UDataTable* DataTable, TMap<FName, T>& OutCache, FName T::* KeyField, const FString& TableName)
{
    if (!DataTable)
    {
        KHS_WARN(TEXT("CacheDataTable — %s 테이블이 null입니다."), *TableName);
        return;
    }

    OutCache.Empty();

    for (const FName& RowName : DataTable->GetRowNames())
    {
        if (T* Row = DataTable->FindRow<T>(RowName, TEXT("")))
        {
            FName Key = (*Row).*KeyField;
            OutCache.Add(Key, *Row);
        }
    }

    KHS_INFO(TEXT(" %s 캐싱 완료 (%d건)"), *TableName, OutCache.Num());
}

template<typename T>
bool UGameDataSubsystem::GetCachedData(const TMap<FName, T>& Cache, FName ID, T& OutData,const TCHAR* DataName) const
{
    if (!bIsDataReady)
    {
        KHS_WARN(TEXT("초기화 미완료 상태에서 조회 요청 — %s / ID: %s"), DataName, *ID.ToString());
        return false;
    }

    const T* Found = Cache.Find(ID);
    if (!Found)
    {
        KHS_WARN(TEXT("데이터 조회 실패 — %s / ID: %s"),DataName, *ID.ToString());
        return false;
    }

    OutData = *Found;
    return true;
}

// FlightData 캐시 특수화 — 특수화된 타입 외 기본 템플릿은 호출되면 컴파일 에러로 방어
template<typename T>
const TMap<FName, T>& UGameDataSubsystem::GetFlightCache() const
{
    static_assert(sizeof(T) == 0, "지원하지 않는 FlightData 타입입니다.");
    static TMap<FName, T> Dummy;
    return Dummy;
}

template<>
inline const TMap<FName, FFlightArcData>& UGameDataSubsystem::GetFlightCache() const
{
    return FlightArcCache;
}

template<>
inline const TMap<FName, FFlightPierceData>& UGameDataSubsystem::GetFlightCache() const
{
    return FlightPierceCache;
}

template<>
inline const TMap<FName, FFlightExplodeData>& UGameDataSubsystem::GetFlightCache() const
{
    return FlightExplodeCache;
}

template<typename T>
bool UGameDataSubsystem::GetFlightData(FName SkillEffectID, T& OutData) const
{
    const TMap<FName, T>& Cache = GetFlightCache<T>();
    return GetCachedData(Cache, SkillEffectID, OutData, TEXT("FlightData"));
}