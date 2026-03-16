// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/GameDataSubsystem.h"
#include "Data/GameDataConfig.h"

void UGameDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LoadDataTables();
    CacheAllData();
    BuildSecondaryIndex();

    bIsDataReady = true;
    KHS_INFO(TEXT("GDS 초기화 완료"));
}

void UGameDataSubsystem::Deinitialize()
{
    WeaponCache.Empty();
    SkillCache.Empty();
    SkillEffectCache.Empty();
    SpawnCache.Empty();
    FlightArcCache.Empty();
    FlightPierceCache.Empty();
    FlightExplodeCache.Empty();
    WeaponByLevelIndex.Empty();
    EnemyCache.Empty();
    StageCache.Empty();
    WaveCache.Empty();
    WaveByStageIndex.Empty();

    bIsDataReady = false;
    Super::Deinitialize();
}

// -----------------------------------------------------------------------------
// 로드
// -----------------------------------------------------------------------------
void UGameDataSubsystem::LoadDataTables()
{
    UGameDataConfig* Config = LoadObject<UGameDataConfig>(
        nullptr,
        TEXT("/Script/RoastStaffGAS.GameDataConfig'/Game/Data/GA_DataConfig.GA_DataConfig'")
    );

    if (!ensureMsgf(Config, TEXT("GameDataConfig 에셋 로드 실패. 에디터에서 경로 재확인.")))
    {
        return;
    }

    LoadDataTable<FWeaponStaticData>(Config->WeaponTable,      LoadedWeaponTable,      TEXT("DT_Weapon"));              //무기 데이터
    LoadDataTable<FSkillStaticData> (Config->SkillTable,       LoadedSkillTable,       TEXT("DT_Skill"));               //스킬 데이터
    LoadDataTable<FSkillEffectData> (Config->SkillEffectTable, LoadedSkillEffectTable, TEXT("DT_SkillEffect"));         //스킬 효과 데이터
    LoadDataTable<FSkillSpawnData>  (Config->SpawnTable,       LoadedSpawnTable,       TEXT("DT_Spawn"));               //스킬 소환 데이터
    LoadDataTable<FFlightArcData>   (Config->FlightArcTable,   LoadedFlightArcTable,   TEXT("DT_Flight_Arc"));          //궤도형 투사체 데이터
    LoadDataTable<FFlightPierceData>(Config->FlightPierceTable,LoadedFlightPierceTable,TEXT("DT_Flight_Pierce"));       //관통형 투사체 데이터
    LoadDataTable<FFlightExplodeData>(Config->FlightExplodeTable,LoadedFlightExplodeTable,TEXT("DT_Flight_Explode"));   //폭발형 투사체 데이터
    LoadDataTable<FEnemyStaticData> (Config->EnemyTable,  LoadedEnemyTable,  TEXT("DT_Enemy"));                         //적 데이터
    LoadDataTable<FStageStaticData> (Config->StageTable,  LoadedStageTable,  TEXT("DT_Stage"));                         //스테이지 스폰 데이터
    LoadDataTable<FWaveStaticData>  (Config->WaveTable,   LoadedWaveTable,   TEXT("DT_WaveData"));                      //스폰 웨이브 데이터
    
    if (!Config->BaseStatCurveTable.IsNull()) //레벨별 스탯 커브 데이터
    {
        LoadedCurveTable = Config->BaseStatCurveTable.LoadSynchronous();
    }
}

