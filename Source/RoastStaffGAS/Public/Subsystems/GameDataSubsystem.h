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
    // 캐릭터 정적 데이터 조회
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Character")
    bool GetCharacterStaticData(FName CharID, FCharacterStaticData& OutData) const;
    /** 전체 캐릭터 목록 반환. 캐릭터 선택 UI 목록 구성에 사용. 캐시 미로드 시 false 반환. */
    bool GetAllCharacterStaticData(TArray<FCharacterStaticData>& OutArray) const;
    /** 레벨별 커브데이터 조회. */
    bool GetLevelCurveValue(FName CurveName, int32 Level, float& OutValue) const;
    /** CharID → 트랜지션 프리로드용 에셋 묶음(Mesh/AnimBP) 반환. */
    bool GetCharacterPreloadBundle(FName CharID, FCharacterPreloadBundle& OutBundle) const;

    // -------------------------------------------------------------------------
    // 무기/스킬 복합 조회
    // -------------------------------------------------------------------------
    //WeaponID → FWeaponSlotEquipData 조합 반환.(장착 + 슬롯 초기화 + UI)
    bool GetWeaponSlotEquipData(FName WeaponID, FWeaponSlotEquipData& OutData) const;
    //SkillID → FSkillExecutionData 조합 반환.(GA 발동 시 투사체/소환물 생성에 필요)
    //WeaponID 전달 시 DT에서 WeaponLevel 파생 → CurveTable로 Amount 결정. 미전달 시 DT 기본값 사용.
    bool GetSkillExecutionData(FName SkillID, FSkillExecutionData& OutData, FName WeaponID = NAME_None) const;
    //SkillID → FSkillFXData 조합 반환.(투사체/소환물 연출(VFX/SFX))                                                                                                  
    bool GetSkillFXData(FName SkillID, FSkillFXData& OutData) const;  
    
    // -------------------------------------------------------------------------
    // 무기 조회
    // -------------------------------------------------------------------------
    /** WeaponLevel 기준 무기 ID 목록 반환. 레벨업 시 무기 풀 구성에 사용 */
    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Weapon")
    TArray<FName> GetWeaponIDsByLevel(int32 WeaponLevel) const;
    /** WeaponID → FWeaponStaticData 단일 조회. LevelUpSubsystem 카드 상태 판단에 사용 */
    bool GetWeaponData(FName WeaponID, FWeaponStaticData& OutData) const;
    /** WeaponID + WeaponLevel → CurveTable에서 데미지 조회. 조회 실패 시 0.f 반환 */
    float GetWeaponDamageFromCurve(FName WeaponID, int32 WeaponLevel) const;

    // -------------------------------------------------------------------------
    // 스킬 조회
    // -------------------------------------------------------------------------
    // MoveTypeData 템플릿 조회
    template<typename T>
    bool GetMoveTypeData(FName SkillEffectID, T& OutData) const;

    // HitTypeData 템플릿 조회
    template<typename T>
    bool GetHitTypeData(FName SkillEffectID, T& OutData) const;
    
    // -------------------------------------------------------------------------
    // 에너미 조회
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Enemy")
    bool GetEnemyData(FName EnemyID, FEnemyStaticData& OutData) const;
    /** EnemyID → Ranged / Elite / Boss 확장 수치 반환. 행 없으면 false. */
    bool GetEnemyExtData(FName EnemyID, FEnemyExtData& OutData) const;
    /** EnemyID → 트랜지션 프리로드용 에셋 묶음(EnemyClass/BehaviorTree) 반환. */
    bool GetEnemyPreloadBundle(FName EnemyID, FEnemyPreloadBundle& OutBundle) const;

    // -------------------------------------------------------------------------
    // 스테이지 / 웨이브 조회
    // -------------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Stage")
    bool GetStageData(FName StageID, FStageStaticData& OutData) const;
    /** 전체 스테이지 목록 반환. 스테이지 선택 노드맵 구성에 사용. 캐시 미로드 시 false 반환. */
    bool GetAllStageStaticData(TArray<FStageStaticData>& OutArray) const;
    /** StageID에 속한 웨이브 목록 반환(WaveIndex 오름차순)  */
    UFUNCTION(BlueprintCallable, Category = "MY|GDS|Stage")
    TArray<FWaveStaticData> GetWaveDataByStage(FName StageID) const;
    
