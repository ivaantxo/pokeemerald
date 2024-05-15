#include "battle.h"
#include "battle_scripts.h"
#include "constants/battle_move_effects.h"

const struct BattleMoveEffect gBattleMoveEffects[NUM_BATTLE_MOVE_EFFECTS] =
{
    [EFFECT_PLACEHOLDER] =
    {
        .battleScript = BattleScript_EffectPlaceholder,
    },

    [EFFECT_HIT] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_SLEEP] =
    {
        .battleScript = BattleScript_EffectSleep,
    },

    [EFFECT_ABSORB] =
    {
        .battleScript = BattleScript_EffectAbsorb,
    },

    [EFFECT_EXPLOSION] =
    {
        .battleScript = BattleScript_EffectExplosion, // TODO: Assign points
    },

    [EFFECT_DREAM_EATER] =
    {
        .battleScript = BattleScript_EffectDreamEater,
        .encourageEncore = TRUE,
    },

    [EFFECT_MIRROR_MOVE] =
    {
        .battleScript = BattleScript_EffectMirrorMove,
    },

    [EFFECT_ATTACK_UP] =
    {
        .battleScript = BattleScript_EffectAttackUp,
        .encourageEncore = TRUE,
    },

    [EFFECT_DEFENSE_UP] =
    {
        .battleScript = BattleScript_EffectDefenseUp,
        .encourageEncore = TRUE,
    },

    [EFFECT_SPEED_UP] =
    {
        .battleScript = BattleScript_EffectSpeedUp,
        .encourageEncore = TRUE,
    },

    [EFFECT_SPECIAL_ATTACK_UP] =
    {
        .battleScript = BattleScript_EffectSpecialAttackUp,
        .encourageEncore = TRUE,
    },

    [EFFECT_SPECIAL_DEFENSE_UP] =
    {
        .battleScript = BattleScript_EffectSpecialDefenseUp,
        .encourageEncore = TRUE,
    },

    [EFFECT_ACCURACY_UP] =
    {
        .battleScript = BattleScript_EffectAccuracyUp,
        .encourageEncore = TRUE,
    },

    [EFFECT_EVASION_UP] =
    {
        .battleScript = BattleScript_EffectEvasionUp,
    },

    [EFFECT_SPECIAL_ATTACK_UP_3] =
    {
        .battleScript = BattleScript_EffectSpecialAttackUp3, // TODO: Assign points
    },

    [EFFECT_ATTACK_DOWN] =
    {
        .battleScript = BattleScript_EffectAttackDown,
    },

    [EFFECT_DEFENSE_DOWN] =
    {
        .battleScript = BattleScript_EffectDefenseDown,
    },

    [EFFECT_SPEED_DOWN] =
    {
        .battleScript = BattleScript_EffectSpeedDown,
    },

    [EFFECT_SPECIAL_ATTACK_DOWN] =
    {
        .battleScript = BattleScript_EffectSpecialAttackDown,
    },

    [EFFECT_SPECIAL_DEFENSE_DOWN] =
    {
        .battleScript = BattleScript_EffectSpecialDefenseDown,
    },

    [EFFECT_ACCURACY_DOWN] =
    {
        .battleScript = BattleScript_EffectAccuracyDown,
    },

    [EFFECT_EVASION_DOWN] =
    {
        .battleScript = BattleScript_EffectEvasionDown,
    },

    [EFFECT_HAZE] =
    {
        .battleScript = BattleScript_EffectHaze,
        .encourageEncore = TRUE,
    },

    [EFFECT_BIDE] =
    {
        .battleScript = BattleScript_EffectBide,
    },

    [EFFECT_ROAR] =
    {
        .battleScript = BattleScript_EffectRoar,
        .encourageEncore = TRUE,
    },

    [EFFECT_MULTI_HIT] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_CONVERSION] =
    {
        .battleScript = BattleScript_EffectConversion,
        .encourageEncore = TRUE,
    },

    [EFFECT_RESTORE_HP] =
    {
        .battleScript = BattleScript_EffectRestoreHp,
        .encourageEncore = TRUE,
    },

    [EFFECT_TOXIC] =
    {
        .battleScript = BattleScript_EffectToxic,
        .encourageEncore = TRUE,
    },

    [EFFECT_LIGHT_SCREEN] =
    {
        .battleScript = BattleScript_EffectLightScreen,
        .encourageEncore = TRUE,
    },

    [EFFECT_REST] =
    {
        .battleScript = BattleScript_EffectRest,
        .encourageEncore = TRUE,
    },

    [EFFECT_OHKO] =
    {
        .battleScript = BattleScript_EffectOHKO,
    },

    [EFFECT_FUSION_COMBO] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_SUPER_FANG] =
    {
        .battleScript = BattleScript_EffectSuperFang,
        .encourageEncore = TRUE,
    },

    [EFFECT_FIXED_DAMAGE_ARG] =
    {
        .battleScript = BattleScript_EffectFixedDamageArg,
    },

    [EFFECT_HEAL_BLOCK] =
    {
        .battleScript = BattleScript_EffectHealBlock, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_RECOIL_IF_MISS] =
    {
        .battleScript = BattleScript_EffectRecoilIfMiss,
    },

    [EFFECT_MIST] =
    {
        .battleScript = BattleScript_EffectMist,
        .encourageEncore = TRUE,
    },

    [EFFECT_FOCUS_ENERGY] =
    {
        .battleScript = BattleScript_EffectFocusEnergy,
        .encourageEncore = TRUE,
    },

    [EFFECT_CONFUSE] =
    {
        .battleScript = BattleScript_EffectConfuse,
        .encourageEncore = TRUE,
    },

    [EFFECT_ATTACK_UP_2] =
    {
        .battleScript = BattleScript_EffectAttackUp2,
        .encourageEncore = TRUE,
    },

    [EFFECT_DEFENSE_UP_2] =
    {
        .battleScript = BattleScript_EffectDefenseUp2,
        .encourageEncore = TRUE,
    },

    [EFFECT_SPEED_UP_2] =
    {
        .battleScript = BattleScript_EffectSpeedUp2,
        .encourageEncore = TRUE,
    },

    [EFFECT_SPECIAL_ATTACK_UP_2] =
    {
        .battleScript = BattleScript_EffectSpecialAttackUp2,
        .encourageEncore = TRUE,
    },

    [EFFECT_SPECIAL_DEFENSE_UP_2] =
    {
        .battleScript = BattleScript_EffectSpecialDefenseUp2,
        .encourageEncore = TRUE,
    },

    [EFFECT_ACCURACY_UP_2] =
    {
        .battleScript = BattleScript_EffectAccuracyUp2,
        .encourageEncore = TRUE,
    },

    [EFFECT_EVASION_UP_2] =
    {
        .battleScript = BattleScript_EffectEvasionUp2,
    },

    [EFFECT_TRANSFORM] =
    {
        .battleScript = BattleScript_EffectTransform, // TODO: Assign points
    },

    [EFFECT_ATTACK_DOWN_2] =
    {
        .battleScript = BattleScript_EffectAttackDown2,
    },

    [EFFECT_DEFENSE_DOWN_2] =
    {
        .battleScript = BattleScript_EffectDefenseDown2,
    },

    [EFFECT_SPEED_DOWN_2] =
    {
        .battleScript = BattleScript_EffectSpeedDown2,
    },

    [EFFECT_SPECIAL_ATTACK_DOWN_2] =
    {
        .battleScript = BattleScript_EffectSpecialAttackDown2,
    },

    [EFFECT_SPECIAL_DEFENSE_DOWN_2] =
    {
        .battleScript = BattleScript_EffectSpecialDefenseDown2,
    },

    [EFFECT_ACCURACY_DOWN_2] =
    {
        .battleScript = BattleScript_EffectAccuracyDown2,
    },

    [EFFECT_EVASION_DOWN_2] =
    {
        .battleScript = BattleScript_EffectEvasionDown2,
    },

    [EFFECT_REFLECT] =
    {
        .battleScript = BattleScript_EffectReflect,
        .encourageEncore = TRUE,
    },

    [EFFECT_POISON] =
    {
        .battleScript = BattleScript_EffectPoison,
        .encourageEncore = TRUE,
    },

    [EFFECT_PARALYZE] =
    {
        .battleScript = BattleScript_EffectParalyze,
        .encourageEncore = TRUE,
    },

    [EFFECT_TWO_TURNS_ATTACK] =
    {
        .battleScript = BattleScript_EffectTwoTurnsAttack,
        .twoTurnEffect = TRUE,
    },

    [EFFECT_SUBSTITUTE] =
    {
        .battleScript = BattleScript_EffectSubstitute,
    },

    [EFFECT_RAGE] =
    {
        .battleScript = BattleScript_EffectRage,
    },

    [EFFECT_MIMIC] =
    {
        .battleScript = BattleScript_EffectMimic,
    },

    [EFFECT_METRONOME] =
    {
        .battleScript = BattleScript_EffectMetronome,
    },

    [EFFECT_LEECH_SEED] =
    {
        .battleScript = BattleScript_EffectLeechSeed,
        .encourageEncore = TRUE,
    },

    [EFFECT_DO_NOTHING] =
    {
        .battleScript = BattleScript_EffectDoNothing,
        .encourageEncore = TRUE,
    },

    [EFFECT_DISABLE] =
    {
        .battleScript = BattleScript_EffectDisable,
    },

    [EFFECT_LEVEL_DAMAGE] =
    {
        .battleScript = BattleScript_EffectLevelDamage,
    },

    [EFFECT_PSYWAVE] =
    {
        .battleScript = BattleScript_EffectPsywave,
    },

    [EFFECT_COUNTER] =
    {
        .battleScript = BattleScript_EffectCounter,
        .encourageEncore = TRUE,
    },

    [EFFECT_ENCORE] =
    {
        .battleScript = BattleScript_EffectEncore,
    },

    [EFFECT_PAIN_SPLIT] =
    {
        .battleScript = BattleScript_EffectPainSplit,
    },

    [EFFECT_SNORE] =
    {
        .battleScript = BattleScript_EffectSnore,
    },

    [EFFECT_CONVERSION_2] =
    {
        .battleScript = BattleScript_EffectConversion2,
        .encourageEncore = TRUE,
    },

    [EFFECT_LOCK_ON] =
    {
        .battleScript = BattleScript_EffectLockOn,
        .encourageEncore = TRUE,
    },

    [EFFECT_SKETCH] =
    {
        .battleScript = BattleScript_EffectSketch,
    },

    [EFFECT_SLEEP_TALK] =
    {
        .battleScript = BattleScript_EffectSleepTalk,
        .encourageEncore = TRUE,
    },

    [EFFECT_DESTINY_BOND] =
    {
        .battleScript = BattleScript_EffectDestinyBond,
    },

    [EFFECT_FLAIL] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_SPITE] =
    {
        .battleScript = BattleScript_EffectSpite,
    },

    [EFFECT_FALSE_SWIPE] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_HEAL_BELL] =
    {
        .battleScript = BattleScript_EffectHealBell,
        .encourageEncore = TRUE,
    },

    [EFFECT_TRIPLE_KICK] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_MEAN_LOOK] =
    {
        .battleScript = BattleScript_EffectMeanLook,
        .encourageEncore = TRUE,
    },

    [EFFECT_NIGHTMARE] =
    {
        .battleScript = BattleScript_EffectNightmare,
        .encourageEncore = TRUE,
    },

    [EFFECT_MINIMIZE] =
    {
        .battleScript = BattleScript_EffectMinimize,
    },

    [EFFECT_CURSE] =
    {
        .battleScript = BattleScript_EffectCurse,
        .encourageEncore = TRUE,
    },

    [EFFECT_HEALING_WISH] =
    {
        .battleScript = BattleScript_EffectHealingWish, // TODO: Assign points
    },

    [EFFECT_PROTECT] =
    {
        .battleScript = BattleScript_EffectProtect,
        .encourageEncore = TRUE,
        .usesProtectCounter = TRUE,
    },

    [EFFECT_SPIKES] =
    {
        .battleScript = BattleScript_EffectSpikes,
    },

    [EFFECT_FORESIGHT] =
    {
        .battleScript = BattleScript_EffectForesight,
        .encourageEncore = TRUE,
    },

    [EFFECT_PERISH_SONG] =
    {
        .battleScript = BattleScript_EffectPerishSong,
        .encourageEncore = TRUE,
    },

    [EFFECT_SANDSTORM] =
    {
        .battleScript = BattleScript_EffectSandstorm,
        .encourageEncore = TRUE,
    },

    [EFFECT_ENDURE] =
    {
        .battleScript = BattleScript_EffectEndure,
        .encourageEncore = TRUE,
        .usesProtectCounter = TRUE,
    },

    [EFFECT_ROLLOUT] =
    {
        .battleScript = BattleScript_EffectRollout,
    },

    [EFFECT_SWAGGER] =
    {
        .battleScript = BattleScript_EffectSwagger,
        .encourageEncore = TRUE,
    },

    [EFFECT_FURY_CUTTER] =
    {
        .battleScript = BattleScript_EffectFuryCutter,
    },

    [EFFECT_ATTRACT] =
    {
        .battleScript = BattleScript_EffectAttract,
        .encourageEncore = TRUE,
    },

    [EFFECT_RETURN] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_PRESENT] =
    {
        .battleScript = BattleScript_EffectPresent,
    },

    [EFFECT_FRUSTRATION] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_SAFEGUARD] =
    {
        .battleScript = BattleScript_EffectSafeguard,
        .encourageEncore = TRUE,
    },

    [EFFECT_MAGNITUDE] =
    {
        .battleScript = BattleScript_EffectMagnitude,
    },

    [EFFECT_BATON_PASS] =
    {
        .battleScript = BattleScript_EffectBatonPass,
    },

    [EFFECT_PURSUIT] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_CAPTIVATE] =
    {
        .battleScript = BattleScript_EffectCaptivate, // TODO: Assign points
    },

    [EFFECT_MORNING_SUN] =
    {
        .battleScript = BattleScript_EffectMorningSun,
        .encourageEncore = TRUE,
    },

    [EFFECT_SYNTHESIS] =
    {
        .battleScript = BattleScript_EffectSynthesis,
        .encourageEncore = TRUE,
    },

    [EFFECT_MOONLIGHT] =
    {
        .battleScript = BattleScript_EffectMoonlight,
        .encourageEncore = TRUE,
    },

    [EFFECT_HIDDEN_POWER] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_RAIN_DANCE] =
    {
        .battleScript = BattleScript_EffectRainDance,
        .encourageEncore = TRUE,
    },

    [EFFECT_SUNNY_DAY] =
    {
        .battleScript = BattleScript_EffectSunnyDay,
        .encourageEncore = TRUE,
    },

    [EFFECT_FELL_STINGER] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_BELLY_DRUM] =
    {
        .battleScript = BattleScript_EffectBellyDrum,
        .encourageEncore = TRUE,
    },

    [EFFECT_PSYCH_UP] =
    {
        .battleScript = BattleScript_EffectPsychUp,
        .encourageEncore = TRUE,
    },

    [EFFECT_MIRROR_COAT] =
    {
        .battleScript = BattleScript_EffectMirrorCoat,
        .encourageEncore = TRUE,
    },

    [EFFECT_EARTHQUAKE] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_FUTURE_SIGHT] =
    {
        .battleScript = BattleScript_EffectFutureSight,
        .encourageEncore = TRUE,
    },

    [EFFECT_SOLAR_BEAM] =
    {
        .battleScript = BattleScript_EffectTwoTurnsAttack,
        .twoTurnEffect = TRUE,
    },

    [EFFECT_THUNDER] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_TELEPORT] =
    {
        .battleScript = BattleScript_EffectTeleport,
    },

    [EFFECT_BEAT_UP] =
    {
        .battleScript = BattleScript_EffectBeatUp,
    },

    [EFFECT_SEMI_INVULNERABLE] =
    {
        .battleScript = BattleScript_EffectTwoTurnsAttack,
        .twoTurnEffect = TRUE,
        .semiInvulnerableEffect = TRUE,
    },

    [EFFECT_DEFENSE_CURL] =
    {
        .battleScript = BattleScript_EffectDefenseCurl,
        .encourageEncore = TRUE,
    },

    [EFFECT_SOFTBOILED] =
    {
        .battleScript = BattleScript_EffectSoftboiled,
        .encourageEncore = TRUE,
    },

    [EFFECT_FIRST_TURN_ONLY] =
    {
        .battleScript = BattleScript_EffectFirstTurnOnly,
        .encourageEncore = TRUE,
    },

    [EFFECT_UPROAR] =
    {
        .battleScript = BattleScript_EffectUproar,
    },

    [EFFECT_STOCKPILE] =
    {
        .battleScript = BattleScript_EffectStockpile,
        .encourageEncore = TRUE,
    },

    [EFFECT_SPIT_UP] =
    {
        .battleScript = BattleScript_EffectSpitUp,
        .encourageEncore = TRUE,
    },

    [EFFECT_SWALLOW] =
    {
        .battleScript = BattleScript_EffectSwallow,
        .encourageEncore = TRUE,
    },

    [EFFECT_WORRY_SEED] =
    {
        .battleScript = BattleScript_EffectWorrySeed, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_HAIL] =
    {
        .battleScript = BattleScript_EffectHail,
        .encourageEncore = TRUE,
    },

    [EFFECT_TORMENT] =
    {
        .battleScript = BattleScript_EffectTorment,
        .encourageEncore = TRUE,
    },

    [EFFECT_FLATTER] =
    {
        .battleScript = BattleScript_EffectFlatter,
    },

    [EFFECT_WILL_O_WISP] =
    {
        .battleScript = BattleScript_EffectWillOWisp,
        .encourageEncore = TRUE,
    },

    [EFFECT_MEMENTO] =
    {
        .battleScript = BattleScript_EffectMemento,
    },

    [EFFECT_FACADE] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_FOCUS_PUNCH] =
    {
        .battleScript = BattleScript_EffectFocusPunch,
    },

    [EFFECT_DOUBLE_POWER_ON_ARG_STATUS] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_FOLLOW_ME] =
    {
        .battleScript = BattleScript_EffectFollowMe,
        .encourageEncore = TRUE,
    },

    [EFFECT_NATURE_POWER] =
    {
        .battleScript = BattleScript_EffectNaturePower, // TODO: Assign points
    },

    [EFFECT_CHARGE] =
    {
        .battleScript = BattleScript_EffectCharge,
        .encourageEncore = TRUE,
    },

    [EFFECT_TAUNT] =
    {
        .battleScript = BattleScript_EffectTaunt,
    },

    [EFFECT_HELPING_HAND] =
    {
        .battleScript = BattleScript_EffectHelpingHand,
    },

    [EFFECT_TRICK] =
    {
        .battleScript = BattleScript_EffectTrick,
        .encourageEncore = TRUE,
    },

    [EFFECT_ROLE_PLAY] =
    {
        .battleScript = BattleScript_EffectRolePlay,
        .encourageEncore = TRUE,
    },

    [EFFECT_WISH] =
    {
        .battleScript = BattleScript_EffectWish,
    },

    [EFFECT_ASSIST] =
    {
        .battleScript = BattleScript_EffectAssist,
    },

    [EFFECT_INGRAIN] =
    {
        .battleScript = BattleScript_EffectIngrain,
        .encourageEncore = TRUE,
    },

    [EFFECT_MAGIC_COAT] =
    {
        .battleScript = BattleScript_EffectMagicCoat,
    },

    [EFFECT_RECYCLE] =
    {
        .battleScript = BattleScript_EffectRecycle,
        .encourageEncore = TRUE,
    },

    [EFFECT_REVENGE] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_BRICK_BREAK] =
    {
        .battleScript = BattleScript_EffectBrickBreak,
    },

    [EFFECT_YAWN] =
    {
        .battleScript = BattleScript_EffectYawn,
    },

    [EFFECT_KNOCK_OFF] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_ENDEAVOR] =
    {
        .battleScript = BattleScript_EffectEndeavor,
    },

    [EFFECT_ERUPTION] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_SKILL_SWAP] =
    {
        .battleScript = BattleScript_EffectSkillSwap,
        .encourageEncore = TRUE,
    },

    [EFFECT_IMPRISON] =
    {
        .battleScript = BattleScript_EffectImprison,
        .encourageEncore = TRUE,
    },

    [EFFECT_REFRESH] =
    {
        .battleScript = BattleScript_EffectRefresh,
        .encourageEncore = TRUE,
    },

    [EFFECT_GRUDGE] =
    {
        .battleScript = BattleScript_EffectGrudge,
        .encourageEncore = TRUE,
    },

    [EFFECT_SNATCH] =
    {
        .battleScript = BattleScript_EffectSnatch,
    },

    [EFFECT_LOW_KICK] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_HIT_ESCAPE] =
    {
        .battleScript = BattleScript_EffectHitEscape,
    },

    [EFFECT_MUD_SPORT] =
    {
        .battleScript = BattleScript_EffectMudSport, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_WEATHER_BALL] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_TICKLE] =
    {
        .battleScript = BattleScript_EffectTickle,
    },

    [EFFECT_COSMIC_POWER] =
    {
        .battleScript = BattleScript_EffectCosmicPower,
        .encourageEncore = TRUE,
    },

    [EFFECT_BULK_UP] =
    {
        .battleScript = BattleScript_EffectBulkUp,
        .encourageEncore = TRUE,
    },

    [EFFECT_WATER_SPORT] =
    {
        .battleScript = BattleScript_EffectWaterSport,
        .encourageEncore = TRUE,
    },

    [EFFECT_CALM_MIND] =
    {
        .battleScript = BattleScript_EffectCalmMind,
        .encourageEncore = TRUE,
    },

    [EFFECT_DRAGON_DANCE] =
    {
        .battleScript = BattleScript_EffectDragonDance,
        .encourageEncore = TRUE,
    },

    [EFFECT_CAMOUFLAGE] =
    {
        .battleScript = BattleScript_EffectCamouflage,
        .encourageEncore = TRUE,
    },

    [EFFECT_PLEDGE] =
    {
        .battleScript = BattleScript_EffectPledge, // TODO: Assign points
    },

    [EFFECT_FLING] =
    {
        .battleScript = BattleScript_EffectFling, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_NATURAL_GIFT] =
    {
        .battleScript = BattleScript_EffectNaturalGift, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_VARY_POWER_BASED_ON_HP] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_ASSURANCE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_TRUMP_CARD] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_ACROBATICS] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_HEAT_CRASH] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_PUNISHMENT] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_STORED_POWER] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_ELECTRO_BALL] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_GYRO_BALL] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_ECHOED_VOICE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_PAYBACK] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_ROUND] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_BRINE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_RETALIATE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_BULLDOZE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_FOUL_PLAY] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_PSYSHOCK] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_ROOST] =
    {
        .battleScript = BattleScript_EffectRoost, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_GRAVITY] =
    {
        .battleScript = BattleScript_EffectGravity, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_MIRACLE_EYE] =
    {
        .battleScript = BattleScript_EffectMircleEye, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_TAILWIND] =
    {
        .battleScript = BattleScript_EffectTailwind, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_EMBARGO] =
    {
        .battleScript = BattleScript_EffectEmbargo, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_AQUA_RING] =
    {
        .battleScript = BattleScript_EffectAquaRing, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_TRICK_ROOM] =
    {
        .battleScript = BattleScript_EffectTrickRoom, // TODO: Assign points
    },

    [EFFECT_WONDER_ROOM] =
    {
        .battleScript = BattleScript_EffectWonderRoom, // TODO: Assign points
    },

    [EFFECT_MAGIC_ROOM] =
    {
        .battleScript = BattleScript_EffectMagicRoom, // TODO: Assign points
    },

    [EFFECT_MAGNET_RISE] =
    {
        .battleScript = BattleScript_EffectMagnetRise, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_TOXIC_SPIKES] =
    {
        .battleScript = BattleScript_EffectToxicSpikes, // TODO: Assign points
    },

    [EFFECT_GASTRO_ACID] =
    {
        .battleScript = BattleScript_EffectGastroAcid, // TODO: Assign points
    },

    [EFFECT_STEALTH_ROCK] =
    {
        .battleScript = BattleScript_EffectStealthRock, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_TELEKINESIS] =
    {
        .battleScript = BattleScript_EffectTelekinesis, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_POWER_SWAP] =
    {
        .battleScript = BattleScript_EffectPowerSwap, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_GUARD_SWAP] =
    {
        .battleScript = BattleScript_EffectGuardSwap, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_HEART_SWAP] =
    {
        .battleScript = BattleScript_EffectHeartSwap, // TODO: Assign points
    },

    [EFFECT_POWER_SPLIT] =
    {
        .battleScript = BattleScript_EffectPowerSplit, // TODO: Assign points
    },

    [EFFECT_GUARD_SPLIT] =
    {
        .battleScript = BattleScript_EffectGuardSplit, // TODO: Assign points
    },

    [EFFECT_STICKY_WEB] =
    {
        .battleScript = BattleScript_EffectStickyWeb, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_METAL_BURST] =
    {
        .battleScript = BattleScript_EffectMetalBurst, // TODO: Assign points
    },

    [EFFECT_LUCKY_CHANT] =
    {
        .battleScript = BattleScript_EffectLuckyChant, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_SUCKER_PUNCH] =
    {
        .battleScript = BattleScript_EffectSuckerPunch, // TODO: Assign points
    },

    [EFFECT_SIMPLE_BEAM] =
    {
        .battleScript = BattleScript_EffectSimpleBeam, // TODO: Assign points
    },

    [EFFECT_ENTRAINMENT] =
    {
        .battleScript = BattleScript_EffectEntrainment, // TODO: Assign points
    },

    [EFFECT_HEAL_PULSE] =
    {
        .battleScript = BattleScript_EffectHealPulse, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_QUASH] =
    {
        .battleScript = BattleScript_EffectQuash, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_ION_DELUGE] =
    {
        .battleScript = BattleScript_EffectIonDeluge, // TODO: Assign points
    },

    [EFFECT_SUPER_EFFECTIVE_ON_ARG] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_TOPSY_TURVY] =
    {
        .battleScript = BattleScript_EffectTopsyTurvy, // TODO: Assign points
    },

    [EFFECT_MISTY_TERRAIN] =
    {
        .battleScript = BattleScript_EffectMistyTerrain, // TODO: Assign points
    },

    [EFFECT_GRASSY_TERRAIN] =
    {
        .battleScript = BattleScript_EffectGrassyTerrain, // TODO: Assign points
    },

    [EFFECT_ELECTRIC_TERRAIN] =
    {
        .battleScript = BattleScript_EffectElectricTerrain, // TODO: Assign points
    },

    [EFFECT_PSYCHIC_TERRAIN] =
    {
        .battleScript = BattleScript_EffectPsychicTerrain, // TODO: Assign points
    },

    [EFFECT_ATTACK_ACCURACY_UP] =
    {
        .battleScript = BattleScript_EffectAttackAccUp, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_ATTACK_SPATK_UP] =
    {
        .battleScript = BattleScript_EffectAttackSpAttackUp, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_TWO_TYPED_MOVE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_ME_FIRST] =
    {
        .battleScript = BattleScript_EffectMeFirst, // TODO: Assign points
    },

    [EFFECT_QUIVER_DANCE] =
    {
        .battleScript = BattleScript_EffectQuiverDance, // TODO: Assign points
    },

    [EFFECT_COIL] =
    {
        .battleScript = BattleScript_EffectCoil, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_ELECTRIFY] =
    {
        .battleScript = BattleScript_EffectElectrify, // TODO: Assign points
    },

    [EFFECT_REFLECT_TYPE] =
    {
        .battleScript = BattleScript_EffectReflectType, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_SOAK] =
    {
        .battleScript = BattleScript_EffectSoak, // TODO: Assign points
    },

    [EFFECT_GROWTH] =
    {
        .battleScript = BattleScript_EffectGrowth, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_LAST_RESORT] =
    {
        .battleScript = BattleScript_EffectLastResort, // TODO: Assign points
    },

    [EFFECT_SHELL_SMASH] =
    {
        .battleScript = BattleScript_EffectShellSmash, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_SHIFT_GEAR] =
    {
        .battleScript = BattleScript_EffectShiftGear, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_DEFENSE_UP_3] =
    {
        .battleScript = BattleScript_EffectDefenseUp3, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_NOBLE_ROAR] =
    {
        .battleScript = BattleScript_EffectNobleRoar, // TODO: Assign points
    },

    [EFFECT_VENOM_DRENCH] =
    {
        .battleScript = BattleScript_EffectVenomDrench, // TODO: Assign points
    },

    [EFFECT_TOXIC_THREAD] =
    {
        .battleScript = BattleScript_EffectToxicThread, // TODO: Assign points
    },

    [EFFECT_HIT_SWITCH_TARGET] =
    {
        .battleScript = BattleScript_EffectHitSwitchTarget, // TODO: Assign points
    },

    [EFFECT_FINAL_GAMBIT] =
    {
        .battleScript = BattleScript_EffectFinalGambit, // TODO: Assign points
    },

    [EFFECT_CHANGE_TYPE_ON_ITEM] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_AUTOTOMIZE] =
    {
        .battleScript = BattleScript_EffectAutotomize, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_COPYCAT] =
    {
        .battleScript = BattleScript_EffectCopycat, // TODO: Assign points
    },

    [EFFECT_DEFOG] =
    {
        .battleScript = BattleScript_EffectDefog, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_HIT_ENEMY_HEAL_ALLY] =
    {
        .battleScript = BattleScript_EffectHitEnemyHealAlly, // TODO: Assign points
    },

    [EFFECT_SYNCHRONOISE] =
    {
        .battleScript = BattleScript_EffectSynchronoise, // TODO: Assign points
    },

    [EFFECT_PSYCHO_SHIFT] =
    {
        .battleScript = BattleScript_EffectPsychoShift, // TODO: Assign points
    },

    [EFFECT_POWER_TRICK] =
    {
        .battleScript = BattleScript_EffectPowerTrick, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_AFTER_YOU] =
    {
        .battleScript = BattleScript_EffectAfterYou, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_BESTOW] =
    {
        .battleScript = BattleScript_EffectBestow, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_ROTOTILLER] =
    {
        .battleScript = BattleScript_EffectRototiller, // TODO: Assign points
    },

    [EFFECT_FLOWER_SHIELD] =
    {
        .battleScript = BattleScript_EffectFlowerShield, // TODO: Assign points
    },

    [EFFECT_SPEED_SWAP] =
    {
        .battleScript = BattleScript_EffectSpeedSwap, // TODO: Assign points
    },

    [EFFECT_REVELATION_DANCE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_AURORA_VEIL] =
    {
        .battleScript = BattleScript_EffectAuroraVeil, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_THIRD_TYPE] =
    {
        .battleScript = BattleScript_EffectThirdType, // TODO: Assign points
    },

    [EFFECT_ACUPRESSURE] =
    {
        .battleScript = BattleScript_EffectAcupressure, // TODO: Assign points
    },

    [EFFECT_AROMATIC_MIST] =
    {
        .battleScript = BattleScript_EffectAromaticMist, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_POWDER] =
    {
        .battleScript = BattleScript_EffectPowder, // TODO: Assign points
    },

    [EFFECT_BELCH] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_PARTING_SHOT] =
    {
        .battleScript = BattleScript_EffectPartingShot, // TODO: Assign points
    },

    [EFFECT_MAT_BLOCK] =
    {
        .battleScript = BattleScript_EffectMatBlock, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_STOMPING_TANTRUM] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_INSTRUCT] =
    {
        .battleScript = BattleScript_EffectInstruct, // TODO: Assign points
    },

    [EFFECT_LASER_FOCUS] =
    {
        .battleScript = BattleScript_EffectLaserFocus, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_MAGNETIC_FLUX] =
    {
        .battleScript = BattleScript_EffectMagneticFlux, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_GEAR_UP] =
    {
        .battleScript = BattleScript_EffectGearUp, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_STRENGTH_SAP] =
    {
        .battleScript = BattleScript_EffectStrengthSap, // TODO: Assign points
    },

    [EFFECT_PURIFY] =
    {
        .battleScript = BattleScript_EffectPurify, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_FAIL_IF_NOT_ARG_TYPE] =
    {
        .battleScript = BattleScript_FailIfNotArgType, // TODO: Assign points
    },

    [EFFECT_SHORE_UP] =
    {
        .battleScript = BattleScript_EffectShoreUp, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_GEOMANCY] =
    {
        .battleScript = BattleScript_EffectGeomancy, // TODO: Assign points
        .twoTurnEffect = TRUE,
    },

    [EFFECT_FAIRY_LOCK] =
    {
        .battleScript = BattleScript_EffectFairyLock, // TODO: Assign points
    },

    [EFFECT_ALLY_SWITCH] =
    {
        .battleScript = BattleScript_EffectAllySwitch, // TODO: Assign points
    },

    [EFFECT_RELIC_SONG] =
    {
        .battleScript = BattleScript_EffectRelicSong, // TODO: Assign points
    },

    [EFFECT_BODY_PRESS] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_EERIE_SPELL] =
    {
        .battleScript = BattleScript_EffectEerieSpell, // TODO: Assign points
    },

    [EFFECT_JUNGLE_HEALING] =
    {
        .battleScript = BattleScript_EffectJungleHealing, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_COACHING] =
    {
        .battleScript = BattleScript_EffectCoaching, // TODO: Assign points
    },

    [EFFECT_LASH_OUT] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_GRASSY_GLIDE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_DYNAMAX_DOUBLE_DMG] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_DECORATE] =
    {
        .battleScript = BattleScript_EffectDecorate, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_SNIPE_SHOT] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_RECOIL_HP_25] =
    {
        .battleScript = BattleScript_EffectRecoilHP25, // TODO: Assign points
    },

    [EFFECT_STUFF_CHEEKS] =
    {
        .battleScript = BattleScript_EffectStuffCheeks, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_GRAV_APPLE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_GLITZY_GLOW] =
    {
        .battleScript = BattleScript_EffectGlitzyGlow, // TODO: Assign points
    },

    [EFFECT_BADDY_BAD] =
    {
        .battleScript = BattleScript_EffectBaddyBad, // TODO: Assign points
    },

    [EFFECT_SAPPY_SEED] =
    {
        .battleScript = BattleScript_EffectSappySeed, // TODO: Assign points
    },

    [EFFECT_FREEZY_FROST] =
    {
        .battleScript = BattleScript_EffectFreezyFrost, // TODO: Assign points
    },

    [EFFECT_SPARKLY_SWIRL] =
    {
        .battleScript = BattleScript_EffectSparklySwirl, // TODO: Assign points
    },

    [EFFECT_PLASMA_FISTS] =
    {
        .battleScript = BattleScript_EffectPlasmaFists, // TODO: Assign points
    },

    [EFFECT_HYPERSPACE_FURY] =
    {
        .battleScript = BattleScript_EffectHyperspaceFury, // TODO: Assign points
    },

    [EFFECT_AURA_WHEEL] =
    {
        .battleScript = BattleScript_EffectAuraWheel, // TODO: Assign points
    },

    [EFFECT_PHOTON_GEYSER] =
    {
        .battleScript = BattleScript_EffectPhotonGeyser, // TODO: Assign points
    },

    [EFFECT_SHELL_SIDE_ARM] =
    {
        .battleScript = BattleScript_EffectShellSideArm, // TODO: Assign points
    },

    [EFFECT_TERRAIN_PULSE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_NO_RETREAT] =
    {
        .battleScript = BattleScript_EffectNoRetreat, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_TAR_SHOT] =
    {
        .battleScript = BattleScript_EffectTarShot, // TODO: Assign points
    },

    [EFFECT_POLTERGEIST] =
    {
        .battleScript = BattleScript_EffectPoltergeist, // TODO: Assign points
    },

    [EFFECT_OCTOLOCK] =
    {
        .battleScript = BattleScript_EffectOctolock, // TODO: Assign points
    },

    [EFFECT_CLANGOROUS_SOUL] =
    {
        .battleScript = BattleScript_EffectClangorousSoul, // TODO: Assign points
    },

    [EFFECT_BOLT_BEAK] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_SKY_DROP] =
    {
        .battleScript = BattleScript_EffectSkyDrop, // TODO: Assign points
        .twoTurnEffect = TRUE,
        .semiInvulnerableEffect = TRUE,
    },

    [EFFECT_EXPANDING_FORCE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_RISING_VOLTAGE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_BEAK_BLAST] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_COURT_CHANGE] =
    {
        .battleScript = BattleScript_EffectCourtChange, // TODO: Assign points
    },

    [EFFECT_MAX_HP_50_RECOIL] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_MIND_BLOWN] =
    {
        .battleScript = BattleScript_EffectHit,
    },

    [EFFECT_EXTREME_EVOBOOST] =
    {
        .battleScript = BattleScript_EffectExtremeEvoboost, // TODO: Assign points
    },

    [EFFECT_HIT_SET_REMOVE_TERRAIN] =
    {
        .battleScript = BattleScript_EffectHitSetRemoveTerrain, // TODO: Assign points
    },

    [EFFECT_DARK_VOID] =
    {
        .battleScript = BattleScript_EffectDarkVoid, // TODO: Assign points
    },

    [EFFECT_VICTORY_DANCE] =
    {
        .battleScript = BattleScript_EffectVictoryDance, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_TEATIME] =
    {
        .battleScript = BattleScript_EffectTeatime, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_ATTACK_UP_USER_ALLY] =
    {
        .battleScript = BattleScript_EffectAttackUpUserAlly, // TODO: Assign points
    },

    [EFFECT_SHELL_TRAP] =
    {
        .battleScript = BattleScript_EffectShellTrap, // TODO: Assign points
    },

    [EFFECT_PSYBLADE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_HYDRO_STEAM] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_REVIVAL_BLESSING] =
    {
        .battleScript = BattleScript_EffectRevivalBlessing, // TODO: Assign points
    },

    [EFFECT_SNOWSCAPE] =
    {
        .battleScript = BattleScript_EffectSnow,
    },

    [EFFECT_TAKE_HEART] =
    {
        .battleScript = BattleScript_EffectTakeHeart, // TODO: Assign points
    },

    [EFFECT_COLLISION_COURSE] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_CORROSIVE_GAS] =
    {
        .battleScript = BattleScript_EffectCorrosiveGas, // TODO: Assign points
    },

    [EFFECT_POPULATION_BOMB] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_SALT_CURE] =
    {
        .battleScript = BattleScript_EffectSaltCure, // TODO: Assign points
    },

    [EFFECT_CHILLY_RECEPTION] =
    {
        .battleScript = BattleScript_EffectChillyReception, // TODO: Assign points
    },

    [EFFECT_MAX_MOVE] =
    {
        .battleScript = BattleScript_EffectMaxMove, // TODO: Assign points
    },

    [EFFECT_GLAIVE_RUSH] =
    {
        .battleScript = BattleScript_EffectGlaiveRush, // TODO: Assign points
    },

    [EFFECT_RAGING_BULL] =
    {
        .battleScript = BattleScript_EffectBrickBreak, // TODO: Assign points
    },

    [EFFECT_RAGE_FIST] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_DOODLE] =
    {
        .battleScript = BattleScript_EffectDoodle, // TODO: Assign points
    },

    [EFFECT_FILLET_AWAY] =
    {
        .battleScript = BattleScript_EffectFilletAway, // TODO: Assign points
    },

    [EFFECT_IVY_CUDGEL] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_FICKLE_BEAM] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_BLIZZARD] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_RAIN_ALWAYS_HIT] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_SHED_TAIL] =
    {
        .battleScript = BattleScript_EffectShedTail, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_UPPER_HAND] =
    {
        .battleScript = BattleScript_EffectUpperHand, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_DRAGON_CHEER] =
    {
        .battleScript = BattleScript_EffectFocusEnergy,
        .encourageEncore = TRUE,
    },

    [EFFECT_LAST_RESPECTS] =
    {
        .battleScript = BattleScript_EffectHit, // TODO: Assign points
    },

    [EFFECT_TIDY_UP] =
    {
        .battleScript = BattleScript_EffectTidyUp, // TODO: Assign points
        .encourageEncore = TRUE,
    },

    [EFFECT_TERA_BLAST] =
    {
        .battleScript = BattleScript_EffectPhotonGeyser,
    },

    [EFFECT_TERA_STARSTORM] =
    {
        .battleScript = BattleScript_EffectPhotonGeyser,
    },
};
