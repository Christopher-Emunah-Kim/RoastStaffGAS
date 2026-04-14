"""Append dummy rows to Passive and LevelUpCard CSVs. Run once."""

PSV_NS  = 'DT_Passive_Static_Data [F3B442BA2F60ABED6685D074489782CB]'
CARD_NS = 'DT_LevelUp_Card_Static_Data [1DD81BF0568E6373CEDAB02F9F63BD50]'
PSV_FILE  = 'DT_Passive_Static_Data.csv'
CARD_FILE = 'DT_LevelUp_Card_Static_Data.csv'

GE = {
    'ATK':       '/Game/GAS/GE/Skill/Buff/GE_Passive_Buff_ATK.GE_Passive_Buff_ATK_C',
    'DEF':       '/Game/GAS/GE/Skill/Buff/GE_Passive_Buff_DEF.GE_Passive_Buff_DEF_C',
    'MoveSpeed': '/Game/GAS/GE/Skill/Buff/GE_Passive_Buff_MoveSpeed.GE_Passive_Buff_MoveSpeed_C',
    'CritRate':  '/Game/GAS/GE/Skill/Buff/GE_Passive_Buff_CritRate.GE_Passive_Buff_CritRate_C',
    'HP':        '/Game/GAS/GE/Skill/Buff/GE_Passive_Buff_HP.GE_Passive_Buff_HP_C',
}
MAG = {5:'1.050000', 10:'1.100000', 15:'1.150000', 20:'1.200000'}

# Base icons per type (user will reassign in editor)
ICO = {
    'FIRE':    '/Game/Assets/UIAsset/rpg-splash-game/3.3',
    'ICE':     '/Game/Assets/UIAsset/rpg-currency-game/background/1.1',
    'THUNDER': '/Game/Assets/UIAsset/cyberpunk-medicine-icons/without_background/1.1',
    'SHADOW':  '/Game/Assets/UIAsset/rpg-warlock-skill-icons/1.1',
    'POISON':  '/Game/Assets/UIAsset/futuristic-plant-icon/without_background/3.3',
}
ETAG = {'FIRE':'Fireball','ICE':'Iceball','THUNDER':'Thunder','SHADOW':'Shadow','POISON':'Poison'}

DN = {
    'FIRE':    '화염의 오라',
    'ICE':     '얼음 갑주',
    'THUNDER': '번개의 발',
    'SHADOW':  '그림자 장막',
    'POISON':  '독의 심장',
}
DESC = {
    'FIRE':    '공격력을 {}% 증가시킨다',
    'ICE':     '방어력을 {}% 증가시킨다',
    'THUNDER': '이동 속도를 {}% 증가시킨다',
    'SHADOW':  '치명타 확률을 {}% 증가시킨다',
    'POISON':  '체력을 {}% 증가시킨다',
}

def N(ns, key, text):
    return f'NSLOCTEXT(""{ns}"", ""{key}"", ""{text}"")'

def psv(key, t, pct, ico=None, weight='1.000000'):
    ico = ico or ICO[t]
    dn   = N(PSV_NS, f'{key}_DisplayName', f'{DN[t]}')
    desc = N(PSV_NS, f'{key}_Description',  DESC[t].format(pct))
    return f'{key},"{key}","{dn}","{desc}","{GE[{"FIRE":"ATK","ICE":"DEF","THUNDER":"MoveSpeed","SHADOW":"CritRate","POISON":"HP"}[t]]}","{ico}","{ETAG[t]}","{MAG[pct]}","{weight}"'

def stat_card(key, stat, mod_f, disp, dn_txt, desc_txt, ico, weight='1.000000'):
    dn   = N(CARD_NS, f'{key}_DisplayName', dn_txt)
    desc = N(CARD_NS, f'{key}_Description', desc_txt.format(disp))
    return f'{key},"{key}","StatUpgrade","{stat}","{mod_f}","None","{weight}","{dn}","{desc}","{ico}"'

def pass_card(key, psv_id, dn_txt, weight='0.800000'):
    dn = N(CARD_NS, f'{key}_DisplayName', dn_txt)
    return f'{key},"{key}","PassiveAdd","None","0.000000","{psv_id}","{weight}","{dn}","","None"'

# ── New Passive rows ──────────────────────────────────────────────────────────
psv_rows = []
psv_rows.append(psv('PSV_FIRE_AURA_4',  'FIRE',   20))
for i,p in enumerate([5,10,15,20],1): psv_rows.append(psv(f'PSV_ICE_ARMOR_{i}',    'ICE',    p))
for i,p in enumerate([5,10,15,20],1): psv_rows.append(psv(f'PSV_THUNDER_FEET_{i}', 'THUNDER',p, weight='1.000000'))
for i,p in enumerate([5,10,15,20],1): psv_rows.append(psv(f'PSV_SHADOW_VEIL_{i}',  'SHADOW', p, weight='0.800000'))
for i,p in enumerate([5,10,15,20],1): psv_rows.append(psv(f'PSV_POISON_HEART_{i}', 'POISON', p))