private:
    // -------------------------------------------------------------------------
    // 초기화 헬퍼
    // -------------------------------------------------------------------------
    void LoadDataTables();
    void CacheAllData();
    void BuildSecondaryIndex();

    // -------------------------------------------------------------------------
    // 개별 테이블 조회 (복합 조회 내부 헬퍼)
    // -------------------------------------------------------------------------
    bool GetSkillStaticData(FName SkillID, FSkillCommonStaticData& OutData) const;
    bool GetSkillResourceData(FName SkillID, FSkillCommonResourceData& OutData) const;
    bool GetSkillCommonParamData(FName SkillEffectID, FSkillCommonParamData& OutData) const;
    bool GetSkillAttackCommonParamsData(FName SkillEffectID, FSkillAttackCommonParamsData& OutData) const;
    bool GetSkillAttackSpawnParamsData(FName SkillEffectID, FSkillAttackSpawnParamsData& OutData) const;                                   
    bool GetSkillDefenseCommonParamsData(FName SkillEffectID, FSkillDefenseCommonParamsData& OutData) const;                                       
    bool GetStatusEffectData(FName StatusEffectID, FStatusEffectData& OutData) const;  
    
    // -------------------------------------------------------------------------
    // 템플릿 헬퍼
    // -------------------------------------------------------------------------
    template<typename T>
    void LoadDataTable(TSoftObjectPtr<UDataTable>& SoftPtr, UDataTable*& OutTable, const FString& TableName);
    template<typename T>
    void CacheDataTable(UDataTable* DataTable, TMap<FName, T>& OutCache, FName T::* KeyField, const FString& TableName);
    template<typename T>
    bool GetCachedData(const TMap<FName, T>& Cache, FName ID, T& OutData, const TCHAR* DataName) const;

    // MoveTypeData 캐시 선택 특수화 탬플릿
    template<typename T>
    const TMap<FName, T>& GetMoveTypeCache() const;
    // HitTypeData 캐시 선택 특수화 탬플릿
    template<typename T>
    const TMap<FName, T>& GetHitTypeCache() const;
    
private:
    // -------------------------------------------------------------------------
    // 초기화 상태
    // -------------------------------------------------------------------------
    bool bIsDataReady = false;

    // -------------------------------------------------------------------------
    // 로드된 DataTable 포인터 (UPROPERTY로 GC 방지)
    // -------------------------------------------------------------------------
    UPROPERTY() UCurveTable* LoadedCurveTable = nullptr;
    UPROPERTY() UCurveTable* LoadedWeaponDamageCurveTable = nullptr;
    UPROPERTY() UDataTable* LoadedCharacterTable = nullptr;
    UPROPERTY() UDataTable* LoadedWeaponTable = nullptr;
    UPROPERTY() UDataTable* LoadedEnemyTable    = nullptr;
    UPROPERTY() UDataTable* LoadedEnemyExtTable = nullptr;
    UPROPERTY() UDataTable* LoadedStageTable = nullptr;
    UPROPERTY() UDataTable* LoadedWaveTable = nullptr;
    UPROPERTY() UDataTable* LoadedStatusEffectTable = nullptr;  
    //SKill 관련 
    UPROPERTY() UDataTable* LoadedSkillCommonStaticTable = nullptr;
    UPROPERTY() UDataTable* LoadedSkillCommonResourceTable = nullptr;
    UPROPERTY() UDataTable* LoadedSkillCommonParamsTable = nullptr;
    UPROPERTY() UDataTable* LoadedSkillAttackCommonParamsTable = nullptr;
    UPROPERTY() UDataTable* LoadedSkillAttackSpawnParamsTable = nullptr;
    UPROPERTY() UDataTable* LoadedSkillAttackMoveTypeParamsArcTable = nullptr;
    UPROPERTY() UDataTable* LoadedSkillAttackMoveTypeParamsHomingTable = nullptr; 
    UPROPERTY() UDataTable* LoadedSkillAttackMoveTypeParamsSummonTable = nullptr; 
    UPROPERTY() UDataTable* LoadedSkillAttackHitTypeParamsPierceTable = nullptr;
    UPROPERTY() UDataTable* LoadedSkillAttackHitTypeParamsAreaTable = nullptr;
    UPROPERTY() UDataTable* LoadedSkillDefenseCommonParamsTable = nullptr;        
    
    // -------------------------------------------------------------------------
    // 캐시 (ID → 구조체)
    // -------------------------------------------------------------------------
    UPROPERTY() TMap<FName, FCharacterStaticData>               CharacterCache;
    UPROPERTY() TMap<FName, FWeaponStaticData>                  WeaponCache;
    UPROPERTY() TMap<FName, FEnemyStaticData>                   EnemyCache;
    UPROPERTY() TMap<FName, FStageStaticData>                   StageCache;
    UPROPERTY() TMap<FName, FWaveStaticData>                    WaveCache;
    UPROPERTY() TMap<FName, FStatusEffectData>                  StatusEffectCache; 
    //Skill
    UPROPERTY() TMap<FName, FSkillCommonStaticData>             SkillCommonStaticCache;
    UPROPERTY() TMap<FName, FSkillCommonResourceData>           SkillCommonResourceCache;  
    UPROPERTY() TMap<FName, FSkillCommonParamData>              SkillCommonParamCache;  
    UPROPERTY() TMap<FName, FSkillAttackCommonParamsData>       SkillAttackCommonParamsCache;
    UPROPERTY() TMap<FName, FSkillAttackSpawnParamsData>        SkillAttackSpawnParamsCache;
    UPROPERTY() TMap<FName, FSkillAttackMoveTypeParamsArc>      SkillMoveTypeArcCache;
    UPROPERTY() TMap<FName, FSkillAttackMoveTypeParamsHoming>   SkillMoveTypeHomingCache;     
    UPROPERTY() TMap<FName, FSkillAttackMoveTypeParamsSummon>   SkillMoveTypeSummonCache;  
    UPROPERTY() TMap<FName, FSkillAttackHitTypeParamsPierce>    SkillHitTypePierceCache;
    UPROPERTY() TMap<FName, FSkillAttackHitTypeParamsArea>      SkillHitTypeAreaCache;
    UPROPERTY() TMap<FName, FSkillDefenseCommonParamsData>      SkillDefenseCommonParamsCache; 
    
    // -------------------------------------------------------------------------
    // 보조 인덱스
    // -------------------------------------------------------------------------
    /** WeaponLevel → WeaponID 목록. GetWeaponIDsByLevel() 전용 */
    TMap<int32, TArray<FName>> WeaponByLevelIndex;
    //  StageID → 해당 스테이지의 웨이브 목록
    TMap<FName, TArray<FWaveStaticData>> WaveByStageIndex;
    
};


