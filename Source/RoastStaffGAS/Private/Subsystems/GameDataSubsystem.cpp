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
    WeaponByLevelIndex.Empty();
    
    SkillCommonStaticCache.Empty();
    SkillCommonResourceCache.Empty();
    SkillCommonParamCache.Empty();
    SkillAttackCommonParamsCache.Empty();
    SkillAttackSpawnParamsCache.Empty();
    SkillMoveTypeArcCache.Empty();
    SkillMoveTypeHomingCache.Empty();
    SkillMoveTypeSummonCache.Empty();
    SkillHitTypePierceCache.Empty();
    SkillHitTypeAreaCache.Empty();
    SkillDefenseCommonParamsCache.Empty();
    
    StatusEffectCache.Empty();
    EnemyCache.Empty();
    StageCache.Empty();
    WaveCache.Empty();
    WaveByStageIndex.Empty();

    CharacterSkillCache.Empty();
    PassiveCache.Empty();
    LevelUpCardCache.Empty();

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

    LoadDataTable<FCharacterStaticData>(
        Config->CharacterStaticTable, LoadedCharacterTable, TEXT("DT_CharacterStatic"));   
    LoadDataTable<FWeaponStaticData>(
        Config->WeaponStaticTable,LoadedWeaponTable,TEXT("DT_Weapon"));                                                        //무기 데이터
    LoadDataTable<FSkillCommonStaticData> (
        Config->SkillCommonStaticTable, LoadedSkillCommonStaticTable,TEXT("DT_Skill"));                                        //스킬 정적 데이터
    LoadDataTable<FSkillCommonResourceData>(
        Config->SkillCommonResourceTable, LoadedSkillCommonResourceTable,TEXT("DT_Skill_Common_Resource"));                    //스킬 리소스 데이터
    LoadDataTable<FSkillCommonParamData>(
        Config->SkillCommonParamsTable,LoadedSkillCommonParamsTable, TEXT("DT_Skill_Common_Params"));                          //스킬 공통 파라미터
    LoadDataTable<FSkillAttackCommonParamsData> (
        Config->SkillAttackCommonParamsTable, LoadedSkillAttackCommonParamsTable,TEXT("DT_SkillEffect"));                      //공격 스킬 공통 파라미터
    LoadDataTable<FSkillAttackSpawnParamsData>  (
        Config->SkillAttackSpawnParamsTable, LoadedSkillAttackSpawnParamsTable,TEXT("DT_Spawn"));                              //공격 스킬 소환 파라미터
    LoadDataTable<FSkillAttackMoveTypeParamsArc>   (
        Config->SkillAttackMoveTypeParamsArcTable,   LoadedSkillAttackMoveTypeParamsArcTable,   TEXT("DT_Flight_Arc"));        //궤도형 스킬 데이터
    LoadDataTable<FSkillAttackMoveTypeParamsHoming>(
        Config->SkillAttackMoveTypeParamsHomingTable,LoadedSkillAttackMoveTypeParamsHomingTable, TEXT("DT_MoveType_Homing"));  //유도형 스킬 데이터                             
    LoadDataTable<FSkillAttackMoveTypeParamsSummon>(
        Config->SkillAttackMoveTypeParamsSummonTable, LoadedSkillAttackMoveTypeParamsSummonTable, TEXT("DT_MoveType_Summon")); //소환형 스킬 데이터
    LoadDataTable<FSkillAttackHitTypeParamsPierce>(
        Config->SkillAttackHitTypeParamsPierceTable,LoadedSkillAttackHitTypeParamsPierceTable,TEXT("DT_Flight_Pierce"));       //관통공격 스킬 데이터
    LoadDataTable<FSkillAttackHitTypeParamsArea>(
        Config->SkillAttackHitTypeParamsAreaTable,LoadedSkillAttackHitTypeParamsAreaTable,TEXT("DT_Flight_Explode"));          //범위공격 스킬 데이터
    LoadDataTable<FSkillDefenseCommonParamsData>(
        Config->SkillDefenseCommonParamsTable,LoadedSkillDefenseCommonParamsTable, TEXT("DT_Skill_Defense_Common_Params"));    //방어 스킬 공통 파라미터                       
    LoadDataTable<FStatusEffectData>(
        Config->StatusEffectStaticTable,LoadedStatusEffectTable, TEXT("DT_Status_Effect"));                                    //상태이상 데이터
    LoadDataTable<FEnemyStaticData>(
        Config->EnemyTable,    LoadedEnemyTable,    TEXT("DT_Enemy"));                                                         //적 기본 데이터
    LoadDataTable<FEnemyExtData>(
        Config->EnemyExtTable, LoadedEnemyExtTable, TEXT("DT_EnemyExtData"));                                                  //적 확장 데이터(Ranged/Elite/Boss)
    LoadDataTable<FStageStaticData> (
        Config->StageTable,  LoadedStageTable,  TEXT("DT_Stage"));                                                             //스테이지 스폰 데이터
    LoadDataTable<FWaveStaticData>  (
        Config->WaveTable,   LoadedWaveTable,   TEXT("DT_WaveData"));                                                          //스폰 웨이브 데이터
    LoadDataTable<FCharacterSkillStaticData>(
        Config->CharacterSkillTable, LoadedCharacterSkillTable, TEXT("DT_CharacterSkill"));                                    //캐릭터 고유 스킬
    LoadDataTable<FPassiveStaticData>(
        Config->PassiveTable, LoadedPassiveTable, TEXT("DT_Passive"));                                                         //패시브 스킬 풀
    LoadDataTable<FLevelUpCardStaticData>(
        Config->LevelUpCardTable, LoadedLevelUpCardTable, TEXT("DT_LevelUpCard"));                                             //레벨업 정적 카드 풀
    
    if (!Config->BaseStatCurveTable.IsNull()) //레벨별 스탯 커브 데이터
    {
        LoadedCurveTable = Config->BaseStatCurveTable.LoadSynchronous();
    }

    if (!Config->WeaponDamageCurveTable.IsNull()) //무기 레벨별 데미지 커브
    {
        LoadedWeaponDamageCurveTable = Config->WeaponDamageCurveTable.LoadSynchronous();
        KHS_INFO(TEXT("DT_WeaponDamageCurve 로드 완료"));
    }
}

