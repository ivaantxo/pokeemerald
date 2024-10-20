#ifndef GUARD_CONSTANTS_MOVES_H
#define GUARD_CONSTANTS_MOVES_H

enum BattleMoves
{
    MOVE_NONE,
    MOVE_POUND,
    MOVE_KARATE_CHOP,
    MOVE_DOUBLE_SLAP,
    MOVE_COMET_PUNCH,
    MOVE_MEGA_PUNCH,
    MOVE_PAY_DAY,
    MOVE_FIRE_PUNCH,
    MOVE_ICE_PUNCH,
    MOVE_THUNDER_PUNCH,
    MOVE_SCRATCH,
    MOVE_VISE_GRIP,
    MOVE_GUILLOTINE,
    MOVE_RAZOR_WIND,
    MOVE_SWORDS_DANCE,
    MOVE_CUT,
    MOVE_GUST,
    MOVE_WING_ATTACK,
    MOVE_WHIRLWIND,
    MOVE_FLY,
    MOVE_BIND,
    MOVE_SLAM,
    MOVE_VINE_WHIP,
    MOVE_STOMP,
    MOVE_DOUBLE_KICK,
    MOVE_MEGA_KICK,
    MOVE_JUMP_KICK,
    MOVE_ROLLING_KICK,
    MOVE_SAND_ATTACK,
    MOVE_HEADBUTT,
    MOVE_HORN_ATTACK,
    MOVE_FURY_ATTACK,
    MOVE_HORN_DRILL,
    MOVE_TACKLE,
    MOVE_BODY_SLAM,
    MOVE_WRAP,
    MOVE_TAKE_DOWN,
    MOVE_THRASH,
    MOVE_DOUBLE_EDGE,
    MOVE_TAIL_WHIP,
    MOVE_POISON_STING,
    MOVE_TWINEEDLE,
    MOVE_PIN_MISSILE,
    MOVE_LEER,
    MOVE_BITE,
    MOVE_GROWL,
    MOVE_ROAR,
    MOVE_SING,
    MOVE_SUPERSONIC,
    MOVE_SONIC_BOOM,
    MOVE_DISABLE,
    MOVE_ACID,
    MOVE_EMBER,
    MOVE_FLAMETHROWER,
    MOVE_MIST,
    MOVE_WATER_GUN,
    MOVE_HYDRO_PUMP,
    MOVE_SURF,
    MOVE_ICE_BEAM,
    MOVE_BLIZZARD,
    MOVE_PSYBEAM,
    MOVE_BUBBLE_BEAM,
    MOVE_AURORA_BEAM,
    MOVE_HYPER_BEAM,
    MOVE_PECK,
    MOVE_DRILL_PECK,
    MOVE_SUBMISSION,
    MOVE_LOW_KICK,
    MOVE_COUNTER,
    MOVE_SEISMIC_TOSS,
    MOVE_STRENGTH,
    MOVE_ABSORB,
    MOVE_MEGA_DRAIN,
    MOVE_LEECH_SEED,
    MOVE_GROWTH,
    MOVE_RAZOR_LEAF,
    MOVE_SOLAR_BEAM,
    MOVE_POISON_POWDER,
    MOVE_STUN_SPORE,
    MOVE_SLEEP_POWDER,
    MOVE_PETAL_DANCE,
    MOVE_STRING_SHOT,
    MOVE_DRAGON_RAGE,
    MOVE_FIRE_SPIN,
    MOVE_THUNDER_SHOCK,
    MOVE_THUNDERBOLT,
    MOVE_THUNDER_WAVE,
    MOVE_THUNDER,
    MOVE_ROCK_THROW,
    MOVE_EARTHQUAKE,
    MOVE_FISSURE,
    MOVE_DIG,
    MOVE_TOXIC,
    MOVE_CONFUSION,
    MOVE_PSYCHIC,
    MOVE_HYPNOSIS,
    MOVE_MEDITATE,
    MOVE_AGILITY,
    MOVE_QUICK_ATTACK,
    MOVE_RAGE,
    MOVE_TELEPORT,
    MOVE_NIGHT_SHADE,
    MOVE_MIMIC,
    MOVE_SCREECH,
    MOVE_DOUBLE_TEAM,
    MOVE_RECOVER,
    MOVE_HARDEN,
    MOVE_MINIMIZE,
    MOVE_SMOKESCREEN,
    MOVE_CONFUSE_RAY,
    MOVE_WITHDRAW,
    MOVE_DEFENSE_CURL,
    MOVE_BARRIER,
    MOVE_LIGHT_SCREEN,
    MOVE_HAZE,
    MOVE_REFLECT,
    MOVE_FOCUS_ENERGY,
    MOVE_BIDE,
    MOVE_METRONOME,
    MOVE_MIRROR_MOVE,
    MOVE_SELF_DESTRUCT,
    MOVE_EGG_BOMB,
    MOVE_LICK,
    MOVE_SMOG,
    MOVE_SLUDGE,
    MOVE_BONE_CLUB,
    MOVE_FIRE_BLAST,
    MOVE_WATERFALL,
    MOVE_CLAMP,
    MOVE_SWIFT,
    MOVE_SKULL_BASH,
    MOVE_SPIKE_CANNON,
    MOVE_CONSTRICT,
    MOVE_AMNESIA,
    MOVE_KINESIS,
    MOVE_SOFT_BOILED,
    MOVE_HIGH_JUMP_KICK,
    MOVE_GLARE,
    MOVE_DREAM_EATER,
    MOVE_POISON_GAS,
    MOVE_BARRAGE,
    MOVE_LEECH_LIFE,
    MOVE_LOVELY_KISS,
    MOVE_SKY_ATTACK,
    MOVE_TRANSFORM,
    MOVE_BUBBLE,
    MOVE_DIZZY_PUNCH,
    MOVE_SPORE,
    MOVE_FLASH,
    MOVE_PSYWAVE,
    MOVE_SPLASH,
    MOVE_ACID_ARMOR,
    MOVE_CRABHAMMER,
    MOVE_EXPLOSION,
    MOVE_FURY_SWIPES,
    MOVE_BONEMERANG,
    MOVE_REST,
    MOVE_ROCK_SLIDE,
    MOVE_HYPER_FANG,
    MOVE_SHARPEN,
    MOVE_CONVERSION,
    MOVE_TRI_ATTACK,
    MOVE_SUPER_FANG,
    MOVE_SLASH,
    MOVE_SUBSTITUTE,
    MOVE_STRUGGLE,
    MOVE_SKETCH,
    MOVE_TRIPLE_KICK,
    MOVE_THIEF,
    MOVE_SPIDER_WEB,
    MOVE_MIND_READER,
    MOVE_NIGHTMARE,
    MOVE_FLAME_WHEEL,
    MOVE_SNORE,
    MOVE_CURSE,
    MOVE_FLAIL,
    MOVE_CONVERSION_2,
    MOVE_AEROBLAST,
    MOVE_COTTON_SPORE,
    MOVE_REVERSAL,
    MOVE_SPITE,
    MOVE_POWDER_SNOW,
    MOVE_PROTECT,
    MOVE_MACH_PUNCH,
    MOVE_SCARY_FACE,
    MOVE_FEINT_ATTACK,
    MOVE_SWEET_KISS,
    MOVE_BELLY_DRUM,
    MOVE_SLUDGE_BOMB,
    MOVE_MUD_SLAP,
    MOVE_OCTAZOOKA,
    MOVE_SPIKES,
    MOVE_ZAP_CANNON,
    MOVE_FORESIGHT,
    MOVE_DESTINY_BOND,
    MOVE_PERISH_SONG,
    MOVE_ICY_WIND,
    MOVE_DETECT,
    MOVE_BONE_RUSH,
    MOVE_LOCK_ON,
    MOVE_OUTRAGE,
    MOVE_SANDSTORM,
    MOVE_GIGA_DRAIN,
    MOVE_ENDURE,
    MOVE_CHARM,
    MOVE_ROLLOUT,
    MOVE_FALSE_SWIPE,
    MOVE_SWAGGER,
    MOVE_MILK_DRINK,
    MOVE_SPARK,
    MOVE_FURY_CUTTER,
    MOVE_STEEL_WING,
    MOVE_MEAN_LOOK,
    MOVE_ATTRACT,
    MOVE_SLEEP_TALK,
    MOVE_HEAL_BELL,
    MOVE_RETURN,
    MOVE_PRESENT,
    MOVE_FRUSTRATION,
    MOVE_SAFEGUARD,
    MOVE_PAIN_SPLIT,
    MOVE_SACRED_FIRE,
    MOVE_MAGNITUDE,
    MOVE_DYNAMIC_PUNCH,
    MOVE_MEGAHORN,
    MOVE_DRAGON_BREATH,
    MOVE_BATON_PASS,
    MOVE_ENCORE,
    MOVE_PURSUIT,
    MOVE_RAPID_SPIN,
    MOVE_SWEET_SCENT,
    MOVE_IRON_TAIL,
    MOVE_METAL_CLAW,
    MOVE_VITAL_THROW,
    MOVE_MORNING_SUN,
    MOVE_SYNTHESIS,
    MOVE_MOONLIGHT,
    MOVE_HIDDEN_POWER,
    MOVE_CROSS_CHOP,
    MOVE_TWISTER,
    MOVE_RAIN_DANCE,
    MOVE_SUNNY_DAY,
    MOVE_CRUNCH,
    MOVE_MIRROR_COAT,
    MOVE_PSYCH_UP,
    MOVE_EXTREME_SPEED,
    MOVE_ANCIENT_POWER,
    MOVE_SHADOW_BALL,
    MOVE_FUTURE_SIGHT,
    MOVE_ROCK_SMASH,
    MOVE_WHIRLPOOL,
    MOVE_BEAT_UP,
    MOVE_FAKE_OUT,
    MOVE_UPROAR,
    MOVE_STOCKPILE,
    MOVE_SPIT_UP,
    MOVE_SWALLOW,
    MOVE_HEAT_WAVE,
    MOVE_HAIL,
    MOVE_TORMENT,
    MOVE_FLATTER,
    MOVE_WILL_O_WISP,
    MOVE_MEMENTO,
    MOVE_FACADE,
    MOVE_FOCUS_PUNCH,
    MOVE_SMELLING_SALTS,
    MOVE_FOLLOW_ME,
    MOVE_NATURE_POWER,
    MOVE_CHARGE,
    MOVE_TAUNT,
    MOVE_HELPING_HAND,
    MOVE_TRICK,
    MOVE_ROLE_PLAY,
    MOVE_WISH,
    MOVE_ASSIST,
    MOVE_INGRAIN,
    MOVE_SUPERPOWER,
    MOVE_MAGIC_COAT,
    MOVE_RECYCLE,
    MOVE_REVENGE,
    MOVE_BRICK_BREAK,
    MOVE_YAWN,
    MOVE_KNOCK_OFF,
    MOVE_ENDEAVOR,
    MOVE_ERUPTION,
    MOVE_SKILL_SWAP,
    MOVE_IMPRISON,
    MOVE_REFRESH,
    MOVE_GRUDGE,
    MOVE_SNATCH,
    MOVE_SECRET_POWER,
    MOVE_DIVE,
    MOVE_ARM_THRUST,
    MOVE_CAMOUFLAGE,
    MOVE_TAIL_GLOW,
    MOVE_LUSTER_PURGE,
    MOVE_MIST_BALL,
    MOVE_FEATHER_DANCE,
    MOVE_TEETER_DANCE,
    MOVE_BLAZE_KICK,
    MOVE_MUD_SPORT,
    MOVE_ICE_BALL,
    MOVE_NEEDLE_ARM,
    MOVE_SLACK_OFF,
    MOVE_HYPER_VOICE,
    MOVE_POISON_FANG,
    MOVE_CRUSH_CLAW,
    MOVE_BLAST_BURN,
    MOVE_HYDRO_CANNON,
    MOVE_METEOR_MASH,
    MOVE_ASTONISH,
    MOVE_WEATHER_BALL,
    MOVE_AROMATHERAPY,
    MOVE_FAKE_TEARS,
    MOVE_AIR_CUTTER,
    MOVE_OVERHEAT,
    MOVE_ODOR_SLEUTH,
    MOVE_ROCK_TOMB,
    MOVE_SILVER_WIND,
    MOVE_METAL_SOUND,
    MOVE_GRASS_WHISTLE,
    MOVE_TICKLE,
    MOVE_COSMIC_POWER,
    MOVE_WATER_SPOUT,
    MOVE_SIGNAL_BEAM,
    MOVE_SHADOW_PUNCH,
    MOVE_EXTRASENSORY,
    MOVE_SKY_UPPERCUT,
    MOVE_SAND_TOMB,
    MOVE_SHEER_COLD,
    MOVE_MUDDY_WATER,
    MOVE_BULLET_SEED,
    MOVE_AERIAL_ACE,
    MOVE_ICICLE_SPEAR,
    MOVE_IRON_DEFENSE,
    MOVE_BLOCK,
    MOVE_HOWL,
    MOVE_DRAGON_CLAW,
    MOVE_FRENZY_PLANT,
    MOVE_BULK_UP,
    MOVE_BOUNCE,
    MOVE_MUD_SHOT,
    MOVE_POISON_TAIL,
    MOVE_COVET,
    MOVE_VOLT_TACKLE,
    MOVE_MAGICAL_LEAF,
    MOVE_WATER_SPORT,
    MOVE_CALM_MIND,
    MOVE_LEAF_BLADE,
    MOVE_DRAGON_DANCE,
    MOVE_ROCK_BLAST,
    MOVE_SHOCK_WAVE,
    MOVE_WATER_PULSE,
    MOVE_DOOM_DESIRE,
    MOVE_PSYCHO_BOOST,
    MOVE_ROOST,
    MOVE_GRAVITY,
    MOVE_MIRACLE_EYE,
    MOVE_WAKE_UP_SLAP,
    MOVE_HAMMER_ARM,
    MOVE_GYRO_BALL,
    MOVE_HEALING_WISH,
    MOVE_BRINE,
    MOVE_NATURAL_GIFT,
    MOVE_FEINT,
    MOVE_PLUCK,
    MOVE_TAILWIND,
    MOVE_ACUPRESSURE,
    MOVE_METAL_BURST,
    MOVE_U_TURN,
    MOVE_CLOSE_COMBAT,
    MOVE_PAYBACK,
    MOVE_ASSURANCE,
    MOVE_EMBARGO,
    MOVE_FLING,
    MOVE_PSYCHO_SHIFT,
    MOVE_TRUMP_CARD,
    MOVE_HEAL_BLOCK,
    MOVE_WRING_OUT,
    MOVE_POWER_TRICK,
    MOVE_GASTRO_ACID,
    MOVE_LUCKY_CHANT,
    MOVE_ME_FIRST,
    MOVE_COPYCAT,
    MOVE_POWER_SWAP,
    MOVE_GUARD_SWAP,
    MOVE_PUNISHMENT,
    MOVE_LAST_RESORT,
    MOVE_WORRY_SEED,
    MOVE_SUCKER_PUNCH,
    MOVE_TOXIC_SPIKES,
    MOVE_HEART_SWAP,
    MOVE_AQUA_RING,
    MOVE_MAGNET_RISE,
    MOVE_FLARE_BLITZ,
    MOVE_FORCE_PALM,
    MOVE_AURA_SPHERE,
    MOVE_ROCK_POLISH,
    MOVE_POISON_JAB,
    MOVE_DARK_PULSE,
    MOVE_NIGHT_SLASH,
    MOVE_AQUA_TAIL,
    MOVE_SEED_BOMB,
    MOVE_AIR_SLASH,
    MOVE_X_SCISSOR,
    MOVE_BUG_BUZZ,
    MOVE_DRAGON_PULSE,
    MOVE_DRAGON_RUSH,
    MOVE_POWER_GEM,
    MOVE_DRAIN_PUNCH,
    MOVE_VACUUM_WAVE,
    MOVE_FOCUS_BLAST,
    MOVE_ENERGY_BALL,
    MOVE_BRAVE_BIRD,
    MOVE_EARTH_POWER,
    MOVE_SWITCHEROO,
    MOVE_GIGA_IMPACT,
    MOVE_NASTY_PLOT,
    MOVE_BULLET_PUNCH,
    MOVE_AVALANCHE,
    MOVE_ICE_SHARD,
    MOVE_SHADOW_CLAW,
    MOVE_THUNDER_FANG,
    MOVE_ICE_FANG,
    MOVE_FIRE_FANG,
    MOVE_SHADOW_SNEAK,
    MOVE_MUD_BOMB,
    MOVE_PSYCHO_CUT,
    MOVE_ZEN_HEADBUTT,
    MOVE_MIRROR_SHOT,
    MOVE_FLASH_CANNON,
    MOVE_ROCK_CLIMB,
    MOVE_DEFOG,
    MOVE_TRICK_ROOM,
    MOVE_DRACO_METEOR,
    MOVE_DISCHARGE,
    MOVE_LAVA_PLUME,
    MOVE_LEAF_STORM,
    MOVE_POWER_WHIP,
    MOVE_ROCK_WRECKER,
    MOVE_CROSS_POISON,
    MOVE_GUNK_SHOT,
    MOVE_IRON_HEAD,
    MOVE_MAGNET_BOMB,
    MOVE_STONE_EDGE,
    MOVE_CAPTIVATE,
    MOVE_STEALTH_ROCK,
    MOVE_GRASS_KNOT,
    MOVE_CHATTER,
    MOVE_JUDGMENT,
    MOVE_BUG_BITE,
    MOVE_CHARGE_BEAM,
    MOVE_WOOD_HAMMER,
    MOVE_AQUA_JET,
    MOVE_ATTACK_ORDER,
    MOVE_DEFEND_ORDER,
    MOVE_HEAL_ORDER,
    MOVE_HEAD_SMASH,
    MOVE_DOUBLE_HIT,
    MOVE_ROAR_OF_TIME,
    MOVE_SPACIAL_REND,
    MOVE_LUNAR_DANCE,
    MOVE_CRUSH_GRIP,
    MOVE_MAGMA_STORM,
    MOVE_DARK_VOID,
    MOVE_SEED_FLARE,
    MOVE_OMINOUS_WIND,
    MOVE_SHADOW_FORCE,
    MOVE_HONE_CLAWS,
    MOVE_WIDE_GUARD,
    MOVE_GUARD_SPLIT,
    MOVE_POWER_SPLIT,
    MOVE_WONDER_ROOM,
    MOVE_PSYSHOCK,
    MOVE_VENOSHOCK,
    MOVE_AUTOTOMIZE,
    MOVE_RAGE_POWDER,
    MOVE_TELEKINESIS,
    MOVE_MAGIC_ROOM,
    MOVE_SMACK_DOWN,
    MOVE_STORM_THROW,
    MOVE_FLAME_BURST,
    MOVE_SLUDGE_WAVE,
    MOVE_QUIVER_DANCE,
    MOVE_HEAVY_SLAM,
    MOVE_SYNCHRONOISE,
    MOVE_ELECTRO_BALL,
    MOVE_SOAK,
    MOVE_FLAME_CHARGE,
    MOVE_COIL,
    MOVE_LOW_SWEEP,
    MOVE_ACID_SPRAY,
    MOVE_FOUL_PLAY,
    MOVE_SIMPLE_BEAM,
    MOVE_ENTRAINMENT,
    MOVE_AFTER_YOU,
    MOVE_ROUND,
    MOVE_ECHOED_VOICE,
    MOVE_CHIP_AWAY,
    MOVE_CLEAR_SMOG,
    MOVE_STORED_POWER,
    MOVE_QUICK_GUARD,
    MOVE_ALLY_SWITCH,
    MOVE_SCALD,
    MOVE_SHELL_SMASH,
    MOVE_HEAL_PULSE,
    MOVE_HEX,
    MOVE_SKY_DROP,
    MOVE_SHIFT_GEAR,
    MOVE_CIRCLE_THROW,
    MOVE_INCINERATE,
    MOVE_QUASH,
    MOVE_ACROBATICS,
    MOVE_REFLECT_TYPE,
    MOVE_RETALIATE,
    MOVE_FINAL_GAMBIT,
    MOVE_BESTOW,
    MOVE_INFERNO,
    MOVE_WATER_PLEDGE,
    MOVE_FIRE_PLEDGE,
    MOVE_GRASS_PLEDGE,
    MOVE_VOLT_SWITCH,
    MOVE_STRUGGLE_BUG,
    MOVE_BULLDOZE,
    MOVE_FROST_BREATH,
    MOVE_DRAGON_TAIL,
    MOVE_WORK_UP,
    MOVE_ELECTROWEB,
    MOVE_WILD_CHARGE,
    MOVE_DRILL_RUN,
    MOVE_DUAL_CHOP,
    MOVE_HEART_STAMP,
    MOVE_HORN_LEECH,
    MOVE_SACRED_SWORD,
    MOVE_RAZOR_SHELL,
    MOVE_HEAT_CRASH,
    MOVE_LEAF_TORNADO,
    MOVE_STEAMROLLER,
    MOVE_COTTON_GUARD,
    MOVE_NIGHT_DAZE,
    MOVE_PSYSTRIKE,
    MOVE_TAIL_SLAP,
    MOVE_HURRICANE,
    MOVE_HEAD_CHARGE,
    MOVE_GEAR_GRIND,
    MOVE_SEARING_SHOT,
    MOVE_TECHNO_BLAST,
    MOVE_RELIC_SONG,
    MOVE_SECRET_SWORD,
    MOVE_GLACIATE,
    MOVE_BOLT_STRIKE,
    MOVE_BLUE_FLARE,
    MOVE_FIERY_DANCE,
    MOVE_FREEZE_SHOCK,
    MOVE_ICE_BURN,
    MOVE_SNARL,
    MOVE_ICICLE_CRASH,
    MOVE_V_CREATE,
    MOVE_FUSION_FLARE,
    MOVE_FUSION_BOLT,
    MOVE_FLYING_PRESS,
    MOVE_MAT_BLOCK,
    MOVE_BELCH,
    MOVE_ROTOTILLER,
    MOVE_STICKY_WEB,
    MOVE_FELL_STINGER,
    MOVE_PHANTOM_FORCE,
    MOVE_TRICK_OR_TREAT,
    MOVE_NOBLE_ROAR,
    MOVE_ION_DELUGE,
    MOVE_PARABOLIC_CHARGE,
    MOVE_FORESTS_CURSE,
    MOVE_PETAL_BLIZZARD,
    MOVE_FREEZE_DRY,
    MOVE_DISARMING_VOICE,
    MOVE_PARTING_SHOT,
    MOVE_TOPSY_TURVY,
    MOVE_DRAINING_KISS,
    MOVE_CRAFTY_SHIELD,
    MOVE_FLOWER_SHIELD,
    MOVE_GRASSY_TERRAIN,
    MOVE_MISTY_TERRAIN,
    MOVE_ELECTRIFY,
    MOVE_PLAY_ROUGH,
    MOVE_FAIRY_WIND,
    MOVE_MOONBLAST,
    MOVE_BOOMBURST,
    MOVE_FAIRY_LOCK,
    MOVE_KINGS_SHIELD,
    MOVE_PLAY_NICE,
    MOVE_CONFIDE,
    MOVE_DIAMOND_STORM,
    MOVE_STEAM_ERUPTION,
    MOVE_HYPERSPACE_HOLE,
    MOVE_WATER_SHURIKEN,
    MOVE_MYSTICAL_FIRE,
    MOVE_SPIKY_SHIELD,
    MOVE_AROMATIC_MIST,
    MOVE_EERIE_IMPULSE,
    MOVE_VENOM_DRENCH,
    MOVE_POWDER,
    MOVE_GEOMANCY,
    MOVE_MAGNETIC_FLUX,
    MOVE_HAPPY_HOUR,
    MOVE_ELECTRIC_TERRAIN,
    MOVE_DAZZLING_GLEAM,
    MOVE_CELEBRATE,
    MOVE_HOLD_HANDS,
    MOVE_BABY_DOLL_EYES,
    MOVE_NUZZLE,
    MOVE_HOLD_BACK,
    MOVE_INFESTATION,
    MOVE_POWER_UP_PUNCH,
    MOVE_OBLIVION_WING,
    MOVE_THOUSAND_ARROWS,
    MOVE_THOUSAND_WAVES,
    MOVE_LANDS_WRATH,
    MOVE_LIGHT_OF_RUIN,
    MOVE_ORIGIN_PULSE,
    MOVE_PRECIPICE_BLADES,
    MOVE_DRAGON_ASCENT,
    MOVE_HYPERSPACE_FURY,
    MOVE_SHORE_UP,
    MOVE_FIRST_IMPRESSION,
    MOVE_BANEFUL_BUNKER,
    MOVE_SPIRIT_SHACKLE,
    MOVE_DARKEST_LARIAT,
    MOVE_SPARKLING_ARIA,
    MOVE_ICE_HAMMER,
    MOVE_FLORAL_HEALING,
    MOVE_HIGH_HORSEPOWER,
    MOVE_STRENGTH_SAP,
    MOVE_SOLAR_BLADE,
    MOVE_LEAFAGE,
    MOVE_SPOTLIGHT,
    MOVE_TOXIC_THREAD,
    MOVE_LASER_FOCUS,
    MOVE_GEAR_UP,
    MOVE_THROAT_CHOP,
    MOVE_POLLEN_PUFF,
    MOVE_ANCHOR_SHOT,
    MOVE_PSYCHIC_TERRAIN,
    MOVE_LUNGE,
    MOVE_FIRE_LASH,
    MOVE_POWER_TRIP,
    MOVE_BURN_UP,
    MOVE_SPEED_SWAP,
    MOVE_SMART_STRIKE,
    MOVE_PURIFY,
    MOVE_REVELATION_DANCE,
    MOVE_CORE_ENFORCER,
    MOVE_TROP_KICK,
    MOVE_INSTRUCT,
    MOVE_BEAK_BLAST,
    MOVE_CLANGING_SCALES,
    MOVE_DRAGON_HAMMER,
    MOVE_BRUTAL_SWING,
    MOVE_AURORA_VEIL,
    MOVE_SHELL_TRAP,
    MOVE_FLEUR_CANNON,
    MOVE_PSYCHIC_FANGS,
    MOVE_STOMPING_TANTRUM,
    MOVE_SHADOW_BONE,
    MOVE_ACCELEROCK,
    MOVE_LIQUIDATION,
    MOVE_PRISMATIC_LASER,
    MOVE_SPECTRAL_THIEF,
    MOVE_SUNSTEEL_STRIKE,
    MOVE_MOONGEIST_BEAM,
    MOVE_TEARFUL_LOOK,
    MOVE_ZING_ZAP,
    MOVE_NATURES_MADNESS,
    MOVE_MULTI_ATTACK,
    MOVE_MIND_BLOWN,
    MOVE_PLASMA_FISTS,
    MOVE_PHOTON_GEYSER,
    MOVE_ZIPPY_ZAP,
    MOVE_SPLISHY_SPLASH,
    MOVE_FLOATY_FALL,
    MOVE_PIKA_PAPOW,
    MOVE_BOUNCY_BUBBLE,
    MOVE_BUZZY_BUZZ,
    MOVE_SIZZLY_SLIDE,
    MOVE_GLITZY_GLOW,
    MOVE_BADDY_BAD,
    MOVE_SAPPY_SEED,
    MOVE_FREEZY_FROST,
    MOVE_SPARKLY_SWIRL,
    MOVE_VEEVEE_VOLLEY,
    MOVE_DOUBLE_IRON_BASH,
    MOVE_DYNAMAX_CANNON,
    MOVE_SNIPE_SHOT,
    MOVE_JAW_LOCK,
    MOVE_STUFF_CHEEKS,
    MOVE_NO_RETREAT,
    MOVE_TAR_SHOT,
    MOVE_MAGIC_POWDER,
    MOVE_DRAGON_DARTS,
    MOVE_TEATIME,
    MOVE_OCTOLOCK,
    MOVE_BOLT_BEAK,
    MOVE_FISHIOUS_REND,
    MOVE_COURT_CHANGE,
    MOVE_CLANGOROUS_SOUL,
    MOVE_BODY_PRESS,
    MOVE_DECORATE,
    MOVE_DRUM_BEATING,
    MOVE_SNAP_TRAP,
    MOVE_PYRO_BALL,
    MOVE_BEHEMOTH_BLADE,
    MOVE_BEHEMOTH_BASH,
    MOVE_AURA_WHEEL,
    MOVE_BREAKING_SWIPE,
    MOVE_BRANCH_POKE,
    MOVE_OVERDRIVE,
    MOVE_APPLE_ACID,
    MOVE_GRAV_APPLE,
    MOVE_SPIRIT_BREAK,
    MOVE_STRANGE_STEAM,
    MOVE_LIFE_DEW,
    MOVE_OBSTRUCT,
    MOVE_FALSE_SURRENDER,
    MOVE_METEOR_ASSAULT,
    MOVE_ETERNABEAM,
    MOVE_STEEL_BEAM,
    MOVE_EXPANDING_FORCE,
    MOVE_STEEL_ROLLER,
    MOVE_SCALE_SHOT,
    MOVE_METEOR_BEAM,
    MOVE_SHELL_SIDE_ARM,
    MOVE_MISTY_EXPLOSION,
    MOVE_GRASSY_GLIDE,
    MOVE_RISING_VOLTAGE,
    MOVE_TERRAIN_PULSE,
    MOVE_SKITTER_SMACK,
    MOVE_BURNING_JEALOUSY,
    MOVE_LASH_OUT,
    MOVE_POLTERGEIST,
    MOVE_CORROSIVE_GAS,
    MOVE_COACHING,
    MOVE_FLIP_TURN,
    MOVE_TRIPLE_AXEL,
    MOVE_DUAL_WINGBEAT,
    MOVE_SCORCHING_SANDS,
    MOVE_JUNGLE_HEALING,
    MOVE_WICKED_BLOW,
    MOVE_SURGING_STRIKES,
    MOVE_THUNDER_CAGE,
    MOVE_DRAGON_ENERGY,
    MOVE_FREEZING_GLARE,
    MOVE_FIERY_WRATH,
    MOVE_THUNDEROUS_KICK,
    MOVE_GLACIAL_LANCE,
    MOVE_ASTRAL_BARRAGE,
    MOVE_EERIE_SPELL,
    MOVE_DIRE_CLAW,
    MOVE_PSYSHIELD_BASH,
    MOVE_POWER_SHIFT,
    MOVE_STONE_AXE,
    MOVE_SPRINGTIDE_STORM,
    MOVE_MYSTICAL_POWER,
    MOVE_RAGING_FURY,
    MOVE_WAVE_CRASH,
    MOVE_CHLOROBLAST,
    MOVE_MOUNTAIN_GALE,
    MOVE_VICTORY_DANCE,
    MOVE_HEADLONG_RUSH,
    MOVE_BARB_BARRAGE,
    MOVE_ESPER_WING,
    MOVE_BITTER_MALICE,
    MOVE_SHELTER,
    MOVE_TRIPLE_ARROWS,
    MOVE_INFERNAL_PARADE,
    MOVE_CEASELESS_EDGE,
    MOVE_BLEAKWIND_STORM,
    MOVE_WILDBOLT_STORM,
    MOVE_SANDSEAR_STORM,
    MOVE_LUNAR_BLESSING,
    MOVE_TAKE_HEART,
    MOVE_TERA_BLAST,
    MOVE_SILK_TRAP,
    MOVE_AXE_KICK,
    MOVE_LAST_RESPECTS,
    MOVE_LUMINA_CRASH,
    MOVE_ORDER_UP,
    MOVE_JET_PUNCH,
    MOVE_SPICY_EXTRACT,
    MOVE_SPIN_OUT,
    MOVE_POPULATION_BOMB,
    MOVE_ICE_SPINNER,
    MOVE_GLAIVE_RUSH,
    MOVE_REVIVAL_BLESSING,
    MOVE_SALT_CURE,
    MOVE_TRIPLE_DIVE,
    MOVE_MORTAL_SPIN,
    MOVE_DOODLE,
    MOVE_FILLET_AWAY,
    MOVE_KOWTOW_CLEAVE,
    MOVE_FLOWER_TRICK,
    MOVE_TORCH_SONG,
    MOVE_AQUA_STEP,
    MOVE_RAGING_BULL,
    MOVE_MAKE_IT_RAIN,
    MOVE_RUINATION,
    MOVE_COLLISION_COURSE,
    MOVE_ELECTRO_DRIFT,
    MOVE_SHED_TAIL,
    MOVE_CHILLY_RECEPTION,
    MOVE_TIDY_UP,
    MOVE_SNOWSCAPE,
    MOVE_POUNCE,
    MOVE_TRAILBLAZE,
    MOVE_CHILLING_WATER,
    MOVE_HYPER_DRILL,
    MOVE_TWIN_BEAM,
    MOVE_RAGE_FIST,
    MOVE_ARMOR_CANNON,
    MOVE_BITTER_BLADE,
    MOVE_DOUBLE_SHOCK,
    MOVE_GIGATON_HAMMER,
    MOVE_COMEUPPANCE,
    MOVE_AQUA_CUTTER,
    MOVE_BLAZING_TORQUE,
    MOVE_WICKED_TORQUE,
    MOVE_NOXIOUS_TORQUE,
    MOVE_COMBAT_TORQUE,
    MOVE_MAGICAL_TORQUE,
    MOVE_PSYBLADE,
    MOVE_HYDRO_STEAM,
    MOVE_BLOOD_MOON,
    MOVE_MATCHA_GOTCHA,
    MOVE_SYRUP_BOMB,
    MOVE_IVY_CUDGEL,
    MOVE_ELECTRO_SHOT,
    MOVE_TERA_STARSTORM,
    MOVE_FICKLE_BEAM,
    MOVE_BURNING_BULWARK,
    MOVE_THUNDERCLAP,
    MOVE_MIGHTY_CLEAVE,
    MOVE_TACHYON_CUTTER,
    MOVE_HARD_PRESS,
    MOVE_DRAGON_CHEER,
    MOVE_ALLURING_VOICE,
    MOVE_TEMPER_FLARE,
    MOVE_SUPERCELL_SLAM,
    MOVE_PSYCHIC_NOISE,
    MOVE_UPPER_HAND,
    MOVE_MALIGNANT_CHAIN,
    MOVE_COLMENA,
    MOVE_CARGA_JABATO,
    MOVE_AMOLADORAS,
    MOVE_CHUPASANGRE,

    MOVES_COUNT
};

// Used for checks for moves affected by Disable, Mimic, etc.
#define MOVE_UNAVAILABLE 0xFFFF

#endif  // GUARD_CONSTANTS_MOVES_H
