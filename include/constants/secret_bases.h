#ifndef GUARD_CONSTANTS_SECRET_BASES_H
#define GUARD_CONSTANTS_SECRET_BASES_H
//eliminar todo
#define SECRET_BASE_RED_CAVE 1
#define SECRET_BASE_BROWN_CAVE 2
#define SECRET_BASE_BLUE_CAVE 3
#define SECRET_BASE_YELLOW_CAVE 4
#define SECRET_BASE_TREE 5
#define SECRET_BASE_SHRUB 6

// Each secret base location is assigned an identifier value.
// The secret base's map is determined by (id / 10). The ones
// digit is used to differentiate secret bases using the same map.
// Therefore, each secret base map can be used by up to 10 different
// secret bases in the game.

// * 4 is for each byte of data per secret base group in sSecretBaseEntrancePositions
// They are the map number, the entrance warp id, and the x/y position in front of the computer
#define SECRET_BASE_GROUP(idx) ((idx) * 4)
#define SECRET_BASE_ID_TO_GROUP(baseId) SECRET_BASE_GROUP((baseId) / 10)

#define SECRET_BASE_RED_CAVE1_1     1
#define SECRET_BASE_RED_CAVE1_2     2
#define SECRET_BASE_RED_CAVE1_3     3

#define SECRET_BASE_RED_CAVE1 SECRET_BASE_GROUP(0)

#define SECRET_BASE_RED_CAVE2_1     11
#define SECRET_BASE_RED_CAVE2_2     12
#define SECRET_BASE_RED_CAVE2_3     13

#define SECRET_BASE_RED_CAVE2 SECRET_BASE_GROUP(1)

#define SECRET_BASE_RED_CAVE3_1     21
#define SECRET_BASE_RED_CAVE3_2     22
#define SECRET_BASE_RED_CAVE3_3     23

#define SECRET_BASE_RED_CAVE3 SECRET_BASE_GROUP(2)

#define SECRET_BASE_RED_CAVE4_1     31
#define SECRET_BASE_RED_CAVE4_2     32
#define SECRET_BASE_RED_CAVE4_3     33

#define SECRET_BASE_RED_CAVE4 SECRET_BASE_GROUP(3)

#define SECRET_BASE_BROWN_CAVE1_1   41
#define SECRET_BASE_BROWN_CAVE1_2   42
#define SECRET_BASE_BROWN_CAVE1_3   43

#define SECRET_BASE_BROWN_CAVE1 SECRET_BASE_GROUP(4)

#define SECRET_BASE_BROWN_CAVE2_1   51
#define SECRET_BASE_BROWN_CAVE2_2   52
#define SECRET_BASE_BROWN_CAVE2_3   53

#define SECRET_BASE_BROWN_CAVE2 SECRET_BASE_GROUP(5)

#define SECRET_BASE_BROWN_CAVE3_1   61
#define SECRET_BASE_BROWN_CAVE3_2   62
#define SECRET_BASE_BROWN_CAVE3_3   63

#define SECRET_BASE_BROWN_CAVE3 SECRET_BASE_GROUP(6)

#define SECRET_BASE_BROWN_CAVE4_1   71
#define SECRET_BASE_BROWN_CAVE4_2   72
#define SECRET_BASE_BROWN_CAVE4_3   73

#define SECRET_BASE_BROWN_CAVE4 SECRET_BASE_GROUP(7)

#define SECRET_BASE_BLUE_CAVE1_1    81
#define SECRET_BASE_BLUE_CAVE1_2    82
#define SECRET_BASE_BLUE_CAVE1_3    83

#define SECRET_BASE_BLUE_CAVE1 SECRET_BASE_GROUP(8)

#define SECRET_BASE_BLUE_CAVE2_1    91
#define SECRET_BASE_BLUE_CAVE2_2    92
#define SECRET_BASE_BLUE_CAVE2_3    93

#define SECRET_BASE_BLUE_CAVE2 SECRET_BASE_GROUP(9)

