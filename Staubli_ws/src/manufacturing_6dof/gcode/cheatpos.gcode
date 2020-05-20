;FLAVOR:Marlin
;TIME:6440
;Filament used: 4.68371m
;Layer height: 0.2
;Generated with Cura_SteamEngine 3.6.0
M82 ;absolute extrusion mode
G21 ; set units to millimeters
G90 ; use absolute positioning
M82 ; absolute extrusion mode
M104 S200 ; set extruder temp
M140 S60 ; set bed temp
M190 S60 ; wait for bed temp
M109 S200 ; wait for extruder temp
G28 W ; home all without mesh bed level
G80 ; mesh bed leveling
G92 E0.0 ; reset extruder distance position
G1 F1000.0 ; go outside print area
G1 E9.0 F1000.0 ; intro line
G1  E21.5 F1000.0 ; intro line
G92 E0.0 ; reset extruder distance position
G92 E0
G1 F2100 E-0.8
;LAYER_COUNT:305
;LAYER:0
M107
G0 F3600 X102.451 Y39.65 Z0
;TYPE:SKIRT
G1 F2100 E0
G1 F1350 X99.81 Y39.233 E0.01373
G1 X100.398 Y38.751 E0.03269
G1 X100.838 Y38.459 E0.04586
G1 X101.074 Y38.218 E0.05428
G1 X101.447 Y37.93 E0.06603
G1 X101.503 Y37.841 E0.06866
G1 X101.86 Y37.422 E0.08239
G1 X102.146 Y37.172 E0.09186
;LAYER:1
G0 F3600 X102.451 Y39.65 Z5
;TYPE:SKIRT
G1 F2100 E0
G1 F1350 X99.81 Y39.233 E0.01373
G1 X100.398 Y38.751 E0.03269
G1 X100.838 Y38.459 E0.04586
G1 X101.074 Y38.218 E0.05428
;TIME_ELAPSED:6440.476809
G1 F2100 E525.55228
M140 S0
M107
M104 S0 ; turn off extruder
M140 S0 ; turn off heatbed
M107 ; turn off fan
G1 X0 Y0 Z15 I0 J0 K1000 ; park home
M84 ; disable motors
M82 ;absolute extrusion mode
M104 S0
;End of Gcode
;SETTING_3 {"global_quality": "[general]\\nversion = 4\\nname = Draft #2\\ndefin
;SETTING_3 ition = fdmprinter\\n\\n[metadata]\\ntype = quality_changes\\nquality
;SETTING_3 _type = draft\\nsetting_version = 5\\n\\n[values]\\nadhesion_type = b
;SETTING_3 rim\\nsupport_enable = True\\nsupport_type = buildplate\\n\\n", "extr
;SETTING_3 uder_quality": ["[general]\\nversion = 4\\nname = Draft #2\\ndefiniti
;SETTING_3 on = fdmprinter\\n\\n[metadata]\\ntype = quality_changes\\nposition =
;SETTING_3  0\\nquality_type = draft\\nsetting_version = 5\\n\\n[values]\\nbrim_
;SETTING_3 width = 4\\ninfill_sparse_density = 15\\nspeed_print = 45\\nsupport_a
;SETTING_3 ngle = 70\\n\\n"]}