# ── New Card rows ─────────────────────────────────────────────────────────────
card_rows = []

# StatUpgrade completions
card_rows.append(stat_card('CARD_DEF_UP_3',      'DEF',          '15.000000','15','방어력 강화','방어력을 {} 증가시킨다','/Game/Assets/UIAsset/rpg-warlock-skill-icons/4.4'))
card_rows.append(stat_card('CARD_DEF_UP_4',      'DEF',          '20.000000','20','방어력 강화','방어력을 {} 증가시킨다','/Game/Assets/UIAsset/rpg-currency-game/background/2.2'))
card_rows.append(stat_card('CARD_HP_UP_3',       'MaxHP',        '30.000000','30','최대 체력 강화','최대 체력을 {} 증가시킨다','/Game/Assets/UIAsset/cyberpunk-gloves-and-cloak/without_background/3.3'))
card_rows.append(stat_card('CARD_HP_UP_4',       'MaxHP',        '40.000000','40','최대 체력 강화','최대 체력을 {} 증가시킨다','/Game/Assets/UIAsset/futuristic-plant-icon/without_background/3.3'))
card_rows.append(stat_card('CARD_CRITRATE_UP_3', 'CriticalRate', '0.150000', '15','치명타 확률 강화','치명타 확률을 {}% 증가시킨다','/Game/Assets/UIAsset/rpg-warlock-skill-icons/2.2','0.800000'))
card_rows.append(stat_card('CARD_CRITRATE_UP_4', 'CriticalRate', '0.200000', '20','치명타 확률 강화','치명타 확률을 {}% 증가시킨다','/Game/Assets/UIAsset/rpg-warlock-skill-icons/4.4','0.800000'))

# PassiveAdd — existing PSVs without cards
card_rows.append(pass_card('CARD_PASSIVE_FIRE_1',  'PSV_FIRE_AURA_1',  f'{DN["FIRE"]} 패시브 획득'))
card_rows.append(pass_card('CARD_PASSIVE_FIRE_2',  'PSV_FIRE_AURA_2',  f'{DN["FIRE"]} 패시브 획득'))
card_rows.append(pass_card('CARD_PASSIVE_THUNDER', 'PSV_THUNDER_FEET', f'{DN["THUNDER"]} 패시브 획득', '1.000000'))
card_rows.append(pass_card('CARD_PASSIVE_SHADOW',  'PSV_SHADOW_VEIL',  f'{DN["SHADOW"]} 패시브 획득',  '1.000000'))

# PassiveAdd — all new PSVs
card_rows.append(pass_card('CARD_PASSIVE_FIRE_4', 'PSV_FIRE_AURA_4', f'{DN["FIRE"]} 패시브 획득'))
for i in range(1,5): card_rows.append(pass_card(f'CARD_PASSIVE_ICE_ARMOR_{i}',    f'PSV_ICE_ARMOR_{i}',    f'{DN["ICE"]} Lv.{i} 패시브 획득'))
for i in range(1,5): card_rows.append(pass_card(f'CARD_PASSIVE_THUNDER_{i}',      f'PSV_THUNDER_FEET_{i}', f'{DN["THUNDER"]} Lv.{i} 패시브 획득'))
for i in range(1,5): card_rows.append(pass_card(f'CARD_PASSIVE_SHADOW_{i}',       f'PSV_SHADOW_VEIL_{i}',  f'{DN["SHADOW"]} Lv.{i} 패시브 획득'))
for i in range(1,5): card_rows.append(pass_card(f'CARD_PASSIVE_POISON_HEART_{i}', f'PSV_POISON_HEART_{i}', f'{DN["POISON"]} Lv.{i} 패시브 획득'))

# ── Append ────────────────────────────────────────────────────────────────────
def append_rows(filepath, rows):
    with open(filepath, 'r', encoding='utf-16') as f:
        content = f.read()
    if not content.endswith('\n'):
        content += '\n'
    content += '\n'.join(rows) + '\n'
    with open(filepath, 'w', encoding='utf-16') as f:
        f.write(content)

append_rows(PSV_FILE,  psv_rows)
append_rows(CARD_FILE, card_rows)

print(f'Done. Passive +{len(psv_rows)}, Card +{len(card_rows)}')
