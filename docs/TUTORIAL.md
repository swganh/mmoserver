# The tutorial

The new-player tutorial on the Imperial space station: what the original
(pre-CU) one did, and how ours does it. Script: `data/script/tutorial/part1.lua`,
one per player, started by `Tutorial.cpp`.

## Sources

- The client's string files `string/en/newbie_tutorial/system_messages.stf`
  (prompts, by name: `part_1`, `welcome`, `movement_keyboard`, ...) and
  `newbie_convo.stf` (NPC lines, barks, conversations), read with
  `tools/client/stf.py`. The script refers to them by name; the client shows
  its own text.
- The voice-overs `sound/tut_*.snd` (pre-CU, `patch_12_00.tre`).
- Recordings of the live tutorial, for order and timing.
- Not the leaked server source (`docs/SOURCES.md`).

## Rooms

The station is building 2203318222959; each room is a cell. Room n is cell
2203318222959 + n (`tutorial:getRoom()`). From a run on the VM:

| Room | Cell | What happens | NPCs and objects |
|---|---|---|---|
| 1 | …960 | Movement, camera, chat, Holocron | 36 stormtroopers from the data (KNOWN_ISSUES #20) |
| 2 | …961 | Conversation (Imperial officer), the drum, inventory, food, toolbar | officer 47513079097 (conversation 34), drum 2533274790395904 |
| 3 | …962 | Bank and bazaar | officer 47513079099 (conversation 35), bank terminal 4294968324, bazaar terminal 4294968325 |
| 4-5 | …963-964 | Cloning and insurance | droid 47513085659 (conversation 36), cloning 4294968326, insurance 4294968327 |
| 6-7 | …965-966 | The nervous officer who hands over a weapon, the debris | 47513085665 (conversation 37) |
| 8 | …967 | The bandit | |
| 9-11 | …968-970 | Skill trainer, missions, the Quartermaster and travel | |

## Parts and status

The prompts call it "PART ONE OF NINE" etc. (`part_1` … `part_9`).

| Part | Sub-states | Status |
|---|---|---|
| 1. Movement and chat | 1-5 | on events |
| 2. Conversing with NPCs and inventory | 6-9 | on events |
| 3. Banking and shopping | 10 | old (polls) |
| 4. Cloning and insurance | 11-14 | old |
| 5. Combat moves and radar | 15-17 | old |
| 6. Combat | 18 | old |
| 7. Skill training | 19 | old |
| 8. Missions and waypoints | 20+ | old |
| 9. Bye (the Quartermaster, the oath, travel) | | old; the oath is missing |

"On events": each step checks first whether the player has already done it
(`tutorial/progress.lua` records it from script events from the start), so a
player who is ahead gets no prompt for it; reminders only come after a
pause; prompts never talk over each other. Sub-states are saved as before, so
a reconnect resumes at its step.

## Missing beats (from the client's files)

- The code of conduct oath at the end: `oath_explain`, `the_oath`,
  `make_promise`, `swear_reminder`, `tut_60` (sounds `tut_00_swear`,
  `tut_58/59_pledge`, `tut_60_thankyou`).
- The CTRL-C status screen prompts (`open_status_prompt`,
  `close_status_prompt`, ...). Whether the client reports the status screen
  is not known yet.
- The bandit's grenade (`pirate_taunt3`, `pirate_taunt4`; KNOWN_ISSUES #19)
  and the debris explosion (#5g).
- Barks never used: the trapped refugees, `shout_panic3/4`,
  `quarter_wait`, `mission_repeat`.
- Sounds never played: `tut_00_dispenser1/2`, `tut_30_dispenser`,
  `tut_31_bank`, `tut_34_proceed`, `tut_35_elevator`, `tut_39_gosee`,
  `tut_46_combattoolbar`, `tut_48_targetattribs`, `tut_00_stat_open`,
  `tut_00_statmigration`, `tut_00_keymaps`, `tut_00_keymap_2`,
  `tut_00_follow_arrow`, `tut_24_toolbarslots` and the oath sounds.