#define SECRET_BASE_BLUE_CAVE3_1    101
#define SECRET_BASE_BLUE_CAVE3_2    102
#define SECRET_BASE_BLUE_CAVE3_3    103

#define SECRET_BASE_BLUE_CAVE3 SECRET_BASE_GROUP(10)

#define SECRET_BASE_BLUE_CAVE4_1    111
#define SECRET_BASE_BLUE_CAVE4_2    112
#define SECRET_BASE_BLUE_CAVE4_3    113

#define SECRET_BASE_BLUE_CAVE4 SECRET_BASE_GROUP(11)

#define SECRET_BASE_YELLOW_CAVE1_1  121
#define SECRET_BASE_YELLOW_CAVE1_2  122
#define SECRET_BASE_YELLOW_CAVE1_3  123

#define SECRET_BASE_YELLOW_CAVE1 SECRET_BASE_GROUP(12)

#define SECRET_BASE_YELLOW_CAVE2_1  131
#define SECRET_BASE_YELLOW_CAVE2_2  132
#define SECRET_BASE_YELLOW_CAVE2_3  133

#define SECRET_BASE_YELLOW_CAVE2 SECRET_BASE_GROUP(13)

#define SECRET_BASE_YELLOW_CAVE3_1  141
#define SECRET_BASE_YELLOW_CAVE3_2  142
#define SECRET_BASE_YELLOW_CAVE3_3  143

#define SECRET_BASE_YELLOW_CAVE3 SECRET_BASE_GROUP(14)

#define SECRET_BASE_YELLOW_CAVE4_1  151
#define SECRET_BASE_YELLOW_CAVE4_2  152
#define SECRET_BASE_YELLOW_CAVE4_3  153

#define SECRET_BASE_YELLOW_CAVE4 SECRET_BASE_GROUP(15)

#define SECRET_BASE_TREE1_1         161
#define SECRET_BASE_TREE1_2         162
#define SECRET_BASE_TREE1_3         163
#define SECRET_BASE_TREE1_4         164

#define SECRET_BASE_TREE1 SECRET_BASE_GROUP(16)

#define SECRET_BASE_TREE2_1         171
#define SECRET_BASE_TREE2_2         172
#define SECRET_BASE_TREE2_3         173
#define SECRET_BASE_TREE2_4         174

#define SECRET_BASE_TREE2 SECRET_BASE_GROUP(17)

#define SECRET_BASE_TREE3_1         181
#define SECRET_BASE_TREE3_2         182
#define SECRET_BASE_TREE3_3         183

#define SECRET_BASE_TREE3 SECRET_BASE_GROUP(18)

#define SECRET_BASE_TREE4_1         191
#define SECRET_BASE_TREE4_2         192
#define SECRET_BASE_TREE4_3         193

#define SECRET_BASE_TREE4 SECRET_BASE_GROUP(19)

#define SECRET_BASE_SHRUB1_1        201
#define SECRET_BASE_SHRUB1_2        202
#define SECRET_BASE_SHRUB1_3        203
#define SECRET_BASE_SHRUB1_4        204

#define SECRET_BASE_SHRUB1 SECRET_BASE_GROUP(20)

#define SECRET_BASE_SHRUB2_1        211
#define SECRET_BASE_SHRUB2_2        212
#define SECRET_BASE_SHRUB2_3        213

#define SECRET_BASE_SHRUB2 SECRET_BASE_GROUP(21)

#define SECRET_BASE_SHRUB3_1        221
#define SECRET_BASE_SHRUB3_2        222
#define SECRET_BASE_SHRUB3_3        223

#define SECRET_BASE_SHRUB3 SECRET_BASE_GROUP(22)

#define SECRET_BASE_SHRUB4_1        231
#define SECRET_BASE_SHRUB4_2        232
#define SECRET_BASE_SHRUB4_3        233

#define SECRET_BASE_SHRUB4 SECRET_BASE_GROUP(23)

#define NUM_SECRET_BASE_GROUPS 24

#endif  // GUARD_CONSTANTS_SECRET_BASES_H
