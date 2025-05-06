#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_message.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "battle_z_move.h"
#include "data.h"
#include "event_data.h"
#include "frontier_util.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item.h"
#include "link.h"
#include "menu.h"
#include "palette.h"
#include "string_util.h"
#include "strings.h"
#include "text.h"
#include "trainer_hill.h"
#include "window.h"
#include "constants/abilities.h"
#include "constants/battle_dome.h"
#include "constants/battle_string_ids.h"
#include "constants/frontier_util.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "constants/trainer_hill.h"
#include "constants/weather.h"

struct BattleWindowText
{
    u8 fillValue;
    u8 fontId;
    u8 x;
    u8 y;
    u8 letterSpacing;
    u8 lineSpacing;
    u8 speed;
    u8 fgColor;
    u8 bgColor;
    u8 shadowColor;
};

static EWRAM_DATA u16 sBattlerAbilities[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA struct BattleMsgData *gBattleMsgDataPtr = NULL;

// todo: make some of those names less vague: attacker/target vs pkmn, etc.

static const u8 sText_Trainer1LoseText[] = _("{B_TRAINER1_LOSE_TEXT}");
static const u8 sText_PkmnGainedEXP[] = _("¡{B_BUFF1} ganó {B_BUFF2}\n{B_BUFF3} puntos de experiencia!\p");
static const u8 sText_EmptyString4[] = _("");
static const u8 sText_ABoosted[] = _(" a boosted");
static const u8 sText_PkmnGrewToLv[] = _("¡{B_BUFF1} subió al\nnivel {B_BUFF2}!{WAIT_SE}\p");
static const u8 sText_PkmnLearnedMove[] = _("{B_BUFF1} aprendió\n{B_BUFF2}!{WAIT_SE}\p");
static const u8 sText_TryToLearnMove1[] = _("{B_BUFF1} puede aprender\n{B_BUFF2}.\p");
static const u8 sText_TryToLearnMove2[] = _("Pero {B_BUFF1} ya\nconoce cuatro ataques.\p");
static const u8 sText_TryToLearnMove3[] = _("¿Quieres que\nolvide algún ataque?");
static const u8 sText_PkmnForgotMove[] = _("{B_BUFF1} olvidó\n{B_BUFF2}.\p");
static const u8 sText_StopLearningMove[] = _("{PAUSE 32}¿No quieres que aprenda\n{B_BUFF2}?");
static const u8 sText_DidNotLearnMove[] = _("{B_BUFF1} no aprendió\n{B_BUFF2}.\p");
static const u8 sText_UseNextPkmn[] = _("¿Quieres usar otro Pokémon?");
static const u8 sText_AttackMissed[] = _("¡El ataque de {B_ATK_NAME_WITH_PREFIX}\n falló!");
static const u8 sText_PkmnProtectedItself[] = _("¡{B_DEF_NAME_WITH_PREFIX}\nse protegió!");
static const u8 sText_AvoidedDamage[] = _("{B_DEF_NAME_WITH_PREFIX} avoided\ndamage with {B_DEF_ABILITY}!");
static const u8 sText_PkmnMakesGroundMiss[] = _("{B_DEF_NAME_WITH_PREFIX} makes GROUND\nmoves miss with {B_DEF_ABILITY}!");
static const u8 sText_PkmnAvoidedAttack[] = _("{B_DEF_NAME_WITH_PREFIX} avoided\nthe attack!");
static const u8 sText_ItDoesntAffect[] = _("It doesn't affect\n{B_DEF_NAME_WITH_PREFIX}…");
static const u8 sText_AttackerFainted[] = _("¡{B_ATK_NAME_WITH_PREFIX}\nse debilitó!\p");
static const u8 sText_TargetFainted[] = _("¡{B_DEF_NAME_WITH_PREFIX}\ncayó debilitado!\p");
static const u8 sText_PlayerGotMoney[] = _("{B_PLAYER_NAME} got ¥{B_BUFF1}\nfor winning!\p");
static const u8 sText_PlayerLostToEnemyTrainer[] = _("{B_PLAYER_NAME} is out of\nusable POKéMON!\pPlayer lost against\n{B_TRAINER1_CLASS} {B_TRAINER1_NAME}!{PAUSE_UNTIL_PRESS}");
static const u8 sText_PlayerPaidPrizeMoney[] = _("{B_PLAYER_NAME} paid ¥{B_BUFF1} as the prize\nmoney…\p… … … …\p{B_PLAYER_NAME} whited out!{PAUSE_UNTIL_PRESS}");
static const u8 sText_PlayerWhiteout[] = _("{B_PLAYER_NAME} is out of\nusable POKéMON!\p");
#if B_WHITEOUT_MONEY >= GEN_4
static const u8 sText_PlayerWhiteout2[] = _("You panicked and dropped ¥{B_BUFF1}…\pYou were overwhelmed by your defeat!{PAUSE_UNTIL_PRESS}");
#else
static const u8 sText_PlayerWhiteout2[] = _("You were overwhelmed by your defeat!{PAUSE_UNTIL_PRESS}");
#endif
static const u8 sText_PreventsEscape[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} prevents escape with {B_SCR_ACTIVE_ABILITY}!\p");
static const u8 sText_CantEscape2[] = _("You couldn't get away!\p");
static const u8 sText_AttackerCantEscape[] = _("{B_ATK_NAME_WITH_PREFIX} can't escape!");
static const u8 sText_HitXTimes[] = _("¡Golpeó {B_BUFF1} veces!");
static const u8 sText_PkmnFellAsleep[] = _("{B_EFF_NAME_WITH_PREFIX}\nfell asleep!");
static const u8 sText_PkmnMadeSleep[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_BUFF1}\nmade {B_EFF_NAME_WITH_PREFIX} sleep!");
static const u8 sText_PkmnAlreadyAsleep[] = _("{B_DEF_NAME_WITH_PREFIX} is\nalready asleep!");
static const u8 sText_PkmnAlreadyAsleep2[] = _("{B_ATK_NAME_WITH_PREFIX} is\nalready asleep!");
static const u8 sText_PkmnWasntAffected[] = _("{B_DEF_NAME_WITH_PREFIX}\nwasn't affected!");
static const u8 sText_PkmnWasPoisoned[] = _("¡{B_EFF_NAME_WITH_PREFIX}\nfue envenenado!");
static const u8 sText_PkmnPoisonedBy[] = _("¡{B_EFF_NAME_WITH_PREFIX} fue envenenado por\n{B_BUFF1} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnHurtByPoison[] = _("¡{B_ATK_NAME_WITH_PREFIX} es herido\npor el veneno!");
static const u8 sText_PkmnAlreadyPoisoned[] = _("{B_DEF_NAME_WITH_PREFIX} ya está\nenvenenado.");
static const u8 sText_PkmnBadlyPoisoned[] = _("¡{B_EFF_NAME_WITH_PREFIX} está\ngravemente envenenado!");
static const u8 sText_PkmnEnergyDrained[] = _("¡{B_DEF_NAME_WITH_PREFIX} fue\ndrenado!");
static const u8 sText_PkmnWasBurned[] = _("¡{B_EFF_NAME_WITH_PREFIX} fue quemado!");
static const u8 sText_PkmnGotFrostbite[] = _("¡{B_EFF_NAME_WITH_PREFIX} fue congelado!");
static const u8 sText_PkmnBurnedBy[] = _("¡{B_SCR_ACTIVE_ABILITY} de {B_SCR_ACTIVE_NAME_WITH_PREFIX}\nha quemado a {B_EFF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnHurtByBurn[] = _("¡{B_ATK_NAME_WITH_PREFIX} está dolido\npor su quemadura!");
static const u8 sText_PkmnHurtByFrostbite[] = _("{B_ATK_NAME_WITH_PREFIX} está dolido\npor su congelación!");
static const u8 sText_PkmnAlreadyHasBurn[] = _("{B_DEF_NAME_WITH_PREFIX} already\nhas a burn.");
static const u8 sText_PkmnWasFrozen[] = _("{B_EFF_NAME_WITH_PREFIX} was\nfrozen solid!");
static const u8 sText_PkmnFrozenBy[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_BUFF1}\nfroze {B_EFF_NAME_WITH_PREFIX} solid!");
static const u8 sText_PkmnIsFrozen[] = _("{B_ATK_NAME_WITH_PREFIX} is\nfrozen solid!");
static const u8 sText_PkmnWasDefrosted[] = _("{B_DEF_NAME_WITH_PREFIX} was\ndefrosted!");
static const u8 sText_PkmnWasDefrosted2[] = _("{B_ATK_NAME_WITH_PREFIX} was\ndefrosted!");
static const u8 sText_PkmnWasDefrostedBy[] = _("{B_ATK_NAME_WITH_PREFIX} was\ndefrosted by {B_CURRENT_MOVE}!");
static const u8 sText_PkmnFrostbiteHealed[] = _("{B_DEF_NAME_WITH_PREFIX}'s\nfrostbite was healed!");
static const u8 sText_PkmnFrostbiteHealed2[] = _("{B_ATK_NAME_WITH_PREFIX}'s\nfrostbite was healed!");
static const u8 sText_PkmnFrostbiteHealedBy[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_CURRENT_MOVE}\nhealed its frostbite!");
static const u8 sText_PkmnWasParalyzed[] = _("{B_EFF_NAME_WITH_PREFIX} is paralyzed!\nIt may be unable to move!");
static const u8 sText_PkmnWasParalyzedBy[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_BUFF1}\nparalyzed {B_EFF_NAME_WITH_PREFIX}!\lIt may be unable to move!");
static const u8 sText_PkmnIsParalyzed[] = _("{B_ATK_NAME_WITH_PREFIX} is paralyzed!\nIt can't move!");
static const u8 sText_PkmnIsAlreadyParalyzed[] = _("{B_DEF_NAME_WITH_PREFIX} is\nalready paralyzed!");
static const u8 sText_PkmnHealedParalysis[] = _("{B_DEF_NAME_WITH_PREFIX} was\nhealed of paralysis!");
static const u8 sText_PkmnDreamEaten[] = _("{B_DEF_NAME_WITH_PREFIX}'s\ndream was eaten!");
static const u8 sText_StatsWontIncrease[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1}\nwon't go higher!");
static const u8 sText_StatsWontDecrease[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1}\nwon't go lower!");
static const u8 sText_TeamStoppedWorking[] = _("Your team's {B_BUFF1}\nstopped working!");
static const u8 sText_FoeStoppedWorking[] = _("The foe's {B_BUFF1}\nstopped working!");
static const u8 sText_PkmnIsConfused[] = _("{B_ATK_NAME_WITH_PREFIX} is\nconfused!");
static const u8 sText_PkmnHealedConfusion[] = _("{B_ATK_NAME_WITH_PREFIX} snapped\nout of confusion!");
static const u8 sText_PkmnWasConfused[] = _("{B_EFF_NAME_WITH_PREFIX} became\nconfused!");
static const u8 sText_PkmnAlreadyConfused[] = _("{B_DEF_NAME_WITH_PREFIX} is\nalready confused!");
static const u8 sText_PkmnFellInLove[] = _("{B_DEF_NAME_WITH_PREFIX}\nfell in love!");
static const u8 sText_PkmnInLove[] = _("{B_ATK_NAME_WITH_PREFIX} is in love\nwith {B_SCR_ACTIVE_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnImmobilizedByLove[] = _("{B_ATK_NAME_WITH_PREFIX} is\nimmobilized by love!");
static const u8 sText_PkmnBlownAway[] = _("{B_DEF_NAME_WITH_PREFIX} was\nblown away!");
static const u8 sText_PkmnChangedType[] = _("{B_ATK_NAME_WITH_PREFIX} transformed\ninto the {B_BUFF1} type!");
static const u8 sText_PkmnFlinched[] = _("{B_ATK_NAME_WITH_PREFIX} flinched!");
static const u8 sText_PkmnRegainedHealth[] = _("{B_DEF_NAME_WITH_PREFIX} regained\nhealth!");
static const u8 sText_PkmnHPFull[] = _("{B_DEF_NAME_WITH_PREFIX}'s\nHP is full!");
static const u8 sText_PkmnRaisedSpDef[] = _("{B_ATK_PREFIX2}'s {B_CURRENT_MOVE}\nraised SP. DEF!");
static const u8 sText_PkmnRaisedSpDefALittle[] = _("{B_ATK_PREFIX2}'s {B_CURRENT_MOVE}\nraised SP. DEF a little!");
static const u8 sText_PkmnRaisedDef[] = _("{B_ATK_PREFIX2}'s {B_CURRENT_MOVE}\nraised DEFENSE!");
static const u8 sText_PkmnRaisedDefALittle[] = _("{B_ATK_PREFIX2}'s {B_CURRENT_MOVE}\nraised DEFENSE a little!");
static const u8 sText_PkmnCoveredByVeil[] = _("{B_ATK_PREFIX2}'s party is covered\nby a veil!");
static const u8 sText_PkmnUsedSafeguard[] = _("{B_DEF_NAME_WITH_PREFIX}'s party is protected\nby Safeguard!");
static const u8 sText_PkmnSafeguardExpired[] = _("{B_ATK_PREFIX3}'s party is no longer\nprotected by Safeguard!");
static const u8 sText_PkmnWentToSleep[] = _("{B_ATK_NAME_WITH_PREFIX} went\nto sleep!");
static const u8 sText_PkmnSleptHealthy[] = _("{B_ATK_NAME_WITH_PREFIX} slept and\nbecame healthy!");
static const u8 sText_PkmnWhippedWhirlwind[] = _("{B_ATK_NAME_WITH_PREFIX} whipped\nup a whirlwind!");
static const u8 sText_PkmnTookSunlight[] = _("{B_ATK_NAME_WITH_PREFIX} took\nin sunlight!");
static const u8 sText_PkmnLoweredHead[] = _("{B_ATK_NAME_WITH_PREFIX} lowered\nits head!");
static const u8 sText_PkmnIsGlowing[] = _("{B_ATK_NAME_WITH_PREFIX} is glowing!");
static const u8 sText_PkmnIsCloakedInAHarshLight[] = _("{B_ATK_NAME_WITH_PREFIX} became\ncloaked in a harsh light!");
static const u8 sText_Amoladoras[] = _("¡{B_ATK_NAME_WITH_PREFIX} afiló comillos para\nsubir ataque y críticos!");
static const u8 sText_PkmnFlewHigh[] = _("{B_ATK_NAME_WITH_PREFIX} flew\nup high!");
static const u8 sText_PkmnDugHole[] = _("{B_ATK_NAME_WITH_PREFIX} dug a hole!");
static const u8 sText_PkmnHidUnderwater[] = _("{B_ATK_NAME_WITH_PREFIX} hid\nunderwater!");
static const u8 sText_PkmnSprangUp[] = _("{B_ATK_NAME_WITH_PREFIX} sprang up!");
static const u8 sText_PkmnSqueezedByBind[] = _("{B_DEF_NAME_WITH_PREFIX} was squeezed by\n{B_ATK_NAME_WITH_PREFIX}'s BIND!");
static const u8 sText_PkmnInSnapTrap[] = _("{B_DEF_NAME_WITH_PREFIX} got trapped\nby a snap trap!");
static const u8 sText_PkmnTrappedInVortex[] = _("{B_DEF_NAME_WITH_PREFIX} was trapped\nin the vortex!");
static const u8 sText_PkmnTrappedBySandTomb[] = _("{B_DEF_NAME_WITH_PREFIX} was trapped\nby SAND TOMB!");
static const u8 sText_PkmnWrappedBy[] = _("{B_DEF_NAME_WITH_PREFIX} was WRAPPED by\n{B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnClamped[] = _("{B_ATK_NAME_WITH_PREFIX} CLAMPED\n{B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnHurtBy[] = _("{B_ATK_NAME_WITH_PREFIX} is hurt\nby {B_BUFF1}!");
static const u8 sText_PkmnFreedFrom[] = _("{B_ATK_NAME_WITH_PREFIX} was freed\nfrom {B_BUFF1}!");
static const u8 sText_PkmnCrashed[] = _("{B_ATK_NAME_WITH_PREFIX} kept going\nand crashed!");
const u8 gText_PkmnShroudedInMist[] = _("{B_ATK_PREFIX2} became\nshrouded in MIST!");
static const u8 sText_PkmnProtectedByMist[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is protected\nby MIST!");
const u8 gText_PkmnGettingPumped[] = _("{B_DEF_NAME_WITH_PREFIX} is getting\npumped!");
static const u8 sText_PkmnHitWithRecoil[] = _("{B_ATK_NAME_WITH_PREFIX} is hit\nwith recoil!");
static const u8 sText_PkmnProtectedItself2[] = _("¡{B_ATK_NAME_WITH_PREFIX} se protegió!");
static const u8 sText_PkmnBuffetedBySandstorm[] = _("{B_ATK_NAME_WITH_PREFIX} is buffeted\nby the sandstorm!");
static const u8 sText_PkmnPeltedByHail[] = _("{B_ATK_NAME_WITH_PREFIX} is pelted\nby HAIL!");
static const u8 sText_PkmnsXWoreOff[] = _("{B_ATK_PREFIX1}'s {B_BUFF1}\nwore off!");
static const u8 sText_PkmnSeeded[] = _("{B_DEF_NAME_WITH_PREFIX} was seeded!");
static const u8 sText_PkmnEvadedAttack[] = _("{B_DEF_NAME_WITH_PREFIX} avoided the attack!");
static const u8 sText_PkmnSappedByLeechSeed[] = _("{B_ATK_NAME_WITH_PREFIX}'s health is sapped by Leech Seed!");
static const u8 sText_PkmnFastAsleep[] = _("{B_ATK_NAME_WITH_PREFIX} is fast asleep.");
static const u8 sText_PkmnWokeUp[] = _("{B_ATK_NAME_WITH_PREFIX} woke up!");
static const u8 sText_PkmnUproarKeptAwake[] = _("But the uproar kept {B_SCR_ACTIVE_NAME_WITH_PREFIX} awake!");
static const u8 sText_PkmnWokeUpInUproar[] = _("The uproar woke {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnCausedUproar[] = _("{B_ATK_NAME_WITH_PREFIX} caused an uproar!");
static const u8 sText_PkmnMakingUproar[] = _("{B_ATK_NAME_WITH_PREFIX} is making an uproar!");
static const u8 sText_PkmnCalmedDown[] = _("{B_ATK_NAME_WITH_PREFIX} calmed down.");
static const u8 sText_PkmnCantSleepInUproar[] = _("But {B_DEF_NAME_WITH_PREFIX} can't\nsleep in an UPROAR!");
static const u8 sText_PkmnStockpiled[] = _("{B_ATK_NAME_WITH_PREFIX} stockpiled\n{B_BUFF1}!");
static const u8 sText_PkmnCantStockpile[] = _("{B_ATK_NAME_WITH_PREFIX} can't\nstockpile any more!");
static const u8 sText_PkmnCantSleepInUproar2[] = _("But {B_DEF_NAME_WITH_PREFIX} can't\nsleep in an UPROAR!");
static const u8 sText_UproarKeptPkmnAwake[] = _("But the UPROAR kept\n{B_DEF_NAME_WITH_PREFIX} awake!");
static const u8 sText_PkmnStayedAwakeUsing[] = _("{B_DEF_NAME_WITH_PREFIX} stayed awake\nusing its {B_DEF_ABILITY}!");
static const u8 sText_PkmnStoringEnergy[] = _("{B_ATK_NAME_WITH_PREFIX} is storing\nenergy!");
static const u8 sText_PkmnUnleashedEnergy[] = _("{B_ATK_NAME_WITH_PREFIX} unleashed\nenergy!");
static const u8 sText_PkmnFatigueConfusion[] = _("{B_ATK_NAME_WITH_PREFIX} became\nconfused due to fatigue!");
static const u8 sText_PlayerPickedUpMoney[] = _("{B_PLAYER_NAME} picked up\n¥{B_BUFF1}!\p");
static const u8 sText_PkmnUnaffected[] = _("{B_DEF_NAME_WITH_PREFIX} is\nunaffected!");
static const u8 sText_PkmnTransformedInto[] = _("{B_ATK_NAME_WITH_PREFIX} transformed\ninto {B_BUFF1}!");
static const u8 sText_PkmnMadeSubstitute[] = _("{B_ATK_NAME_WITH_PREFIX} made\na SUBSTITUTE!");
static const u8 sText_PkmnHasSubstitute[] = _("{B_ATK_NAME_WITH_PREFIX} already\nhas a SUBSTITUTE!");
static const u8 sText_SubstituteDamaged[] = _("The SUBSTITUTE took damage\nfor {B_DEF_NAME_WITH_PREFIX}!\p");
static const u8 sText_PkmnSubstituteFaded[] = _("{B_DEF_NAME_WITH_PREFIX}'s\nSUBSTITUTE faded!\p");
static const u8 sText_PkmnMustRecharge[] = _("{B_ATK_NAME_WITH_PREFIX} must\nrecharge!");
static const u8 sText_PkmnRageBuilding[] = _("{B_DEF_NAME_WITH_PREFIX}'s RAGE\nis building!");
static const u8 sText_PkmnMoveWasDisabled[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1}\nwas disabled!");
static const u8 sText_PkmnMoveDisabledNoMore[] = _("{B_ATK_NAME_WITH_PREFIX} is disabled\nno more!");
static const u8 sText_PkmnGotEncore[] = _("{B_DEF_NAME_WITH_PREFIX} got\nan ENCORE!");
static const u8 sText_PkmnEncoreEnded[] = _("{B_ATK_NAME_WITH_PREFIX}'s ENCORE\nended!");
static const u8 sText_PkmnTookAim[] = _("{B_ATK_NAME_WITH_PREFIX} took aim\nat {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnSketchedMove[] = _("{B_ATK_NAME_WITH_PREFIX} SKETCHED\n{B_BUFF1}!");
static const u8 sText_PkmnTryingToTakeFoe[] = _("{B_ATK_NAME_WITH_PREFIX} is trying\nto take its foe with it!");
static const u8 sText_PkmnTookFoe[] = _("{B_DEF_NAME_WITH_PREFIX} took\n{B_ATK_NAME_WITH_PREFIX} with it!");
static const u8 sText_PkmnReducedPP[] = _("Reduced {B_DEF_NAME_WITH_PREFIX}'s\n{B_BUFF1} by {B_BUFF2}!");
static const u8 sText_PkmnStoleItem[] = _("{B_ATK_NAME_WITH_PREFIX} stole\n{B_DEF_NAME_WITH_PREFIX}'s {B_LAST_ITEM}!");
static const u8 sText_TargetCantEscapeNow[] = _("{B_DEF_NAME_WITH_PREFIX} can't\nescape now!");
static const u8 sText_PkmnFellIntoNightmare[] = _("{B_DEF_NAME_WITH_PREFIX} fell into\na NIGHTMARE!");
static const u8 sText_PkmnLockedInNightmare[] = _("{B_ATK_NAME_WITH_PREFIX} is locked\nin a NIGHTMARE!");
static const u8 sText_PkmnLaidCurse[] = _("{B_ATK_NAME_WITH_PREFIX} cut its own HP and\nlaid a CURSE on {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnAfflictedByCurse[] = _("{B_ATK_NAME_WITH_PREFIX} is afflicted\nby the CURSE!");
static const u8 sText_SpikesScattered[] = _("¡Han caído púas\nen el campo de {B_DEF_TEAM2}!");
static const u8 sText_PkmnHurtBySpikes[] = _("¡{B_SCR_ACTIVE_NAME_WITH_PREFIX} se ha pinchado\ncon las púas!");
static const u8 sText_PkmnIdentified[] = _("{B_ATK_NAME_WITH_PREFIX} identified\n{B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnPerishCountFell[] = _("{B_ATK_NAME_WITH_PREFIX}'s PERISH count\nfell to {B_BUFF1}!");
static const u8 sText_PkmnBracedItself[] = _("{B_ATK_NAME_WITH_PREFIX} braced\nitself!");
static const u8 sText_PkmnEnduredHit[] = _("{B_DEF_NAME_WITH_PREFIX} ENDURED\nthe hit!");
static const u8 sText_MagnitudeStrength[] = _("MAGNITUDE {B_BUFF1}!");
static const u8 sText_PkmnCutHPMaxedAttack[] = _("{B_ATK_NAME_WITH_PREFIX} cut its own HP\nand maximized ATTACK!");
static const u8 sText_PkmnCopiedStatChanges[] = _("{B_ATK_NAME_WITH_PREFIX} copied\n{B_DEF_NAME_WITH_PREFIX}'s stat changes!");
static const u8 sText_PkmnGotFree[] = _("{B_ATK_NAME_WITH_PREFIX} got free of\n{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1}!");
static const u8 sText_PkmnShedLeechSeed[] = _("{B_ATK_NAME_WITH_PREFIX} shed\nLEECH SEED!");
static const u8 sText_PkmnBlewAwaySpikes[] = _("{B_ATK_NAME_WITH_PREFIX} blew away\nspikes!");
static const u8 sText_PkmnFledFromBattle[] = _("{B_ATK_NAME_WITH_PREFIX} fled from\nbattle!");
static const u8 sText_PkmnForesawAttack[] = _("{B_ATK_NAME_WITH_PREFIX} foresaw\nan attack!");
static const u8 sText_PkmnTookAttack[] = _("{B_DEF_NAME_WITH_PREFIX} took the\n{B_BUFF1} attack!");
static const u8 sText_PkmnChoseXAsDestiny[] = _("{B_ATK_NAME_WITH_PREFIX} chose\n{B_CURRENT_MOVE} as its destiny!");
static const u8 sText_PkmnAttack[] = _("{B_BUFF1}'s attack!");
static const u8 sText_PkmnCenterAttention[] = _("{B_DEF_NAME_WITH_PREFIX} became the\ncenter of attention!");
static const u8 sText_PkmnChargingPower[] = _("{B_ATK_NAME_WITH_PREFIX} began\ncharging power!");
static const u8 sText_NaturePowerTurnedInto[] = _("NATURE POWER turned into\n{B_CURRENT_MOVE}!");
static const u8 sText_PkmnStatusNormal[] = _("{B_ATK_NAME_WITH_PREFIX}'s status\nreturned to normal!");
static const u8 sText_PkmnSubjectedToTorment[] = _("{B_DEF_NAME_WITH_PREFIX} was subjected\nto torment!");
static const u8 sText_PkmnTighteningFocus[] = _("{B_ATK_NAME_WITH_PREFIX} is tightening\nits focus!");
static const u8 sText_PkmnFellForTaunt[] = _("{B_DEF_NAME_WITH_PREFIX} fell for\nthe Taunt!");
static const u8 sText_PkmnReadyToHelp[] = _("{B_ATK_NAME_WITH_PREFIX} is ready to\nhelp {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnSwitchedItems[] = _("¡{B_ATK_NAME_WITH_PREFIX} cambió\nobjetos con el rival!");
static const u8 sText_PkmnObtainedX[] = _("{B_ATK_NAME_WITH_PREFIX} obtuvo\n{B_BUFF1}.");
static const u8 sText_PkmnObtainedX2[] = _("{B_DEF_NAME_WITH_PREFIX} obtuvo\n{B_BUFF2}.");
static const u8 sText_PkmnObtainedXYObtainedZ[] = _("{B_ATK_NAME_WITH_PREFIX} obtuvo\n{B_BUFF1}.\p{B_DEF_NAME_WITH_PREFIX} obtuvo\n{B_BUFF2}.");
static const u8 sText_PkmnCopiedFoe[] = _("{B_ATK_NAME_WITH_PREFIX} copied\n{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}!");
static const u8 sText_PkmnMadeWish[] = _("{B_ATK_NAME_WITH_PREFIX} made a WISH!");
static const u8 sText_PkmnWishCameTrue[] = _("{B_BUFF1}'s WISH\ncame true!");
static const u8 sText_PkmnPlantedRoots[] = _("{B_ATK_NAME_WITH_PREFIX} planted its roots!");
static const u8 sText_PkmnAbsorbedNutrients[] = _("{B_ATK_NAME_WITH_PREFIX} absorbed\nnutrients with its roots!");
static const u8 sText_PkmnAnchoredItself[] = _("{B_DEF_NAME_WITH_PREFIX} anchored\nitself with its roots!");
static const u8 sText_PkmnWasMadeDrowsy[] = _("{B_ATK_NAME_WITH_PREFIX} made\n{B_DEF_NAME_WITH_PREFIX} drowsy!");
static const u8 sText_PkmnKnockedOff[] = _("{B_ATK_NAME_WITH_PREFIX} knocked off\n{B_DEF_NAME_WITH_PREFIX}'s {B_LAST_ITEM}!");
static const u8 sText_PkmnSwappedAbilities[] = _("{B_ATK_NAME_WITH_PREFIX} swapped abilities\nwith its opponent!");
static const u8 sText_PkmnSealedOpponentMove[] = _("{B_ATK_NAME_WITH_PREFIX} sealed the\nopponent's move(s)!");
static const u8 sText_PkmnWantsGrudge[] = _("{B_ATK_NAME_WITH_PREFIX} wants the\nopponent to bear a GRUDGE!");
static const u8 sText_PkmnLostPPGrudge[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1} lost\nall its PP due to the GRUDGE!");
static const u8 sText_PkmnShroudedItself[] = _("{B_ATK_NAME_WITH_PREFIX} shrouded\nitself in {B_CURRENT_MOVE}!");
static const u8 sText_PkmnMoveBounced[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_CURRENT_MOVE}\nwas bounced back by MAGIC COAT!");
static const u8 sText_PkmnWaitsForTarget[] = _("{B_ATK_NAME_WITH_PREFIX} waits for a target\nto make a move!");
static const u8 sText_PkmnSnatchedMove[] = _("{B_DEF_NAME_WITH_PREFIX} SNATCHED\n{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s move!");
static const u8 sText_ElectricityWeakened[] = _("Electricity's power was\nweakened!");
static const u8 sText_FireWeakened[] = _("Fire's power was\nweakened!");
static const u8 sText_XFoundOneY[] = _("{B_ATK_NAME_WITH_PREFIX} found\none {B_LAST_ITEM}!");
static const u8 sText_SoothingAroma[] = _("A soothing aroma wafted\nthrough the area!");
static const u8 sText_ItemsCantBeUsedNow[] = _("Items can't be used now.{PAUSE 64}");
static const u8 sText_ForXCommaYZ[] = _("For {B_SCR_ACTIVE_NAME_WITH_PREFIX},\n{B_LAST_ITEM} {B_BUFF1}");
static const u8 sText_PkmnUsedXToGetPumped[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} used\n{B_LAST_ITEM} to get pumped!");
static const u8 sText_PkmnLostFocus[] = _("{B_ATK_NAME_WITH_PREFIX} lost its\nfocus and couldn't move!");
static const u8 sText_PkmnWasDraggedOut[] = _("{B_DEF_NAME_WITH_PREFIX} was\ndragged out!\p");
static const u8 sText_TheWallShattered[] = _("The wall shattered!");
static const u8 sText_ButNoEffect[] = _("But it had no effect!");
static const u8 sText_PkmnMoveIsDisabled[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_CURRENT_MOVE}\nis disabled!\p");
static const u8 sText_PkmnCantUseMoveTorment[] = _("{B_ATK_NAME_WITH_PREFIX} can't use the same\nmove in a row due to the Torment!\p");
static const u8 sText_PkmnCantUseMoveTaunt[] = _("{B_ATK_NAME_WITH_PREFIX} can't use\n{B_CURRENT_MOVE} after the Taunt!\p");
static const u8 sText_PkmnCantUseMoveSealed[] = _("{B_ATK_NAME_WITH_PREFIX} can't use the\nsealed {B_CURRENT_MOVE}!\p");
static const u8 sText_PkmnCantUseMoveThroatChop[] = _("{B_ATK_NAME_WITH_PREFIX} can't use\n{B_CURRENT_MOVE} due to Throat Chop!\p");
static const u8 sText_PkmnMadeItRain[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nmade it rain!");
static const u8 sText_PkmnRaisedSpeed[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nraised its SPEED!");
static const u8 sText_PkmnProtectedBy[] = _("{B_DEF_NAME_WITH_PREFIX} was protected\nby {B_DEF_ABILITY}!");
static const u8 sText_PkmnPreventsUsage[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nprevents {B_ATK_NAME_WITH_PREFIX}\lfrom using {B_CURRENT_MOVE}!");
static const u8 sText_PkmnRestoredHPUsing[] = _("{B_DEF_NAME_WITH_PREFIX} restored HP\nusing its {B_DEF_ABILITY}!");
static const u8 sText_PkmnsXMadeYUseless[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nmade {B_CURRENT_MOVE} useless!");
static const u8 sText_PkmnChangedTypeWith[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nmade it the {B_BUFF1} type!");
static const u8 sText_PkmnPreventsParalysisWith[] = _("{B_EFF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nprevents paralysis!");
static const u8 sText_PkmnPreventsRomanceWith[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nprevents romance!");
static const u8 sText_PkmnPreventsPoisoningWith[] = _("{B_EFF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nprevents poisoning!");
static const u8 sText_PkmnPreventsConfusionWith[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nprevents confusion!");
static const u8 sText_PkmnRaisedFirePowerWith[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nraised its FIRE power!");
static const u8 sText_PkmnAnchorsItselfWith[] = _("{B_DEF_NAME_WITH_PREFIX} anchors\nitself with {B_DEF_ABILITY}!");
static const u8 sText_PkmnCutsAttackWith[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\ncuts {B_DEF_NAME_WITH_PREFIX}'s attack!");
static const u8 sText_PkmnPreventsStatLossWith[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nprevents stat loss!");
static const u8 sText_PkmnHurtsWith[] = _("{B_ATK_NAME_WITH_PREFIX} was hurt by\n{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1}!");
static const u8 sText_PkmnTraced[] = _("¡{B_SCR_ACTIVE_NAME_WITH_PREFIX} rastreó\n{B_BUFF2} de {B_BUFF1}!");
static const u8 sText_PkmnsXPreventsBurns[] = _("{B_EFF_NAME_WITH_PREFIX}'s {B_EFF_ABILITY}\nprevents burns!");
static const u8 sText_PkmnsXBlocksY[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nblocks {B_CURRENT_MOVE}!");
static const u8 sText_PkmnsXBlocksY2[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nblocks {B_CURRENT_MOVE}!");
static const u8 sText_PkmnsXRestoredHPALittle2[] = _("¡{B_ATK_ABILITY} de {B_ATK_NAME_WITH_PREFIX} le\nhizo recuperar salud!");
static const u8 sText_PkmnsXWhippedUpSandstorm[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nwhipped up a sandstorm!");
static const u8 sText_PkmnsXIntensifiedSun[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nintensified the sun's rays!");
static const u8 sText_PkmnsXPreventsYLoss[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nprevents {B_BUFF1} loss!");
static const u8 sText_PkmnsXInfatuatedY[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\ninfatuated {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_NueveColas[] = _("¡Se ha activado la maldición de\nlas nueve colas!");
static const u8 sText_Parasito[] = _("¡{B_DEF_NAME_WITH_PREFIX}\nle ha pasado su {B_DEF_ABILITY} a {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnsXMadeYIneffective[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nmade {B_CURRENT_MOVE} ineffective!");
static const u8 sText_PkmnsXCuredYProblem[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\ncured its {B_BUFF1} problem!");
static const u8 sText_ItSuckedLiquidOoze[] = _("It sucked up the\nliquid ooze!");
static const u8 sText_PkmnTransformed[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} transformed!");
static const u8 sText_PkmnsXTookAttack[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY} took the attack!"); //In gen 5+ but without naming the ability
const u8 gText_PkmnsXPreventsSwitching[] = _("{B_BUFF1} is preventing switching out with its {B_LAST_ABILITY} Ability!\p");
static const u8 sText_PkmnsXMadeItIneffective[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY} made it ineffective!"); //not in gen 5+, ability popup
static const u8 sText_PkmnsXPreventsFlinching[] = _("{B_EFF_NAME_WITH_PREFIX}'s {B_EFF_ABILITY} prevents flinching!"); //not in gen 5+, ability popup
static const u8 sText_PkmnsXPreventsYsZ[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_ATK_ABILITY} prevents {B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY} from working!");
static const u8 sText_PkmnsAbilityPreventsAbility[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY} prevents {B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY} from working!"); //not in gen 5+, ability popup
static const u8 sText_PkmnsXCuredItsYProblem[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY} cured its {B_BUFF1} problem!"); //not in gen 5+, ability popup
static const u8 sText_PkmnsXHadNoEffectOnY[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY} had no effect on {B_EFF_NAME_WITH_PREFIX}!"); //not in gen 5+, ability popup
const u8 gText_StatSharply[] = _("sharply ");
const u8 gText_StatRose[] = _("rose!");
static const u8 sText_StatHarshly[] = _("harshly ");
const u8 sText_StatFell[] = _("fell!");
static const u8 sText_AttackersStatRose[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1}\n{B_BUFF2}");
const u8 gText_DefendersStatRose[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1}\n{B_BUFF2}");
static const u8 sText_UsingItemTheStatOfPkmnRose[] = _("Using {B_LAST_ITEM}, the {B_BUFF1}\nof {B_SCR_ACTIVE_NAME_WITH_PREFIX} {B_BUFF2}");
static const u8 sText_AttackersStatFell[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1}\n{B_BUFF2}");
static const u8 sText_DefendersStatFell[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1}\n{B_BUFF2}");
static const u8 sText_StatsWontIncrease2[] = _("¡Las estadísticas de {B_ATK_NAME_WITH_PREFIX}\nno pueden subir más!");
static const u8 sText_StatsWontDecrease2[] = _("{B_DEF_NAME_WITH_PREFIX}'s stats won't\ngo any lower!");
static const u8 sText_CriticalHit[] = _("¡Un golpe crítico!");
static const u8 sText_OneHitKO[] = _("It's a one-hit KO!");
static const u8 sText_123Poof[] = _("{PAUSE 32}1, {PAUSE 15}2, y{PAUSE 15}… {PAUSE 15}… {PAUSE 15}… {PAUSE 15}{PLAY_SE SE_BALL_BOUNCE_1}¡Guau!\p");
static const u8 sText_AndEllipsis[] = _("Y…\p");
static const u8 sText_NotVeryEffective[] = _("No es muy efectivo…");
static const u8 sText_SuperEffective[] = _("¡Es super efectivo!");
static const u8 sText_GotAwaySafely[] = _("{PLAY_SE SE_FLEE}¡Lograste huir!\p");
static const u8 sText_PkmnFledUsingIts[] = _("{PLAY_SE SE_FLEE}{B_ATK_NAME_WITH_PREFIX} fled\nusing its {B_LAST_ITEM}!\p");
static const u8 sText_PkmnFledUsing[] = _("¡{PLAY_SE SE_FLEE}La {B_ATK_ABILITY} de {B_ATK_NAME_WITH_PREFIX}\le permite huir!\p");
static const u8 sText_WildPkmnFled[] = _("¡{PLAY_SE SE_FLEE}El {B_BUFF1} salvaje huyó!");
static const u8 sText_NoRunningFromTrainers[] = _("No! There's no running\nfrom a TRAINER battle!\p");
static const u8 sText_CantEscape[] = _("¡No puedes escapar!\p");
static const u8 sText_DontLeaveBirch[] = _("PROF. BIRCH: Don't leave me like this!\p");
static const u8 sText_ButNothingHappened[] = _("¡Pero no ha pasado nada!");
static const u8 sText_ButItFailed[] = _("¡Pero falló!");
static const u8 sText_ItHurtConfusion[] = _("It hurt itself in its\nconfusion!");
static const u8 sText_MirrorMoveFailed[] = _("The Mirror Move failed!");
static const u8 sText_StartedToRain[] = _("¡Ha empezado a llover!");
static const u8 sText_DownpourStarted[] = _("A downpour started!"); // corresponds to DownpourText in pokegold and pokecrystal and is used by Rain Dance in GSC
static const u8 sText_RainContinues[] = _("Sigue lloviendo…");
static const u8 sText_DownpourContinues[] = _("The downpour continues."); // unused
static const u8 sText_RainStopped[] = _("The rain stopped.");
static const u8 sText_SandstormBrewed[] = _("A sandstorm kicked up!");
static const u8 sText_SandstormRages[] = _("The sandstorm is raging.");
static const u8 sText_SandstormSubsided[] = _("The sandstorm subsided.");
static const u8 sText_SunlightGotBright[] = _("¡El Sol ha empezado a brillar!");
static const u8 sText_SunlightStrong[] = _("El Sol brilla fuerte.");
static const u8 sText_SunlightFaded[] = _("El Sol se ha tapado.");
static const u8 sText_StartedHail[] = _("¡Está granizando!");
static const u8 sText_HailContinues[] = _("Sigue granizando.");
static const u8 sText_HailStopped[] = _("Ha cesado el granizo.");
static const u8 sText_StartedSnow[] = _("¡Ha empezado a nevar!");
static const u8 sText_SnowContinues[] = _("La nieve sigue cayendo");
static const u8 sText_SnowStopped[] = _("La nieve ha parado.");
static const u8 sText_FogCreptUp[] = _("Fog crept up as thick as soup!");
static const u8 sText_FogIsDeep[] = _("The fog is deep…");
static const u8 sText_FogLifted[] = _("The fog lifted.");
static const u8 sText_FailedToSpitUp[] = _("But it failed to spit up a thing!"); //not in gen 5+, uses "but it failed"
static const u8 sText_FailedToSwallow[] = _("But it failed to swallow a thing!"); //not in gen 5+, uses "but it failed"
static const u8 sText_WindBecameHeatWave[] = _("The wind turned into a Heat Wave!"); //unused
static const u8 sText_StatChangesGone[] = _("All stat changes were eliminated!");
static const u8 sText_CoinsScattered[] = _("Coins were scattered everywhere!");
static const u8 sText_TooWeakForSubstitute[] = _("But it does not have enough HP left to make a substitute!");
static const u8 sText_SharedPain[] = _("The battlers shared their pain!");
static const u8 sText_BellChimed[] = _("A bell chimed!");
static const u8 sText_FaintInThree[] = _("All affected POKéMON will\nfaint in three turns!");
static const u8 sText_NoPPLeft[] = _("¡No te quedan PP!\p");
static const u8 sText_ButNoPPLeft[] = _("But there was no PP left\nfor the move!");
static const u8 sText_PkmnIgnoresAsleep[] = _("{B_ATK_NAME_WITH_PREFIX} ignored\norders while asleep!");
static const u8 sText_PkmnIgnoredOrders[] = _("{B_ATK_NAME_WITH_PREFIX} ignored\norders!");
static const u8 sText_PkmnBeganToNap[] = _("{B_ATK_NAME_WITH_PREFIX} began to nap!");
static const u8 sText_PkmnLoafing[] = _("{B_ATK_NAME_WITH_PREFIX} is loafing around!");
static const u8 sText_PkmnWontObey[] = _("{B_ATK_NAME_WITH_PREFIX} won't obey!");
static const u8 sText_PkmnTurnedAway[] = _("{B_ATK_NAME_WITH_PREFIX} turned away!");
static const u8 sText_PkmnPretendNotNotice[] = _("{B_ATK_NAME_WITH_PREFIX} pretended not to notice!");
static const u8 sText_EnemyAboutToSwitchPkmn[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME} is about to send out {B_BUFF2}.\pWill you switch your Pokémon?");
static const u8 sText_PkmnLearnedMove2[] = _("{B_ATK_NAME_WITH_PREFIX} learned {B_BUFF1}!");
static const u8 sText_CreptCloser[] = _("{B_PLAYER_NAME} crept closer to {B_OPPONENT_MON1_NAME}!"); //safari
static const u8 sText_CantGetCloser[] = _("{B_PLAYER_NAME} can't get any closer!"); //safari
static const u8 sText_PkmnWatchingCarefully[] = _("{B_OPPONENT_MON1_NAME} is watching carefully!"); //safari
static const u8 sText_PkmnCuriousAboutX[] = _("{B_OPPONENT_MON1_NAME} is curious about the {B_BUFF1}!"); //safari
static const u8 sText_PkmnEnthralledByX[] = _("{B_OPPONENT_MON1_NAME} is enthralled by the {B_BUFF1}!"); //safari
static const u8 sText_PkmnIgnoredX[] = _("{B_OPPONENT_MON1_NAME} completely ignored the {B_BUFF1}!"); //safari
static const u8 sText_ThrewPokeblockAtPkmn[] = _("{B_PLAYER_NAME} threw a {POKEBLOCK} at the {B_OPPONENT_MON1_NAME}!"); //safari
static const u8 sText_OutOfSafariBalls[] = _("{PLAY_SE SE_DING_DONG}ANNOUNCER: You're out of Safari Balls! Game over!\p"); //safari
static const u8 sText_OpponentMon1Appeared[] = _("{B_OPPONENT_MON1_NAME} appeared!\p");
static const u8 sText_UnPokemonSalvaje[] = _("¡Un {B_OPPONENT_MON1_NAME} salvaje!\p");
static const u8 sText_ElLegendario[] = _("¡El Legendario {B_OPPONENT_MON1_NAME}!\p");
static const u8 sText_EntrenadorQuiereLuchar[] = _("¡{B_TRAINER1_CLASS} {B_TRAINER1_NAME}\nquiere luchar!\p");
static const u8 sText_Trainer1SentOutPkmn[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME} sent\nout {B_OPPONENT_MON1_NAME}!");
static const u8 sText_Trainer1SentOutTwoPkmn[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME} sent\nout {B_OPPONENT_MON1_NAME} and {B_OPPONENT_MON2_NAME}!");
static const u8 sText_Trainer1SentOutPkmn2[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME} sent\nout {B_BUFF1}!");
static const u8 sText_GoPkmn[] = _("¡Vamos, {B_PLAYER_MON1_NAME}!");
static const u8 sText_GoTwoPkmn[] = _("¡Vamos! ¡{B_PLAYER_MON1_NAME} y\n{B_PLAYER_MON2_NAME}!");
static const u8 sText_GoPkmn2[] = _("¡A luchar, {B_BUFF1}!");
static const u8 sText_DoItPkmn[] = _("¡Hazlo, {B_BUFF1}!");
static const u8 sText_GoForItPkmn[] = _("¡A por ello, {B_BUFF1}!");
static const u8 sText_YourFoesWeakGetEmPkmn[] = _("¡Tu rival está débil!\n¡Tú puedes, {B_BUFF1}!");
static const u8 sText_PkmnThatsEnough[] = _("¡{B_BUFF1}, ya es suficiente!\n¡Vuelve!");
static const u8 sText_PkmnComeBack[] = _("¡{B_BUFF1}, ven conmigo!");
static const u8 sText_PkmnOkComeBack[] = _("¡{B_BUFF1}, bien!\n¡Cambio!");
static const u8 sText_PkmnGoodComeBack[] = _("¡Lo has hecho bien, {B_BUFF1}!\n¡Descansa!");
static const u8 sText_Trainer1WithdrewPkmn[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME}\nguardó a {B_BUFF1}!");
static const u8 sText_WildPkmnPrefix[] = _(" salvaje");
static const u8 sText_FoePkmnPrefix[] = _(" rival");
static const u8 sText_FoePkmnPrefix2[] = _("Foe");
static const u8 sText_AllyPkmnPrefix[] = _("Ally");
static const u8 sText_FoePkmnPrefix3[] = _("Opposing");
static const u8 sText_AllyPkmnPrefix2[] = _("Ally");
static const u8 sText_FoePkmnPrefix4[] = _("Opposing");
static const u8 sText_AllyPkmnPrefix3[] = _("Ally");
static const u8 sText_AttackerUsedX[] = _("¡{B_ATK_NAME_WITH_PREFIX} usó\n{B_BUFF3}!");

const u8 *const gStatNamesTable[NUMERO_ESTADISTICAS_BATALLA] =
{
    gText_PS, gText_Ataque, gText_Defensa,
    gText_Velocidad, gText_AtEsp, gText_DefEsp,
    gText_Precision, gText_Evasion
};

static const u8 sText_PokeblockWasTooSpicy[] = _("was too spicy!");
static const u8 sText_PokeblockWasTooDry[] = _("was too dry!");
static const u8 sText_PokeblockWasTooSweet[] = _("was too sweet!");
static const u8 sText_PokeblockWasTooBitter[] = _("was too bitter!");
static const u8 sText_PokeblockWasTooSour[] = _("was too sour!");

const u8 *const gPokeblockWasTooXStringTable[FLAVOR_COUNT] =
{
    [FLAVOR_SPICY]  = sText_PokeblockWasTooSpicy,
    [FLAVOR_DRY]    = sText_PokeblockWasTooDry,
    [FLAVOR_SWEET]  = sText_PokeblockWasTooSweet,
    [FLAVOR_BITTER] = sText_PokeblockWasTooBitter,
    [FLAVOR_SOUR]   = sText_PokeblockWasTooSour
};

static const u8 sText_PlayerUsedItem[] = _("¡Ahí va la\n{B_LAST_ITEM}!");
static const u8 sText_WallyUsedItem[] = _("WALLY used\n{B_LAST_ITEM}!");
static const u8 sText_Trainer1UsedItem[] = _("{B_ATK_TRAINER_CLASS} {B_ATK_TRAINER_NAME}\nused {B_LAST_ITEM}!");
static const u8 sText_TrainerBlockedBall[] = _("The TRAINER blocked the BALL!");
static const u8 sText_DontBeAThief[] = _("Don't be a thief!");
static const u8 sText_ItDodgedBall[] = _("It dodged the thrown BALL!\nThis POKéMON can't be caught!");
static const u8 sText_YouMissedPkmn[] = _("You missed the POKéMON!");
static const u8 sText_PkmnBrokeFree[] = _("Oh, no!\nThe POKéMON broke free!");
static const u8 sText_ItAppearedCaught[] = _("Aww!\nIt appeared to be caught!");
static const u8 sText_AarghAlmostHadIt[] = _("Aargh!\nAlmost had it!");
static const u8 sText_ShootSoClose[] = _("Shoot!\nIt was so close, too!");
static const u8 sText_GotchaPkmnCaughtPlayer[] = _("¡Lo tienes!\n¡Capturaste a {B_DEF_NAME}!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}\p");
static const u8 sText_GotchaPkmnCaughtWally[] = _("Gotcha!\n{B_DEF_NAME} was caught!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}{PAUSE 127}");
static const u8 sText_GiveNicknameCaptured[] = _("¿Quieres ponerle un\nmote a {B_DEF_NAME}?");
static const u8 sText_PkmnSentToPC[] = _("{B_DEF_NAME} was sent to\n{B_PC_CREATOR_NAME} PC.");
static const u8 sText_Someones[] = _("someone's");
static const u8 sText_Lanettes[] = _("LANETTE's");
static const u8 sText_PkmnDataAddedToDex[] = _("Los datos de {B_DEF_NAME}\nse añadieron a la Pokédex.\p");
static const u8 sText_ItIsRaining[] = _("Está lloviendo.");
static const u8 sText_ItIsSnowing[] = _("Está nevando.");
static const u8 sText_HaceMuchoCalor[] = _("Hace mucho calor.");
static const u8 sText_SandstormIsRaging[] = _("La tormenta de arena arrecia.");
static const u8 sText_BoxIsFull[] = _("La caja está llena.\n¡No puedes capturar más!\p");
static const u8 sText_BerrySuffix[] = _(" BERRY");
static const u8 sText_PkmnsItemCuredParalysis[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\ncured paralysis!");
static const u8 sText_PkmnsItemCuredPoison[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\ncured poison!");
static const u8 sText_PkmnsItemHealedBurn[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nhealed its burn!");
static const u8 sText_PkmnsItemHealedFrostbite[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nhealed its frostbite!");
static const u8 sText_PkmnsItemDefrostedIt[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\ndefrosted it!");
static const u8 sText_PkmnsItemWokeIt[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nwoke it from its sleep!");
static const u8 sText_PkmnsItemSnappedOut[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nsnapped it out of confusion!");
static const u8 sText_PkmnsItemCuredProblem[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\ncured its {B_BUFF1} problem!");
static const u8 sText_PkmnsItemNormalizedStatus[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nnormalized its status!");
static const u8 sText_PkmnsItemRestoredHealth[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nrestored health!");
static const u8 sText_PkmnsItemRestoredPP[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nrestored {B_BUFF1}'s PP!");
static const u8 sText_PkmnsItemRestoredStatus[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nrestored its status!");
static const u8 sText_PkmnsItemRestoredHPALittle[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ITEM}\nrestored its HP a little!");
static const u8 sText_ItemAllowsOnlyYMove[] = _("{B_LAST_ITEM} allows the\nuse of only {B_CURRENT_MOVE}!\p");
static const u8 sText_PkmnHungOnWithX[] = _("{B_DEF_NAME_WITH_PREFIX} hung on\nusing its {B_LAST_ITEM}!");
static const u8 sText_YouThrowABallNowRight[] = _("You throw a BALL now, right?\nI… I'll do my best!");

// early declaration of strings
static const u8 sText_PkmnIncapableOfPower[];
static const u8 sText_GlintAppearsInEye[];
static const u8 sText_PkmnGettingIntoPosition[];
static const u8 sText_PkmnBeganGrowlingDeeply[];
static const u8 sText_PkmnEagerForMore[];
static const u8 sText_DefeatedOpponentByReferee[];
static const u8 sText_LostToOpponentByReferee[];
static const u8 sText_TiedOpponentByReferee[];
static const u8 sText_QuestionForfeitMatch[];
static const u8 sText_ForfeitedMatch[];
static const u8 sText_Trainer1WinText[];
static const u8 sText_Trainer2WinText[];
static const u8 sText_TwoInGameTrainersDefeated[];
static const u8 sText_Trainer2LoseText[];

// New battle strings.
static const u8 sText_EnduredViaSturdy[] = _("{B_DEF_NAME_WITH_PREFIX} endured the hit using {B_DEF_ABILITY}!");
static const u8 sText_PowerHerbActivation[] = _("{B_ATK_NAME_WITH_PREFIX} became fully charged due to its {B_LAST_ITEM}!");
static const u8 sText_HurtByItem[] = _("{B_ATK_NAME_WITH_PREFIX} was hurt by the {B_LAST_ITEM}!");
static const u8 sText_BadlyPoisonedByItem[] = _("{B_EFF_NAME_WITH_PREFIX} was badly poisoned by the {B_LAST_ITEM}!");
static const u8 sText_BurnedByItem[] = _("{B_EFF_NAME_WITH_PREFIX} was burned by the {B_LAST_ITEM}!");
static const u8 sText_TargetAbilityActivates[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY} activates!");
static const u8 sText_GravityIntensified[] = _("Gravity intensified!");
static const u8 sText_TargetIdentified[] = _("{B_DEF_NAME_WITH_PREFIX} was identified!");
static const u8 sText_TargetWokeUp[] = _("{B_DEF_NAME_WITH_PREFIX} woke up!");
static const u8 sText_PkmnStoleAndAteItem[] = _("{B_ATK_NAME_WITH_PREFIX} stole and\nate {B_DEF_NAME_WITH_PREFIX}'s {B_LAST_ITEM}!");
static const u8 sText_TailWindBlew[] = _("The tailwind blew from\nbehind {B_ATK_TEAM2} team!");
static const u8 sText_PkmnWentBack[] = _("{B_ATK_NAME_WITH_PREFIX} went back\nto {B_ATK_TRAINER_NAME}!");
static const u8 sText_PkmnCantUseItemsAnymore[] = _("{B_DEF_NAME_WITH_PREFIX} can't use\nitems anymore!");
static const u8 sText_PkmnFlung[] = _("{B_ATK_NAME_WITH_PREFIX} flung its\n{B_LAST_ITEM}!");
static const u8 sText_PkmnPreventedFromHealing[] = _("{B_DEF_NAME_WITH_PREFIX} was prevented\nfrom healing!");
static const u8 sText_PkmnSwitchedAtkAndDef[] = _("{B_ATK_NAME_WITH_PREFIX} switched its\nAttack and Defense!");
static const u8 sText_PkmnsAbilitySuppressed[] = _("{B_DEF_NAME_WITH_PREFIX}'s ability\nwas suppressed!");
static const u8 sText_ShieldedFromCriticalHits[] = _("The {B_CURRENT_MOVE} shielded {B_ATK_TEAM2}\nteam from critical hits!");
static const u8 sText_SwitchedAtkAndSpAtk[] = _("{B_ATK_NAME_WITH_PREFIX} switched all its\nchanges to its Attack and\pSp. Atk with the target!");
static const u8 sText_SwitchedDefAndSpDef[] = _("{B_ATK_NAME_WITH_PREFIX} switched all its\nchanges to its Defense and\pSp. Def with the target!");
static const u8 sText_PkmnAcquiredAbility[] = _("{B_DEF_NAME_WITH_PREFIX} acquired\n{B_DEF_ABILITY}!");
static const u8 sText_PoisonSpikesScattered[] = _("¡Han caído púas tóxicas\nen el campo de {B_DEF_TEAM2}!");
static const u8 sText_PkmnSwitchedStatChanges[] = _("{B_ATK_NAME_WITH_PREFIX} switched stat changes\nwith the target!");
static const u8 sText_PkmnSurroundedWithVeilOfWater[] = _("{B_ATK_NAME_WITH_PREFIX} surrounded itself\nwith a veil of water!");
static const u8 sText_PkmnLevitatedOnElectromagnetism[] = _("{B_ATK_NAME_WITH_PREFIX} levitated on\nelectromagnetism!");
static const u8 sText_PkmnTwistedDimensions[] = _("{B_ATK_NAME_WITH_PREFIX} twisted\nthe dimensions!");
static const u8 sText_DimensionsWereTwisted[] = _("The dimensions were\ntwisted!");
static const u8 sText_PointedStonesFloat[] =_("Pointed stones float in the air\naround {B_DEF_TEAM2} team!");
static const u8 sText_CloakedInMysticalMoonlight[] =_("It became cloaked in mystical\nmoonlight!");
static const u8 sText_TrappedBySwirlingMagma[] =_("{B_DEF_NAME_WITH_PREFIX} became\ntrapped by swirling magma!");
static const u8 sText_VanishedInstantly[] =_("{B_ATK_NAME_WITH_PREFIX} vanished\ninstantly!");
static const u8 sText_ProtectedTeam[] =_("¡{B_CURRENT_MOVE} protegió\n{B_ATK_TEAM2} equipo!");
static const u8 sText_SharedItsGuard[] =_("{B_ATK_NAME_WITH_PREFIX} shared its\nguard with the target!");
static const u8 sText_SharedItsPower[] =_("{B_ATK_NAME_WITH_PREFIX} shared its\npower with the target!");
static const u8 sText_SwapsDefAndSpDefOfAllPkmn[] =_("It created a bizarre area in which the\nDefense and Sp. Def stats are swapped!");
static const u8 sText_BizzareAreaCreated[] =_("A bizarre area was created in which the\nDefense and Sp. Def stats are swapped!");
static const u8 sText_BecameNimble[] =_("{B_ATK_NAME_WITH_PREFIX} became nimble!");
static const u8 sText_HurledIntoTheAir[] =_("{B_DEF_NAME_WITH_PREFIX} was hurled into the air!");
static const u8 sText_HeldItemsLoseEffects[] =_("It created a bizarre area in which Pokémon's held items lose their effects!");
static const u8 sText_BizarreArenaCreated[] =_("A bizarre area was created in which Pokémon's held items lose their effects!");
static const u8 sText_FellStraightDown[] =_("{B_DEF_NAME_WITH_PREFIX} fell straight down!");
static const u8 sText_TargetChangedType[] =_("{B_DEF_NAME_WITH_PREFIX} transformed into the {B_BUFF1} type!");
static const u8 sText_PkmnAcquiredSimple[] =_("{B_DEF_NAME_WITH_PREFIX} acquired Simple!"); //shouldn't directly use the name
static const u8 sText_KindOffer[] =_("{B_DEF_NAME_WITH_PREFIX} took the kind offer!");
static const u8 sText_ResetsTargetsStatLevels[] =_("{B_DEF_NAME_WITH_PREFIX}'s stat changes were removed!");
static const u8 sText_AllySwitchPosition[] =_("{B_ATK_NAME_WITH_PREFIX} and {B_SCR_ACTIVE_NAME_WITH_PREFIX} switched places!");
static const u8 sText_RestoreTargetsHealth[] =_("{B_DEF_NAME_WITH_PREFIX}'s HP was restored!");
static const u8 sText_TookPkmnIntoTheSky[] =_("{B_ATK_NAME_WITH_PREFIX} took {B_DEF_NAME_WITH_PREFIX} into the sky!");
static const u8 sText_FreedFromSkyDrop[] =_("{B_DEF_NAME_WITH_PREFIX} was freed from the Sky Drop!");
static const u8 sText_PostponeTargetMove[] =_("{B_DEF_NAME_WITH_PREFIX}'s move was postponed!");
static const u8 sText_ReflectTargetsType[] =_("{B_ATK_NAME_WITH_PREFIX} became the same type as {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_TransferHeldItem[] =_("{B_DEF_NAME_WITH_PREFIX} received {B_LAST_ITEM} from {B_ATK_NAME_WITH_PREFIX}");
static const u8 sText_EmbargoEnds[] = _("{B_ATK_NAME_WITH_PREFIX} can use items again!");
static const u8 sText_Electromagnetism[] = _("electromagnetism");
static const u8 sText_BufferEnds[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1} wore off!");
static const u8 sText_ThroatChopEnds[] = _("{B_ATK_NAME_WITH_PREFIX} can use sound-based moves again!");
static const u8 sText_TelekinesisEnds[] = _("{B_ATK_NAME_WITH_PREFIX} was freed from the telekinesis!");
static const u8 sText_TailwindEnds[] = _("{B_ATK_TEAM1} team's Tailwind petered out!");
static const u8 sText_LuckyChantEnds[] = _("{B_ATK_TEAM1} team's Lucky Chant wore off!");
static const u8 sText_TrickRoomEnds[] = _("The twisted dimensions returned to normal!");
static const u8 sText_WonderRoomEnds[] = _("Wonder Room wore off, and Defense and Sp. Def stats returned to normal!");
static const u8 sText_MagicRoomEnds[] = _("Magic Room wore off, and held items' effects returned to normal!");
static const u8 sText_MudSportEnds[] = _("The effects of Mud Sport have faded.");
static const u8 sText_WaterSportEnds[] = _("The effects of Water Sport have faded.");
static const u8 sText_GravityEnds[] = _("Gravity returned to normal!");
static const u8 sText_AquaRingHeal[] = _("Aqua Ring restored\n{B_ATK_NAME_WITH_PREFIX}'s HP!");
static const u8 sText_TargetAbilityRaisedStat[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nraised its {B_BUFF1}!");
static const u8 sText_TargetAbilityLoweredStat[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\nlowered its {B_BUFF1}!");
static const u8 sText_AttackerAbilityRaisedStat[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_ATK_ABILITY}\nraised its {B_BUFF1}!");
static const u8 sText_ScriptingAbilityRaisedStat[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY}\nraised its {B_BUFF1}!");
static const u8 sText_ElectricTerrainEnds[] = _("The electricity disappeared\nfrom the battlefield.");
static const u8 sText_MistyTerrainEnds[] = _("The mist disappeared\nfrom the battlefield.");
static const u8 sText_PsychicTerrainEnds[] = _("The weirdness disappeared\nfrom the battlefield.");
static const u8 sText_GrassyTerrainEnds[] = _("The grass disappeared\nfrom the battlefield.");
static const u8 sText_TargetsStatWasMaxedOut[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY} maxed\nits {B_BUFF1}!");
static const u8 sText_PoisonHealHpUp[] = _("The poisoning healed {B_ATK_NAME_WITH_PREFIX}\na little bit!");
static const u8 sText_Carnivoro[] = _("¡{B_ATK_NAME_WITH_PREFIX} ha usado a {B_DEF_NAME_WITH_PREFIX}\npara recuperarse!");
static const u8 sText_BadDreamsDmg[] = _("{B_DEF_NAME_WITH_PREFIX} is tormented!");
static const u8 sText_MoldBreakerEnters[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} breaks the mold!");
static const u8 sText_TeravoltEnters[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is radiating a bursting aura!");
static const u8 sText_TurboblazeEnters[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is radiating a blazing aura!");
static const u8 sText_SlowStartEnters[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is slow to get going!");
static const u8 sText_SlowStartEnd[] = _("{B_ATK_NAME_WITH_PREFIX} finally got its act together!");
static const u8 sText_SolarPowerHpDrop[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_ATK_ABILITY} takes its toll!"); //don't think this message is displayed anymore
static const u8 sText_AftermathDmg[] = _("{B_ATK_NAME_WITH_PREFIX} was hurt!");
static const u8 sText_AnticipationActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} shuddered!");
static const u8 sText_ForewarnActivates[] = _("{B_SCR_ACTIVE_ABILITY} alerted {B_SCR_ACTIVE_NAME_WITH_PREFIX} to {B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1}!");
static const u8 sText_IceBodyHpGain[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_ATK_ABILITY} healed it a little bit!"); //don't think this message is displayed anymore
static const u8 sText_SnowWarningHail[] = _("It started to hail!");
static const u8 sText_SnowWarningSnow[] = _("It started to snow!");
static const u8 sText_FriskActivates[] = _("{B_ATK_NAME_WITH_PREFIX} frisked {B_DEF_NAME_WITH_PREFIX} and\nfound its {B_LAST_ITEM}!");
static const u8 sText_UnnerveEnters[] = _("The opposing team is too nervous\nto eat Berries!");
static const u8 sText_HarvestBerry[] = _("{B_ATK_NAME_WITH_PREFIX} harvested\nits {B_LAST_ITEM}!");
static const u8 sText_LastAbilityRaisedBuff1[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_LAST_ABILITY}\nraised its {B_BUFF1}!");
static const u8 sText_MagicBounceActivates[] = _("The {B_DEF_NAME_WITH_PREFIX} bounced the\n{B_ATK_NAME_WITH_PREFIX} back!");
static const u8 sText_ProteanTypeChange[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_ATK_ABILITY} transformed\nit into the {B_BUFF1} type!");
static const u8 sText_SymbiosisItemPass[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} passed its {B_LAST_ITEM}\nto {B_ATK_NAME_WITH_PREFIX} through {B_LAST_ABILITY}!");
static const u8 sText_StealthRockDmg[] = _("Pointed stones dug into\n{B_SCR_ACTIVE_NAME_WITH_PREFIX}!");
static const u8 sText_ToxicSpikesAbsorbed[] = _("The poison spikes disappeared\nfrom around {B_DEF_TEAM2} team's feet!");
static const u8 sText_ToxicSpikesPoisoned[] = _("¡{B_SCR_ACTIVE_NAME_WITH_PREFIX} fue envenenado\npor las púas tóxicas!");
static const u8 sText_StickyWebSwitchIn[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} was\ncaught in a Sticky Web!");
static const u8 sText_HealingWishCameTrue[] = _("The healing wish came true\nfor {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_HealingWishHealed[] = _("{B_ATK_NAME_WITH_PREFIX} regained health!");
static const u8 sText_LunarDanceCameTrue[] = _("{B_ATK_NAME_WITH_PREFIX} became cloaked in mystical moonlight!");
static const u8 sText_CursedBodyDisabled[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1} was disabled by {B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}!");
static const u8 sText_AttackerAcquiredAbility[] = _("{B_ATK_NAME_WITH_PREFIX} acquired {B_ATK_ABILITY}!");
static const u8 sText_TargetStatWontGoHigher[] = _("{B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1} won't go any higher!");
static const u8 sText_PkmnMoveBouncedViaAbility[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_CURRENT_MOVE} was bounced back by {B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}!");
static const u8 sText_ImposterTransform[] = _("{B_ATK_NAME_WITH_PREFIX} transformed into {B_DEF_NAME_WITH_PREFIX} using {B_LAST_ABILITY}!");
static const u8 sText_NotDoneYet[] = _("This move effect is not done yet!\p");
static const u8 sText_PkmnBlewAwayToxicSpikes[] = _("{B_ATK_NAME_WITH_PREFIX} blew away Toxic Spikes!");
static const u8 sText_PkmnBlewAwayStickyWeb[] = _("{B_ATK_NAME_WITH_PREFIX} blew away Sticky Web!");
static const u8 sText_PkmnBlewAwayStealthRock[] = _("{B_ATK_NAME_WITH_PREFIX} blew away Stealth Rock!");
static const u8 sText_SpikesDisappearedFromTeam[] = _("The spikes disappeared from the ground around {B_ATK_TEAM2} team!");
static const u8 sText_ToxicSpikesDisappearedFromTeam[] = _("The poison spikes disappeared from the ground around {B_ATK_TEAM2} team!");
static const u8 sText_StealthRockDisappearedFromTeam[] = _("The pointed stones disappeared from around {B_ATK_TEAM2} team!");
static const u8 sText_StickyWebDisappearedFromTeam[] = _("The sticky web has disappeared from the ground around {B_ATK_TEAM2} team!");
static const u8 sText_StickyWebUsed[] = _("A sticky web has been laid out on the ground around {B_DEF_TEAM2} team!");
static const u8 sText_QuashSuccess[] = _("{B_DEF_NAME_WITH_PREFIX}'s move was postponed!");
static const u8 sText_IonDelugeOn[] = _("A deluge of ions showers the battlefield!");
static const u8 sText_TopsyTurvySwitchedStats[] = _("All stat changes on {B_DEF_NAME_WITH_PREFIX} were inverted!");
static const u8 sText_TerrainBecomesMisty[] = _("Mist swirled around the battlefield!");
static const u8 sText_TerrainBecomesGrassy[] = _("Grass grew to cover the battlefield!");
static const u8 sText_TerrainBecomesElectric[] = _("An electric current ran across the battlefield!");
static const u8 sText_TerrainBecomesPsychic[] = _("The battlefield got weird!");
static const u8 sText_TargetElectrified[] = _("{B_DEF_NAME_WITH_PREFIX}'s moves\nhave been electrified!");
static const u8 sText_AssaultVestDoesntAllow[] = _("{B_LAST_ITEM} impide el uso\nde ataques de estado.\p");
static const u8 sText_GravityPreventsUsage[] = _("{B_ATK_NAME_WITH_PREFIX} can't use {B_CURRENT_MOVE}\nbecause of gravity!\p");
static const u8 sText_HealBlockPreventsUsage[] = _("{B_ATK_NAME_WITH_PREFIX} was\nprevented from healing!\p");
static const u8 sText_MegaEvoReacting[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_LAST_ITEM} is\nreacting to {B_ATK_TRAINER_NAME}'s Mega Ring!");
static const u8 sText_FerventWishReached[] = _("{B_ATK_TRAINER_NAME}'s fervent wish\nhas reached {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_MegaEvoEvolved[] = _("{B_ATK_NAME_WITH_PREFIX} has Mega Evolved into\nMega {B_BUFF1}!");
const u8 sText_drastically[] = _("drastically ");
const u8 sText_severely[] = _("severely ");
static const u8 sText_Infestation[] = _("{B_DEF_NAME_WITH_PREFIX} has been afflicted\nwith an infestation by {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_NoEffectOnTarget[] = _("It had no effect\non {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_BurstingFlames[] = _("The bursting flames\nhit {B_SCR_ACTIVE_NAME_WITH_PREFIX}!");
static const u8 sText_BestowItemGiving[] = _("{B_DEF_NAME_WITH_PREFIX} received {B_LAST_ITEM}\nfrom {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_ThirdTypeAdded[] = _("{B_BUFF1} type was added to\n{B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_FellForFeint[] = _("{B_DEF_NAME_WITH_PREFIX} fell for\nthe feint!");
static const u8 sText_PokemonCannotUseMove[] = _("{B_ATK_NAME_WITH_PREFIX} no puede\nusar {B_CURRENT_MOVE}!");
static const u8 sText_CoveredInPowder[] = _("{B_DEF_NAME_WITH_PREFIX} is covered in powder!");
static const u8 sText_PowderExplodes[] = _("When the flame touched the powder\non the Pokémon, it exploded!");
static const u8 sText_BelchCantUse[] = _("Belch cannot be used!\p");
static const u8 sText_SpectralThiefSteal[] = _("{B_ATK_NAME_WITH_PREFIX} stole the target's\nboosted stats!");
static const u8 sText_GravityGrounding[] = _("{B_DEF_NAME_WITH_PREFIX} can't stay airborne\nbecause of gravity!");
static const u8 sText_MistyTerrainPreventsStatus[] = _("{B_DEF_NAME_WITH_PREFIX} surrounds itself\nwith a protective mist!");
static const u8 sText_GrassyTerrainHeals[] = _("{B_ATK_NAME_WITH_PREFIX} is healed\nby the grassy terrain!");
static const u8 sText_ElectricTerrainPreventsSleep[] = _("{B_DEF_NAME_WITH_PREFIX} surrounds itself\nwith electrified terrain!");
static const u8 sText_PsychicTerrainPreventsPriority[] = _("{B_DEF_NAME_WITH_PREFIX} surrounds itself\nwith psychic terrain!");
static const u8 sText_SafetyGogglesProtected[] = _("{B_DEF_NAME_WITH_PREFIX} is not affected\nthanks to its {B_LAST_ITEM}!");
static const u8 sText_FlowerVeilProtected[] = _("¡{B_DEF_NAME_WITH_PREFIX} protegió al equipo\ncon un velo de pétalos!");
static const u8 sText_SweetVeilProtected[] = _("{B_DEF_NAME_WITH_PREFIX} surrounded itself\nwith a veil of sweetness!");
static const u8 sText_AromaVeilProtected[] = _("{B_DEF_NAME_WITH_PREFIX} is protected\nby an aromatic veil!");
static const u8 sText_CelebrateMessage[] = _("Congratulations, {B_PLAYER_NAME}!");
static const u8 sText_UsedInstructedMove[] = _("{B_ATK_NAME_WITH_PREFIX} followed  {B_BUFF1}'s instructions!");
static const u8 sText_LaserFocusMessage[] = _("{B_ATK_NAME_WITH_PREFIX} concentrated intensely!");
static const u8 sText_GemActivates[] = _("The {B_LAST_ITEM} strengthened {B_ATK_NAME_WITH_PREFIX}'s power!");
static const u8 sText_BerryDmgReducing[] = _("The {B_LAST_ITEM} weakened the damage to {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_TargetAteItem[] = _("{B_DEF_NAME_WITH_PREFIX} ate its {B_LAST_ITEM}!");
static const u8 sText_AirBalloonFloat[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} floats in the air with its Air Balloon!");
static const u8 sText_AirBalloonPop[] = _("{B_DEF_NAME_WITH_PREFIX}'s Air Balloon popped!");
static const u8 sText_IncinerateBurn[] = _("{B_EFF_NAME_WITH_PREFIX}'s {B_LAST_ITEM} was burnt up!");
static const u8 sText_BugBite[] = _("{B_ATK_NAME_WITH_PREFIX} stole and ate its target's {B_LAST_ITEM}!");
static const u8 sText_IllusionWoreOff[] = _("{B_DEF_NAME_WITH_PREFIX}'s illusion wore off!");
static const u8 sText_AttackerCuredTargetStatus[] = _("{B_ATK_NAME_WITH_PREFIX} cured {B_DEF_NAME_WITH_PREFIX}'s problem!");
static const u8 sText_AttackerLostFireType[] = _("{B_ATK_NAME_WITH_PREFIX} burned itself out!");
static const u8 sText_HealerCure[] = _("{B_ATK_NAME_WITH_PREFIX}'s {B_LAST_ABILITY} cured {B_SCR_ACTIVE_NAME_WITH_PREFIX}'s problem!");
static const u8 sText_ReceiverAbilityTakeOver[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY} was taken over!");
static const u8 sText_PkmnAbsorbingPower[] = _("{B_ATK_NAME_WITH_PREFIX} is absorbing power!");
static const u8 sText_NoOneWillBeAbleToRun[] = _("No one will be able to run away during the next turn!");
static const u8 sText_DestinyKnotActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} fell in love because of the {B_LAST_ITEM}!");
static const u8 sText_CloakedInAFreezingLight[] = _("{B_ATK_NAME_WITH_PREFIX} became cloaked in a freezing light!");
static const u8 sText_ClearAmuletWontLowerStats[] = _("The effects of the {B_LAST_ITEM} held by {B_DEF_NAME_WITH_PREFIX} prevents its stats from being lowered!");
static const u8 sText_AuraFlaredToLife[] = _("{B_DEF_NAME_WITH_PREFIX}'s aura flared to life!");
static const u8 sText_AirLockActivates[] = _("The effects of weather\ndisappeared.");
static const u8 sText_PressureActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is exerting its\npressure!");
static const u8 sText_DarkAuraActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is radiating\na dark aura!");
static const u8 sText_Generador[] = _("¡{B_SCR_ACTIVE_NAME_WITH_PREFIX} genera energía y\npotencia ataques eléctricos de su equipo!");
static const u8 sText_EstaSomnoliento[] = _("¡{B_ATK_NAME_WITH_PREFIX} está echando una cabezada!");
static const u8 sText_FairyAuraActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is radiating\na fairy aura!");
static const u8 sText_AuraBreakActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} reversed all\nother Pokémon's auras!");
static const u8 sText_ComatoseActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is drowsing!");
static const u8 sText_ScreenCleanerActivates[] = _("All screens on the field were cleansed!");
static const u8 sText_FetchedPokeBall[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} found a {B_LAST_ITEM}!");
static const u8 sText_BattlerAbilityRaisedStat[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY} raised its {B_BUFF1}!");
static const u8 sText_ASandstormKickedUp[] = _("A sandstorm kicked up!");
static const u8 sText_PkmnsWillPerishIn3Turns[] = _("Both Pokémon will perish in three turns!");  //don't think this message is displayed anymore
static const u8 sText_AbilityRaisedStatDrastically[] = _("{B_DEF_ABILITY} raised {B_DEF_NAME_WITH_PREFIX}'s {B_BUFF1} drastically!");
static const u8 sText_AsOneEnters[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} has two Abilities!");
static const u8 sText_PkmnMadeShellGleam[] = _("{B_DEF_NAME_WITH_PREFIX} made its shell gleam! It's distorting type matchups!");
static const u8 sText_CuriousMedicineEnters[] = _("{B_EFF_NAME_WITH_PREFIX}'s stat changes were removed!");
static const u8 sText_CanActFaster[] = _("{B_ATK_NAME_WITH_PREFIX} can act faster than normal, thanks to its {B_BUFF1}!");
static const u8 sText_MicleBerryActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} boosted the accuracy of its next move using {B_LAST_ITEM}!");
static const u8 sText_PkmnShookOffTheTaunt[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} shook off the taunt!");
static const u8 sText_PkmnGotOverItsInfatuation[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} got over its infatuation!");
static const u8 sText_TerrainReturnedToNormal[] = _("The terrain returned to normal!");
static const u8 sText_ItemCannotBeRemoved[] = _("{B_ATK_NAME_WITH_PREFIX}'s item cannot be removed!");
static const u8 sText_StickyBarbTransfer[] = _("The {B_LAST_ITEM} attached itself to {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnBurnHealed[] = _("{B_DEF_NAME_WITH_PREFIX}'s burn was cured!");
static const u8 sText_RedCardActivate[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} held up its Red Card against {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_EjectButtonActivate[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is switched out with the {B_LAST_ITEM}!");
static const u8 sText_AttackerGotOverInfatuation[] =_("{B_ATK_NAME_WITH_PREFIX} got over its infatuation!");
static const u8 sText_TormentedNoMore[] = _("{B_ATK_NAME_WITH_PREFIX} is no longer tormented!");
static const u8 sText_HealBlockedNoMore[] = _("{B_ATK_NAME_WITH_PREFIX} is cured of its heal block!");
static const u8 sText_AttackerBecameFullyCharged[] = _("{B_ATK_NAME_WITH_PREFIX} became fully charged due to its bond with its trainer!\p");
static const u8 sText_AttackerBecameAshSpecies[] = _("{B_ATK_NAME_WITH_PREFIX} became Ash-Greninja!\p");
static const u8 sText_ExtremelyHarshSunlight[] = _("The sunlight turned extremely harsh!");
static const u8 sText_ExtremeSunlightFaded[] = _("The extremely harsh sunlight faded!");
static const u8 sText_MoveEvaporatedInTheHarshSunlight[] = _("The Water-type attack evaporated in the extremely harsh sunlight!");
static const u8 sText_ExtremelyHarshSunlightWasNotLessened[] = _("The extremely harsh sunlight was not lessened at all!");
static const u8 sText_HeavyRain[] = _("A heavy rain began to fall!");
static const u8 sText_HeavyRainLifted[] = _("The heavy rain has lifted!");
static const u8 sText_MoveFizzledOutInTheHeavyRain[] = _("The Fire-type attack fizzled out in the heavy rain!");
static const u8 sText_NoReliefFromHeavyRain[] = _("There is no relief from this heavy rain!");
static const u8 sText_MysteriousAirCurrent[] = _("Mysterious strong winds are protecting Flying-type Pokémon!");
static const u8 sText_StrongWindsDissipated[] = _("The mysterious strong winds have dissipated!");
static const u8 sText_MysteriousAirCurrentBlowsOn[] = _("The mysterious strong winds blow on regardless!");
static const u8 sText_AttackWeakenedByStrongWinds[] = _("The mysterious strong winds weakened the attack!");
static const u8 sText_StuffCheeksCantSelect[] = _("It can't use the move because it doesn't have a Berry!\p");
static const u8 sText_PkmnRevertedToPrimal[] = _("{B_ATK_NAME_WITH_PREFIX}’s Primal Reversion! It reverted to its primal state!");
static const u8 sText_ButPokemonCantUseTheMove[] = _("But {B_ATK_NAME_WITH_PREFIX} can't use the move!");
static const u8 sText_ButHoopaCantUseIt[] = _("But {B_ATK_NAME_WITH_PREFIX} can't use it the way it is now!");
static const u8 sText_BrokeThroughProtection[] = _("It broke through {B_DEF_NAME_WITH_PREFIX}'s protection!");
static const u8 sText_AbilityAllowsOnlyMove[] = _("{B_ATK_ABILITY} only allows the use of {B_CURRENT_MOVE}!\p");
static const u8 sText_SwappedAbilities[] = _("{B_DEF_NAME_WITH_PREFIX} swapped Abilities with its target!");
static const u8 sText_PastelVeilProtected[] = _("{B_DEF_NAME_WITH_PREFIX} is protected by a pastel veil!");
static const u8 sText_PastelVeilEnters[] = _("{B_DEF_NAME_WITH_PREFIX} was cured of its poisoning!");
static const u8 sText_BattlerTypeChangedTo[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s type changed to {B_BUFF1}!");
static const u8 sText_BothCanNoLongerEscape[] = _("Neither Pokémon can run away!");
static const u8 sText_CantEscapeDueToUsedMove[] = _("{B_ATK_NAME_WITH_PREFIX} can no longer escape because it used No Retreat!");
static const u8 sText_PkmnBecameWeakerToFire[] = _("{B_DEF_NAME_WITH_PREFIX} became weaker to fire!");
static const u8 sText_PkmnAboutToBeAttackedByItsItem[] = _("{B_DEF_NAME_WITH_PREFIX} is about to be attacked by its {B_BUFF1}!");
static const u8 sText_CantEscapeBecauseOfCurrentMove[] = _("{B_DEF_NAME_WITH_PREFIX} can no longer escape because of Octolock!");
static const u8 sText_NeutralizingGasEnters[] = _("Neutralizing gas filled the area!");
static const u8 sText_NeutralizingGasOver[] = _("The effects of the neutralizing gas wore off!");
static const u8 sText_PkmnTookTargetHigh[] = _("{B_ATK_NAME_WITH_PREFIX} took {B_DEF_NAME_WITH_PREFIX} into the sky!");
static const u8 sText_TargetTooHeavy[] = _("{B_DEF_NAME_WITH_PREFIX} is too heavy to be lifted!");
static const u8 sText_MeteorBeamCharging[] = _("{B_ATK_NAME_WITH_PREFIX} is overflowing with space power!");
static const u8 sText_HeatingUpBeak[] = _("{B_ATK_NAME_WITH_PREFIX} started heating up its beak!");
static const u8 sText_CourtChange[] = _("{B_ATK_NAME_WITH_PREFIX} swapped the battle effects affecting each side of the field!");
static const u8 sText_AttackerExpelledThePoison[] = _("{B_ATK_NAME_WITH_PREFIX} managed to expel the poison so you wouldn't worry!");
static const u8 sText_AttackerShookItselfAwake[] = _("{B_ATK_NAME_WITH_PREFIX} shook itself awake so you wouldn't worry!");
static const u8 sText_AttackerBrokeThroughParalysis[] = _("{B_ATK_NAME_WITH_PREFIX} gathered all its energy to break through its paralysis so you wouldn't worry!");
static const u8 sText_AttackerHealedItsBurn[] = _("{B_ATK_NAME_WITH_PREFIX} cured its burn through sheer determination so you wouldn't worry!");
static const u8 sText_AttackerHealedItsFrostbite[] = _("{B_ATK_NAME_WITH_PREFIX} cured its frostbite through sheer determination so you wouldn't worry!");
static const u8 sText_AttackerMeltedTheIce[] = _("{B_ATK_NAME_WITH_PREFIX} melted the ice with its fiery determination so you wouldn't worry!");
static const u8 sText_TargetToughedItOut[] = _("{B_DEF_NAME_WITH_PREFIX} toughed it out so you wouldn't feel sad!");
static const u8 sText_AttackerLostElectricType[] = _("{B_ATK_NAME_WITH_PREFIX} used up all its electricity!");
static const u8 sText_AttackerSwitchedStatWithTarget[] = _("{B_ATK_NAME_WITH_PREFIX} switched {B_BUFF1} with its target!");
static const u8 sText_BeingHitChargedPkmnWithPower[] = _("Being hit by {B_CURRENT_MOVE} charged {B_DEF_NAME_WITH_PREFIX} with power!");
static const u8 sText_SunlightActivatedAbility[] = _("The harsh sunlight activated {B_SCR_ACTIVE_NAME_WITH_PREFIX}'s Protosynthesis!");
static const u8 sText_StatWasHeightened[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_BUFF1} was heightened!");
static const u8 sText_BoosterEnergyActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} used its {B_LAST_ITEM} to activate {B_SCR_ACTIVE_ABILITY}!");
static const u8 sText_ElectricTerrainActivatedAbility[] = _("The Electric Terrain activated {B_SCR_ACTIVE_NAME_WITH_PREFIX}'s Quark Drive!");
static const u8 sText_AbilityWeakenedSurroundingMonsStat[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s {B_SCR_ACTIVE_ABILITY} weakened the {B_BUFF1} of all surrounding Pokémon!\p");
static const u8 sText_AttackerGainedStrengthFromTheFallen[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} gained strength from the fallen!");
static const u8 sText_PrepareShellTrap[] = _("{B_ATK_NAME_WITH_PREFIX} set a shell trap!");
static const u8 sText_ShellTrapDidntWork[] = _("{B_ATK_NAME_WITH_PREFIX}'s shell trap didn't work!");
static const u8 sText_SharpSteelFloats[] = _("Sharp-pointed pieces of steel started floating around {B_DEF_TEAM2} Pokémon!");
static const u8 sText_SharpSteelDmg[] = _("The sharp steel bit into {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_PkmnBlewAwaySharpSteel[] = _("{B_ATK_NAME_WITH_PREFIX} blew away sharp steel!");
static const u8 sText_SharpSteelDisappearedFromTeam[] = _("The pieces of steel surrounding {B_ATK_TEAM2} Pokémon disappeared!");
static const u8 sText_TeamTrappedWithVines[] = _("{B_DEF_TEAM1} Pokémon got trapped with vines!");
static const u8 sText_PkmnHurtByVines[] = _("{B_ATK_NAME_WITH_PREFIX} is hurt by G-Max Vine Lash's ferocious beating!");
static const u8 sText_TeamCaughtInVortex[] = _("{B_DEF_TEAM1} Pokémon got caught in a vortex of water!");
static const u8 sText_PkmnHurtByVortex[] = _("{B_ATK_NAME_WITH_PREFIX} is hurt by G-Max Cannonade's vortex!");
static const u8 sText_TeamSurroundedByFire[] = _("{B_DEF_TEAM1} Pokémon were surrounded by fire!");
static const u8 sText_PkmnBurningUp[] = _("{B_ATK_NAME_WITH_PREFIX} is burning up within G-Max Wildfire's flames!");
static const u8 sText_TeamSurroundedByRocks[] = _("{B_DEF_TEAM1} Pokémon became surrounded by rocks!");
static const u8 sText_PkmnHurtByRocksThrown[] = _("{B_ATK_NAME_WITH_PREFIX} is hurt by rocks thrown out by G-Max Volcalith!");
static const u8 sText_CouldntFullyProtect[] = _("{B_DEF_NAME_WITH_PREFIX} couldn't fully protect itself and got hurt!");
static const u8 sText_StockpiledEffectWoreOff[] = _("{B_ATK_NAME_WITH_PREFIX}'s stockpiled effect wore off!");
static const u8 sText_MoveBlockedByDynamax[] = _("The move was blocked by the power of Dynamax!");
static const u8 sText_PkmnRevivedReadyToFight[] = _("{B_BUFF1} was revived and is ready to fight again!");
static const u8 sText_ItemRestoredSpeciesHealth[] = _("{B_BUFF1} had its HP restored.");
static const u8 sText_ItemCuredSpeciesStatus[] = _("{B_BUFF1} had its status healed!");
static const u8 sText_ItemRestoredSpeciesPP[] = _("{B_BUFF1} had its PP restored!");
static const u8 sText_AtkTrappedDef[] = _("{B_ATK_NAME_WITH_PREFIX} trapped {B_DEF_NAME_WITH_PREFIX}!");
static const u8 sText_MirrorHerbCopied[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} used its Mirror Herb to mirror its opponent's stat changes!");
static const u8 sText_PkmnItemMelted[] = _("{B_ATK_NAME_WITH_PREFIX} corroded {B_DEF_NAME_WITH_PREFIX}'s {B_LAST_ITEM}!");
static const u8 sText_UltraBurstReacting[] = _("Bright light is about to burst out of {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_UltraBurstCompleted[] = _("{B_ATK_NAME_WITH_PREFIX} regained its true power through Ultra Burst!");
static const u8 sText_TeamGainedEXP[] = _("The rest of your team gained Exp. Points thanks to the Exp. Share!\p");
static const u8 sText_CurrentMoveCantSelect[] = _("{B_BUFF1} cannot be used!\p");
static const u8 sText_TargetIsBeingSaltCured[] = _("{B_DEF_NAME_WITH_PREFIX} is being salt cured!");
static const u8 sText_TargetIsHurtBySaltCure[] = _("{B_DEF_NAME_WITH_PREFIX} is hurt by {B_BUFF1}!");
static const u8 sText_TargetCoveredInStickyCandySyrup[] = _("{B_DEF_NAME_WITH_PREFIX} got covered in sticky candy syrup!");
static const u8 sText_PkmnTellChillingReceptionJoke[] = _("{B_ATK_NAME_WITH_PREFIX} is preparing to tell a chillingly bad joke!");
static const u8 sText_ZeroToHeroTransformation[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} underwent a heroic transformation!");
static const u8 sText_CommanderActivates[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} was swallowed by Dondozo and became Dondozo's commander!");
static const u8 sText_TheTwoMovesBecomeOne[] = _("The two moves have become one! It's a combined move!{PAUSE 16}");
static const u8 sText_ARainbowAppearedOnSide[] = _("A rainbow appeared in the sky on {B_ATK_TEAM2} team's side!");
static const u8 sText_TheRainbowDisappeared[] = _("The rainbow on {B_ATK_TEAM2} team's side disappeared!");
static const u8 sText_WaitingForPartnersMove[] = _("{B_ATK_NAME_WITH_PREFIX} is waiting for {B_ATK_PARTNER_NAME}'s move…{PAUSE 16}");
static const u8 sText_SeaOfFireEnvelopedSide[] = _("A sea of fire enveloped {B_DEF_TEAM2} team!");
static const u8 sText_HurtByTheSeaOfFire[] = _("{B_ATK_NAME_WITH_PREFIX} was hurt by the sea of fire!");
static const u8 sText_TheSeaOfFireDisappeared[] = _("The sea of fire around {B_ATK_TEAM2} team disappeared!");
static const u8 sText_SwampEnvelopedSide[] = _("A swamp enveloped {B_DEF_TEAM2} team!");
static const u8 sText_TheSwampDisappeared[] = _("The swamp around {B_ATK_TEAM2} team disappeared!");
static const u8 sText_HospitalityRestoration[] = _("{B_ATK_PARTNER_NAME} drank down all the matcha that {B_ATK_NAME_WITH_PREFIX} made!");
static const u8 sText_ElectroShotCharging[] = _("{B_ATK_NAME_WITH_PREFIX} absorbed electricity!");
static const u8 sText_ItemWasUsedUp[] = _("The {B_LAST_ITEM} was used up…");
static const u8 sText_AttackerLostItsType[] = _("{B_ATK_NAME_WITH_PREFIX} lost its {B_BUFF1} type!");
static const u8 sText_ShedItsTail[] = _("{B_ATK_NAME_WITH_PREFIX} shed its tail to create a decoy!");
static const u8 sText_PkmnTerastallizedInto[] = _("{B_ATK_NAME_WITH_PREFIX} terastallized into the {B_BUFF1} type!");
static const u8 sText_SupersweetAromaWafts[] = _("A supersweet aroma is wafting from the syrup covering {B_ATK_NAME_WITH_PREFIX}!");
static const u8 sText_TidyingUpComplete[] = _("Tidying up complete!");
static const u8 sText_FickleBeamDoubled[] = _("{B_ATK_NAME_WITH_PREFIX} is going all out for this attack!");
static const u8 sText_WagglingAFinger[] = _("Waggling a finger let it use {B_CURRENT_MOVE}!");

const u8 *const gBattleStringsTable[BATTLESTRINGS_COUNT] =
{
    [STRINGID_FICKLEBEAMDOUBLED] = sText_FickleBeamDoubled,
    [STRINGID_PKMNTERASTALLIZEDINTO] = sText_PkmnTerastallizedInto,
    [STRINGID_TIDYINGUPCOMPLETE] = sText_TidyingUpComplete,
    [STRINGID_SUPERSWEETAROMAWAFTS] = sText_SupersweetAromaWafts,
    [STRINGID_SHEDITSTAIL] = sText_ShedItsTail,
    [STRINGID_ELECTROSHOTCHARGING] = sText_ElectroShotCharging,
    [STRINGID_HOSPITALITYRESTORATION] = sText_HospitalityRestoration,
    [STRINGID_THESWAMPDISAPPEARED] = sText_TheSwampDisappeared,
    [STRINGID_SWAMPENVELOPEDSIDE] = sText_SwampEnvelopedSide,
    [STRINGID_THESEAOFFIREDISAPPEARED] = sText_TheSeaOfFireDisappeared,
    [STRINGID_HURTBYTHESEAOFFIRE] = sText_HurtByTheSeaOfFire,
    [STRINGID_SEAOFFIREENVELOPEDSIDE] = sText_SeaOfFireEnvelopedSide,
    [STRINGID_WAITINGFORPARTNERSMOVE] = sText_WaitingForPartnersMove,
    [STRINGID_THERAINBOWDISAPPEARED] = sText_TheRainbowDisappeared,
    [STRINGID_ARAINBOWAPPEAREDONSIDE] = sText_ARainbowAppearedOnSide,
    [STRINGID_THETWOMOVESBECOMEONE] = sText_TheTwoMovesBecomeOne,
    [STRINGID_ZEROTOHEROTRANSFORMATION] = sText_ZeroToHeroTransformation,
    [STRINGID_COMMANDERACTIVATES] = sText_CommanderActivates,
    [STRINGID_PKMNTELLCHILLINGRECEPTIONJOKE] = sText_PkmnTellChillingReceptionJoke,
    [STRINGID_MOVEBLOCKEDBYDYNAMAX] = sText_MoveBlockedByDynamax,
    [STRINGID_TARGETISHURTBYSALTCURE] = sText_TargetIsHurtBySaltCure,
    [STRINGID_TARGETISBEINGSALTCURED] = sText_TargetIsBeingSaltCured,
    [STRINGID_CURRENTMOVECANTSELECT] = sText_CurrentMoveCantSelect,
    [STRINGID_PKMNITEMMELTED] = sText_PkmnItemMelted,
    [STRINGID_MIRRORHERBCOPIED] = sText_MirrorHerbCopied,
    [STRINGID_THUNDERCAGETRAPPED] = sText_AtkTrappedDef,
    [STRINGID_ITEMRESTOREDSPECIESHEALTH] = sText_ItemRestoredSpeciesHealth,
    [STRINGID_ITEMCUREDSPECIESSTATUS] = sText_ItemCuredSpeciesStatus,
    [STRINGID_ITEMRESTOREDSPECIESPP] = sText_ItemRestoredSpeciesPP,
    [STRINGID_PKMNREVIVEDREADYTOFIGHT] = sText_PkmnRevivedReadyToFight,
    [STRINGID_STOCKPILEDEFFECTWOREOFF] = sText_StockpiledEffectWoreOff,
    [STRINGID_COULDNTFULLYPROTECT] = sText_CouldntFullyProtect,
    [STRINGID_PKMNHURTBYROCKSTHROWN] = sText_PkmnHurtByRocksThrown,
    [STRINGID_TEAMSURROUNDEDBYROCKS] = sText_TeamSurroundedByRocks,
    [STRINGID_PKMNBURNINGUP] = sText_PkmnBurningUp,
    [STRINGID_TEAMSURROUNDEDBYFIRE] = sText_TeamSurroundedByFire,
    [STRINGID_PKMNHURTBYVORTEX] = sText_PkmnHurtByVortex,
    [STRINGID_TEAMCAUGHTINVORTEX] = sText_TeamCaughtInVortex,
    [STRINGID_PKMNHURTBYVINES] = sText_PkmnHurtByVines,
    [STRINGID_TEAMTRAPPEDWITHVINES] = sText_TeamTrappedWithVines,
    [STRINGID_PKMNBLEWAWAYSHARPSTEEL] = sText_PkmnBlewAwaySharpSteel,
    [STRINGID_SHARPSTEELDMG] = sText_SharpSteelDmg,
    [STRINGID_SHARPSTEELFLOATS] = sText_SharpSteelFloats,
    [STRINGID_ATTACKERGAINEDSTRENGTHFROMTHEFALLEN] = sText_AttackerGainedStrengthFromTheFallen,
    [STRINGID_ABILITYWEAKENEDSURROUNDINGMONSSTAT] = sText_AbilityWeakenedSurroundingMonsStat,
    [STRINGID_ELECTRICTERRAINACTIVATEDABILITY] = sText_ElectricTerrainActivatedAbility,
    [STRINGID_STATWASHEIGHTENED] = sText_StatWasHeightened,
    [STRINGID_BOOSTERENERGYACTIVATES] = sText_BoosterEnergyActivates,
    [STRINGID_SUNLIGHTACTIVATEDABILITY] = sText_SunlightActivatedAbility,
    [STRINGID_BEINGHITCHARGEDPKMNWITHPOWER] = sText_BeingHitChargedPkmnWithPower,
    [STRINGID_ATTACKERSWITCHEDSTATWITHTARGET] = sText_AttackerSwitchedStatWithTarget,
    [STRINGID_TARGETTOUGHEDITOUT] = sText_TargetToughedItOut,
    [STRINGID_ATTACKERMELTEDTHEICE] = sText_AttackerMeltedTheIce,
    [STRINGID_ATTACKERHEALEDITSBURN] = sText_AttackerHealedItsBurn,
    [STRINGID_ATTACKERBROKETHROUGHPARALYSIS] = sText_AttackerBrokeThroughParalysis,
    [STRINGID_ATTACKERSHOOKITSELFAWAKE] = sText_AttackerShookItselfAwake,
    [STRINGID_ATTACKEREXPELLEDTHEPOISON] = sText_AttackerExpelledThePoison,
    [STRINGID_PLAYERLOSTTOENEMYTRAINER] = sText_PlayerLostToEnemyTrainer,
    [STRINGID_PLAYERPAIDPRIZEMONEY] = sText_PlayerPaidPrizeMoney,
    [STRINGID_SHELLTRAPDIDNTWORK] = sText_ShellTrapDidntWork,
    [STRINGID_PREPARESHELLTRAP] = sText_PrepareShellTrap,
    [STRINGID_COURTCHANGE] = sText_CourtChange,
    [STRINGID_HEATUPBEAK] = sText_HeatingUpBeak,
    [STRINGID_METEORBEAMCHARGING] = sText_MeteorBeamCharging,
    [STRINGID_PKMNINSNAPTRAP] = sText_PkmnInSnapTrap,
    [STRINGID_NEUTRALIZINGGASOVER] = sText_NeutralizingGasOver,
    [STRINGID_NEUTRALIZINGGASENTERS] = sText_NeutralizingGasEnters,
    [STRINGID_BATTLERTYPECHANGEDTO] = sText_BattlerTypeChangedTo,
    [STRINGID_PASTELVEILENTERS] = sText_PastelVeilEnters,
    [STRINGID_PASTELVEILPROTECTED] = sText_PastelVeilProtected,
    [STRINGID_SWAPPEDABILITIES] = sText_SwappedAbilities,
    [STRINGID_ABILITYALLOWSONLYMOVE] = sText_AbilityAllowsOnlyMove,
    [STRINGID_BROKETHROUGHPROTECTION] = sText_BrokeThroughProtection,
    [STRINGID_BUTPOKEMONCANTUSETHEMOVE] = sText_ButPokemonCantUseTheMove,
    [STRINGID_BUTHOOPACANTUSEIT] = sText_ButHoopaCantUseIt,
    [STRINGID_PKMNREVERTEDTOPRIMAL] = sText_PkmnRevertedToPrimal,
    [STRINGID_STUFFCHEEKSCANTSELECT] = sText_StuffCheeksCantSelect,
    [STRINGID_ATTACKWEAKENEDBSTRONGWINDS] = sText_AttackWeakenedByStrongWinds,
    [STRINGID_MYSTERIOUSAIRCURRENTBLOWSON] = sText_MysteriousAirCurrentBlowsOn,
    [STRINGID_STRONGWINDSDISSIPATED] = sText_StrongWindsDissipated,
    [STRINGID_MYSTERIOUSAIRCURRENT] = sText_MysteriousAirCurrent,
    [STRINGID_NORELIEFROMHEAVYRAIN] = sText_NoReliefFromHeavyRain,
    [STRINGID_MOVEFIZZLEDOUTINTHEHEAVYRAIN] = sText_MoveFizzledOutInTheHeavyRain,
    [STRINGID_HEAVYRAINLIFTED] = sText_HeavyRainLifted,
    [STRINGID_HEAVYRAIN] = sText_HeavyRain,
    [STRINGID_EXTREMELYHARSHSUNLIGHTWASNOTLESSENED] = sText_ExtremelyHarshSunlightWasNotLessened,
    [STRINGID_MOVEEVAPORATEDINTHEHARSHSUNLIGHT] = sText_MoveEvaporatedInTheHarshSunlight,
    [STRINGID_EXTREMESUNLIGHTFADED] = sText_ExtremeSunlightFaded,
    [STRINGID_EXTREMELYHARSHSUNLIGHT] = sText_ExtremelyHarshSunlight,
    [STRINGID_ATTACKERBECAMEASHSPECIES] = sText_AttackerBecameAshSpecies,
    [STRINGID_ATTACKERBECAMEFULLYCHARGED] = sText_AttackerBecameFullyCharged,
    [STRINGID_HEALBLOCKEDNOMORE] = sText_HealBlockedNoMore,
    [STRINGID_TORMENTEDNOMORE] = sText_TormentedNoMore,
    [STRINGID_ATKGOTOVERINFATUATION] = sText_AttackerGotOverInfatuation,
    [STRINGID_EJECTBUTTONACTIVATE] = sText_EjectButtonActivate,
    [STRINGID_REDCARDACTIVATE] = sText_RedCardActivate,
    [STRINGID_PKMNBURNHEALED] = sText_PkmnBurnHealed,
    [STRINGID_STICKYBARBTRANSFER] = sText_StickyBarbTransfer,
    [STRINGID_ITEMCANNOTBEREMOVED] = sText_ItemCannotBeRemoved,
    [STRINGID_PKMNGOTOVERITSINFATUATION] = sText_PkmnGotOverItsInfatuation,
    [STRINGID_PKMNSHOOKOFFTHETAUNT] = sText_PkmnShookOffTheTaunt,
    [STRINGID_MICLEBERRYACTIVATES] = sText_MicleBerryActivates,
    [STRINGID_CANACTFASTERTHANKSTO] = sText_CanActFaster,
    [STRINGID_CURIOUSMEDICINEENTERS] = sText_CuriousMedicineEnters,
    [STRINGID_ASONEENTERS] = sText_AsOneEnters,
    [STRINGID_PKMNMADESHELLGLEAM] = sText_PkmnMadeShellGleam,
    [STRINGID_ABILITYRAISEDSTATDRASTICALLY] = sText_AbilityRaisedStatDrastically,
    [STRINGID_PKMNSWILLPERISHIN3TURNS] = sText_PkmnsWillPerishIn3Turns,
    [STRINGID_ASANDSTORMKICKEDUP] = sText_ASandstormKickedUp,
    [STRINGID_BATTLERABILITYRAISEDSTAT] = sText_BattlerAbilityRaisedStat,
    [STRINGID_FETCHEDPOKEBALL] = sText_FetchedPokeBall,
    [STRINGID_CLEARAMULETWONTLOWERSTATS] = sText_ClearAmuletWontLowerStats,
    [STRINGID_CLOAKEDINAFREEZINGLIGHT] = sText_CloakedInAFreezingLight,
    [STRINGID_DESTINYKNOTACTIVATES] = sText_DestinyKnotActivates,
    [STRINGID_NOONEWILLBEABLETORUNAWAY] = sText_NoOneWillBeAbleToRun,
    [STRINGID_PKNMABSORBINGPOWER] = sText_PkmnAbsorbingPower,
    [STRINGID_RECEIVERABILITYTAKEOVER] = sText_ReceiverAbilityTakeOver,
    [STRINGID_SCRIPTINGABILITYSTATRAISE] = sText_ScriptingAbilityRaisedStat,
    [STRINGID_HEALERCURE] = sText_HealerCure,
    [STRINGID_ATTACKERLOSTFIRETYPE] = sText_AttackerLostFireType,
    [STRINGID_ATTACKERCUREDTARGETSTATUS] = sText_AttackerCuredTargetStatus,
    [STRINGID_ILLUSIONWOREOFF] = sText_IllusionWoreOff,
    [STRINGID_BUGBITE] = sText_BugBite,
    [STRINGID_INCINERATEBURN] = sText_IncinerateBurn,
    [STRINGID_AIRBALLOONPOP] = sText_AirBalloonPop,
    [STRINGID_AIRBALLOONFLOAT] = sText_AirBalloonFloat,
    [STRINGID_TARGETATEITEM] = sText_TargetAteItem,
    [STRINGID_BERRYDMGREDUCES] = sText_BerryDmgReducing,
    [STRINGID_GEMACTIVATES] = sText_GemActivates,
    [STRINGID_LASERFOCUS] = sText_LaserFocusMessage,
    [STRINGID_THROATCHOPENDS] = sText_ThroatChopEnds,
    [STRINGID_PKMNCANTUSEMOVETHROATCHOP] = sText_PkmnCantUseMoveThroatChop,
    [STRINGID_USEDINSTRUCTEDMOVE] = sText_UsedInstructedMove,
    [STRINGID_CELEBRATEMESSAGE] = sText_CelebrateMessage,
    [STRINGID_AROMAVEILPROTECTED] = sText_AromaVeilProtected,
    [STRINGID_SWEETVEILPROTECTED] = sText_SweetVeilProtected,
    [STRINGID_FLOWERVEILPROTECTED] = sText_FlowerVeilProtected,
    [STRINGID_SAFETYGOGGLESPROTECTED] = sText_SafetyGogglesProtected,
    [STRINGID_SPECTRALTHIEFSTEAL] = sText_SpectralThiefSteal,
    [STRINGID_BELCHCANTSELECT] = sText_BelchCantUse,
    [STRINGID_TRAINER1LOSETEXT] = sText_Trainer1LoseText,
    [STRINGID_PKMNGAINEDEXP] = sText_PkmnGainedEXP,
    [STRINGID_PKMNGREWTOLV] = sText_PkmnGrewToLv,
    [STRINGID_PKMNLEARNEDMOVE] = sText_PkmnLearnedMove,
    [STRINGID_TRYTOLEARNMOVE1] = sText_TryToLearnMove1,
    [STRINGID_TRYTOLEARNMOVE2] = sText_TryToLearnMove2,
    [STRINGID_TRYTOLEARNMOVE3] = sText_TryToLearnMove3,
    [STRINGID_PKMNFORGOTMOVE] = sText_PkmnForgotMove,
    [STRINGID_STOPLEARNINGMOVE] = sText_StopLearningMove,
    [STRINGID_DIDNOTLEARNMOVE] = sText_DidNotLearnMove,
    [STRINGID_PKMNLEARNEDMOVE2] = sText_PkmnLearnedMove2,
    [STRINGID_ATTACKMISSED] = sText_AttackMissed,
    [STRINGID_PKMNPROTECTEDITSELF] = sText_PkmnProtectedItself,
    [STRINGID_STATSWONTINCREASE2] = sText_StatsWontIncrease2,
    [STRINGID_AVOIDEDDAMAGE] = sText_AvoidedDamage,
    [STRINGID_ITDOESNTAFFECT] = sText_ItDoesntAffect,
    [STRINGID_ATTACKERFAINTED] = sText_AttackerFainted,
    [STRINGID_TARGETFAINTED] = sText_TargetFainted,
    [STRINGID_PLAYERGOTMONEY] = sText_PlayerGotMoney,
    [STRINGID_PLAYERWHITEOUT] = sText_PlayerWhiteout,
    [STRINGID_PLAYERWHITEOUT2] = sText_PlayerWhiteout2,
    [STRINGID_PREVENTSESCAPE] = sText_PreventsEscape,
    [STRINGID_HITXTIMES] = sText_HitXTimes,
    [STRINGID_PKMNFELLASLEEP] = sText_PkmnFellAsleep,
    [STRINGID_PKMNMADESLEEP] = sText_PkmnMadeSleep,
    [STRINGID_PKMNALREADYASLEEP] = sText_PkmnAlreadyAsleep,
    [STRINGID_PKMNALREADYASLEEP2] = sText_PkmnAlreadyAsleep2,
    [STRINGID_PKMNWASNTAFFECTED] = sText_PkmnWasntAffected,
    [STRINGID_PKMNWASPOISONED] = sText_PkmnWasPoisoned,
    [STRINGID_PKMNPOISONEDBY] = sText_PkmnPoisonedBy,
    [STRINGID_PKMNHURTBYPOISON] = sText_PkmnHurtByPoison,
    [STRINGID_PKMNALREADYPOISONED] = sText_PkmnAlreadyPoisoned,
    [STRINGID_PKMNBADLYPOISONED] = sText_PkmnBadlyPoisoned,
    [STRINGID_PKMNENERGYDRAINED] = sText_PkmnEnergyDrained,
    [STRINGID_PKMNWASBURNED] = sText_PkmnWasBurned,
    [STRINGID_PKMNBURNEDBY] = sText_PkmnBurnedBy,
    [STRINGID_PKMNHURTBYBURN] = sText_PkmnHurtByBurn,
    [STRINGID_PKMNWASFROZEN] = sText_PkmnWasFrozen,
    [STRINGID_PKMNFROZENBY] = sText_PkmnFrozenBy,
    [STRINGID_PKMNISFROZEN] = sText_PkmnIsFrozen,
    [STRINGID_PKMNWASDEFROSTED] = sText_PkmnWasDefrosted,
    [STRINGID_PKMNWASDEFROSTED2] = sText_PkmnWasDefrosted2,
    [STRINGID_PKMNWASDEFROSTEDBY] = sText_PkmnWasDefrostedBy,
    [STRINGID_PKMNWASPARALYZED] = sText_PkmnWasParalyzed,
    [STRINGID_PKMNWASPARALYZEDBY] = sText_PkmnWasParalyzedBy,
    [STRINGID_PKMNISPARALYZED] = sText_PkmnIsParalyzed,
    [STRINGID_PKMNISALREADYPARALYZED] = sText_PkmnIsAlreadyParalyzed,
    [STRINGID_PKMNHEALEDPARALYSIS] = sText_PkmnHealedParalysis,
    [STRINGID_PKMNDREAMEATEN] = sText_PkmnDreamEaten,
    [STRINGID_STATSWONTINCREASE] = sText_StatsWontIncrease,
    [STRINGID_STATSWONTDECREASE] = sText_StatsWontDecrease,
    [STRINGID_TEAMSTOPPEDWORKING] = sText_TeamStoppedWorking,
    [STRINGID_FOESTOPPEDWORKING] = sText_FoeStoppedWorking,
    [STRINGID_PKMNISCONFUSED] = sText_PkmnIsConfused,
    [STRINGID_PKMNHEALEDCONFUSION] = sText_PkmnHealedConfusion,
    [STRINGID_PKMNWASCONFUSED] = sText_PkmnWasConfused,
    [STRINGID_PKMNALREADYCONFUSED] = sText_PkmnAlreadyConfused,
    [STRINGID_PKMNFELLINLOVE] = sText_PkmnFellInLove,
    [STRINGID_PKMNINLOVE] = sText_PkmnInLove,
    [STRINGID_PKMNIMMOBILIZEDBYLOVE] = sText_PkmnImmobilizedByLove,
    [STRINGID_PKMNBLOWNAWAY] = sText_PkmnBlownAway,
    [STRINGID_PKMNCHANGEDTYPE] = sText_PkmnChangedType,
    [STRINGID_PKMNFLINCHED] = sText_PkmnFlinched,
    [STRINGID_PKMNREGAINEDHEALTH] = sText_PkmnRegainedHealth,
    [STRINGID_PKMNHPFULL] = sText_PkmnHPFull,
    [STRINGID_PKMNRAISEDSPDEF] = sText_PkmnRaisedSpDef,
    [STRINGID_PKMNRAISEDDEF] = sText_PkmnRaisedDef,
    [STRINGID_PKMNCOVEREDBYVEIL] = sText_PkmnCoveredByVeil,
    [STRINGID_PKMNUSEDSAFEGUARD] = sText_PkmnUsedSafeguard,
    [STRINGID_PKMNSAFEGUARDEXPIRED] = sText_PkmnSafeguardExpired,
    [STRINGID_PKMNWENTTOSLEEP] = sText_PkmnWentToSleep,
    [STRINGID_PKMNSLEPTHEALTHY] = sText_PkmnSleptHealthy,
    [STRINGID_PKMNWHIPPEDWHIRLWIND] = sText_PkmnWhippedWhirlwind,
    [STRINGID_PKMNTOOKSUNLIGHT] = sText_PkmnTookSunlight,
    [STRINGID_PKMNLOWEREDHEAD] = sText_PkmnLoweredHead,
    [STRINGID_PKMNISGLOWING] = sText_PkmnIsGlowing,
    [STRINGID_PKMNFLEWHIGH] = sText_PkmnFlewHigh,
    [STRINGID_PKMNDUGHOLE] = sText_PkmnDugHole,
    [STRINGID_PKMNSQUEEZEDBYBIND] = sText_PkmnSqueezedByBind,
    [STRINGID_PKMNTRAPPEDINVORTEX] = sText_PkmnTrappedInVortex,
    [STRINGID_PKMNWRAPPEDBY] = sText_PkmnWrappedBy,
    [STRINGID_PKMNCLAMPED] = sText_PkmnClamped,
    [STRINGID_PKMNHURTBY] = sText_PkmnHurtBy,
    [STRINGID_PKMNFREEDFROM] = sText_PkmnFreedFrom,
    [STRINGID_PKMNCRASHED] = sText_PkmnCrashed,
    [STRINGID_PKMNSHROUDEDINMIST] = gText_PkmnShroudedInMist,
    [STRINGID_PKMNPROTECTEDBYMIST] = sText_PkmnProtectedByMist,
    [STRINGID_PKMNGETTINGPUMPED] = gText_PkmnGettingPumped,
    [STRINGID_PKMNHITWITHRECOIL] = sText_PkmnHitWithRecoil,
    [STRINGID_PKMNPROTECTEDITSELF2] = sText_PkmnProtectedItself2,
    [STRINGID_PKMNBUFFETEDBYSANDSTORM] = sText_PkmnBuffetedBySandstorm,
    [STRINGID_PKMNPELTEDBYHAIL] = sText_PkmnPeltedByHail,
    [STRINGID_PKMNSEEDED] = sText_PkmnSeeded,
    [STRINGID_PKMNEVADEDATTACK] = sText_PkmnEvadedAttack,
    [STRINGID_PKMNSAPPEDBYLEECHSEED] = sText_PkmnSappedByLeechSeed,
    [STRINGID_PKMNFASTASLEEP] = sText_PkmnFastAsleep,
    [STRINGID_PKMNWOKEUP] = sText_PkmnWokeUp,
    [STRINGID_PKMNUPROARKEPTAWAKE] = sText_PkmnUproarKeptAwake,
    [STRINGID_PKMNWOKEUPINUPROAR] = sText_PkmnWokeUpInUproar,
    [STRINGID_PKMNCAUSEDUPROAR] = sText_PkmnCausedUproar,
    [STRINGID_PKMNMAKINGUPROAR] = sText_PkmnMakingUproar,
    [STRINGID_PKMNCALMEDDOWN] = sText_PkmnCalmedDown,
    [STRINGID_PKMNCANTSLEEPINUPROAR] = sText_PkmnCantSleepInUproar,
    [STRINGID_PKMNSTOCKPILED] = sText_PkmnStockpiled,
    [STRINGID_PKMNCANTSTOCKPILE] = sText_PkmnCantStockpile,
    [STRINGID_PKMNCANTSLEEPINUPROAR2] = sText_PkmnCantSleepInUproar2,
    [STRINGID_UPROARKEPTPKMNAWAKE] = sText_UproarKeptPkmnAwake,
    [STRINGID_PKMNSTAYEDAWAKEUSING] = sText_PkmnStayedAwakeUsing,
    [STRINGID_PKMNSTORINGENERGY] = sText_PkmnStoringEnergy,
    [STRINGID_PKMNUNLEASHEDENERGY] = sText_PkmnUnleashedEnergy,
    [STRINGID_PKMNFATIGUECONFUSION] = sText_PkmnFatigueConfusion,
    [STRINGID_PLAYERPICKEDUPMONEY] = sText_PlayerPickedUpMoney,
    [STRINGID_PKMNUNAFFECTED] = sText_PkmnUnaffected,
    [STRINGID_PKMNTRANSFORMEDINTO] = sText_PkmnTransformedInto,
    [STRINGID_PKMNMADESUBSTITUTE] = sText_PkmnMadeSubstitute,
    [STRINGID_PKMNHASSUBSTITUTE] = sText_PkmnHasSubstitute,
    [STRINGID_SUBSTITUTEDAMAGED] = sText_SubstituteDamaged,
    [STRINGID_PKMNSUBSTITUTEFADED] = sText_PkmnSubstituteFaded,
    [STRINGID_PKMNMUSTRECHARGE] = sText_PkmnMustRecharge,
    [STRINGID_PKMNRAGEBUILDING] = sText_PkmnRageBuilding,
    [STRINGID_PKMNMOVEWASDISABLED] = sText_PkmnMoveWasDisabled,
    [STRINGID_PKMNMOVEISDISABLED] = sText_PkmnMoveIsDisabled,
    [STRINGID_PKMNMOVEDISABLEDNOMORE] = sText_PkmnMoveDisabledNoMore,
    [STRINGID_PKMNGOTENCORE] = sText_PkmnGotEncore,
    [STRINGID_PKMNENCOREENDED] = sText_PkmnEncoreEnded,
    [STRINGID_PKMNTOOKAIM] = sText_PkmnTookAim,
    [STRINGID_PKMNSKETCHEDMOVE] = sText_PkmnSketchedMove,
    [STRINGID_PKMNTRYINGTOTAKEFOE] = sText_PkmnTryingToTakeFoe,
    [STRINGID_PKMNTOOKFOE] = sText_PkmnTookFoe,
    [STRINGID_PKMNREDUCEDPP] = sText_PkmnReducedPP,
    [STRINGID_PKMNSTOLEITEM] = sText_PkmnStoleItem,
    [STRINGID_TARGETCANTESCAPENOW] = sText_TargetCantEscapeNow,
    [STRINGID_PKMNFELLINTONIGHTMARE] = sText_PkmnFellIntoNightmare,
    [STRINGID_PKMNLOCKEDINNIGHTMARE] = sText_PkmnLockedInNightmare,
    [STRINGID_PKMNLAIDCURSE] = sText_PkmnLaidCurse,
    [STRINGID_PKMNAFFLICTEDBYCURSE] = sText_PkmnAfflictedByCurse,
    [STRINGID_SPIKESSCATTERED] = sText_SpikesScattered,
    [STRINGID_PKMNHURTBYSPIKES] = sText_PkmnHurtBySpikes,
    [STRINGID_PKMNIDENTIFIED] = sText_PkmnIdentified,
    [STRINGID_PKMNPERISHCOUNTFELL] = sText_PkmnPerishCountFell,
    [STRINGID_PKMNBRACEDITSELF] = sText_PkmnBracedItself,
    [STRINGID_PKMNENDUREDHIT] = sText_PkmnEnduredHit,
    [STRINGID_MAGNITUDESTRENGTH] = sText_MagnitudeStrength,
    [STRINGID_PKMNCUTHPMAXEDATTACK] = sText_PkmnCutHPMaxedAttack,
    [STRINGID_PKMNCOPIEDSTATCHANGES] = sText_PkmnCopiedStatChanges,
    [STRINGID_PKMNGOTFREE] = sText_PkmnGotFree,
    [STRINGID_PKMNSHEDLEECHSEED] = sText_PkmnShedLeechSeed,
    [STRINGID_PKMNBLEWAWAYSPIKES] = sText_PkmnBlewAwaySpikes,
    [STRINGID_PKMNFLEDFROMBATTLE] = sText_PkmnFledFromBattle,
    [STRINGID_PKMNFORESAWATTACK] = sText_PkmnForesawAttack,
    [STRINGID_PKMNTOOKATTACK] = sText_PkmnTookAttack,
    [STRINGID_PKMNATTACK] = sText_PkmnAttack,
    [STRINGID_PKMNCENTERATTENTION] = sText_PkmnCenterAttention,
    [STRINGID_PKMNCHARGINGPOWER] = sText_PkmnChargingPower,
    [STRINGID_NATUREPOWERTURNEDINTO] = sText_NaturePowerTurnedInto,
    [STRINGID_PKMNSTATUSNORMAL] = sText_PkmnStatusNormal,
    [STRINGID_PKMNHASNOMOVESLEFT] = COMPOUND_STRING("¡A {B_ATK_NAME_WITH_PREFIX} no le\nquedan PPs!\p"),
    [STRINGID_PKMNSUBJECTEDTOTORMENT] = sText_PkmnSubjectedToTorment,
    [STRINGID_PKMNCANTUSEMOVETORMENT] = sText_PkmnCantUseMoveTorment,
    [STRINGID_PKMNTIGHTENINGFOCUS] = sText_PkmnTighteningFocus,
    [STRINGID_PKMNFELLFORTAUNT] = sText_PkmnFellForTaunt,
    [STRINGID_PKMNCANTUSEMOVETAUNT] = sText_PkmnCantUseMoveTaunt,
    [STRINGID_PKMNREADYTOHELP] = sText_PkmnReadyToHelp,
    [STRINGID_PKMNSWITCHEDITEMS] = sText_PkmnSwitchedItems,
    [STRINGID_PKMNCOPIEDFOE] = sText_PkmnCopiedFoe,
    [STRINGID_PKMNMADEWISH] = sText_PkmnMadeWish,
    [STRINGID_PKMNWISHCAMETRUE] = sText_PkmnWishCameTrue,
    [STRINGID_PKMNPLANTEDROOTS] = sText_PkmnPlantedRoots,
    [STRINGID_PKMNABSORBEDNUTRIENTS] = sText_PkmnAbsorbedNutrients,
    [STRINGID_PKMNANCHOREDITSELF] = sText_PkmnAnchoredItself,
    [STRINGID_PKMNWASMADEDROWSY] = sText_PkmnWasMadeDrowsy,
    [STRINGID_PKMNKNOCKEDOFF] = sText_PkmnKnockedOff,
    [STRINGID_PKMNSWAPPEDABILITIES] = sText_PkmnSwappedAbilities,
    [STRINGID_PKMNSEALEDOPPONENTMOVE] = sText_PkmnSealedOpponentMove,
    [STRINGID_PKMNCANTUSEMOVESEALED] = sText_PkmnCantUseMoveSealed,
    [STRINGID_PKMNWANTSGRUDGE] = sText_PkmnWantsGrudge,
    [STRINGID_PKMNLOSTPPGRUDGE] = sText_PkmnLostPPGrudge,
    [STRINGID_PKMNSHROUDEDITSELF] = sText_PkmnShroudedItself,
    [STRINGID_PKMNMOVEBOUNCED] = sText_PkmnMoveBounced,
    [STRINGID_PKMNWAITSFORTARGET] = sText_PkmnWaitsForTarget,
    [STRINGID_PKMNSNATCHEDMOVE] = sText_PkmnSnatchedMove,
    [STRINGID_PKMNMADEITRAIN] = sText_PkmnMadeItRain,
    [STRINGID_PKMNRAISEDSPEED] = sText_PkmnRaisedSpeed,
    [STRINGID_PKMNPROTECTEDBY] = sText_PkmnProtectedBy,
    [STRINGID_PKMNPREVENTSUSAGE] = sText_PkmnPreventsUsage,
    [STRINGID_PKMNRESTOREDHPUSING] = sText_PkmnRestoredHPUsing,
    [STRINGID_PKMNCHANGEDTYPEWITH] = sText_PkmnChangedTypeWith,
    [STRINGID_PKMNPREVENTSPARALYSISWITH] = sText_PkmnPreventsParalysisWith,
    [STRINGID_PKMNPREVENTSROMANCEWITH] = sText_PkmnPreventsRomanceWith,
    [STRINGID_PKMNPREVENTSPOISONINGWITH] = sText_PkmnPreventsPoisoningWith,
    [STRINGID_PKMNPREVENTSCONFUSIONWITH] = sText_PkmnPreventsConfusionWith,
    [STRINGID_PKMNRAISEDFIREPOWERWITH] = sText_PkmnRaisedFirePowerWith,
    [STRINGID_PKMNANCHORSITSELFWITH] = sText_PkmnAnchorsItselfWith,
    [STRINGID_PKMNCUTSATTACKWITH] = sText_PkmnCutsAttackWith,
    [STRINGID_PKMNPREVENTSSTATLOSSWITH] = sText_PkmnPreventsStatLossWith,
    [STRINGID_PKMNHURTSWITH] = sText_PkmnHurtsWith,
    [STRINGID_PKMNTRACED] = sText_PkmnTraced,
    [STRINGID_STATSHARPLY] = gText_StatSharply,
    [STRINGID_STATROSE] = gText_StatRose,
    [STRINGID_STATHARSHLY] = sText_StatHarshly,
    [STRINGID_STATFELL] = sText_StatFell,
    [STRINGID_ATTACKERSSTATROSE] = sText_AttackersStatRose,
    [STRINGID_DEFENDERSSTATROSE] = gText_DefendersStatRose,
    [STRINGID_ATTACKERSSTATFELL] = sText_AttackersStatFell,
    [STRINGID_DEFENDERSSTATFELL] = sText_DefendersStatFell,
    [STRINGID_CRITICALHIT] = sText_CriticalHit,
    [STRINGID_ONEHITKO] = sText_OneHitKO,
    [STRINGID_123POOF] = sText_123Poof,
    [STRINGID_ANDELLIPSIS] = sText_AndEllipsis,
    [STRINGID_NOTVERYEFFECTIVE] = sText_NotVeryEffective,
    [STRINGID_SUPEREFFECTIVE] = sText_SuperEffective,
    [STRINGID_GOTAWAYSAFELY] = sText_GotAwaySafely,
    [STRINGID_WILDPKMNFLED] = sText_WildPkmnFled,
    [STRINGID_NORUNNINGFROMTRAINERS] = sText_NoRunningFromTrainers,
    [STRINGID_CANTESCAPE] = sText_CantEscape,
    [STRINGID_DONTLEAVEBIRCH] = sText_DontLeaveBirch,
    [STRINGID_BUTNOTHINGHAPPENED] = sText_ButNothingHappened,
    [STRINGID_BUTITFAILED] = sText_ButItFailed,
    [STRINGID_ITHURTCONFUSION] = sText_ItHurtConfusion,
    [STRINGID_MIRRORMOVEFAILED] = sText_MirrorMoveFailed,
    [STRINGID_STARTEDTORAIN] = sText_StartedToRain,
    [STRINGID_DOWNPOURSTARTED] = sText_DownpourStarted,
    [STRINGID_RAINCONTINUES] = sText_RainContinues,
    [STRINGID_DOWNPOURCONTINUES] = sText_DownpourContinues,
    [STRINGID_RAINSTOPPED] = sText_RainStopped,
    [STRINGID_SANDSTORMBREWED] = sText_SandstormBrewed,
    [STRINGID_SANDSTORMRAGES] = sText_SandstormRages,
    [STRINGID_SANDSTORMSUBSIDED] = sText_SandstormSubsided,
    [STRINGID_SUNLIGHTGOTBRIGHT] = sText_SunlightGotBright,
    [STRINGID_SUNLIGHTSTRONG] = sText_SunlightStrong,
    [STRINGID_SUNLIGHTFADED] = sText_SunlightFaded,
    [STRINGID_STARTEDHAIL] = sText_StartedHail,
    [STRINGID_HAILCONTINUES] = sText_HailContinues,
    [STRINGID_HAILSTOPPED] = sText_HailStopped,
    [STRINGID_STARTEDSNOW] = sText_StartedSnow,
    [STRINGID_SNOWCONTINUES] = sText_SnowContinues,
    [STRINGID_SNOWSTOPPED] = sText_SnowStopped,
    [STRINGID_FOGCREPTUP] = sText_FogCreptUp,
    [STRINGID_FOGISDEEP] = sText_FogIsDeep,
    [STRINGID_FOGLIFTED] = sText_FogLifted,
    [STRINGID_FAILEDTOSPITUP] = sText_FailedToSpitUp,
    [STRINGID_FAILEDTOSWALLOW] = sText_FailedToSwallow,
    [STRINGID_WINDBECAMEHEATWAVE] = sText_WindBecameHeatWave,
    [STRINGID_STATCHANGESGONE] = sText_StatChangesGone,
    [STRINGID_COINSSCATTERED] = sText_CoinsScattered,
    [STRINGID_TOOWEAKFORSUBSTITUTE] = sText_TooWeakForSubstitute,
    [STRINGID_SHAREDPAIN] = sText_SharedPain,
    [STRINGID_BELLCHIMED] = sText_BellChimed,
    [STRINGID_FAINTINTHREE] = sText_FaintInThree,
    [STRINGID_NOPPLEFT] = sText_NoPPLeft,
    [STRINGID_BUTNOPPLEFT] = sText_ButNoPPLeft,
    [STRINGID_PLAYERUSEDITEM] = sText_PlayerUsedItem,
    [STRINGID_WALLYUSEDITEM] = sText_WallyUsedItem,
    [STRINGID_TRAINERBLOCKEDBALL] = sText_TrainerBlockedBall,
    [STRINGID_DONTBEATHIEF] = sText_DontBeAThief,
    [STRINGID_ITDODGEDBALL] = sText_ItDodgedBall,
    [STRINGID_YOUMISSEDPKMN] = sText_YouMissedPkmn,
    [STRINGID_PKMNBROKEFREE] = sText_PkmnBrokeFree,
    [STRINGID_ITAPPEAREDCAUGHT] = sText_ItAppearedCaught,
    [STRINGID_AARGHALMOSTHADIT] = sText_AarghAlmostHadIt,
    [STRINGID_SHOOTSOCLOSE] = sText_ShootSoClose,
    [STRINGID_GOTCHAPKMNCAUGHTPLAYER] = sText_GotchaPkmnCaughtPlayer,
    [STRINGID_GOTCHAPKMNCAUGHTWALLY] = sText_GotchaPkmnCaughtWally,
    [STRINGID_GIVENICKNAMECAPTURED] = sText_GiveNicknameCaptured,
    [STRINGID_PKMNSENTTOPC] = sText_PkmnSentToPC,
    [STRINGID_PKMNDATAADDEDTODEX] = sText_PkmnDataAddedToDex,
    [STRINGID_ITISRAINING] = sText_ItIsRaining,
    [STRINGID_ITISSNOWING] = sText_ItIsSnowing,
    [STRINGID_HACEMUCHOCALOR] = sText_HaceMuchoCalor,
    [STRINGID_SANDSTORMISRAGING] = sText_SandstormIsRaging,
    [STRINGID_CANTESCAPE2] = sText_CantEscape2,
    [STRINGID_PKMNIGNORESASLEEP] = sText_PkmnIgnoresAsleep,
    [STRINGID_PKMNIGNOREDORDERS] = sText_PkmnIgnoredOrders,
    [STRINGID_PKMNBEGANTONAP] = sText_PkmnBeganToNap,
    [STRINGID_PKMNLOAFING] = sText_PkmnLoafing,
    [STRINGID_PKMNWONTOBEY] = sText_PkmnWontObey,
    [STRINGID_PKMNTURNEDAWAY] = sText_PkmnTurnedAway,
    [STRINGID_PKMNPRETENDNOTNOTICE] = sText_PkmnPretendNotNotice,
    [STRINGID_ENEMYABOUTTOSWITCHPKMN] = sText_EnemyAboutToSwitchPkmn,
    [STRINGID_CREPTCLOSER] = sText_CreptCloser,
    [STRINGID_CANTGETCLOSER] = sText_CantGetCloser,
    [STRINGID_PKMNWATCHINGCAREFULLY] = sText_PkmnWatchingCarefully,
    [STRINGID_PKMNCURIOUSABOUTX] = sText_PkmnCuriousAboutX,
    [STRINGID_PKMNENTHRALLEDBYX] = sText_PkmnEnthralledByX,
    [STRINGID_PKMNIGNOREDX] = sText_PkmnIgnoredX,
    [STRINGID_THREWPOKEBLOCKATPKMN] = sText_ThrewPokeblockAtPkmn,
    [STRINGID_OUTOFSAFARIBALLS] = sText_OutOfSafariBalls,
    [STRINGID_PKMNSITEMCUREDPARALYSIS] = sText_PkmnsItemCuredParalysis,
    [STRINGID_PKMNSITEMCUREDPOISON] = sText_PkmnsItemCuredPoison,
    [STRINGID_PKMNSITEMHEALEDBURN] = sText_PkmnsItemHealedBurn,
    [STRINGID_PKMNSITEMDEFROSTEDIT] = sText_PkmnsItemDefrostedIt,
    [STRINGID_PKMNSITEMWOKEIT] = sText_PkmnsItemWokeIt,
    [STRINGID_PKMNSITEMSNAPPEDOUT] = sText_PkmnsItemSnappedOut,
    [STRINGID_PKMNSITEMCUREDPROBLEM] = sText_PkmnsItemCuredProblem,
    [STRINGID_PKMNSITEMRESTOREDHEALTH] = sText_PkmnsItemRestoredHealth,
    [STRINGID_PKMNSITEMRESTOREDPP] = sText_PkmnsItemRestoredPP,
    [STRINGID_PKMNSITEMRESTOREDSTATUS] = sText_PkmnsItemRestoredStatus,
    [STRINGID_PKMNSITEMRESTOREDHPALITTLE] = sText_PkmnsItemRestoredHPALittle,
    [STRINGID_ITEMALLOWSONLYYMOVE] = sText_ItemAllowsOnlyYMove,
    [STRINGID_PKMNHUNGONWITHX] = sText_PkmnHungOnWithX,
    [STRINGID_EMPTYSTRING3] = gText_EmptyString,
    [STRINGID_PKMNSXPREVENTSBURNS] = sText_PkmnsXPreventsBurns,
    [STRINGID_PKMNSXBLOCKSY] = sText_PkmnsXBlocksY,
    [STRINGID_PKMNSXRESTOREDHPALITTLE2] = sText_PkmnsXRestoredHPALittle2,
    [STRINGID_PKMNSXWHIPPEDUPSANDSTORM] = sText_PkmnsXWhippedUpSandstorm,
    [STRINGID_PKMNSXPREVENTSYLOSS] = sText_PkmnsXPreventsYLoss,
    [STRINGID_PKMNSXINFATUATEDY] = sText_PkmnsXInfatuatedY,
    [STRINGID_NUEVECOLAS] = sText_NueveColas,
    [STRINGID_PARASITO] = sText_Parasito,
    [STRINGID_PKMNSXMADEYINEFFECTIVE] = sText_PkmnsXMadeYIneffective,
    [STRINGID_PKMNSXCUREDYPROBLEM] = sText_PkmnsXCuredYProblem,
    [STRINGID_ITSUCKEDLIQUIDOOZE] = sText_ItSuckedLiquidOoze,
    [STRINGID_PKMNTRANSFORMED] = sText_PkmnTransformed,
    [STRINGID_ELECTRICITYWEAKENED] = sText_ElectricityWeakened,
    [STRINGID_FIREWEAKENED] = sText_FireWeakened,
    [STRINGID_PKMNHIDUNDERWATER] = sText_PkmnHidUnderwater,
    [STRINGID_PKMNSPRANGUP] = sText_PkmnSprangUp,
    [STRINGID_XFOUNDONEY] = sText_XFoundOneY,
    [STRINGID_PLAYERDEFEATEDTRAINER] = COMPOUND_STRING("¡Has vencido a {B_TRAINER1_CLASS} {B_TRAINER1_NAME}!\p"),
    [STRINGID_SOOTHINGAROMA] = sText_SoothingAroma,
    [STRINGID_ITEMSCANTBEUSEDNOW] = sText_ItemsCantBeUsedNow,
    [STRINGID_FORXCOMMAYZ] = sText_ForXCommaYZ,
    [STRINGID_USINGITEMSTATOFPKMNROSE] = sText_UsingItemTheStatOfPkmnRose,
    [STRINGID_PKMNUSEDXTOGETPUMPED] = sText_PkmnUsedXToGetPumped,
    [STRINGID_PKMNSXMADEYUSELESS] = sText_PkmnsXMadeYUseless,
    [STRINGID_PKMNTRAPPEDBYSANDTOMB] = sText_PkmnTrappedBySandTomb,
    [STRINGID_EMPTYSTRING4] = sText_EmptyString4,
    [STRINGID_ABOOSTED] = sText_ABoosted,
    [STRINGID_PKMNSXINTENSIFIEDSUN] = sText_PkmnsXIntensifiedSun,
    [STRINGID_PKMNMAKESGROUNDMISS] = sText_PkmnMakesGroundMiss,
    [STRINGID_YOUTHROWABALLNOWRIGHT] = sText_YouThrowABallNowRight,
    [STRINGID_PKMNSXTOOKATTACK] = sText_PkmnsXTookAttack,
    [STRINGID_PKMNCHOSEXASDESTINY] = sText_PkmnChoseXAsDestiny,
    [STRINGID_PKMNLOSTFOCUS] = sText_PkmnLostFocus,
    [STRINGID_USENEXTPKMN] = sText_UseNextPkmn,
    [STRINGID_PKMNFLEDUSINGITS] = sText_PkmnFledUsingIts,
    [STRINGID_PKMNFLEDUSING] = sText_PkmnFledUsing,
    [STRINGID_PKMNWASDRAGGEDOUT] = sText_PkmnWasDraggedOut,
    [STRINGID_PKMNSITEMNORMALIZEDSTATUS] = sText_PkmnsItemNormalizedStatus,
    [STRINGID_TRAINER1USEDITEM] = sText_Trainer1UsedItem,
    [STRINGID_BOXISFULL] = sText_BoxIsFull,
    [STRINGID_PKMNAVOIDEDATTACK] = sText_PkmnAvoidedAttack,
    [STRINGID_PKMNSXMADEITINEFFECTIVE] = sText_PkmnsXMadeItIneffective,
    [STRINGID_PKMNSXPREVENTSFLINCHING] = sText_PkmnsXPreventsFlinching,
    [STRINGID_PKMNALREADYHASBURN] = sText_PkmnAlreadyHasBurn,
    [STRINGID_STATSWONTDECREASE2] = sText_StatsWontDecrease2,
    [STRINGID_PKMNSXBLOCKSY2] = sText_PkmnsXBlocksY2,
    [STRINGID_PKMNSXWOREOFF] = sText_PkmnsXWoreOff,
    [STRINGID_PKMNRAISEDDEFALITTLE] = sText_PkmnRaisedDefALittle,
    [STRINGID_PKMNRAISEDSPDEFALITTLE] = sText_PkmnRaisedSpDefALittle,
    [STRINGID_THEWALLSHATTERED] = sText_TheWallShattered,
    [STRINGID_PKMNSXPREVENTSYSZ] = sText_PkmnsXPreventsYsZ,
    [STRINGID_PKMNSXCUREDITSYPROBLEM] = sText_PkmnsXCuredItsYProblem,
    [STRINGID_ATTACKERCANTESCAPE] = sText_AttackerCantEscape,
    [STRINGID_PKMNOBTAINEDX] = sText_PkmnObtainedX,
    [STRINGID_PKMNOBTAINEDX2] = sText_PkmnObtainedX2,
    [STRINGID_PKMNOBTAINEDXYOBTAINEDZ] = sText_PkmnObtainedXYObtainedZ,
    [STRINGID_BUTNOEFFECT] = sText_ButNoEffect,
    [STRINGID_PKMNSXHADNOEFFECTONY] = sText_PkmnsXHadNoEffectOnY,
    [STRINGID_TWOENEMIESDEFEATED] = sText_TwoInGameTrainersDefeated,
    [STRINGID_TRAINER2LOSETEXT] = sText_Trainer2LoseText,
    [STRINGID_PKMNINCAPABLEOFPOWER] = sText_PkmnIncapableOfPower,
    [STRINGID_GLINTAPPEARSINEYE] = sText_GlintAppearsInEye,
    [STRINGID_PKMNGETTINGINTOPOSITION] = sText_PkmnGettingIntoPosition,
    [STRINGID_PKMNBEGANGROWLINGDEEPLY] = sText_PkmnBeganGrowlingDeeply,
    [STRINGID_PKMNEAGERFORMORE] = sText_PkmnEagerForMore,
    [STRINGID_DEFEATEDOPPONENTBYREFEREE] = sText_DefeatedOpponentByReferee,
    [STRINGID_LOSTTOOPPONENTBYREFEREE] = sText_LostToOpponentByReferee,
    [STRINGID_TIEDOPPONENTBYREFEREE] = sText_TiedOpponentByReferee,
    [STRINGID_QUESTIONFORFEITMATCH] = sText_QuestionForfeitMatch,
    [STRINGID_FORFEITEDMATCH] = sText_ForfeitedMatch,
    [STRINGID_PKMNTRANSFERREDSOMEONESPC] = gText_PkmnFueTransferidoAlPC,
    [STRINGID_TRAINER1WINTEXT] = sText_Trainer1WinText,
    [STRINGID_TRAINER2WINTEXT] = sText_Trainer2WinText,
    [STRINGID_ENDUREDSTURDY] = sText_EnduredViaSturdy,
    [STRINGID_POWERHERB] = sText_PowerHerbActivation,
    [STRINGID_HURTBYITEM] = sText_HurtByItem,
    [STRINGID_PSNBYITEM] = sText_BadlyPoisonedByItem,
    [STRINGID_BRNBYITEM] = sText_BurnedByItem,
    [STRINGID_DEFABILITYIN] = sText_TargetAbilityActivates,
    [STRINGID_GRAVITYINTENSIFIED] = sText_GravityIntensified,
    [STRINGID_TARGETIDENTIFIED] = sText_TargetIdentified,
    [STRINGID_TARGETWOKEUP] = sText_TargetWokeUp,
    [STRINGID_PKMNSTOLEANDATEITEM] = sText_PkmnStoleAndAteItem,
    [STRINGID_TAILWINDBLEW] = sText_TailWindBlew,
    [STRINGID_PKMNWENTBACK] = sText_PkmnWentBack,
    [STRINGID_PKMNCANTUSEITEMSANYMORE] = sText_PkmnCantUseItemsAnymore,
    [STRINGID_PKMNFLUNG] = sText_PkmnFlung,
    [STRINGID_PKMNPREVENTEDFROMHEALING] = sText_PkmnPreventedFromHealing,
    [STRINGID_PKMNSWITCHEDATKANDDEF] = sText_PkmnSwitchedAtkAndDef,
    [STRINGID_PKMNSABILITYSUPPRESSED] = sText_PkmnsAbilitySuppressed,
    [STRINGID_SHIELDEDFROMCRITICALHITS] = sText_ShieldedFromCriticalHits,
    [STRINGID_SWITCHEDATKANDSPATK] = sText_SwitchedAtkAndSpAtk,
    [STRINGID_SWITCHEDDEFANDSPDEF] = sText_SwitchedDefAndSpDef,
    [STRINGID_PKMNACQUIREDABILITY] = sText_PkmnAcquiredAbility,
    [STRINGID_POISONSPIKESSCATTERED] = sText_PoisonSpikesScattered,
    [STRINGID_PKMNSWITCHEDSTATCHANGES] = sText_PkmnSwitchedStatChanges,
    [STRINGID_PKMNSURROUNDEDWITHVEILOFWATER] = sText_PkmnSurroundedWithVeilOfWater,
    [STRINGID_PKMNLEVITATEDONELECTROMAGNETISM] = sText_PkmnLevitatedOnElectromagnetism,
    [STRINGID_PKMNTWISTEDDIMENSIONS] = sText_PkmnTwistedDimensions,
    [STRINGID_DIMENSIONSWERETWISTED] = sText_DimensionsWereTwisted,
    [STRINGID_POINTEDSTONESFLOAT] = sText_PointedStonesFloat,
    [STRINGID_CLOAKEDINMYSTICALMOONLIGHT] = sText_CloakedInMysticalMoonlight,
    [STRINGID_TRAPPEDBYSWIRLINGMAGMA] = sText_TrappedBySwirlingMagma,
    [STRINGID_VANISHEDINSTANTLY] = sText_VanishedInstantly,
    [STRINGID_PROTECTEDTEAM] = sText_ProtectedTeam,
    [STRINGID_SHAREDITSGUARD] = sText_SharedItsGuard,
    [STRINGID_SHAREDITSPOWER] = sText_SharedItsPower,
    [STRINGID_SWAPSDEFANDSPDEFOFALLPOKEMON] = sText_SwapsDefAndSpDefOfAllPkmn,
    [STRINGID_BIZARREAREACREATED] = sText_BizzareAreaCreated,
    [STRINGID_BECAMENIMBLE] = sText_BecameNimble,
    [STRINGID_HURLEDINTOTHEAIR] = sText_HurledIntoTheAir,
    [STRINGID_HELDITEMSLOSEEFFECTS] = sText_HeldItemsLoseEffects,
    [STRINGID_BIZARREARENACREATED] = sText_BizarreArenaCreated,
    [STRINGID_FELLSTRAIGHTDOWN] = sText_FellStraightDown,
    [STRINGID_TARGETCHANGEDTYPE ] = sText_TargetChangedType,
    [STRINGID_PKMNACQUIREDSIMPLE] = sText_PkmnAcquiredSimple,
    [STRINGID_EMPTYSTRING5] = sText_EmptyString4,
    [STRINGID_KINDOFFER] = sText_KindOffer,
    [STRINGID_RESETSTARGETSSTATLEVELS] = sText_ResetsTargetsStatLevels,
    [STRINGID_EMPTYSTRING6] = sText_EmptyString4,
    [STRINGID_ALLYSWITCHPOSITION] = sText_AllySwitchPosition,
    [STRINGID_RESTORETARGETSHEALTH] = sText_RestoreTargetsHealth,
    [STRINGID_TOOKPJMNINTOTHESKY] = sText_TookPkmnIntoTheSky,
    [STRINGID_FREEDFROMSKYDROP] = sText_FreedFromSkyDrop,
    [STRINGID_POSTPONETARGETMOVE] = sText_PostponeTargetMove,
    [STRINGID_REFLECTTARGETSTYPE] = sText_ReflectTargetsType,
    [STRINGID_TRANSFERHELDITEM] = sText_TransferHeldItem,
    [STRINGID_EMBARGOENDS] = sText_EmbargoEnds,
    [STRINGID_ELECTROMAGNETISM] = sText_Electromagnetism,
    [STRINGID_BUFFERENDS] = sText_BufferEnds,
    [STRINGID_TELEKINESISENDS] = sText_TelekinesisEnds,
    [STRINGID_TAILWINDENDS] = sText_TailwindEnds,
    [STRINGID_LUCKYCHANTENDS] = sText_LuckyChantEnds,
    [STRINGID_TRICKROOMENDS] = sText_TrickRoomEnds,
    [STRINGID_WONDERROOMENDS] = sText_WonderRoomEnds,
    [STRINGID_MAGICROOMENDS] = sText_MagicRoomEnds,
    [STRINGID_MUDSPORTENDS] = sText_MudSportEnds,
    [STRINGID_WATERSPORTENDS] = sText_WaterSportEnds,
    [STRINGID_GRAVITYENDS] = sText_GravityEnds,
    [STRINGID_AQUARINGHEAL] = sText_AquaRingHeal,
    [STRINGID_ELECTRICTERRAINENDS] = sText_ElectricTerrainEnds,
    [STRINGID_MISTYTERRAINENDS] = sText_MistyTerrainEnds,
    [STRINGID_PSYCHICTERRAINENDS] = sText_PsychicTerrainEnds,
    [STRINGID_GRASSYTERRAINENDS] = sText_GrassyTerrainEnds,
    [STRINGID_TARGETABILITYSTATRAISE] = sText_TargetAbilityRaisedStat,
    [STRINGID_TARGETSSTATWASMAXEDOUT] = sText_TargetsStatWasMaxedOut,
    [STRINGID_ATTACKERABILITYSTATRAISE] = sText_AttackerAbilityRaisedStat,
    [STRINGID_POISONHEALHPUP] = sText_PoisonHealHpUp,
    [STRINGID_CARNIVORO] = sText_Carnivoro,
    [STRINGID_BADDREAMSDMG] = sText_BadDreamsDmg,
    [STRINGID_MOLDBREAKERENTERS] = sText_MoldBreakerEnters,
    [STRINGID_TERAVOLTENTERS] = sText_TeravoltEnters,
    [STRINGID_TURBOBLAZEENTERS] = sText_TurboblazeEnters,
    [STRINGID_SLOWSTARTENTERS] = sText_SlowStartEnters,
    [STRINGID_SLOWSTARTEND] = sText_SlowStartEnd,
    [STRINGID_SOLARPOWERHPDROP] = sText_SolarPowerHpDrop,
    [STRINGID_AFTERMATHDMG] = sText_AftermathDmg,
    [STRINGID_ANTICIPATIONACTIVATES] = sText_AnticipationActivates,
    [STRINGID_FOREWARNACTIVATES] = sText_ForewarnActivates,
    [STRINGID_ICEBODYHPGAIN] = sText_IceBodyHpGain,
    [STRINGID_SNOWWARNINGHAIL] = sText_SnowWarningHail,
    [STRINGID_SNOWWARNINGSNOW] = sText_SnowWarningSnow,
    [STRINGID_FRISKACTIVATES] = sText_FriskActivates,
    [STRINGID_UNNERVEENTERS] = sText_UnnerveEnters,
    [STRINGID_HARVESTBERRY] = sText_HarvestBerry,
    [STRINGID_LASTABILITYRAISEDSTAT] = sText_LastAbilityRaisedBuff1,
    [STRINGID_MAGICBOUNCEACTIVATES] = sText_MagicBounceActivates,
    [STRINGID_PROTEANTYPECHANGE] = sText_ProteanTypeChange,
    [STRINGID_SYMBIOSISITEMPASS] = sText_SymbiosisItemPass,
    [STRINGID_STEALTHROCKDMG] = sText_StealthRockDmg,
    [STRINGID_TOXICSPIKESABSORBED] = sText_ToxicSpikesAbsorbed,
    [STRINGID_TOXICSPIKESPOISONED] = sText_ToxicSpikesPoisoned,
    [STRINGID_STICKYWEBSWITCHIN] = sText_StickyWebSwitchIn,
    [STRINGID_HEALINGWISHCAMETRUE] = sText_HealingWishCameTrue,
    [STRINGID_HEALINGWISHHEALED] = sText_HealingWishHealed,
    [STRINGID_LUNARDANCECAMETRUE] = sText_LunarDanceCameTrue,
    [STRINGID_CUSEDBODYDISABLED] = sText_CursedBodyDisabled,
    [STRINGID_ATTACKERACQUIREDABILITY] = sText_AttackerAcquiredAbility,
    [STRINGID_TARGETABILITYSTATLOWER] = sText_TargetAbilityLoweredStat,
    [STRINGID_TARGETSTATWONTGOHIGHER] = sText_TargetStatWontGoHigher,
    [STRINGID_PKMNMOVEBOUNCEDABILITY] = sText_PkmnMoveBouncedViaAbility,
    [STRINGID_IMPOSTERTRANSFORM] = sText_ImposterTransform,
    [STRINGID_ASSAULTVESTDOESNTALLOW] = sText_AssaultVestDoesntAllow,
    [STRINGID_GRAVITYPREVENTSUSAGE] = sText_GravityPreventsUsage,
    [STRINGID_HEALBLOCKPREVENTSUSAGE] = sText_HealBlockPreventsUsage,
    [STRINGID_NOTDONEYET] = sText_NotDoneYet,
    [STRINGID_STICKYWEBUSED] = sText_StickyWebUsed,
    [STRINGID_QUASHSUCCESS] = sText_QuashSuccess,
    [STRINGID_PKMNBLEWAWAYTOXICSPIKES] = sText_PkmnBlewAwayToxicSpikes,
    [STRINGID_PKMNBLEWAWAYSTICKYWEB] = sText_PkmnBlewAwayStickyWeb,
    [STRINGID_PKMNBLEWAWAYSTEALTHROCK] = sText_PkmnBlewAwayStealthRock,
    [STRINGID_SPIKESDISAPPEAREDFROMTEAM] = sText_SpikesDisappearedFromTeam,
    [STRINGID_TOXICSPIKESDISAPPEAREDFROMTEAM] = sText_ToxicSpikesDisappearedFromTeam,
    [STRINGID_STEALTHROCKDISAPPEAREDFROMTEAM] = sText_StealthRockDisappearedFromTeam,
    [STRINGID_STICKYWEBDISAPPEAREDFROMTEAM] = sText_StickyWebDisappearedFromTeam,
    [STRINGID_SHARPSTEELDISAPPEAREDFROMTEAM] = sText_SharpSteelDisappearedFromTeam,
    [STRINGID_IONDELUGEON] = sText_IonDelugeOn,
    [STRINGID_TOPSYTURVYSWITCHEDSTATS] = sText_TopsyTurvySwitchedStats,
    [STRINGID_TERRAINBECOMESMISTY] = sText_TerrainBecomesMisty,
    [STRINGID_TERRAINBECOMESGRASSY] = sText_TerrainBecomesGrassy,
    [STRINGID_TERRAINBECOMESELECTRIC] = sText_TerrainBecomesElectric,
    [STRINGID_TERRAINBECOMESPSYCHIC] = sText_TerrainBecomesPsychic,
    [STRINGID_TARGETELECTRIFIED] = sText_TargetElectrified,
    [STRINGID_MEGAEVOREACTING] = sText_MegaEvoReacting,
    [STRINGID_FERVENTWISHREACHED] = sText_FerventWishReached,
    [STRINGID_MEGAEVOEVOLVED] = sText_MegaEvoEvolved,
    [STRINGID_DRASTICALLY] = sText_drastically,
    [STRINGID_SEVERELY] = sText_severely,
    [STRINGID_INFESTATION] = sText_Infestation,
    [STRINGID_NOEFFECTONTARGET] = sText_NoEffectOnTarget,
    [STRINGID_BURSTINGFLAMESHIT] = sText_BurstingFlames,
    [STRINGID_BESTOWITEMGIVING] = sText_BestowItemGiving,
    [STRINGID_THIRDTYPEADDED] = sText_ThirdTypeAdded,
    [STRINGID_FELLFORFEINT] = sText_FellForFeint,
    [STRINGID_POKEMONCANNOTUSEMOVE] = sText_PokemonCannotUseMove,
    [STRINGID_COVEREDINPOWDER] = sText_CoveredInPowder,
    [STRINGID_POWDEREXPLODES] = sText_PowderExplodes,
    [STRINGID_GRAVITYGROUNDING] = sText_GravityGrounding,
    [STRINGID_MISTYTERRAINPREVENTS] = sText_MistyTerrainPreventsStatus,
    [STRINGID_GRASSYTERRAINHEALS] = sText_GrassyTerrainHeals,
    [STRINGID_ELECTRICTERRAINPREVENTS] = sText_ElectricTerrainPreventsSleep,
    [STRINGID_PSYCHICTERRAINPREVENTS] = sText_PsychicTerrainPreventsPriority,
    [STRINGID_AURAFLAREDTOLIFE] = sText_AuraFlaredToLife,
    [STRINGID_AIRLOCKACTIVATES] = sText_AirLockActivates,
    [STRINGID_PRESSUREENTERS] = sText_PressureActivates,
    [STRINGID_DARKAURAENTERS] = sText_DarkAuraActivates,
    [STRINGID_GENERADOR] = sText_Generador,
    [STRINGID_ESTASOMNOLIENTO] = sText_EstaSomnoliento,
    [STRINGID_FAIRYAURAENTERS] = sText_FairyAuraActivates,
    [STRINGID_AURABREAKENTERS] = sText_AuraBreakActivates,
    [STRINGID_COMATOSEENTERS] = sText_ComatoseActivates,
    [STRINGID_SCREENCLEANERENTERS] = sText_ScreenCleanerActivates,
    [STRINGID_BOTHCANNOLONGERESCAPE] = sText_BothCanNoLongerEscape,
    [STRINGID_CANTESCAPEDUETOUSEDMOVE] = sText_CantEscapeDueToUsedMove,
    [STRINGID_PKMNBECAMEWEAKERTOFIRE] = sText_PkmnBecameWeakerToFire,
    [STRINGID_ABOUTTOUSEPOLTERGEIST] = sText_PkmnAboutToBeAttackedByItsItem,
    [STRINGID_CANTESCAPEBECAUSEOFCURRENTMOVE] = sText_CantEscapeBecauseOfCurrentMove,
    [STRINGID_PKMNTOOKTARGETHIGH] = sText_PkmnTookTargetHigh,
    [STRINGID_TARGETTOOHEAVY] = sText_TargetTooHeavy,
    [STRINGID_ATTACKERLOSTELECTRICTYPE] = sText_AttackerLostElectricType,
    [STRINGID_PKMNSABILITYPREVENTSABILITY] = sText_PkmnsAbilityPreventsAbility,
    [STRINGID_PKMNHURTBYFROSTBITE] = sText_PkmnHurtByFrostbite,
    [STRINGID_PKMNGOTFROSTBITE] = sText_PkmnGotFrostbite,
    [STRINGID_PKMNSITEMHEALEDFROSTBITE] = sText_PkmnsItemHealedFrostbite,
    [STRINGID_ATTACKERHEALEDITSFROSTBITE] = sText_AttackerHealedItsFrostbite,
    [STRINGID_PKMNFROSTBITEHEALED] = sText_PkmnFrostbiteHealed,
    [STRINGID_PKMNFROSTBITEHEALED2] = sText_PkmnFrostbiteHealed2,
    [STRINGID_PKMNFROSTBITEHEALEDBY] = sText_PkmnFrostbiteHealedBy,
    [STRINGID_ULTRABURSTREACTING] = sText_UltraBurstReacting,
    [STRINGID_ULTRABURSTCOMPLETED] = sText_UltraBurstCompleted,
    [STRINGID_TEAMGAINEDEXP] = sText_TeamGainedEXP,
    [STRINGID_TARGETCOVEREDINSTICKYCANDYSYRUP] = sText_TargetCoveredInStickyCandySyrup,
    [STRINGID_ITEMWASUSEDUP] = sText_ItemWasUsedUp,
    [STRINGID_ATTACKERLOSTITSTYPE] = sText_AttackerLostItsType,
    [STRINGID_CLOAKEDINAHARSHLIGHT] = sText_PkmnIsCloakedInAHarshLight,
    [STRINGID_AMOLADORAS] = sText_Amoladoras,
};

const u16 gTrainerUsedItemStringIds[] =
{
    STRINGID_PLAYERUSEDITEM, STRINGID_TRAINER1USEDITEM
};

const u16 gZEffectStringIds[] =
{
    [B_MSG_Z_RESET_STATS] = STRINGID_ZMOVERESETSSTATS,
    [B_MSG_Z_ALL_STATS_UP]= STRINGID_ZMOVEALLSTATSUP,
    [B_MSG_Z_BOOST_CRITS] = STRINGID_ZMOVEZBOOSTCRIT,
    [B_MSG_Z_FOLLOW_ME]   = STRINGID_PKMNCENTERATTENTION,
    [B_MSG_Z_RECOVER_HP]  = STRINGID_ZMOVERESTOREHP,
    [B_MSG_Z_STAT_UP]     = STRINGID_ZMOVESTATUP,
    [B_MSG_Z_HP_TRAP]     = STRINGID_ZMOVEHPTRAP,
};

const u16 gMentalHerbCureStringIds[] =
{
    [B_MSG_MENTALHERBCURE_INFATUATION] = STRINGID_ATKGOTOVERINFATUATION,
    [B_MSG_MENTALHERBCURE_TAUNT]       = STRINGID_BUFFERENDS,
    [B_MSG_MENTALHERBCURE_ENCORE]      = STRINGID_PKMNENCOREENDED,
    [B_MSG_MENTALHERBCURE_TORMENT]     = STRINGID_TORMENTEDNOMORE,
    [B_MSG_MENTALHERBCURE_HEALBLOCK]   = STRINGID_HEALBLOCKEDNOMORE,
    [B_MSG_MENTALHERBCURE_DISABLE]     = STRINGID_PKMNMOVEDISABLEDNOMORE,
};

const u16 gStartingStatusStringIds[B_MSG_STARTING_STATUS_COUNT] =
{
    [B_MSG_TERRAIN_SET_MISTY]       = STRINGID_TERRAINBECOMESMISTY,
    [B_MSG_TERRAIN_SET_ELECTRIC]    = STRINGID_TERRAINBECOMESELECTRIC,
    [B_MSG_TERRAIN_SET_PSYCHIC]     = STRINGID_TERRAINBECOMESPSYCHIC,
    [B_MSG_TERRAIN_SET_GRASSY]      = STRINGID_TERRAINBECOMESGRASSY,
    [B_MSG_SET_TRICK_ROOM]          = STRINGID_DIMENSIONSWERETWISTED,
    [B_MSG_SET_MAGIC_ROOM]          = STRINGID_BIZARREARENACREATED,
    [B_MSG_SET_WONDER_ROOM]         = STRINGID_BIZARREAREACREATED,
    [B_MSG_SET_TAILWIND_PLAYER]     = STRINGID_TAILWINDBLEW,
    [B_MSG_SET_TAILWIND_OPPONENT]   = STRINGID_TAILWINDBLEW,
};

const u16 gTerrainStringIds[B_MSG_TERRAIN_COUNT] =
{
    [B_MSG_TERRAIN_SET_MISTY] = STRINGID_TERRAINBECOMESMISTY,
    [B_MSG_TERRAIN_SET_ELECTRIC] = STRINGID_TERRAINBECOMESELECTRIC,
    [B_MSG_TERRAIN_SET_PSYCHIC] = STRINGID_TERRAINBECOMESPSYCHIC,
    [B_MSG_TERRAIN_SET_GRASSY] = STRINGID_TERRAINBECOMESGRASSY,
    [B_MSG_TERRAIN_END_MISTY] = STRINGID_MISTYTERRAINENDS,
    [B_MSG_TERRAIN_END_ELECTRIC] = STRINGID_ELECTRICTERRAINENDS,
    [B_MSG_TERRAIN_END_PSYCHIC] = STRINGID_PSYCHICTERRAINENDS,
    [B_MSG_TERRAIN_END_GRASSY] = STRINGID_GRASSYTERRAINENDS,
};

const u16 gTerrainPreventsStringIds[] =
{
    [B_MSG_TERRAINPREVENTS_MISTY]    = STRINGID_MISTYTERRAINPREVENTS,
    [B_MSG_TERRAINPREVENTS_ELECTRIC] = STRINGID_ELECTRICTERRAINPREVENTS,
    [B_MSG_TERRAINPREVENTS_PSYCHIC]  = STRINGID_PSYCHICTERRAINPREVENTS
};

const u16 gHealingWishStringIds[] =
{
    STRINGID_HEALINGWISHCAMETRUE, STRINGID_LUNARDANCECAMETRUE
};

const u16 gDmgHazardsStringIds[] =
{
    [B_MSG_PKMNHURTBYSPIKES]   = STRINGID_PKMNHURTBYSPIKES,
    [B_MSG_STEALTHROCKDMG]     = STRINGID_STEALTHROCKDMG,
    [B_MSG_SHARPSTEELDMG]      = STRINGID_SHARPSTEELDMG,
    [B_MSG_POINTEDSTONESFLOAT] = STRINGID_POINTEDSTONESFLOAT,
    [B_MSG_SPIKESSCATTERED]    = STRINGID_SPIKESSCATTERED,
    [B_MSG_SHARPSTEELFLOATS]   = STRINGID_SHARPSTEELFLOATS,
};

const u16 gSwitchInAbilityStringIds[] =
{
    [B_MSG_SWITCHIN_MOLDBREAKER] = STRINGID_MOLDBREAKERENTERS,
    [B_MSG_SWITCHIN_TERAVOLT] = STRINGID_TERAVOLTENTERS,
    [B_MSG_SWITCHIN_TURBOBLAZE] = STRINGID_TURBOBLAZEENTERS,
    [B_MSG_SWITCHIN_SLOWSTART] = STRINGID_SLOWSTARTENTERS,
    [B_MSG_SWITCHIN_UNNERVE] = STRINGID_UNNERVEENTERS,
    [B_MSG_SWITCHIN_ANTICIPATION] = STRINGID_ANTICIPATIONACTIVATES,
    [B_MSG_SWITCHIN_FOREWARN] = STRINGID_FOREWARNACTIVATES,
    [B_MSG_SWITCHIN_PRESSURE] = STRINGID_PRESSUREENTERS,
    [B_MSG_SWITCHIN_DARKAURA] = STRINGID_DARKAURAENTERS,
    [B_MSG_SWITCHIN_GENERADOR] = STRINGID_GENERADOR,
    [B_MSG_SWITCHIN_FAIRYAURA] = STRINGID_FAIRYAURAENTERS,
    [B_MSG_SWITCHIN_AURABREAK] = STRINGID_AURABREAKENTERS,
    [B_MSG_SWITCHIN_COMATOSE] = STRINGID_COMATOSEENTERS,
    [B_MSG_SWITCHIN_SCREENCLEANER] = STRINGID_SCREENCLEANERENTERS,
    [B_MSG_SWITCHIN_ASONE] = STRINGID_ASONEENTERS,
    [B_MSG_SWITCHIN_CURIOUS_MEDICINE] = STRINGID_CURIOUSMEDICINEENTERS,
    [B_MSG_SWITCHIN_PASTEL_VEIL] = STRINGID_PASTELVEILENTERS,
    [B_MSG_SWITCHIN_NEUTRALIZING_GAS] = STRINGID_NEUTRALIZINGGASENTERS,
};

const u16 gMissStringIds[] =
{
    [B_MSG_MISSED]      = STRINGID_ATTACKMISSED,
    [B_MSG_PROTECTED]   = STRINGID_PKMNPROTECTEDITSELF,
    [B_MSG_AVOIDED_ATK] = STRINGID_PKMNAVOIDEDATTACK,
    [B_MSG_AVOIDED_DMG] = STRINGID_AVOIDEDDAMAGE,
    [B_MSG_GROUND_MISS] = STRINGID_PKMNMAKESGROUNDMISS
};

const u16 gNoEscapeStringIds[] =
{
    [B_MSG_CANT_ESCAPE]          = STRINGID_CANTESCAPE,
    [B_MSG_DONT_LEAVE_BIRCH]     = STRINGID_DONTLEAVEBIRCH,
    [B_MSG_PREVENTS_ESCAPE]      = STRINGID_PREVENTSESCAPE,
    [B_MSG_CANT_ESCAPE_2]        = STRINGID_CANTESCAPE2,
    [B_MSG_ATTACKER_CANT_ESCAPE] = STRINGID_ATTACKERCANTESCAPE
};

const u16 gMoveWeatherChangeStringIds[] =
{
    [B_MSG_STARTED_RAIN]      = STRINGID_STARTEDTORAIN,
    [B_MSG_STARTED_DOWNPOUR]  = STRINGID_DOWNPOURSTARTED, // Unused
    [B_MSG_WEATHER_FAILED]    = STRINGID_BUTITFAILED,
    [B_MSG_STARTED_SANDSTORM] = STRINGID_SANDSTORMBREWED,
    [B_MSG_STARTED_SUNLIGHT]  = STRINGID_SUNLIGHTGOTBRIGHT,
    [B_MSG_STARTED_HAIL]      = STRINGID_STARTEDHAIL,
    [B_MSG_STARTED_SNOW]      = STRINGID_STARTEDSNOW,
    [B_MSG_STARTED_FOG]       = STRINGID_FOGCREPTUP, // Unused, can use for custom moves that set fog
};

const u16 gWeatherEndsStringIds[B_MSG_WEATHER_END_COUNT] =
{
    [B_MSG_WEATHER_END_RAIN] = STRINGID_RAINSTOPPED,
    [B_MSG_WEATHER_END_SANDSTORM] = STRINGID_SANDSTORMSUBSIDED,
    [B_MSG_WEATHER_END_SUN] = STRINGID_SUNLIGHTFADED,
    [B_MSG_WEATHER_END_HAIL] = STRINGID_HAILSTOPPED,
    [B_MSG_WEATHER_END_STRONG_WINDS] = STRINGID_STRONGWINDSDISSIPATED,
    [B_MSG_WEATHER_END_SNOW] = STRINGID_SNOWSTOPPED,
    [B_MSG_WEATHER_END_FOG] = STRINGID_FOGLIFTED,
};

const u16 gSandStormHailSnowContinuesStringIds[] =
{
    [B_MSG_SANDSTORM] = STRINGID_SANDSTORMRAGES,
    [B_MSG_HAIL]      = STRINGID_HAILCONTINUES,
    [B_MSG_SNOW]      = STRINGID_SNOWCONTINUES,
};

const u16 gSandStormHailDmgStringIds[] =
{
    [B_MSG_SANDSTORM] = STRINGID_PKMNBUFFETEDBYSANDSTORM,
    [B_MSG_HAIL]      = STRINGID_PKMNPELTEDBYHAIL
};

const u16 gSandStormHailSnowEndStringIds[] =
{
    [B_MSG_SANDSTORM] = STRINGID_SANDSTORMSUBSIDED,
    [B_MSG_HAIL]      = STRINGID_HAILSTOPPED,
    [B_MSG_SNOW]      = STRINGID_SNOWSTOPPED,
};

const u16 gRainContinuesStringIds[] =
{
    [B_MSG_RAIN_CONTINUES]     = STRINGID_RAINCONTINUES,
    [B_MSG_DOWNPOUR_CONTINUES] = STRINGID_DOWNPOURCONTINUES,
    [B_MSG_RAIN_STOPPED]       = STRINGID_RAINSTOPPED
};

const u16 gProtectLikeUsedStringIds[] =
{
    [B_MSG_PROTECTED_ITSELF] = STRINGID_PKMNPROTECTEDITSELF2,
    [B_MSG_BRACED_ITSELF]    = STRINGID_PKMNBRACEDITSELF,
    [B_MSG_PROTECT_FAILED]   = STRINGID_BUTITFAILED,
    [B_MSG_PROTECTED_TEAM]   = STRINGID_PROTECTEDTEAM,
};

const u16 gReflectLightScreenSafeguardStringIds[] =
{
    [B_MSG_SIDE_STATUS_FAILED]     = STRINGID_BUTITFAILED,
    [B_MSG_SET_REFLECT_SINGLE]     = STRINGID_PKMNRAISEDDEF,
    [B_MSG_SET_REFLECT_DOUBLE]     = STRINGID_PKMNRAISEDDEF,
    [B_MSG_SET_LIGHTSCREEN_SINGLE] = STRINGID_PKMNRAISEDSPDEF,
    [B_MSG_SET_LIGHTSCREEN_DOUBLE] = STRINGID_PKMNRAISEDSPDEF,
    [B_MSG_SET_SAFEGUARD]          = STRINGID_PKMNCOVEREDBYVEIL,
};

const u16 gLeechSeedStringIds[] =
{
    [B_MSG_LEECH_SEED_SET]   = STRINGID_PKMNSEEDED,
    [B_MSG_LEECH_SEED_MISS]  = STRINGID_PKMNEVADEDATTACK,
    [B_MSG_LEECH_SEED_FAIL]  = STRINGID_ITDOESNTAFFECT,
    [B_MSG_LEECH_SEED_DRAIN] = STRINGID_PKMNSAPPEDBYLEECHSEED,
    [B_MSG_LEECH_SEED_OOZE]  = STRINGID_ITSUCKEDLIQUIDOOZE,
};

const u16 gRestUsedStringIds[] =
{
    [B_MSG_REST]          = STRINGID_PKMNWENTTOSLEEP,
    [B_MSG_REST_STATUSED] = STRINGID_PKMNSLEPTHEALTHY
};

const u16 gUproarOverTurnStringIds[] =
{
    [B_MSG_UPROAR_CONTINUES] = STRINGID_PKMNMAKINGUPROAR,
    [B_MSG_UPROAR_ENDS]      = STRINGID_PKMNCALMEDDOWN
};

const u16 gStockpileUsedStringIds[] =
{
    [B_MSG_STOCKPILED]     = STRINGID_PKMNSTOCKPILED,
    [B_MSG_CANT_STOCKPILE] = STRINGID_PKMNCANTSTOCKPILE,
};

const u16 gWokeUpStringIds[] =
{
    [B_MSG_WOKE_UP]        = STRINGID_PKMNWOKEUP,
    [B_MSG_WOKE_UP_UPROAR] = STRINGID_PKMNWOKEUPINUPROAR
};

const u16 gSwallowFailStringIds[] =
{
    [B_MSG_SWALLOW_FAILED]  = STRINGID_FAILEDTOSWALLOW,
    [B_MSG_SWALLOW_FULL_HP] = STRINGID_PKMNHPFULL
};

const u16 gUproarAwakeStringIds[] =
{
    [B_MSG_CANT_SLEEP_UPROAR]  = STRINGID_PKMNCANTSLEEPINUPROAR2,
    [B_MSG_UPROAR_KEPT_AWAKE]  = STRINGID_UPROARKEPTPKMNAWAKE,
};

const u16 gStatUpStringIds[] =
{
    [B_MSG_ATTACKER_STAT_ROSE] = STRINGID_ATTACKERSSTATROSE,
    [B_MSG_DEFENDER_STAT_ROSE] = STRINGID_DEFENDERSSTATROSE,
    [B_MSG_STAT_WONT_INCREASE] = STRINGID_STATSWONTINCREASE,
    [B_MSG_STAT_ROSE_EMPTY]    = STRINGID_EMPTYSTRING3,
    [B_MSG_STAT_ROSE_ITEM]     = STRINGID_USINGITEMSTATOFPKMNROSE,
    [B_MSG_USED_DIRE_HIT]      = STRINGID_PKMNUSEDXTOGETPUMPED,
};

const u16 gStatDownStringIds[] =
{
    [B_MSG_ATTACKER_STAT_FELL] = STRINGID_ATTACKERSSTATFELL,
    [B_MSG_DEFENDER_STAT_FELL] = STRINGID_DEFENDERSSTATFELL,
    [B_MSG_STAT_WONT_DECREASE] = STRINGID_STATSWONTDECREASE,
    [B_MSG_STAT_FELL_EMPTY]    = STRINGID_EMPTYSTRING3,
};

// Index copied from move's index in sTrappingMoves
const u16 gWrappedStringIds[NUM_TRAPPING_MOVES] =
{
    [B_MSG_WRAPPED_BIND]        = STRINGID_PKMNSQUEEZEDBYBIND,     // MOVE_BIND
    [B_MSG_WRAPPED_WRAP]        = STRINGID_PKMNWRAPPEDBY,          // MOVE_WRAP
    [B_MSG_WRAPPED_FIRE_SPIN]   = STRINGID_PKMNTRAPPEDINVORTEX,    // MOVE_FIRE_SPIN
    [B_MSG_WRAPPED_CLAMP]       = STRINGID_PKMNCLAMPED,            // MOVE_CLAMP
    [B_MSG_WRAPPED_WHIRLPOOL]   = STRINGID_PKMNTRAPPEDINVORTEX,    // MOVE_WHIRLPOOL
    [B_MSG_WRAPPED_SAND_TOMB]   = STRINGID_PKMNTRAPPEDBYSANDTOMB,  // MOVE_SAND_TOMB
    [B_MSG_WRAPPED_MAGMA_STORM] = STRINGID_TRAPPEDBYSWIRLINGMAGMA, // MOVE_MAGMA_STORM
    [B_MSG_WRAPPED_INFESTATION] = STRINGID_INFESTATION,            // MOVE_INFESTATION
    [B_MSG_WRAPPED_SNAP_TRAP]   = STRINGID_PKMNINSNAPTRAP,         // MOVE_SNAP_TRAP
    [B_MSG_WRAPPED_THUNDER_CAGE]= STRINGID_THUNDERCAGETRAPPED,     // MOVE_THUNDER_CAGE
};

const u16 gMistUsedStringIds[] =
{
    [B_MSG_SET_MIST]    = STRINGID_PKMNSHROUDEDINMIST,
    [B_MSG_MIST_FAILED] = STRINGID_BUTITFAILED
};

const u16 gFocusEnergyUsedStringIds[] =
{
    [B_MSG_GETTING_PUMPED]      = STRINGID_PKMNGETTINGPUMPED,
    [B_MSG_FOCUS_ENERGY_FAILED] = STRINGID_BUTITFAILED
};

const u16 gTransformUsedStringIds[] =
{
    [B_MSG_TRANSFORMED]      = STRINGID_PKMNTRANSFORMEDINTO,
    [B_MSG_TRANSFORM_FAILED] = STRINGID_BUTITFAILED
};

const u16 gSubstituteUsedStringIds[] =
{
    [B_MSG_SET_SUBSTITUTE]    = STRINGID_PKMNMADESUBSTITUTE,
    [B_MSG_SUBSTITUTE_FAILED] = STRINGID_TOOWEAKFORSUBSTITUTE
};

const u16 gGotPoisonedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASPOISONED,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNPOISONEDBY
};

const u16 gGotParalyzedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASPARALYZED,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNWASPARALYZEDBY
};

const u16 gFellAsleepStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNFELLASLEEP,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNMADESLEEP,
};

const u16 gGotBurnedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASBURNED,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNBURNEDBY
};

const u16 gGotFrostbiteStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNGOTFROSTBITE
};

const u16 gFrostbiteHealedStringIds[] =
{
    [B_MSG_FROSTBITE_HEALED]         = STRINGID_PKMNFROSTBITEHEALED2,
    [B_MSG_FROSTBITE_HEALED_BY_MOVE] = STRINGID_PKMNFROSTBITEHEALEDBY
};

const u16 gGotFrozenStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNWASFROZEN,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNFROZENBY
};

const u16 gGotDefrostedStringIds[] =
{
    [B_MSG_DEFROSTED]         = STRINGID_PKMNWASDEFROSTED2,
    [B_MSG_DEFROSTED_BY_MOVE] = STRINGID_PKMNWASDEFROSTEDBY
};

const u16 gKOFailedStringIds[] =
{
    [B_MSG_KO_MISS]       = STRINGID_ATTACKMISSED,
    [B_MSG_KO_UNAFFECTED] = STRINGID_PKMNUNAFFECTED
};

const u16 gAttractUsedStringIds[] =
{
    [B_MSG_STATUSED]            = STRINGID_PKMNFELLINLOVE,
    [B_MSG_STATUSED_BY_ABILITY] = STRINGID_PKMNSXINFATUATEDY
};

const u16 gAbsorbDrainStringIds[] =
{
    [B_MSG_ABSORB]      = STRINGID_PKMNENERGYDRAINED,
    [B_MSG_ABSORB_OOZE] = STRINGID_ITSUCKEDLIQUIDOOZE
};

const u16 gSportsUsedStringIds[] =
{
    [B_MSG_WEAKEN_ELECTRIC] = STRINGID_ELECTRICITYWEAKENED,
    [B_MSG_WEAKEN_FIRE]     = STRINGID_FIREWEAKENED
};

const u16 gPartyStatusHealStringIds[] =
{
    [B_MSG_BELL]                     = STRINGID_BELLCHIMED,
    [B_MSG_BELL_SOUNDPROOF_ATTACKER] = STRINGID_BELLCHIMED,
    [B_MSG_BELL_SOUNDPROOF_PARTNER]  = STRINGID_BELLCHIMED,
    [B_MSG_BELL_BOTH_SOUNDPROOF]     = STRINGID_BELLCHIMED,
    [B_MSG_SOOTHING_AROMA]           = STRINGID_SOOTHINGAROMA
};

const u16 gFutureMoveUsedStringIds[] =
{
    [B_MSG_FUTURE_SIGHT] = STRINGID_PKMNFORESAWATTACK,
    [B_MSG_DOOM_DESIRE]  = STRINGID_PKMNCHOSEXASDESTINY
};

const u16 gBallEscapeStringIds[] =
{
    [BALL_NO_SHAKES]     = STRINGID_PKMNBROKEFREE,
    [BALL_1_SHAKE]       = STRINGID_ITAPPEAREDCAUGHT,
    [BALL_2_SHAKES]      = STRINGID_AARGHALMOSTHADIT,
    [BALL_3_SHAKES_FAIL] = STRINGID_SHOOTSOCLOSE
};

// Overworld weathers that don't have an associated battle weather default to "It is raining."
const u16 gWeatherStartsStringIds[] =
{
    [WEATHER_NONE]               = STRINGID_ITISRAINING,
    [WEATHER_SUNNY_CLOUDS]       = STRINGID_ITISRAINING,
    [WEATHER_SUNNY]              = STRINGID_ITISRAINING,
    [WEATHER_RAIN]               = STRINGID_ITISRAINING,
    [WEATHER_SNOW]               = STRINGID_ITISSNOWING,
    [WEATHER_RAIN_THUNDERSTORM]  = STRINGID_ITISRAINING,
    [WEATHER_FOG_HORIZONTAL]     = STRINGID_FOGISDEEP,
    [WEATHER_VOLCANIC_ASH]       = STRINGID_ITISRAINING,
    [WEATHER_SANDSTORM]          = STRINGID_SANDSTORMISRAGING,
    [WEATHER_FOG_DIAGONAL]       = STRINGID_FOGISDEEP,
    [WEATHER_UNDERWATER]         = STRINGID_ITISRAINING,
    [WEATHER_SHADE]              = STRINGID_ITISRAINING,
    [WEATHER_DROUGHT]            = STRINGID_HACEMUCHOCALOR,
    [WEATHER_DOWNPOUR]           = STRINGID_ITISRAINING,
    [WEATHER_UNDERWATER_BUBBLES] = STRINGID_ITISRAINING,
    [WEATHER_ABNORMAL]           = STRINGID_ITISRAINING
};

const u16 gPrimalWeatherBlocksStringIds[] =
{
    [B_MSG_PRIMAL_WEATHER_FIZZLED_BY_RAIN]      = STRINGID_MOVEFIZZLEDOUTINTHEHEAVYRAIN,
    [B_MSG_PRIMAL_WEATHER_EVAPORATED_IN_SUN]    = STRINGID_MOVEEVAPORATEDINTHEHARSHSUNLIGHT,
};

const u16 gInobedientStringIds[] =
{
    [B_MSG_LOAFING]            = STRINGID_PKMNLOAFING,
    [B_MSG_WONT_OBEY]          = STRINGID_PKMNWONTOBEY,
    [B_MSG_TURNED_AWAY]        = STRINGID_PKMNTURNEDAWAY,
    [B_MSG_PRETEND_NOT_NOTICE] = STRINGID_PKMNPRETENDNOTNOTICE,
    [B_MSG_INCAPABLE_OF_POWER] = STRINGID_PKMNINCAPABLEOFPOWER
};

const u16 gSafariGetNearStringIds[] =
{
    [B_MSG_CREPT_CLOSER]    = STRINGID_CREPTCLOSER,
    [B_MSG_CANT_GET_CLOSER] = STRINGID_CANTGETCLOSER
};

const u16 gSafariPokeblockResultStringIds[] =
{
    [B_MSG_MON_CURIOUS]    = STRINGID_PKMNCURIOUSABOUTX,
    [B_MSG_MON_ENTHRALLED] = STRINGID_PKMNENTHRALLEDBYX,
    [B_MSG_MON_IGNORED]    = STRINGID_PKMNIGNOREDX
};

const u16 gBerryEffectStringIds[] =
{
    [B_MSG_CURED_PROBLEM]     = STRINGID_PKMNSITEMCUREDPROBLEM,
    [B_MSG_NORMALIZED_STATUS] = STRINGID_PKMNSITEMNORMALIZEDSTATUS
};

const u16 gBRNPreventionStringIds[] =
{
    [B_MSG_ABILITY_PREVENTS_MOVE_STATUS]    = STRINGID_PKMNSXPREVENTSBURNS,
    [B_MSG_ABILITY_PREVENTS_ABILITY_STATUS] = STRINGID_PKMNSXPREVENTSYSZ,
    [B_MSG_STATUS_HAD_NO_EFFECT]            = STRINGID_PKMNSXHADNOEFFECTONY
};

const u16 gPRLZPreventionStringIds[] =
{
    [B_MSG_ABILITY_PREVENTS_MOVE_STATUS]    = STRINGID_PKMNPREVENTSPARALYSISWITH,
    [B_MSG_ABILITY_PREVENTS_ABILITY_STATUS] = STRINGID_PKMNSXPREVENTSYSZ,
    [B_MSG_STATUS_HAD_NO_EFFECT]            = STRINGID_PKMNSXHADNOEFFECTONY
};

const u16 gPSNPreventionStringIds[] =
{
    [B_MSG_ABILITY_PREVENTS_MOVE_STATUS]    = STRINGID_PKMNPREVENTSPOISONINGWITH,
    [B_MSG_ABILITY_PREVENTS_ABILITY_STATUS] = STRINGID_PKMNSXPREVENTSYSZ,
    [B_MSG_STATUS_HAD_NO_EFFECT]            = STRINGID_PKMNSXHADNOEFFECTONY
};

const u16 gItemSwapStringIds[] =
{
    [B_MSG_ITEM_SWAP_TAKEN] = STRINGID_PKMNOBTAINEDX,
    [B_MSG_ITEM_SWAP_GIVEN] = STRINGID_PKMNOBTAINEDX2,
    [B_MSG_ITEM_SWAP_BOTH]  = STRINGID_PKMNOBTAINEDXYOBTAINEDZ
};

const u16 gFlashFireStringIds[] =
{
    [B_MSG_FLASH_FIRE_BOOST]    = STRINGID_PKMNRAISEDFIREPOWERWITH,
    [B_MSG_FLASH_FIRE_NO_BOOST] = STRINGID_PKMNSXMADEYINEFFECTIVE
};

const u16 gCaughtMonStringIds[] =
{
    [B_MSG_SENT_SOMEONES_PC]  = STRINGID_PKMNTRANSFERREDSOMEONESPC,
};

const u16 gRoomsStringIds[] =
{
    STRINGID_PKMNTWISTEDDIMENSIONS, STRINGID_TRICKROOMENDS,
    STRINGID_SWAPSDEFANDSPDEFOFALLPOKEMON, STRINGID_WONDERROOMENDS,
    STRINGID_HELDITEMSLOSEEFFECTS, STRINGID_MAGICROOMENDS,
    STRINGID_EMPTYSTRING3
};

const u16 gStatusConditionsStringIds[] =
{
    STRINGID_PKMNWASPOISONED, STRINGID_PKMNBADLYPOISONED, STRINGID_PKMNWASBURNED, STRINGID_PKMNWASPARALYZED, STRINGID_PKMNFELLASLEEP, STRINGID_PKMNGOTFROSTBITE
};

const u16 gStatus2StringIds[] =
{
    STRINGID_PKMNWASCONFUSED, STRINGID_PKMNFELLINLOVE, STRINGID_TARGETCANTESCAPENOW, STRINGID_PKMNSUBJECTEDTOTORMENT
};

const u16 gDamageNonTypesStartStringIds[] =
{
    [B_MSG_TRAPPED_WITH_VINES]  = STRINGID_TEAMTRAPPEDWITHVINES,
    [B_MSG_CAUGHT_IN_VORTEX]    = STRINGID_TEAMCAUGHTINVORTEX,
    [B_MSG_SURROUNDED_BY_FIRE]  = STRINGID_TEAMSURROUNDEDBYFIRE,
    [B_MSG_SURROUNDED_BY_ROCKS] = STRINGID_TEAMSURROUNDEDBYROCKS,
};

const u16 gDamageNonTypesDmgStringIds[] =
{
    [B_MSG_HURT_BY_VINES]        = STRINGID_PKMNHURTBYVINES,
    [B_MSG_HURT_BY_VORTEX]       = STRINGID_PKMNHURTBYVORTEX,
    [B_MSG_BURNING_UP]           = STRINGID_PKMNBURNINGUP,
    [B_MSG_HURT_BY_ROCKS_THROWN] = STRINGID_PKMNHURTBYROCKSTHROWN,
};

const u8 gText_PkmnIsEvolving[] = _("What?\n{STR_VAR_1} is evolving!");
const u8 gText_CongratsPkmnEvolved[] = _("Congratulations! Your {STR_VAR_1}\nevolved into {STR_VAR_2}!{WAIT_SE}\p");
const u8 gText_PkmnStoppedEvolving[] = _("Huh? {STR_VAR_1}\nstopped evolving!\p");
const u8 gText_EllipsisQuestionMark[] = _("……?\p");
const u8 gText_SafariZoneMenu[] = _("BALL{CLEAR_TO 56}{POKEBLOCK}\nGO NEAR{CLEAR_TO 56}RUN");
const u8 gText_MoveInterfacePP[] = _("PP ");
const u8 gText_MoveInterfaceDynamicColors[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW DYNAMIC_COLOR4 DYNAMIC_COLOR5 DYNAMIC_COLOR6}");
const u8 gText_WhichMoveToForget4[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW DYNAMIC_COLOR4 DYNAMIC_COLOR5 DYNAMIC_COLOR6}Which move should\nbe forgotten?");
const u8 gText_BattleYesNoChoice[] = _("{PALETTE 5}{COLOR_HIGHLIGHT_SHADOW DYNAMIC_COLOR4 DYNAMIC_COLOR5 DYNAMIC_COLOR6}Yes\nNo");

const u8 gText_SafariBalls[] = _("{HIGHLIGHT DARK_GRAY}Safari Balls");
const u8 gText_SafariBallLeft[] = _("{HIGHLIGHT DARK_GRAY}Left: $" "{HIGHLIGHT DARK_GRAY}");
const u8 gText_Sleep[] = _("sleep");
const u8 gText_Poison[] = _("poison");
const u8 gText_Burn[] = _("burn");
const u8 gText_Paralysis[] = _("paralysis");
const u8 gText_Ice[] = _("ice");
const u8 gText_Confusion[] = _("confusion");
const u8 gText_Love[] = _("love");
const u8 gText_SpaceAndSpace[] = _(" and ");
const u8 gText_CommaSpace[] = _(", ");
const u8 gText_Space2[] = _(" ");
const u8 gText_LineBreak[] = _("\l");
const u8 gText_NewLine[] = _("\n");
const u8 gText_Are[] = _("are");
const u8 gText_Are2[] = _("are");
const u8 gText_BadEgg[] = _("Bad Egg");
const u8 gText_BattleWallyName[] = _("WALLY");
const u8 gText_Win[] = _("{HIGHLIGHT TRANSPARENT}Win");
const u8 gText_Loss[] = _("{HIGHLIGHT TRANSPARENT}Loss");
const u8 gText_Draw[] = _("{HIGHLIGHT TRANSPARENT}Draw");
static const u8 sText_SpaceIs[] = _(" is");
static const u8 sText_ApostropheS[] = _("'s");
const u8 gText_BattleTourney[] = _("BATTLE TOURNEY");
static const u8 sText_Round1[] = _("Round 1");
static const u8 sText_Round2[] = _("Round 2");
static const u8 sText_Semifinal[] = _("Semifinal");
static const u8 sText_Final[] = _("Final");

const u8 *const gRoundsStringTable[DOME_ROUNDS_COUNT] =
{
    [DOME_ROUND1]    = sText_Round1,
    [DOME_ROUND2]    = sText_Round2,
    [DOME_SEMIFINAL] = sText_Semifinal,
    [DOME_FINAL]     = sText_Final
};

const u8 gText_TheGreatNewHope[] = _("The great new hope!\p");
const u8 gText_WillChampionshipDreamComeTrue[] = _("Will the championship dream come true?!\p");
const u8 gText_AFormerChampion[] = _("A former champion!\p");
const u8 gText_ThePreviousChampion[] = _("The previous champion!\p");
const u8 gText_TheUnbeatenChampion[] = _("The unbeaten champion!\p");
const u8 gText_PlayerMon1Name[] = _("{B_PLAYER_MON1_NAME}");
const u8 gText_Vs[] = _("VS");
const u8 gText_OpponentMon1Name[] = _("{B_OPPONENT_MON1_NAME}");
const u8 gText_Mind[] = _("Mind");
const u8 gText_Skill[] = _("Skill");
const u8 gText_Body[] = _("Body");
const u8 gText_Judgment[] = _("{B_BUFF1}{CLEAR 13}Judgment{CLEAR 13}{B_BUFF2}");
static const u8 sText_TwoTrainersSentPkmn[] = _("{B_TRAINER1_CLASS} {B_TRAINER1_NAME} sent out {B_OPPONENT_MON1_NAME}!\p{B_TRAINER2_CLASS} {B_TRAINER2_NAME} sent out {B_OPPONENT_MON2_NAME}!");
static const u8 sText_Trainer2SentOutPkmn[] = _("{B_TRAINER2_CLASS} {B_TRAINER2_NAME} sent out {B_BUFF1}!");
static const u8 sText_TwoTrainersWantToBattle[] = _("You are challenged by {B_TRAINER1_CLASS} {B_TRAINER1_NAME} and {B_TRAINER2_CLASS} {B_TRAINER2_NAME}!\p");
static const u8 sText_InGamePartnerSentOutZGoN[] = _("{B_PARTNER_CLASS} {B_PARTNER_NAME} sent out {B_PLAYER_MON2_NAME}! Go, {B_PLAYER_MON1_NAME}!");
static const u8 sText_TwoInGameTrainersDefeated[] = _("You defeated {B_TRAINER1_CLASS} {B_TRAINER1_NAME} and {B_TRAINER2_CLASS} {B_TRAINER2_NAME}!\p");
static const u8 sText_Trainer2LoseText[] = _("{B_TRAINER2_LOSE_TEXT}");
static const u8 sText_PkmnIncapableOfPower[] = _("{B_ATK_NAME_WITH_PREFIX} appears incapable of using its power!");
static const u8 sText_GlintAppearsInEye[] = _("A glint appears in {B_SCR_ACTIVE_NAME_WITH_PREFIX}'s eyes!");
static const u8 sText_PkmnGettingIntoPosition[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is getting into position!");
static const u8 sText_PkmnBeganGrowlingDeeply[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} began growling deeply!");
static const u8 sText_PkmnEagerForMore[] = _("{B_SCR_ACTIVE_NAME_WITH_PREFIX} is eager for more!");

const u16 gBattlePalaceFlavorTextTable[] =
{
    [B_MSG_GLINT_IN_EYE]   = STRINGID_GLINTAPPEARSINEYE,
    [B_MSG_GETTING_IN_POS] = STRINGID_PKMNGETTINGINTOPOSITION,
    [B_MSG_GROWL_DEEPLY]   = STRINGID_PKMNBEGANGROWLINGDEEPLY,
    [B_MSG_EAGER_FOR_MORE] = STRINGID_PKMNEAGERFORMORE,
};

static const u8 sText_RefIfNothingIsDecided[] = _("REFEREE: If nothing is decided in 3 turns, we will go to judging!");
static const u8 sText_RefThatsIt[] = _("REFEREE: That's it! We will now go to judging to determine the winner!");
static const u8 sText_RefJudgeMind[] = _("REFEREE: Judging category 1, Mind! The POKéMON showing the most guts!\p");
static const u8 sText_RefJudgeSkill[] = _("REFEREE: Judging category 2, Skill! The POKéMON using moves the best!\p");
static const u8 sText_RefJudgeBody[] = _("REFEREE: Judging category 3, Body! The POKéMON with the most vitality!\p");
static const u8 sText_RefPlayerWon[] = _("REFEREE: Judgment: {B_BUFF1} to {B_BUFF2}! The winner is {B_PLAYER_NAME}'s {B_PLAYER_MON1_NAME}!\p");
static const u8 sText_RefOpponentWon[] = _("REFEREE: Judgment: {B_BUFF1} to {B_BUFF2}! The winner is {B_TRAINER1_NAME}'s {B_OPPONENT_MON1_NAME}!\p");
static const u8 sText_RefDraw[] = _("REFEREE: Judgment: 3 to 3! We have a draw!\p");
static const u8 sText_DefeatedOpponentByReferee[] = _("{B_PLAYER_MON1_NAME} defeated the opponent {B_OPPONENT_MON1_NAME} in a REFEREE's decision!");
static const u8 sText_LostToOpponentByReferee[] = _("{B_PLAYER_MON1_NAME} lost to the opponent {B_OPPONENT_MON1_NAME} in a REFEREE's decision!");
static const u8 sText_TiedOpponentByReferee[] = _("{B_PLAYER_MON1_NAME} tied the opponent {B_OPPONENT_MON1_NAME} in a REFEREE's decision!");
static const u8 sText_RefCommenceBattle[] = _("REFEREE: {B_PLAYER_MON1_NAME} VS {B_OPPONENT_MON1_NAME}! Commence battling!");

const u8 *const gRefereeStringsTable[] =
{
    [B_MSG_REF_NOTHING_IS_DECIDED] = sText_RefIfNothingIsDecided,
    [B_MSG_REF_THATS_IT]           = sText_RefThatsIt,
    [B_MSG_REF_JUDGE_MIND]         = sText_RefJudgeMind,
    [B_MSG_REF_JUDGE_SKILL]        = sText_RefJudgeSkill,
    [B_MSG_REF_JUDGE_BODY]         = sText_RefJudgeBody,
    [B_MSG_REF_PLAYER_WON]         = sText_RefPlayerWon,
    [B_MSG_REF_OPPONENT_WON]       = sText_RefOpponentWon,
    [B_MSG_REF_DRAW]               = sText_RefDraw,
    [B_MSG_REF_COMMENCE_BATTLE]    = sText_RefCommenceBattle,
};

static const u8 sText_QuestionForfeitMatch[] = _("Would you like to forfeit the match and quit now?");
static const u8 sText_ForfeitedMatch[] = _("The match was forfeited.");
static const u8 sText_Trainer1WinText[] = _("{B_TRAINER1_WIN_TEXT}");
static const u8 sText_Trainer2WinText[] = _("{B_TRAINER2_WIN_TEXT}");
static const u8 sText_Trainer1Fled[] = _( "{PLAY_SE SE_FLEE}{B_TRAINER1_CLASS} {B_TRAINER1_NAME} fled!");
static const u8 sText_PlayerLostAgainstTrainer1[] = _("You lost to {B_TRAINER1_CLASS} {B_TRAINER1_NAME}!");
static const u8 sText_PlayerBattledToDrawTrainer1[] = _("You battled to a draw against {B_TRAINER1_CLASS} {B_TRAINER1_NAME}!");
const u8 gText_RecordBattleToPass[] = _("Would you like to record your battle\non your Frontier Pass?");
const u8 gText_BattleRecordedOnPass[] = _("{B_PLAYER_NAME}'s battle result was recorded\non the Frontier Pass.");
static const u8 sText_Your1[] = _("Your");
static const u8 sText_Opposing1[] = _("The opposing");
static const u8 sText_Your2[] = _("your");
static const u8 sText_Opposing2[] = _("the opposing");

static const u8 sText_EmptyStatus[] = _("$$$$$$$");

static const struct BattleWindowText sTextOnWindowsInfo_Normal[] =
{
    [B_WIN_MSG] = {
        .fillValue = PIXEL_FILL(2),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 1,
        .fgColor = 6,
        .bgColor = 2,
        .shadowColor = 4,
    },
    [B_WIN_MOVE_NAME_1] = {
        .fillValue = PIXEL_FILL(2),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 4,
        .speed = 0,
        .fgColor = 4,
        .bgColor = 2,
        .shadowColor = 5,
    },
    [B_WIN_MOVE_NAME_2] = {
        .fillValue = PIXEL_FILL(2),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 4,
        .speed = 0,
        .fgColor = 4,
        .bgColor = 2,
        .shadowColor = 5,
    },
    [B_WIN_MOVE_NAME_3] = {
        .fillValue = PIXEL_FILL(2),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 4,
        .speed = 0,
        .fgColor = 4,
        .bgColor = 2,
        .shadowColor = 5,
    },
    [B_WIN_MOVE_NAME_4] = {
        .fillValue = PIXEL_FILL(2),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 4,
        .speed = 0,
        .fgColor = 4,
        .bgColor = 2,
        .shadowColor = 5,
    },
    [B_WIN_PP_1] = {
        .fillValue = PIXEL_FILL(3),
        .fontId = FONT_NARROW,
        .x = 2,
        .y = 4,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 3,
        .shadowColor = 2,
    },
    [B_WIN_PP_2] = {
        .fillValue = PIXEL_FILL(3),
        .fontId = FONT_NARROW,
        .x = 2,
        .y = 4,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 3,
        .shadowColor = 2,
    },
    [B_WIN_PP_3] = {
        .fillValue = PIXEL_FILL(3),
        .fontId = FONT_NARROW,
        .x = 2,
        .y = 4,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 3,
        .shadowColor = 2,
    },
    [B_WIN_PP_4] = {
        .fillValue = PIXEL_FILL(3),
        .fontId = FONT_NARROW,
        .x = 2,
        .y = 4,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 3,
        .shadowColor = 2,
    },
    [B_WIN_YESNO] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_LEVEL_UP_BOX] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_LEVEL_UP_BANNER] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = 32,
        .y = 1,
        .speed = 0,
        .fgColor = 1,
        .shadowColor = 2,
    },
    [B_WIN_VS_PLAYER] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_OPPONENT] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_1] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_2] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_3] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_MULTI_PLAYER_4] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_VS_OUTCOME_DRAW] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 1,
        .shadowColor = 6,
    },
    [B_WIN_VS_OUTCOME_LEFT] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 1,
        .shadowColor = 6,
    },
    [B_WIN_VS_OUTCOME_RIGHT] = {
        .fillValue = PIXEL_FILL(0x0),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 1,
        .shadowColor = 6,
    },
    [B_WIN_MOVE_DESCRIPTION] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .letterSpacing = 0,
        .lineSpacing = 0,
        .speed = 0,
        .fgColor = TEXT_DYNAMIC_COLOR_4,
        .bgColor = TEXT_DYNAMIC_COLOR_5,
        .shadowColor = TEXT_DYNAMIC_COLOR_6,
    },
};

static const struct BattleWindowText sTextOnWindowsInfo_Arena[] =
{
    [B_WIN_MSG] = {
        .fillValue = PIXEL_FILL(0xF),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 1,
        .fgColor = 1,
        .bgColor = 15,
        .shadowColor = 6,
    },
    [B_WIN_MOVE_NAME_1] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_MOVE_NAME_2] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_MOVE_NAME_3] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_MOVE_NAME_4] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_PP_1] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 12,
        .bgColor = 14,
        .shadowColor = 11,
    },
    [B_WIN_PP_2] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 12,
        .bgColor = 14,
        .shadowColor = 11,
    },
    [B_WIN_PP_3] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 12,
        .bgColor = 14,
        .shadowColor = 11,
    },
    [B_WIN_PP_4] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NARROW,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 12,
        .bgColor = 14,
        .shadowColor = 11,
    },
    [B_WIN_YESNO] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_LEVEL_UP_BOX] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [B_WIN_LEVEL_UP_BANNER] = {
        .fillValue = PIXEL_FILL(0),
        .fontId = FONT_NORMAL,
        .x = 32,
        .y = 1,
        .speed = 0,
        .fgColor = 1,
        .shadowColor = 2,
    },
    [ARENA_WIN_PLAYER_NAME] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 1,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [ARENA_WIN_VS] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [ARENA_WIN_OPPONENT_NAME] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [ARENA_WIN_MIND] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [ARENA_WIN_SKILL] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [ARENA_WIN_BODY] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [ARENA_WIN_JUDGMENT_TITLE] = {
        .fillValue = PIXEL_FILL(0xE),
        .fontId = FONT_NORMAL,
        .x = -1,
        .y = 1,
        .speed = 0,
        .fgColor = 13,
        .bgColor = 14,
        .shadowColor = 15,
    },
    [ARENA_WIN_JUDGMENT_TEXT] = {
        .fillValue = PIXEL_FILL(0x1),
        .fontId = FONT_NORMAL,
        .x = 0,
        .y = 1,
        .speed = 1,
        .fgColor = 1,
        .bgColor = 3,
        .shadowColor = 2,
    },
};

static const struct BattleWindowText *const sBattleTextOnWindowsInfo[] =
{
    [B_WIN_TYPE_NORMAL] = sTextOnWindowsInfo_Normal,
    [B_WIN_TYPE_ARENA]  = sTextOnWindowsInfo_Arena
};

void BufferStringBattle(u16 stringID, u32 battler)
{
    s32 i;
    const u8 *stringPtr = NULL;

    gBattleMsgDataPtr = (struct BattleMsgData *)(&gBattleResources->bufferA[battler][4]);
    gLastUsedItem = gBattleMsgDataPtr->lastItem;
    gLastUsedAbility = gBattleMsgDataPtr->lastAbility;
    gBattleScripting.battler = gBattleMsgDataPtr->scrActive;
    gBattleStruct->scriptPartyIdx = gBattleMsgDataPtr->bakScriptPartyIdx;
    gBattleStruct->hpScale = gBattleMsgDataPtr->hpScale;
    gPotentialItemEffectBattler = gBattleMsgDataPtr->itemEffectBattler;
    gBattleStruct->stringMoveType = gBattleMsgDataPtr->moveType;

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        sBattlerAbilities[i] = gBattleMsgDataPtr->abilities[i];
    }
    for (i = 0; i < TEXT_BUFF_ARRAY_COUNT; i++)
    {
        gBattleTextBuff1[i] = gBattleMsgDataPtr->textBuffs[0][i];
        gBattleTextBuff2[i] = gBattleMsgDataPtr->textBuffs[1][i];
        gBattleTextBuff3[i] = gBattleMsgDataPtr->textBuffs[2][i];
    }

    switch (stringID)
    {
    case TEXTO_BATALLA_INTRO:
        if (EsContraEntrenador())
        {
            stringPtr = sText_EntrenadorQuiereLuchar;
        }
        else
        {
            if (gBattleTypeFlags & TIPO_BATALLA_LEGENDARIO)
                stringPtr = sText_ElLegendario;
            else
                stringPtr = sText_UnPokemonSalvaje;
        }
        break;
    case TEXTO_BATALLA_ENVIAR_POKEMON:
        if (GetBattlerSide(battler) == B_SIDE_PLAYER)
        {
            if (EsContraEntrenador() && IsValidForBattle(&gPlayerParty[gBattlerPartyIndexes[BATTLE_PARTNER(battler)]]))
            {
                stringPtr = sText_GoTwoPkmn;
            }
            else
            {
                stringPtr = sText_GoPkmn;
            }
        }
        else
        {
            if (EsContraEntrenador() && IsValidForBattle(&gEnemyParty[gBattlerPartyIndexes[BATTLE_PARTNER(battler)]]))
            {
                stringPtr = sText_Trainer1SentOutTwoPkmn;
            }
            else
            {
                stringPtr = sText_Trainer1SentOutPkmn;
            }
        }
        break;
    case STRINGID_RETURNMON:
        if (GetBattlerSide(battler) == B_SIDE_PLAYER)
        {
            if (*(&gBattleStruct->hpScale) == 0)
                stringPtr = sText_PkmnThatsEnough;
            else if (*(&gBattleStruct->hpScale) == 1 || EsContraEntrenador())
                stringPtr = sText_PkmnComeBack;
            else if (*(&gBattleStruct->hpScale) == 2)
                stringPtr = sText_PkmnOkComeBack;
            else
                stringPtr = sText_PkmnGoodComeBack;
        }
        else
        {
            stringPtr = sText_Trainer1WithdrewPkmn;
        }
        break;
    case STRINGID_SWITCHINMON:
        if (GetBattlerSide(gBattleScripting.battler) == B_SIDE_PLAYER)
        {
            if (*(&gBattleStruct->hpScale) == 0 || EsContraEntrenador())
                stringPtr = sText_GoPkmn2;
            else if (*(&gBattleStruct->hpScale) == 1)
                stringPtr = sText_DoItPkmn;
            else if (*(&gBattleStruct->hpScale) == 2)
                stringPtr = sText_GoForItPkmn;
            else
                stringPtr = sText_YourFoesWeakGetEmPkmn;
        }
        else
        {
            stringPtr = sText_Trainer1SentOutPkmn2;
        }
        break;
    case STRINGID_USEDMOVE:
        StringCopy(gBattleTextBuff3, GetMoveName(gBattleMsgDataPtr->currentMove));
        stringPtr = sText_AttackerUsedX;
        break;
    case STRINGID_TRAINERSLIDE:
        stringPtr = gBattleStruct->trainerSlideMsg;
        break;
    default:
        if (stringID >= BATTLESTRINGS_COUNT)
        {
            gDisplayedStringBattle[0] = EOS;
            return;
        }
        else
        {
            stringPtr = gBattleStringsTable[stringID];
        }
        break;
    }

    BattleStringExpandPlaceholdersToDisplayedString(stringPtr);
}

u32 BattleStringExpandPlaceholdersToDisplayedString(const u8 *src)
{
#ifndef NDEBUG
    u32 j, strWidth;
    u32 dstID = BattleStringExpandPlaceholders(src, gDisplayedStringBattle, sizeof(gDisplayedStringBattle));
    for (j = 1;; j++)
    {
        strWidth = GetStringLineWidth(0, gDisplayedStringBattle, 0, j, sizeof(gDisplayedStringBattle), TRUE);
        if (strWidth == 0)
            break;
    }
    return dstID;
#else
    return BattleStringExpandPlaceholders(src, gDisplayedStringBattle, sizeof(gDisplayedStringBattle));
#endif
}

static const u8 *TryGetStatusString(u8 *src)
{
    u32 i;
    u8 status[8];
    u32 chars1, chars2;
    u8 *statusPtr;

    memcpy(status, sText_EmptyStatus, min(ARRAY_COUNT(status), ARRAY_COUNT(sText_EmptyStatus)));

    statusPtr = status;
    for (i = 0; i < ARRAY_COUNT(status); i++)
    {
        if (*src == EOS) break; // one line required to match -g
        *statusPtr = *src;
        src++;
        statusPtr++;
    }

    chars1 = *(u32 *)(&status[0]);
    chars2 = *(u32 *)(&status[4]);

    for (i = 0; i < ARRAY_COUNT(gStatusConditionStringsTable); i++)
    {
        if (chars1 == *(u32 *)(&gStatusConditionStringsTable[i][0])
            && chars2 == *(u32 *)(&gStatusConditionStringsTable[i][4]))
            return gStatusConditionStringsTable[i];
    }
    return NULL;
}

static void GetBattlerNick(u32 battler, u8 *dst)
{
    struct Pokemon *illusionMon = GetIllusionMonPtr(battler);
    struct Pokemon *mon = GetPartyBattlerData(battler);

    if (illusionMon != NULL)
        mon = illusionMon;
    GetMonData(mon, MON_DATA_NICKNAME, dst);
    StringGet_Nickname(dst);
}

#define HANDLE_NICKNAME_STRING_CASE(battler)                          \
    GetBattlerNick(battler, text);                                    \
    toCpy = text;                                                     \
        while (*toCpy != EOS)                                         \
        {                                                             \
            dst[dstID] = *toCpy;                                      \
            dstID++;                                                  \
            toCpy++;                                                  \
        }                                                             \
    if (GetBattlerSide(battler) != B_SIDE_PLAYER)                     \
    {                                                                 \
        if (EsContraEntrenador())               \
            toCpy = sText_FoePkmnPrefix;                              \
        else                                                          \
            toCpy = sText_WildPkmnPrefix;                             \
    }

static const u8 *BattleStringGetOpponentNameByTrainerId(u16 trainerId, u8 *text, u8 multiplayerId, u8 battler)
{
    const u8 *toCpy = GetTrainerNameFromId(trainerId);

    return toCpy;
}

static const u8 *BattleStringGetOpponentName(u8 *text, u8 multiplayerId, u8 battler)
{
    const u8 *toCpy = NULL;

    switch (GetBattlerPosition(battler))
    {
    case B_POSITION_OPPONENT_LEFT:
    case B_POSITION_OPPONENT_RIGHT:
        toCpy = BattleStringGetOpponentNameByTrainerId(gTrainerBattleOpponent_A, text, multiplayerId, battler);
        break;
    }

    return toCpy;
}

static const u8 *BattleStringGetTrainerName(u8 *text, u8 multiplayerId, u8 battler)
{
    if (GetBattlerSide(battler) == B_SIDE_PLAYER)
        return gSaveBlockPtr->playerName;
    else
        return BattleStringGetOpponentName(text, multiplayerId, battler);
}

static const u8 *BattleStringGetOpponentClassByTrainerId(u16 trainerId)
{
    const u8 *toCpy = gTrainerClasses[GetTrainerClassFromId(trainerId)].name;

    return toCpy;
}

u32 BattleStringExpandPlaceholders(const u8 *src, u8 *dst, u32 dstSize)
{
    u32 dstID = 0; // if they used dstID, why not use srcID as well?
    const u8 *toCpy = NULL;
    u32 lastValidSkip = 0;
    u32 toCpyWidth = 0;
    u32 dstWidth = 0;
    // This buffer may hold either the name of a trainer, Pokémon, or item.
    u8 text[max(max(max(32, TRAINER_NAME_LENGTH + 1), POKEMON_NAME_LENGTH + 1), ITEM_NAME_LENGTH)];
    u8 multiplayerId;
    u8 fontId = FONT_NORMAL;
    s16 letterSpacing = 0;
    u32 lineNum = 1;

    multiplayerId = GetMultiplayerId();

    // Clear destination first
    while (dstID < dstSize)
    {
        dst[dstID] = EOS;
        dstID++;
    }

    dstID = 0;
    while (*src != EOS)
    {
        toCpy = NULL;
        dstWidth = GetStringLineWidth(fontId, dst, letterSpacing, lineNum, dstSize, FALSE);

        if (*src == PLACEHOLDER_BEGIN)
        {
            src++;
            switch (*src)
            {
            case B_TXT_BUFF1:
                if (gBattleTextBuff1[0] == B_BUFF_PLACEHOLDER_BEGIN)
                {
                    ExpandBattleTextBuffPlaceholders(gBattleTextBuff1, gStringVar1);
                    toCpy = gStringVar1;
                }
                else
                {
                    toCpy = TryGetStatusString(gBattleTextBuff1);
                    if (toCpy == NULL)
                        toCpy = gBattleTextBuff1;
                }
                break;
            case B_TXT_BUFF2:
                if (gBattleTextBuff2[0] == B_BUFF_PLACEHOLDER_BEGIN)
                {
                    ExpandBattleTextBuffPlaceholders(gBattleTextBuff2, gStringVar2);
                    toCpy = gStringVar2;
                }
                else
                    toCpy = gBattleTextBuff2;
                break;
            case B_TXT_BUFF3:
                if (gBattleTextBuff3[0] == B_BUFF_PLACEHOLDER_BEGIN)
                {
                    ExpandBattleTextBuffPlaceholders(gBattleTextBuff3, gStringVar3);
                    toCpy = gStringVar3;
                }
                else
                    toCpy = gBattleTextBuff3;
                break;
            case B_TXT_COPY_VAR_1:
                toCpy = gStringVar1;
                break;
            case B_TXT_COPY_VAR_2:
                toCpy = gStringVar2;
                break;
            case B_TXT_COPY_VAR_3:
                toCpy = gStringVar3;
                break;
            case B_TXT_PLAYER_MON1_NAME: // first player poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT), text);
                toCpy = text;
                break;
            case B_TXT_OPPONENT_MON1_NAME: // first enemy poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), text);
                toCpy = text;
                break;
            case B_TXT_PLAYER_MON2_NAME: // second player poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT), text);
                toCpy = text;
                break;
            case B_TXT_OPPONENT_MON2_NAME: // second enemy poke name
                GetBattlerNick(GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT), text);
                toCpy = text;
                break;
            case B_TXT_ATK_PARTNER_NAME: // attacker partner name
                GetBattlerNick(BATTLE_PARTNER(gBattlerAttacker), text);
                toCpy = text;
                break;
            case B_TXT_ATK_NAME_WITH_PREFIX: // attacker name with prefix
                HANDLE_NICKNAME_STRING_CASE(gBattlerAttacker)
                break;
            case B_TXT_DEF_NAME_WITH_PREFIX: // target name with prefix
                HANDLE_NICKNAME_STRING_CASE(gBattlerTarget)
                break;
            case B_TXT_DEF_NAME: // target name
                GetBattlerNick(gBattlerTarget, text);
                toCpy = text;
                break;
            case B_TXT_EFF_NAME_WITH_PREFIX: // effect battler name with prefix
                HANDLE_NICKNAME_STRING_CASE(gEffectBattler)
                break;
            case B_TXT_SCR_ACTIVE_NAME_WITH_PREFIX: // scripting active battler name with prefix
                HANDLE_NICKNAME_STRING_CASE(gBattleScripting.battler)
                break;
            case B_TXT_CURRENT_MOVE: // current move name
                toCpy = GetMoveName(gBattleMsgDataPtr->currentMove);
                break;
            case B_TXT_LAST_MOVE: // originally used move name
                toCpy = GetMoveName(gBattleMsgDataPtr->originallyUsedMove);
                break;
            case B_TXT_LAST_ITEM: // last used item
                CopyItemName(gLastUsedItem, text);
                toCpy = text;
                break;
            case B_TXT_LAST_ABILITY: // last used ability
                toCpy = gAbilitiesInfo[gLastUsedAbility].name;
                break;
            case B_TXT_ATK_ABILITY: // attacker ability
                toCpy = gAbilitiesInfo[sBattlerAbilities[gBattlerAttacker]].name;
                break;
            case B_TXT_DEF_ABILITY: // target ability
                toCpy = gAbilitiesInfo[sBattlerAbilities[gBattlerTarget]].name;
                break;
            case B_TXT_SCR_ACTIVE_ABILITY: // scripting active ability
                toCpy = gAbilitiesInfo[sBattlerAbilities[gBattleScripting.battler]].name;
                break;
            case B_TXT_EFF_ABILITY: // effect battler ability
                toCpy = gAbilitiesInfo[sBattlerAbilities[gEffectBattler]].name;
                break;
            case B_TXT_TRAINER1_CLASS: // trainer class name
                toCpy = BattleStringGetOpponentClassByTrainerId(gTrainerBattleOpponent_A);
                break;
            case B_TXT_TRAINER1_NAME: // trainer1 name
                toCpy = BattleStringGetOpponentNameByTrainerId(gTrainerBattleOpponent_A, text, multiplayerId, GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT));
                break;
            case B_TXT_PLAYER_NAME: // player name
                toCpy = gSaveBlockPtr->playerName;
                break;
            case B_TXT_TRAINER1_LOSE_TEXT: // trainerA lose text
                toCpy = GetTrainerALoseText();
                break;
            case B_TXT_TRAINER1_WIN_TEXT: // trainerA win text
                break;
            case B_TXT_PC_CREATOR_NAME: // lanette pc
                if (FlagGet(FLAG_SYS_PC_LANETTE))
                    toCpy = sText_Lanettes;
                else
                    toCpy = sText_Someones;
                break;
            case B_TXT_ATK_PREFIX2:
                if (GetBattlerSide(gBattlerAttacker) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix2;
                else
                    toCpy = sText_FoePkmnPrefix3;
                break;
            case B_TXT_DEF_PREFIX2:
                if (GetBattlerSide(gBattlerTarget) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix2;
                else
                    toCpy = sText_FoePkmnPrefix3;
                break;
            case B_TXT_ATK_PREFIX1:
                if (GetBattlerSide(gBattlerAttacker) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix;
                else
                    toCpy = sText_FoePkmnPrefix2;
                break;
            case B_TXT_DEF_PREFIX1:
                if (GetBattlerSide(gBattlerTarget) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix;
                else
                    toCpy = sText_FoePkmnPrefix2;
                break;
            case B_TXT_ATK_PREFIX3:
                if (GetBattlerSide(gBattlerAttacker) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix3;
                else
                    toCpy = sText_FoePkmnPrefix4;
                break;
            case B_TXT_DEF_PREFIX3:
                if (GetBattlerSide(gBattlerTarget) == B_SIDE_PLAYER)
                    toCpy = sText_AllyPkmnPrefix3;
                else
                    toCpy = sText_FoePkmnPrefix4;
                break;
            case B_TXT_TRAINER2_CLASS:
                toCpy = BattleStringGetOpponentClassByTrainerId(gTrainerBattleOpponent_B);
                break;
            case B_TXT_TRAINER2_NAME:
                toCpy = BattleStringGetOpponentNameByTrainerId(gTrainerBattleOpponent_B, text, multiplayerId, GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT));
                break;
            case B_TXT_TRAINER2_LOSE_TEXT:
                toCpy = GetTrainerBLoseText();
                break;
            case B_TXT_TRAINER2_WIN_TEXT:
                break;
            case B_TXT_PARTNER_CLASS:
                toCpy = gTrainerClasses[GetFrontierOpponentClass(gPartnerTrainerId)].name;
                break;
            case B_TXT_PARTNER_NAME:
                toCpy = gSaveBlockPtr->playerName;
                break;
            case B_TXT_ATK_TRAINER_NAME:
                toCpy = BattleStringGetTrainerName(text, multiplayerId, gBattlerAttacker);
                break;
            case B_TXT_ATK_TRAINER_CLASS:
                switch (GetBattlerPosition(gBattlerAttacker))
                {
                case B_POSITION_PLAYER_RIGHT:
                    break;
                case B_POSITION_OPPONENT_LEFT:
                case B_POSITION_OPPONENT_RIGHT:
                    toCpy = BattleStringGetOpponentClassByTrainerId(gTrainerBattleOpponent_A);
                    break;
                }
                break;
            case B_TXT_ATK_TEAM1:
                if (GetBattlerSide(gBattlerAttacker) == B_SIDE_PLAYER)
                    toCpy = sText_Your1;
                else
                    toCpy = sText_Opposing1;
                break;
            case B_TXT_ATK_TEAM2:
                if (GetBattlerSide(gBattlerAttacker) == B_SIDE_PLAYER)
                    toCpy = sText_Your2;
                else
                    toCpy = sText_Opposing2;
                break;
            case B_TXT_DEF_TEAM1:
                if (GetBattlerSide(gBattlerTarget) == B_SIDE_PLAYER)
                    toCpy = sText_Your1;
                else
                    toCpy = sText_Opposing1;
                break;
            case B_TXT_DEF_TEAM2:
                if (GetBattlerSide(gBattlerTarget) == B_SIDE_PLAYER)
                    toCpy = sText_Your2;
                else
                    toCpy = sText_Opposing2;
                break;
            }

            if (toCpy != NULL)
            {
                toCpyWidth = GetStringLineWidth(fontId, toCpy, letterSpacing, 1, dstSize, FALSE);

                if (dstWidth + toCpyWidth > BATTLE_MSG_MAX_WIDTH)
                {
                    dst[lastValidSkip] = lineNum == 1 ? CHAR_NEWLINE : CHAR_PROMPT_SCROLL;
                    dstWidth = GetStringLineWidth(fontId, dst, letterSpacing, lineNum, dstSize, FALSE);
                    lineNum++;
                }
                while (*toCpy != EOS)
                {
                    dst[dstID] = *toCpy;
                    dstID++;
                    toCpy++;
                }
            }

            if (*src == B_TXT_TRAINER1_LOSE_TEXT || *src == B_TXT_TRAINER2_LOSE_TEXT
                || *src == B_TXT_TRAINER1_WIN_TEXT || *src == B_TXT_TRAINER2_WIN_TEXT)
            {
                dst[dstID] = EXT_CTRL_CODE_BEGIN;
                dstID++;
                dst[dstID] = EXT_CTRL_CODE_PAUSE_UNTIL_PRESS;
                dstID++;
            }
        }
        else
        {
            toCpyWidth = GetGlyphWidth(dst[dstID + 1], FALSE, fontId);
            dst[dstID] = *src;
            if (dstWidth + toCpyWidth > BATTLE_MSG_MAX_WIDTH)
            {
                dst[lastValidSkip] = lineNum == 1 ? CHAR_NEWLINE : CHAR_PROMPT_SCROLL;
                lineNum++;
                dstWidth = 0;
            }
            switch (*src)
            {
            case CHAR_NEWLINE:
            case CHAR_PROMPT_SCROLL:
            case CHAR_PROMPT_CLEAR:
                lineNum++;
                dstWidth = 0;
                //fallthrough
            case CHAR_SPACE:
                lastValidSkip = dstID;
                break;
            }
            dstID++;
        }
        src++;
    }

    dst[dstID] = *src;
    dstID++;

    return dstID;
}

static void IllusionNickHack(u32 battler, u32 partyId, u8 *dst)
{
    s32 id, i;
    // we know it's gEnemyParty
    struct Pokemon *mon = &gEnemyParty[partyId], *partnerMon;

    if (GetMonAbility(mon) == ABILITY_ILLUSION)
    {
        if (IsBattlerAlive(BATTLE_PARTNER(battler)))
            partnerMon = &gEnemyParty[gBattlerPartyIndexes[BATTLE_PARTNER(battler)]];
        else
            partnerMon = mon;

        // Find last alive non-egg pokemon.
        for (i = PARTY_SIZE - 1; i >= 0; i--)
        {
            id = i;
            if (GetMonData(&gEnemyParty[id], MON_DATA_SANITY_HAS_SPECIES)
                && GetMonData(&gEnemyParty[id], MON_DATA_HP)
                && &gEnemyParty[id] != mon
                && &gEnemyParty[id] != partnerMon)
            {
                GetMonData(&gEnemyParty[id], MON_DATA_NICKNAME, dst);
                return;
            }
        }
    }

    GetMonData(mon, MON_DATA_NICKNAME, dst);
}

void ExpandBattleTextBuffPlaceholders(const u8 *src, u8 *dst)
{
    u32 srcID = 1;
    u32 value = 0;
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u16 hword;

    *dst = EOS;
    while (src[srcID] != B_BUFF_EOS)
    {
        switch (src[srcID])
        {
        case B_BUFF_STRING: // battle string
            hword = T1_READ_16(&src[srcID + 1]);
            StringAppend(dst, gBattleStringsTable[hword]);
            srcID += 3;
            break;
        case B_BUFF_NUMBER: // int to string
            switch (src[srcID + 1])
            {
            case 1:
                value = src[srcID + 3];
                break;
            case 2:
                value = T1_READ_16(&src[srcID + 3]);
                break;
            case 4:
                value = T1_READ_32(&src[srcID + 3]);
                break;
            }
            ConvertIntToDecimalStringN(dst, value, STR_CONV_MODE_LEFT_ALIGN, src[srcID + 2]);
            srcID += src[srcID + 1] + 3;
            break;
        case B_BUFF_MOVE: // move name
            StringAppend(dst, GetMoveName(T1_READ_16(&src[srcID + 1])));
            srcID += 3;
            break;
        case B_BUFF_TYPE: // type name
            StringAppend(dst, gTypesInfo[src[srcID + 1]].name);
            srcID += 2;
            break;
        case B_BUFF_MON_NICK_WITH_PREFIX: // poke nick with prefix
        case B_BUFF_MON_NICK_WITH_PREFIX_LOWER: // poke nick with lowercase prefix
            if (GetBattlerSide(src[srcID + 1]) == B_SIDE_PLAYER)
            {
                GetMonData(&gPlayerParty[src[srcID + 2]], MON_DATA_NICKNAME, nickname);
            }
            else
            {
                if (EsContraEntrenador())
                    StringAppend(dst, sText_FoePkmnPrefix);
                else
                    StringAppend(dst, sText_WildPkmnPrefix);

                GetMonData(&gEnemyParty[src[srcID + 2]], MON_DATA_NICKNAME, nickname);
            }
            StringGet_Nickname(nickname);
            StringAppend(dst, nickname);
            srcID += 3;
            break;
        case B_BUFF_STAT: // stats
            StringAppend(dst, gStatNamesTable[src[srcID + 1]]);
            srcID += 2;
            break;
        case B_BUFF_SPECIES: // species name
            StringCopy(dst, GetSpeciesName(T1_READ_16(&src[srcID + 1])));
            srcID += 3;
            break;
        case B_BUFF_MON_NICK: // poke nick without prefix
            if (src[srcID + 2] == gBattlerPartyIndexes[src[srcID + 1]])
            {
                GetBattlerNick(src[srcID + 1], dst);
            }
            else if (gBattleScripting.illusionNickHack) // for STRINGID_ENEMYABOUTTOSWITCHPKMN
            {
                gBattleScripting.illusionNickHack = 0;
                IllusionNickHack(src[srcID + 1], src[srcID + 2], dst);
                StringGet_Nickname(dst);
            }
            else
            {
                if (GetBattlerSide(src[srcID + 1]) == B_SIDE_PLAYER)
                    GetMonData(&gPlayerParty[src[srcID + 2]], MON_DATA_NICKNAME, dst);
                else
                    GetMonData(&gEnemyParty[src[srcID + 2]], MON_DATA_NICKNAME, dst);
                StringGet_Nickname(dst);
            }
            srcID += 3;
            break;
        case B_BUFF_NEGATIVE_FLAVOR: // flavor table
            StringAppend(dst, gPokeblockWasTooXStringTable[src[srcID + 1]]);
            srcID += 2;
            break;
        case B_BUFF_ABILITY: // ability names
            StringAppend(dst, gAbilitiesInfo[T1_READ_16(&src[srcID + 1])].name);
            srcID += 3;
            break;
        case B_BUFF_ITEM: // item name
            hword = T1_READ_16(&src[srcID + 1]);
            CopyItemName(hword, dst);
            srcID += 3;
            break;
        }
    }
}

void BattlePutTextOnWindow(const u8 *text, u8 windowId)
{
    const struct BattleWindowText *textInfo = sBattleTextOnWindowsInfo[gBattleScripting.windowsType];
    bool32 copyToVram;
    struct TextPrinterTemplate printerTemplate;
    u8 speed;

    if (windowId & B_WIN_COPYTOVRAM)
    {
        windowId &= ~B_WIN_COPYTOVRAM;
        copyToVram = FALSE;
    }
    else
    {
        FillWindowPixelBuffer(windowId, textInfo[windowId].fillValue);
        copyToVram = TRUE;
    }

    printerTemplate.currentChar = text;
    printerTemplate.windowId = windowId;
    printerTemplate.fontId = textInfo[windowId].fontId;
    printerTemplate.x = textInfo[windowId].x;
    printerTemplate.y = textInfo[windowId].y;
    printerTemplate.currentX = printerTemplate.x;
    printerTemplate.currentY = printerTemplate.y;
    printerTemplate.letterSpacing = textInfo[windowId].letterSpacing;
    printerTemplate.lineSpacing = textInfo[windowId].lineSpacing;
    printerTemplate.fgColor = textInfo[windowId].fgColor;
    printerTemplate.bgColor = textInfo[windowId].bgColor;
    printerTemplate.shadowColor = textInfo[windowId].shadowColor;

    if (B_WIN_MOVE_NAME_1 <= windowId && windowId <= B_WIN_MOVE_NAME_4)
    {
        // We cannot check the actual width of the window because
        // B_WIN_MOVE_NAME_1 and B_WIN_MOVE_NAME_3 are 16 wide for
        // Z-move details.
        if (gBattleStruct->zmove.viewing && windowId == B_WIN_MOVE_NAME_1)
            printerTemplate.fontId = GetFontIdToFit(text, printerTemplate.fontId, printerTemplate.letterSpacing, 16 * TILE_WIDTH);
        else
            printerTemplate.fontId = GetFontIdToFit(text, printerTemplate.fontId, printerTemplate.letterSpacing, 8 * TILE_WIDTH);
    }

    if (printerTemplate.x == 0xFF)
    {
        u32 width = GetBattleWindowTemplatePixelWidth(gBattleScripting.windowsType, windowId);
        s32 alignX = GetStringCenterAlignXOffsetWithLetterSpacing(printerTemplate.fontId, printerTemplate.currentChar, width, printerTemplate.letterSpacing);
        printerTemplate.x = printerTemplate.currentX = alignX;
    }

    gTextFlags.useAlternateDownArrow = TRUE;
    gTextFlags.autoScroll = FALSE;

    if (windowId == B_WIN_MSG)
    {
        speed = GetPlayerTextSpeedDelay();
        gTextFlags.canABSpeedUpPrint = 1;
    }
    else
    {
        speed = textInfo[windowId].speed;
        gTextFlags.canABSpeedUpPrint = 0;
    }

    AddTextPrinter(&printerTemplate, speed, NULL);

    if (copyToVram)
    {
        PutWindowTilemap(windowId);
        CopyWindowToVram(windowId, COPYWIN_FULL);
    }
}

u8 GetCurrentPpToMaxPpState(u8 currentPp, u8 maxPp)
{
    if (maxPp == currentPp)
    {
        return 3;
    }
    else if (maxPp <= 2)
    {
        if (currentPp > 1)
            return 3;
        else
            return 2 - currentPp;
    }
    else if (maxPp <= 7)
    {
        if (currentPp > 2)
            return 3;
        else
            return 2 - currentPp;
    }
    else
    {
        if (currentPp == 0)
            return 2;
        if (currentPp <= maxPp / 4)
            return 1;
        if (currentPp > maxPp / 2)
            return 3;
    }

    return 0;
}

struct TrainerSlide
{
    u16 trainerId;
    bool8 isFrontierTrainer;
    const u8 *msgLastSwitchIn;
    const u8 *msgLastLowHp;
    const u8 *msgFirstDown;
    const u8 *msgLastHalfHp;
    const u8 *msgFirstCriticalHit;
    const u8 *msgFirstSuperEffectiveHit;
    const u8 *msgFirstSTABMove;
    const u8 *msgPlayerMonUnaffected;
    const u8 *msgMegaEvolution;
    const u8 *msgZMove;
    const u8 *msgBeforeFirstTurn;
    const u8 *msgDynamax;
};

static const struct TrainerSlide sTrainerSlides[] =
{
    /* Put any trainer slide-in messages inside this array.
    Example:
    {
        .trainerId = TRAINER_WALLY_VR_2,
        .isFrontierTrainer = FALSE,
        .msgLastSwitchIn = sText_AarghAlmostHadIt,
        .msgLastLowHp = sText_BoxIsFull,
        .msgFirstDown = sText_123Poof,
        .msgLastHalfHp = sText_ShootSoClose,
        .msgFirstCriticalHit = sText_CriticalHit,
        .msgFirstSuperEffectiveHit = sText_SuperEffective,
        .msgFirstSTABMove = sText_ABoosted,
        .msgPlayerMonUnaffected = sText_ButNoEffect,
        .msgMegaEvolution = sText_PowderExplodes,
        .msgZMove = sText_Electromagnetism,
        .msgBeforeFirstTurn = sText_GravityIntensified,
        .msgDynamax = sText_TargetWokeUp,
    },
    */
};

static u32 GetEnemyMonCount(u32 firstId, u32 lastId, bool32 onlyAlive)
{
    u32 i, count = 0;

    for (i = firstId; i < lastId; i++)
    {
        u32 species = GetMonData(&gEnemyParty[i], MON_DATA_SPECIES_OR_EGG, NULL);
        if (species != SPECIES_NONE
            && species != SPECIES_EGG
            && (!onlyAlive || GetMonData(&gEnemyParty[i], MON_DATA_HP, NULL)))
            count++;
    }

    return count;
}

enum
{
    LESS_THAN,
    EQUAL,
    GREATER_THAN,
    LESS_THAN_OR_EQUAL,
    GREATER_THAN_OR_EQUAL,
    NOT_EQUAL,
};

u32 BattlerHPPercentage(u32 battler, u32 operation, u32 threshold)
{
    switch (operation)
    {
    case LESS_THAN:
        return gBattleMons[battler].hp < (gBattleMons[battler].maxHP / threshold);
    case EQUAL:
        return gBattleMons[battler].hp == (gBattleMons[battler].maxHP / threshold);
    case GREATER_THAN:
        return gBattleMons[battler].hp > (gBattleMons[battler].maxHP / threshold);
    case LESS_THAN_OR_EQUAL:
        return gBattleMons[battler].hp <= (gBattleMons[battler].maxHP / threshold);
    case GREATER_THAN_OR_EQUAL:
        return gBattleMons[battler].hp >= (gBattleMons[battler].maxHP / threshold);
    case NOT_EQUAL:
    default:
        return gBattleMons[battler].hp != (gBattleMons[battler].maxHP / threshold);
    }
}

u32 ShouldDoTrainerSlide(u32 battler, u32 which)
{
    u32 i, firstId, lastId, trainerId, retValue = 1;

    if (!(EsContraEntrenador()) || GetBattlerSide(battler) != B_SIDE_OPPONENT)
        return 0;

    firstId = 0, lastId = PARTY_SIZE;
    trainerId = gTrainerBattleOpponent_A;

    for (i = 0; i < ARRAY_COUNT(sTrainerSlides); i++)
    {
        if (trainerId == sTrainerSlides[i].trainerId && (!sTrainerSlides[i].isFrontierTrainer))
        {
            gBattleScripting.battler = battler;
            switch (which)
            {
            case TRAINER_SLIDE_LAST_SWITCHIN:
                if (sTrainerSlides[i].msgLastSwitchIn != NULL && !CanBattlerSwitch(battler))
                {
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgLastSwitchIn;
                    return retValue;
                }
                break;
            case TRAINER_SLIDE_LAST_LOW_HP:
                if (sTrainerSlides[i].msgLastLowHp != NULL
                    && GetEnemyMonCount(firstId, lastId, TRUE) == 1
                    && BattlerHPPercentage(battler, LESS_THAN_OR_EQUAL, 4)
                    && !gBattleStruct->trainerSlideLowHpMsgDone)
                {
                    gBattleStruct->trainerSlideLowHpMsgDone = TRUE;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgLastLowHp;
                    return retValue;
                }
                break;
            case TRAINER_SLIDE_FIRST_DOWN:
                if (sTrainerSlides[i].msgFirstDown != NULL && GetEnemyMonCount(firstId, lastId, TRUE) == GetEnemyMonCount(firstId, lastId, FALSE) - 1)
                {
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgFirstDown;
                    return retValue;
                }
                break;
            case TRAINER_SLIDE_LAST_HALF_HP:
                if (sTrainerSlides[i].msgLastHalfHp != NULL
                 && GetEnemyMonCount(firstId, lastId, TRUE) == GetEnemyMonCount(firstId, lastId, FALSE) - 1
                 && BattlerHPPercentage(battler, LESS_THAN_OR_EQUAL, 2) && BattlerHPPercentage(battler, GREATER_THAN, 4)
                 && !gBattleStruct->trainerSlideHalfHpMsgDone)
                {
                    gBattleStruct->trainerSlideHalfHpMsgDone = TRUE;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgLastHalfHp;
                    return TRUE;
                }
                break;
            case TRAINER_SLIDE_FIRST_CRITICAL_HIT:
                if (sTrainerSlides[i].msgFirstCriticalHit != NULL && gBattleStruct->trainerSlideFirstCriticalHitMsgState == 1)
                {
                    gBattleStruct->trainerSlideFirstCriticalHitMsgState = 2;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgFirstCriticalHit;
                    return TRUE;
                }
                break;
            case TRAINER_SLIDE_FIRST_SUPER_EFFECTIVE_HIT:
                if (sTrainerSlides[i].msgFirstSuperEffectiveHit != NULL
                    && gBattleStruct->trainerSlideFirstSuperEffectiveHitMsgState == 1
                    && gBattleMons[battler].hp)
                {
                    gBattleStruct->trainerSlideFirstSuperEffectiveHitMsgState = 2;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgFirstSuperEffectiveHit;
                    return TRUE;
                }
                break;
            case TRAINER_SLIDE_FIRST_STAB_MOVE:
                if (sTrainerSlides[i].msgFirstSTABMove != NULL
                 && gBattleStruct->trainerSlideFirstSTABMoveMsgState == 1
                 && GetEnemyMonCount(firstId, lastId, TRUE) == GetEnemyMonCount(firstId, lastId, FALSE))
                {
                    gBattleStruct->trainerSlideFirstSTABMoveMsgState = 2;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgFirstSTABMove;
                    return TRUE;
                }
                break;
            case TRAINER_SLIDE_PLAYER_MON_UNAFFECTED:
                if (sTrainerSlides[i].msgPlayerMonUnaffected != NULL
                 && gBattleStruct->trainerSlidePlayerMonUnaffectedMsgState == 1
                 && GetEnemyMonCount(firstId, lastId, TRUE) == GetEnemyMonCount(firstId, lastId, FALSE))
                {
                    gBattleStruct->trainerSlidePlayerMonUnaffectedMsgState = 2;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgPlayerMonUnaffected;
                    return TRUE;
                }
                break;
            case TRAINER_SLIDE_MEGA_EVOLUTION:
                if (sTrainerSlides[i].msgMegaEvolution != NULL && !gBattleStruct->trainerSlideMegaEvolutionMsgDone)
                {
                    gBattleStruct->trainerSlideMegaEvolutionMsgDone = TRUE;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgMegaEvolution;
                    return TRUE;
                }
                break;
            case TRAINER_SLIDE_Z_MOVE:
                if (sTrainerSlides[i].msgZMove != NULL && !gBattleStruct->trainerSlideZMoveMsgDone)
                {
                    gBattleStruct->trainerSlideZMoveMsgDone = TRUE;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgZMove;
                    return TRUE;
                }
                break;
            case TRAINER_SLIDE_BEFORE_FIRST_TURN:
                if (sTrainerSlides[i].msgBeforeFirstTurn != NULL && !gBattleStruct->trainerSlideBeforeFirstTurnMsgDone)
                {
                    gBattleStruct->trainerSlideBeforeFirstTurnMsgDone = TRUE;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgBeforeFirstTurn;
                    return TRUE;
                }
                break;
            case TRAINER_SLIDE_DYNAMAX:
                if (sTrainerSlides[i].msgDynamax != NULL && !gBattleStruct->trainerSlideDynamaxMsgDone)
                {
                    gBattleStruct->trainerSlideDynamaxMsgDone = TRUE;
                    gBattleStruct->trainerSlideMsg = sTrainerSlides[i].msgDynamax;
                    return TRUE;
                }
                break;
            }
            break;
        }
    }

    return 0;
}