// -----------------------------------------------------------------------------
// 캐싱
// -----------------------------------------------------------------------------
void UGameDataSubsystem::CacheAllData()
{
    CacheDataTable<FWeaponStaticData>(LoadedWeaponTable,       WeaponCache,      &FWeaponStaticData::WeaponID,    TEXT("DT_Weapon"));
    CacheDataTable<FSkillStaticData> (LoadedSkillTable,        SkillCache,       &FSkillStaticData::SkillID,      TEXT("DT_Skill"));
    CacheDataTable<FSkillEffectData> (LoadedSkillEffectTable,  SkillEffectCache, &FSkillEffectData::SkillEffectID,TEXT("DT_SkillEffect"));
    CacheDataTable<FSkillSpawnData>  (LoadedSpawnTable,        SpawnCache,       &FSkillSpawnData::SkillEffectID, TEXT("DT_Spawn"));
    CacheDataTable<FFlightArcData>   (LoadedFlightArcTable,    FlightArcCache,   &FFlightArcData::SkillEffectID,  TEXT("DT_Flight_Arc"));
    CacheDataTable<FFlightPierceData>(LoadedFlightPierceTable, FlightPierceCache,&FFlightPierceData::SkillEffectID,TEXT("DT_Flight_Pierce"));
    CacheDataTable<FFlightExplodeData>(LoadedFlightExplodeTable,FlightExplodeCache,&FFlightExplodeData::SkillEffectID,TEXT("DT_Flight_Explode"));
    CacheDataTable<FEnemyStaticData>(LoadedEnemyTable, EnemyCache, &FEnemyStaticData::EnemyID, TEXT("DT_Enemy"));
    CacheDataTable<FStageStaticData>(LoadedStageTable, StageCache, &FStageStaticData::StageID, TEXT("DT_Stage"));
    CacheDataTable<FWaveStaticData> (LoadedWaveTable,  WaveCache,  &FWaveStaticData::StageID,  TEXT("DT_WaveData"));
}

// -----------------------------------------------------------------------------
// 보조 인덱스 구성
// -----------------------------------------------------------------------------
void UGameDataSubsystem::BuildSecondaryIndex()
{
    WeaponByLevelIndex.Empty();

    for (const auto& Pair : WeaponCache)
    {
        int32 Level = Pair.Value.WeaponLevel;
        WeaponByLevelIndex.FindOrAdd(Level).Add(Pair.Key);
    }
    
    KHS_INFO(TEXT("WeaponByLevel 보조 인덱스 구성 완료 (%d개 레벨)"),WeaponByLevelIndex.Num());
    
    // WaveByStage 보조 인덱스 구성
    WaveByStageIndex.Empty();
    for (const FName& RowName : LoadedWaveTable->GetRowNames())
    {
        FWaveStaticData* Row = LoadedWaveTable->FindRow<FWaveStaticData>(RowName, TEXT(""));
        if (!Row)
        {
            KHS_WARN(TEXT("CANNOT FIND ROWNAME"));
            continue;
        }
        WaveByStageIndex.FindOrAdd(Row->StageID).Add(*Row);
    }

    // 각 스테이지의 웨이브 목록을 WaveIndex 오름차순 정렬
    for (auto& Pair : WaveByStageIndex)
    {
        Pair.Value.Sort([](const FWaveStaticData& A, const FWaveStaticData& B)
        {
            return A.WaveIndex < B.WaveIndex;
        });
    }

    KHS_INFO(TEXT("WaveByStage 보조 인덱스 구성 완료 (%d개 스테이지)"), WaveByStageIndex.Num());
}

bool UGameDataSubsystem::GetLevelCurveValue(FName CurveName, int32 Level, float& OutValue) const
{
    if (!LoadedCurveTable)
    {
        KHS_WARN(TEXT("CurveTable is null"));
        return false;
    }

    const FRealCurve* Curve = LoadedCurveTable->FindCurve(CurveName, TEXT(""));
    if (!Curve)
    {
        KHS_WARN(TEXT("커브 조회 실패 — CurveName: %s"), *CurveName.ToString());
        return false;
    }

    OutValue = Curve->Eval(static_cast<float>(Level));
    return true;
}

// -----------------------------------------------------------------------------
// 무기 조회
// -----------------------------------------------------------------------------
bool UGameDataSubsystem::GetWeaponData(FName WeaponID, FWeaponStaticData& OutData) const
{
    return GetCachedData(WeaponCache, WeaponID, OutData, TEXT("FWeaponStaticData"));
}

TArray<FName> UGameDataSubsystem::GetWeaponIDsByLevel(int32 WeaponLevel) const
{
    if (!bIsDataReady)
    {
        KHS_WARN(TEXT("GDS 초기화 미완료 상태에서 호출"));
        return {};
    }

    const TArray<FName>* Found = WeaponByLevelIndex.Find(WeaponLevel);
    if (!Found)
    {
        KHS_WARN(TEXT("WeaponLevel %d에 해당하는 무기가 없습니다."), WeaponLevel);
        return {};
    }

    return *Found;
}

// -----------------------------------------------------------------------------
// 스킬 조회
// -----------------------------------------------------------------------------
bool UGameDataSubsystem::GetSkillData(FName SkillID, FSkillStaticData& OutData) const
{
    return GetCachedData(SkillCache, SkillID, OutData, TEXT("FSkillStaticData"));
}