// =============================================================================
// 템플릿 구현부
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

// MoveData 캐시 특수화 — 특수화된 타입 외 기본 템플릿은 호출되면 컴파일 에러로 방어
template<typename T>
const TMap<FName, T>& UGameDataSubsystem::GetMoveTypeCache() const
{
    static_assert(sizeof(T) == 0, "지원하지 않는 MoveType 타입입니다.");
    static TMap<FName, T> Dummy;
    return Dummy;
}

template<>
inline const TMap<FName, FSkillAttackMoveTypeParamsArc>& UGameDataSubsystem::GetMoveTypeCache() const
{
    return SkillMoveTypeArcCache;
}

template<>
inline const TMap<FName, FSkillAttackMoveTypeParamsHoming>& UGameDataSubsystem::GetMoveTypeCache() const
{
    return SkillMoveTypeHomingCache;
}

template<>
inline const TMap<FName, FSkillAttackMoveTypeParamsSummon>& UGameDataSubsystem::GetMoveTypeCache() const
{
    return SkillMoveTypeSummonCache;
}

template<typename T>
bool UGameDataSubsystem::GetMoveTypeData(FName SkillEffectID, T& OutData) const
{
    const TMap<FName, T>& Cache = GetMoveTypeCache<T>();
    return GetCachedData(Cache, SkillEffectID, OutData, TEXT("MoveType Data"));
}

// MoveData 캐시 특수화 — 특수화된 타입 외 기본 템플릿은 호출되면 컴파일 에러로 방어
template<typename T>
const TMap<FName, T>& UGameDataSubsystem::GetHitTypeCache() const
{
    static_assert(sizeof(T) == 0, "지원하지 않는 HitType 타입입니다.");
    static TMap<FName, T> Dummy;
    return Dummy;
}

template<>
inline const TMap<FName, FSkillAttackHitTypeParamsPierce>& UGameDataSubsystem::GetHitTypeCache() const
{
    return SkillHitTypePierceCache;
}

template<>
inline const TMap<FName, FSkillAttackHitTypeParamsArea>& UGameDataSubsystem::GetHitTypeCache() const
{
    return SkillHitTypeAreaCache;
}

template<typename T>
bool UGameDataSubsystem::GetHitTypeData(FName SkillEffectID, T& OutData) const
{
    const TMap<FName, T>& Cache = GetHitTypeCache<T>();
    return GetCachedData(Cache, SkillEffectID, OutData, TEXT("HitType Data"));
}