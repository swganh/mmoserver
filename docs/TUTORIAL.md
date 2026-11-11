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

| Part | Sub-states | What |
|---|---|---|
| 1. Movement and chat | 1-5 | camera, chat, Holocron, to room 2 |
| 2. Conversing with NPCs and inventory | 6-9 | the officer, the drum, food, toolbar |
| 3. Banking and shopping | 10 | the officer, bank and bazaar terminals |
| 4. Cloning and insurance | 11-14 | the ramp barks, the droid, cloning, insurance, radar, panic |
| 5. Combat moves and radar | 15-17 | the nervous refugee, the soldier's weapon (16), the debris |
| 6. Combat | 18 | the bandit and his grenade |
| 7. Skill training | 19-20 | the officer, the skill trainer |
| 8. Missions and waypoints | 21-23 | the mission terminal, the release documents (22) |
| 9. Bye | 24-27 | the Quartermaster (25), the oath (26), travel (27) |

All parts run on events (`tutorial/progress.lua`): each step checks first
whether the player has already done it, so a player who is ahead gets no
prompt for it; reminders only come after a pause; prompts never talk over
each other. The sub-states that C++ checks are gates: on every tick,
whatever the script is saying, walking into the soldier's room sets 16 (he
hands over the weapon), the mission room 22 (the release documents), and
the Quartermaster's room 25 (only with the documents, 23). The travel
terminal works from 27, after the oath. A step the player has gone past
stops waiting and skips its prompts, and the sub-state never goes back.
Sub-states are saved at every step, so a reconnect resumes there.

`src/ScriptEngine/tutorial_playthrough_unittest.cc` plays the whole script
on a fake station (`testdata/`) with a player who races ahead (no
reminders, no waiting, no NPC refusing; under 1.5 simulated minutes), one
who skips cloning, insurance and training, and one who waits for every
prompt.

## Missing beats (from the client's files)

- The `newbie_mail` prompt ("You've received a message...") after
  arrival. The welcome and profession mails themselves are sent again
  (KNOWN_ISSUES #22).

- The CTRL-C status screen prompts (`open_status_prompt`,
  `close_status_prompt`, ...). Whether the client reports the status screen
  is not known yet.
- Stat migration: `stat_open` ("...in this Tutorial, you may migrate your
  stats and the changes will take effect immediately", sound
  `tut_00_stat_open`) is never played. `stat_migration` is the Planet
  Selection Room's version, played only when the tutorial is skipped. The
  server handles the client's Stat Migration window (CTRL-C). Where the
  original played `stat_open` is to be checked against recordings; likely
  with the status screen prompts.
- Barks never used: the trapped refugees (`trapped_guy1-5`),
  `shout_panic4`, `mission_repeat`, `pirate_taunt1/2`, `nervous_guy5`.
- Sounds never played: `tut_00_dispenser1/2`, `tut_30_dispenser`,
  `tut_31_bank`, `tut_34_proceed`, `tut_35_elevator`, `tut_39_gosee`,
  `tut_46_combattoolbar`, `tut_48_targetattribs`, `tut_00_stat_open`,
  `tut_00_keymaps`, `tut_00_keymap_2`, `tut_00_follow_arrow`,
  `tut_24_toolbarslots`, `tut_59_pledge`.

## Added from the client's files, to check against recordings

- The oath (sub-state 26): `oath_explain` with `tut_00_code_of_conduct`,
  `the_oath` with `tut_58_pledge`, `make_promise` with `tut_00_swear`, then
  waiting for I PROMISE in chat (`swear_reminder` with
  `tut_00_swear_reminder` every 30 s), then `tut_60` with
  `tut_60_thankyou`. Which pledge sound goes with which line is a guess.
- The debris explodes when destroyed
  (`clienteffect/combat_explosion_lair_large.cef`).
- The bank officer's greeting is `banker_greeting` ("...if you need
  assistance with this terminal"); the old script used the cloning droid's.
- The Quartermaster's "Wait a minute there!" at the travel terminal is the
  client's `quarter_wait`, not a copy in C++.