bool UGameDataSubsystem::GetSkillEffectData(FName SkillEffectID, FSkillEffectData& OutData) const
{
    return GetCachedData(SkillEffectCache, SkillEffectID, OutData, TEXT("FSkillEffectData"));
}

bool UGameDataSubsystem::GetSkillSpawnData(FName SkillEffectID, FSkillSpawnData& OutData) const
{
    return GetCachedData(SpawnCache, SkillEffectID, OutData, TEXT("FSkillSpawnData"));
}

// -----------------------------------------------------------------------------
// 에너미 조회
// -----------------------------------------------------------------------------

bool UGameDataSubsystem::GetEnemyData(FName EnemyID, FEnemyStaticData& OutData) const
{
    return GetCachedData(EnemyCache, EnemyID, OutData, TEXT("FEnemyStaticData"));
}

// -----------------------------------------------------------------------------
// 스테이지 / 웨이브 조회
// -----------------------------------------------------------------------------

bool UGameDataSubsystem::GetStageData(FName StageID, FStageStaticData& OutData) const
{
    return GetCachedData(StageCache, StageID, OutData, TEXT("FStageStaticData"));
}

TArray<FWaveStaticData> UGameDataSubsystem::GetWaveDataByStage(FName StageID) const
{
    if (!bIsDataReady)
    {
        KHS_WARN(TEXT("GDS 초기화 미완료 상태에서 웨이브 조회 요청 — StageID: %s"), *StageID.ToString());
        return {};
    }

    const TArray<FWaveStaticData>* Found = WaveByStageIndex.Find(StageID);
    if (!Found)
    {
        KHS_WARN(TEXT("웨이브 데이터 조회 실패 — StageID: %s"), *StageID.ToString());
        return {};
    }

    return *Found;
}

// -----------------------------------------------------------------------------
// 복합 조회
// -----------------------------------------------------------------------------
bool UGameDataSubsystem::GetWeaponEquipData(FName WeaponID, FWeaponEquipData& OutData) const
{
    // DT_Weapon 조회
    FWeaponStaticData WeaponData;
    if (!GetWeaponData(WeaponID, WeaponData))
    {
        KHS_WARN(TEXT("GetWeaponEquipData 실패 — DT_Weapon 조회 실패. WeaponID: %s"), *WeaponID.ToString());
        return false;
    }

    // DT_Skill 조회
    FSkillStaticData SkillData;
    if (!GetSkillData(WeaponData.SkillID, SkillData))
    {
        KHS_WARN(TEXT("GetWeaponEquipData 실패 — DT_Skill 조회 실패. SkillID: %s"), *WeaponData.SkillID.ToString());
        return false;
    }

    //DT_SkillEffect 조회
    FSkillEffectData EffectData;
    if (!GetSkillEffectData(SkillData.SkillEffectID, EffectData))
    {
        KHS_WARN(TEXT("GetWeaponEquipData 실패 — DT_SkillEffect 조회 실패. SkillEffectID: %s"), *SkillData.SkillEffectID.ToString());
        return false;
    }

    //FWeaponEquipData 레퍼런스 반환
    OutData.WeaponID        = WeaponID;
    OutData.SkillID         = WeaponData.SkillID;
    OutData.SkillEffectID   = SkillData.SkillEffectID;
    OutData.WeaponName      = WeaponData.WeaponName;
    OutData.SlotType        = WeaponData.SlotType;          
    OutData.GAClass         = SkillData.GAClass;
    OutData.ProjectileClass = SkillData.ProjectileClass;
    OutData.DamageGEClass   = SkillData.DamageGEClass;
    OutData.StatusGEClass   = SkillData.StatusGEClass;
    OutData.Cooldown        = EffectData.Cooldown;
    OutData.SkillIcon       = SkillData.SkillIcon;
    OutData.Damage          = EffectData.Damage;            
    OutData.Speed           = EffectData.Speed;             
    OutData.Lifetime        = EffectData.Lifetime;          
    OutData.SkillType       = EffectData.SkillType;
    OutData.FlightType      = EffectData.FlightType;        
    OutData.HitType         = EffectData.HitType;           
    OutData.ExpireCondition = EffectData.ExpireCondition;   

    return true;
}