// -----------------------------------------------------------------------------
// 캐싱
// -----------------------------------------------------------------------------
void UGameDataSubsystem::CacheAllData()
{
    CacheDataTable<FCharacterStaticData>(LoadedCharacterTable, CharacterCache, &FCharacterStaticData::CharacterID, TEXT("DT_CharacterStatic"));
    CacheDataTable<FWeaponStaticData>(LoadedWeaponTable,       WeaponCache,      &FWeaponStaticData::WeaponID,    TEXT("DT_Weapon"));
    CacheDataTable<FSkillCommonStaticData> (LoadedSkillCommonStaticTable,        SkillCommonStaticCache,       &FSkillCommonStaticData::SkillID,      TEXT("DT_Skill_Common_Static"));
    CacheDataTable<FSkillCommonResourceData>(LoadedSkillCommonResourceTable,SkillCommonResourceCache, &FSkillCommonResourceData::SkillID, TEXT("DT_Skill_Common_Resource"));         
    CacheDataTable<FSkillCommonParamData>(LoadedSkillCommonParamsTable,SkillCommonParamCache, &FSkillCommonParamData::SkillEffectID, TEXT("DT_Skill_Common_Params")); 
    CacheDataTable<FSkillAttackCommonParamsData> (LoadedSkillAttackCommonParamsTable,  SkillAttackCommonParamsCache, &FSkillAttackCommonParamsData::SkillEffectID,TEXT("DT_Skill_Attack_Common_Params"));
    CacheDataTable<FSkillAttackSpawnParamsData>  (LoadedSkillAttackSpawnParamsTable,        SkillAttackSpawnParamsCache,       &FSkillAttackSpawnParamsData::SkillEffectID, TEXT("DT_Skill_Attack_Spawn_Param"));
    CacheDataTable<FSkillAttackMoveTypeParamsArc>   (LoadedSkillAttackMoveTypeParamsArcTable,    SkillMoveTypeArcCache,   &FSkillAttackMoveTypeParamsArc::SkillEffectID,  TEXT("DT_Skill_Attack_MoveType_Param_Arc"));
    CacheDataTable<FSkillAttackMoveTypeParamsHoming>(LoadedSkillAttackMoveTypeParamsHomingTable,SkillMoveTypeHomingCache, &FSkillAttackMoveTypeParamsHoming::SkillEffectID, TEXT("DT_Skill_Attack_MoveType_Param_Homing"));
    CacheDataTable<FSkillAttackMoveTypeParamsSummon>(LoadedSkillAttackMoveTypeParamsSummonTable,   SkillMoveTypeSummonCache, &FSkillAttackMoveTypeParamsSummon::SkillEffectID, TEXT("DT_Skill_Attack_MoveType_Param_Summon")); 
    CacheDataTable<FSkillAttackHitTypeParamsPierce>(LoadedSkillAttackHitTypeParamsPierceTable, SkillHitTypePierceCache,&FSkillAttackHitTypeParamsPierce::SkillEffectID,TEXT("DT_Skill_Attack_HitType_Param_Pierce"));
    CacheDataTable<FSkillAttackHitTypeParamsArea>(LoadedSkillAttackHitTypeParamsAreaTable,SkillHitTypeAreaCache,&FSkillAttackHitTypeParamsArea::SkillEffectID,TEXT("DT_Skill_Attack_HitType_Param_Area"));
    CacheDataTable<FSkillDefenseCommonParamsData>(LoadedSkillDefenseCommonParamsTable,SkillDefenseCommonParamsCache, &FSkillDefenseCommonParamsData::SkillEffectID,TEXT("DT_Skill_Defense_Common_Params"));                                                                         
    CacheDataTable<FStatusEffectData>(LoadedStatusEffectTable, StatusEffectCache, &FStatusEffectData::StatusEffectID, TEXT("DT_Status_Effect"));  
    CacheDataTable<FEnemyStaticData>(LoadedEnemyTable, EnemyCache, &FEnemyStaticData::EnemyID, TEXT("DT_Enemy"));
    CacheDataTable<FStageStaticData>(LoadedStageTable, StageCache, &FStageStaticData::StageID, TEXT("DT_Stage"));
    CacheDataTable<FWaveStaticData> (LoadedWaveTable,  WaveCache,  &FWaveStaticData::StageID,  TEXT("DT_WaveData"));
    CacheDataTable<FCharacterSkillStaticData>(LoadedCharacterSkillTable, CharacterSkillCache, &FCharacterSkillStaticData::SkillID, TEXT("DT_CharacterSkill"));
    CacheDataTable<FPassiveStaticData>       (LoadedPassiveTable,        PassiveCache,        &FPassiveStaticData::PassiveID,      TEXT("DT_Passive"));
    CacheDataTable<FLevelUpCardStaticData>   (LoadedLevelUpCardTable,    LevelUpCardCache,    &FLevelUpCardStaticData::CardID,     TEXT("DT_LevelUpCard"));
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
// 캐릭터 조회
// -----------------------------------------------------------------------------
bool UGameDataSubsystem::GetCharacterStaticData(FName CharID, FCharacterStaticData& OutData) const
{
    return GetCachedData(CharacterCache, CharID, OutData, TEXT("FCharacterStaticData"));
}

bool UGameDataSubsystem::GetAllCharacterStaticData(TArray<FCharacterStaticData>& OutArray) const
{
    if (!bIsDataReady || CharacterCache.IsEmpty())
    {
        KHS_WARN(TEXT("캐릭터 캐시 미로드 또는 비어있음."));
        return false;
    }

    CharacterCache.GenerateValueArray(OutArray);
    return true;
}

// -----------------------------------------------------------------------------
// 무기 조회
// -----------------------------------------------------------------------------
bool UGameDataSubsystem::GetWeaponData(FName WeaponID, FWeaponStaticData& OutData) const
{
    return GetCachedData(WeaponCache, WeaponID, OutData, TEXT("FWeaponStaticData"));
}

float UGameDataSubsystem::GetWeaponDamageFromCurve(FName WeaponID, int32 WeaponLevel) const
{
    if (!LoadedWeaponDamageCurveTable)
    {
        KHS_WARN(TEXT("WeaponDamageCurveTable is null — WeaponID: %s"), *WeaponID.ToString());
        return 0.f;
    }

    FWeaponStaticData WeaponData;
    if (!GetWeaponData(WeaponID, WeaponData))
    {
        KHS_WARN(TEXT("GetWeaponDamageFromCurve — WeaponData 조회 실패. WeaponID: %s"), *WeaponID.ToString());
        return 0.f;
    }

    const UEnum* EnumPtr = StaticEnum<EWeaponBaseType>();
    const FString BaseTypeName = EnumPtr->GetDisplayValueAsText(WeaponData.BaseType).ToString();
    const FRealCurve* Curve = LoadedWeaponDamageCurveTable->FindCurve(FName(*BaseTypeName), TEXT(""));
    if (!Curve)
    {
        KHS_WARN(TEXT("CurveTable 행 조회 실패 — RowName: %s"), *BaseTypeName);
        return 0.f;
    }

    return Curve->Eval(static_cast<float>(WeaponLevel));
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
bool UGameDataSubsystem::GetSkillStaticData(FName SkillID, FSkillCommonStaticData& OutData) const
{
    return GetCachedData(SkillCommonStaticCache, SkillID, OutData, TEXT("FSkillCommonStaticData"));
}

bool UGameDataSubsystem::GetSkillResourceData(FName SkillID, FSkillCommonResourceData& OutData) const
{
    return GetCachedData(SkillCommonResourceCache, SkillID, OutData, TEXT("FSkillCommonResourceData"));
}

bool UGameDataSubsystem::GetSkillCommonParamData(FName SkillEffectID, FSkillCommonParamData& OutData) const
{
    return GetCachedData(SkillCommonParamCache, SkillEffectID, OutData, TEXT("FSkillCommonParamData")); 
}

bool UGameDataSubsystem::GetSkillAttackCommonParamsData(FName SkillEffectID, FSkillAttackCommonParamsData& OutData) const
{
    return GetCachedData(SkillAttackCommonParamsCache, SkillEffectID, OutData, TEXT("FSkillAttackCommonParamsData"));
}

bool UGameDataSubsystem::GetSkillAttackSpawnParamsData(FName SkillEffectID, FSkillAttackSpawnParamsData& OutData) const
{
    return GetCachedData(SkillAttackSpawnParamsCache, SkillEffectID, OutData, TEXT("FSkillAttackSpawnParamsData"));
}

bool UGameDataSubsystem::GetSkillDefenseCommonParamsData(FName SkillEffectID,
    FSkillDefenseCommonParamsData& OutData) const
{
    return GetCachedData(SkillDefenseCommonParamsCache, SkillEffectID, OutData, TEXT("FSkillDefenseCommonParamsData"));  
}

bool UGameDataSubsystem::GetStatusEffectData(FName StatusEffectID, FStatusEffectData& OutData) const
{
    return GetCachedData(StatusEffectCache, StatusEffectID, OutData, TEXT("FStatusEffectData"));  
}

// -----------------------------------------------------------------------------
// 에너미 조회
// -----------------------------------------------------------------------------

bool UGameDataSubsystem::GetEnemyData(FName EnemyID, FEnemyStaticData& OutData) const
{
    return GetCachedData(EnemyCache, EnemyID, OutData, TEXT("FEnemyStaticData"));
}

bool UGameDataSubsystem::GetEnemyExtData(FName EnemyID, FEnemyExtData& OutData) const
{
    if (!LoadedEnemyExtTable)
    {
        KHS_WARN(TEXT("GetEnemyExtData — LoadedEnemyExtTable null. GDA에서 EnemyExtTable 할당 필요."));
        return false;
    }

    if (FEnemyExtData* Row = LoadedEnemyExtTable->FindRow<FEnemyExtData>(EnemyID, TEXT("")))
    {
        OutData = *Row;
        return true;
    }

    KHS_WARN(TEXT("EnemyID '%s' 행 없음. DT_EnemyExtData 확인 필요."), *EnemyID.ToString());
    return false;
}

// -----------------------------------------------------------------------------
// 스테이지 / 웨이브 조회
// -----------------------------------------------------------------------------

bool UGameDataSubsystem::GetStageData(FName StageID, FStageStaticData& OutData) const
{
    return GetCachedData(StageCache, StageID, OutData, TEXT("FStageStaticData"));
}

bool UGameDataSubsystem::GetAllStageStaticData(TArray<FStageStaticData>& OutArray) const
{
    if (!bIsDataReady || StageCache.IsEmpty())
    {
        KHS_WARN(TEXT("스테이지 캐시 미로드 또는 비어있음."));
        return false;
    }

    StageCache.GenerateValueArray(OutArray);
    return true;
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
bool UGameDataSubsystem::GetWeaponSlotEquipData(FName WeaponID, FWeaponSlotEquipData& OutData) const
{
    // DT_Weapon 조회
    FWeaponStaticData WeaponData;
    if (!GetWeaponData(WeaponID, WeaponData))
    {
        KHS_WARN(TEXT("DT_Weapon 조회 실패. WeaponID: %s"), *WeaponID.ToString());
        return false;
    }

    // Skill Common Static 조회
    FSkillCommonStaticData SkillStaticData;
    if (!GetSkillStaticData(WeaponData.SkillID, SkillStaticData))
    {
        KHS_WARN(TEXT("DT_Skill 조회 실패. SkillID: %s"), *WeaponData.SkillID.ToString());
        return false;
    }

    //Skill Common Resource 조회
    FSkillCommonResourceData ResourceData;
    if (!GetSkillResourceData(WeaponData.SkillID, ResourceData))                                                 
    {                                                                                                          
        KHS_WARN(TEXT("SkillResource 조회 실패. SkillID: %s"), *WeaponData.SkillID.ToString());                                                                                 
        return false;
    }    
    
    //Skill Common Param 조회
    FSkillCommonParamData CommonParamData;                                                                     
    if (!GetSkillCommonParamData(SkillStaticData.SkillEffectID, CommonParamData))                                
    {                                                                                                            
        KHS_WARN(TEXT("SkillCommonParam 조회 실패. SkillEffectID: %s"),*SkillStaticData.SkillEffectID.ToString());                                                                      
        return false;                                                                                          
    }
    
    //FWeaponSlotEquipData 레퍼런스 채우기
    OutData.WeaponID           = WeaponID;
    OutData.WeaponName         = WeaponData.WeaponName;   
    OutData.SkillID            = WeaponData.SkillID;
    OutData.SkillEffectID      = SkillStaticData.SkillEffectID;
    OutData.Cooldown           = SkillStaticData.Cooldown;
    OutData.SkillIcon          = ResourceData.SkillIcon;
    OutData.GAClass            = ResourceData.GAClass;
    OutData.SkillType          = CommonParamData.SkillType;   

    // ATTACK 스킬일 경우 AttackCommonParams에서 MoveType 조회 (DEFENSE는 해당 테이블 없음)                               
    if (CommonParamData.SkillType == ESkillType::ATTACK)                                                             
    {                                                                                                                
        FSkillAttackCommonParamsData AttackParamData;                                                                
        if (!GetSkillAttackCommonParamsData(SkillStaticData.SkillEffectID, AttackParamData))                       
        {                                                                                                            
            KHS_WARN(TEXT("AttackCommonParam 조회 실패. SkillEffectID: %s"), *SkillStaticData.SkillEffectID.ToString());                                                                      
            return false;                                                                                            
        }                                                                                                          
        OutData.MoveType = AttackParamData.MoveType;                                                                 
    }   
    
    return true;
}

bool UGameDataSubsystem::GetSkillExecutionData(FName SkillID, FSkillExecutionData& OutData, FName WeaponID) const
{
    FSkillCommonStaticData SkillStaticData;                                                                      
    if (!GetSkillStaticData(SkillID, SkillStaticData))                                                         
    {
        KHS_WARN(TEXT("DT_Skill_Common_Static 조회 실패. SkillID: %s"), *SkillID.ToString());
        return false;                                                                                            
    }
                                                                                                                 
    FSkillCommonResourceData ResourceData;                                                                     
    if (!GetSkillResourceData(SkillID, ResourceData))
    {                                                                                                            
        KHS_WARN(TEXT("DT_Skill_Common_Resource 조회 실패. SkillID: %s"), *SkillID.ToString());                  
        return false;                                                                                            
    }                                                                                                            
                                                                                                               
    const FName SkillEffectID = SkillStaticData.SkillEffectID;                                                   
 
    FSkillCommonParamData CommonParamData;                                                                       
    if (!GetSkillCommonParamData(SkillEffectID, CommonParamData))                                              
    {                                                                                                            
        KHS_WARN(TEXT("DT_Skill_Common_Param 조회 실패. SkillEffectID: %s"), *SkillEffectID.ToString());
        return false;                                                                                            
    }                                                                                                          
                                                                                                                 
    FSkillAttackCommonParamsData AttackParamData;                                                                
    if (!GetSkillAttackCommonParamsData(SkillEffectID, AttackParamData))                                         
    {                                                                                                            
        KHS_WARN(TEXT("DT_Skill_Attack_Common_Param 조회 실패. SkillEffectID: %s"), *SkillEffectID.ToString());
        return false;                                                                                            
    }
                                                                                                                 
    FSkillAttackSpawnParamsData SpawnParamData;                                                                  
    if (!GetSkillAttackSpawnParamsData(SkillEffectID, SpawnParamData))
    {                                                                                                            
        KHS_WARN(TEXT("DT_Skill_Attack_Spawn_Param 조회 실패. SkillEffectID: %s"), *SkillEffectID.ToString()); 
        return false;                                                                                            
    }
                                                                                                                 
    OutData.SkillID            = SkillID;                                                                      
    OutData.SkillEffectID      = SkillEffectID;
  
    OutData.DamageGEClass      = ResourceData.DamageGEClass;                                                     
    OutData.StatusGEClass      = ResourceData.StatusGEClass;
    OutData.ProjectileClass    = ResourceData.ProjectileClass;                                                   
    OutData.SummonObjectClass  = ResourceData.SummonObjectClass;                                                 
    OutData.SummonPreviewClass = ResourceData.SummonPreviewClass;
  
    OutData.SkillType          = CommonParamData.SkillType;                                                      
    OutData.Lifetime           = CommonParamData.Lifetime;                                                     
    OutData.Range              = CommonParamData.Range;        
  
    OutData.MoveType           = AttackParamData.MoveType;                                                       
    OutData.HitType            = AttackParamData.HitType;                                                        
    OutData.SpawnPattern       = AttackParamData.SpawnType; 
    OutData.Speed              = AttackParamData.Speed;       
  
    OutData.SpawnCount         = SpawnParamData.SpawnCount;                                                      
    OutData.SocketName         = SpawnParamData.SocketName;                                                      
    OutData.SpreadAngle        = SpawnParamData.SpreadAngle;
    
    // WeaponID 전달 시 DT에서 WeaponLevel 파생 → CurveTable 조회
    if (WeaponID != NAME_None)
    {
        FWeaponStaticData WeaponData;
        if (GetWeaponData(WeaponID, WeaponData) && WeaponData.WeaponLevel > 0)
        {
            OutData.Amount = GetWeaponDamageFromCurve(WeaponID, WeaponData.WeaponLevel);
        }
        else
        {
            OutData.Amount = AttackParamData.Amount;
        }
    }
    else
    {
        OutData.Amount = AttackParamData.Amount;
    }
                                                                                                                   
    return true;                                            
}

bool UGameDataSubsystem::GetSkillFXData(FName SkillID, FSkillFXData& OutData) const
{
    FSkillCommonResourceData ResourceData;                                                                       
    if (!GetSkillResourceData(SkillID, ResourceData))                                                          
    {                                                                                                            
        KHS_WARN(TEXT("DT_Skill_Common_Resource 조회 실패 (FX). SkillID: %s"), *SkillID.ToString());
        return false;                                                                                            
    }                                                                                                          
                                                                                                                   
    OutData.SpawnVFX  = ResourceData.SpawnVFX;
    OutData.SpawnSFX  = ResourceData.SpawnSFX;
    OutData.TrailVFX  = ResourceData.TrailVFX;
    OutData.ImpactVFX = ResourceData.ImpactVFX;
    OutData.ImpactSFX = ResourceData.ImpactSFX;

    return true;
}

// -----------------------------------------------------------------------------
// 프리로드 번들 조회
// -----------------------------------------------------------------------------
bool UGameDataSubsystem::GetCharacterPreloadBundle(FName CharID, FCharacterPreloadBundle& OutBundle) const
{
    FCharacterStaticData Data;
    if (!GetCharacterStaticData(CharID, Data))
    {
        return false;
    }

    OutBundle.Mesh   = Data.Mesh;
    OutBundle.AnimBP = Data.AnimBP;

    for (const FCharacterSkillStaticData& Skill : GetSkillsByCharacter(CharID))
    {
        if (!Skill.SkillFX.IsNull())
        {
            OutBundle.SkillFXList.AddUnique(Skill.SkillFX);
        }
    }

    return true;
}

bool UGameDataSubsystem::GetEnemyPreloadBundle(FName EnemyID, FEnemyPreloadBundle& OutBundle) const
{
    FEnemyStaticData Data;
    if (!GetEnemyData(EnemyID, Data))
    {
        return false;
    }

    OutBundle.EnemyClass   = Data.EnemyClass;
    OutBundle.BehaviorTree = Data.BehaviorTree;
    return true;
}

// -----------------------------------------------------------------------------
// 캐릭터 고유 스킬 조회
// -----------------------------------------------------------------------------
bool UGameDataSubsystem::GetCharacterSkillExecData(FName CharacterID, int32 SkillSlot, FCharacterSkillExecData& OutData) const
{
    if (!bIsDataReady)
    {
        KHS_WARN(TEXT("GDS 초기화 미완료 — CharacterID: %s"), *CharacterID.ToString());
        return false;
    }

    const FCharacterSkillStaticData* Found = nullptr;
    for (const auto& Pair : CharacterSkillCache)
    {
        if (Pair.Value.OwnerCharacterID == CharacterID && Pair.Value.SkillSlot == SkillSlot)
        {
            Found = &Pair.Value;
            break;
        }
    }

    if (!Found)
    {
        KHS_WARN(TEXT("CharacterSkill 조회 실패 — CharacterID: %s, Slot: %d"), *CharacterID.ToString(), SkillSlot);
        return false;
    }

    OutData.SkillID           = Found->SkillID;
    OutData.Cooldown          = Found->Cooldown;
    OutData.TargetingType     = Found->TargetingType;
    OutData.EffectType        = Found->EffectType;
    OutData.ProjectileMoveType = Found->ProjectileMoveType;
    OutData.SpawnPattern      = Found->SpawnPattern;
    OutData.SpawnCount        = FMath::Max(1, Found->SpawnCount);
    OutData.DamageMultiplier  = Found->DamageMultiplier;
    OutData.EffectRadius      = Found->EffectRadius;
    OutData.Duration          = Found->Duration;
    OutData.ProjectileSpeed   = Found->ProjectileSpeed;
    OutData.ProjectileRange   = Found->ProjectileRange;
    OutData.FireInterval      = Found->FireInterval;
    OutData.PierceCount       = Found->PierceCount;
    OutData.DamageDecay       = Found->DamageDecay;
    OutData.ElementTag        = Found->ElementTag;
    OutData.GAClass           = Found->GAClass;
    OutData.SkillGEClass      = Found->SkillGEClass;
    OutData.StatusGEClass     = Found->StatusGEClass;
    OutData.PreviewActorClass = Found->PreviewActorClass;
    OutData.EffectActorClass  = Found->EffectActorClass;
    OutData.ProjectileClass   = Found->ProjectileClass;
    OutData.SkillFX           = Found->SkillFX;
    OutData.SkillIconSoftRef  = Found->SkillIcon;

    return true;
}

TArray<FCharacterSkillStaticData> UGameDataSubsystem::GetSkillsByCharacter(FName CharacterID) const
{
    TArray<FCharacterSkillStaticData> Result;

    if (!bIsDataReady)
    {
        KHS_WARN(TEXT("GDS 초기화 미완료 — GetSkillsByCharacter CharacterID: %s"), *CharacterID.ToString());
        return Result;
    }

    for (const auto& Pair : CharacterSkillCache)
    {
        if (Pair.Value.OwnerCharacterID == CharacterID)
        {
            Result.Add(Pair.Value);
        }
    }

    Result.Sort([](const FCharacterSkillStaticData& A, const FCharacterSkillStaticData& B)
    {
        return A.SkillSlot < B.SkillSlot;
    });

    return Result;
}

// -----------------------------------------------------------------------------
// 패시브 조회
// -----------------------------------------------------------------------------
bool UGameDataSubsystem::GetPassiveData(FName PassiveID, FPassiveStaticData& OutData) const
{
    return GetCachedData(PassiveCache, PassiveID, OutData, TEXT("FPassiveStaticData"));
}

TArray<FPassiveStaticData> UGameDataSubsystem::GetAllPassives() const
{
    TArray<FPassiveStaticData> Result;

    if (!bIsDataReady || PassiveCache.IsEmpty())
    {
        KHS_WARN(TEXT("패시브 캐시 미로드 또는 비어있음."));
        return Result;
    }

    PassiveCache.GenerateValueArray(Result);
    return Result;
}

// -----------------------------------------------------------------------------
// 레벨업 카드 조회
// -----------------------------------------------------------------------------
bool UGameDataSubsystem::GetLevelUpCardData(FName CardID, FLevelUpCardStaticData& OutData) const
{
    return GetCachedData(LevelUpCardCache, CardID, OutData, TEXT("FLevelUpCardStaticData"));
}

TArray<FLevelUpCardStaticData> UGameDataSubsystem::GetAllLevelUpCards() const
{
    TArray<FLevelUpCardStaticData> Result;

    if (!bIsDataReady || LevelUpCardCache.IsEmpty())
    {
        KHS_WARN(TEXT("레벨업 카드 캐시 미로드 또는 비어있음."));
        return Result;
    }

    LevelUpCardCache.GenerateValueArray(Result);
    return Result;
}
