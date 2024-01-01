;
; Text 'en'- Generated from gametext.txt
;

;
; Text Table
;

.segment    "RODATA"
_text_table:
.export _text_table

.byte    <tr_game_title, >tr_game_title
.byte    <tr_new_game, >tr_new_game
.byte    <tr_continue, >tr_continue
.byte    <tr_version, >tr_version
.byte    <tr_copyright, >tr_copyright
.byte    <tr_tower_name, >tr_tower_name
.byte    <tr_boss_1_location, >tr_boss_1_location
.byte    <tr_boss_1_name, >tr_boss_1_name
.byte    <tr_boss_2_location, >tr_boss_2_location
.byte    <tr_boss_2_name, >tr_boss_2_name
.byte    <tr_boss_3_location, >tr_boss_3_location
.byte    <tr_boos_3_name, >tr_boos_3_name
.byte    <tr_boss_4_location, >tr_boss_4_location
.byte    <tr_boss_4_name, >tr_boss_4_name
.byte    <tr_boss_5_location, >tr_boss_5_location
.byte    <tr_boss_5_name, >tr_boss_5_name
.byte    <tr_boss_6_location, >tr_boss_6_location
.byte    <tr_boss_6_name, >tr_boss_6_name
.byte    <tr_boss_7_location, >tr_boss_7_location
.byte    <tr_boss_7_name, >tr_boss_7_name
.byte    <tr_boss_8_location, >tr_boss_8_location
.byte    <tr_boss_8_name, >tr_boss_8_name
.byte    <tr_cutscene_intro_0, >tr_cutscene_intro_0
.byte    <tr_cutscene_intro_1, >tr_cutscene_intro_1
.byte    <tr_cutscene_intro_2, >tr_cutscene_intro_2
.byte    <tr_cutscene_intro_3, >tr_cutscene_intro_3
.byte    <tr_cutscene_intro_4, >tr_cutscene_intro_4

;
; Text
;

.segment    "RODATA"

tr_game_title:                   TR "Shadowborn"
tr_new_game:                     TR "New Game"
tr_continue:                     TR "Continue"
tr_version:                      TR .concat("Version ", VERSION)
tr_copyright:                    TR .concat("(C) ", COPYRIGHT_YEAR)
tr_tower_name:                   TR "Tower of Luxluma"
tr_boss_1_location:              TR "Tower of Luxluma Bridge"
tr_boss_1_name:                  TR "Yrkargarin, Last Knight of Luxluma"
tr_boss_2_location:              TR "abTower of Luxluma Courtyard"
tr_boss_2_name:                  TR "Brightbeast"
tr_boss_3_location:              TR "Tower of Luxluma Upper Chapel"
tr_boos_3_name:                  TR "Corrupted High Radiance Auhmn"
tr_boss_4_location:              TR "Tower of Luxluma Gleaming Road (walkway up to the top of the tower)"
tr_boss_4_name:                  TR "Iridecence Dragon"
tr_boss_5_location:              TR "Tower of Luxluma Sewers"
tr_boss_5_name:                  TR "Ophiotroph Dragon Corpse"
tr_boss_6_location:              TR "Tower of Luxluma Ramparts"
tr_boss_6_name:                  TR "Shadowkin Invader"
tr_boss_7_location:              TR "Tower of Luxluma Gleaming Road End (door to Radiant Throne)"
tr_boss_7_name:                  TR "Specular Gaurds Dirk & Quince"
tr_boss_8_location:              TR "Tower of Luxluma Radiant Throne"
tr_boss_8_name:                  TR "Lightlord of Sin"
tr_cutscene_intro_0:             TR "Those born of shadow are forever doomed to serve the light."
tr_cutscene_intro_1:             TR "This corrupted light must be snuffed out. Those born of shadow have devine right to fufill this task."
tr_cutscene_intro_2:             TR "When the light is stained; dim; corrupted. Those born to shadow are bound to act."
tr_cutscene_intro_3:             TR "fa"
tr_cutscene_intro_4:             TR "ee"
