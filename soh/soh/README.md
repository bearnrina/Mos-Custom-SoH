# Bank Hylia dialogue update

Uses hyliabankingdialogue(1).rtf, with the agreed grammar correction and a short unavailable-withdrawal label.

## Install

Close SoH. Save a Git checkpoint of the working bank-counter version before installing.
Extract this entire ZIP using File Explorer. Put the extracted folder at:
D:\SoH-Custom\BankHylia-dialogue-update

Run in PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "D:\SoH-Custom\BankHylia-dialogue-update\Install-BankHyliaDialogue.ps1"
```

This checks the completeness of every package source and checks installed source versions before copying. It backs up the three existing source files under BankHylia-dialogue-backups. If a source mismatch appears, attach that current source file for a merge.

Rebuild with SoH closed:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" --build "D:\SoH-Custom\build\x64" --config Release --parallel 2
```

No manual CMake configuration is needed. The counter/HUD file z_parameter.c requires no changes.

## Exact source destinations

- D:\SoH-Custom\soh\include\z64save.h
- D:\SoH-Custom\soh\soh\SaveManager.cpp
- D:\SoH-Custom\soh\src\overlays\actors\ovl_En_Hy\z_en_hy.c

## Resulting behavior

- Existing one-time introductions, recognition, and closure remain special visits, using the revised document wording.
- New customers get a Yes/No enrollment offer. No shows the decline text and repeats the introduction on the next visit. Yes is remembered when the game is saved.
- Ordinary visits start with: How do you fare, [player name]? You have [balance] Rupees saved.
- The separate choice screen asks: What do you want to do?
- Adult choices are Deposit all Rupees, Withdraw / Fill wallet, Do nothing.
- Child choices before an adult bank visit are Deposit all Rupees, Withdraw (unavailable), Do nothing. Selecting unavailable withdrawal shows the full explanatory sentence, then the farewell.
- Children cannot withdraw, including through the underlying transaction helper.
- First actual adult bank contact still permanently closes childhood deposits, even if adult enrollment is declined.
- After adult contact, a childhood visit shows the one-time closure when needed, the balance greeting, and the farewell. There is no transaction menu.
- Transfers keep their working real-time counting and sounds. Progress, completion receipt, and farewell remain separate messages. The rupee HUD stays visible through the transfer and closing receipt/farewell.
- Adult farewell: Good day, old friend! Child farewell: Good day, young sir!
- Do nothing also gives the farewell. Transaction errors lead to the farewell.
- Childhood deposits retain exact doubled adult value; adult-only deposits remain separate. The 9,999 adult-value cap and bank protection from rupee loss on death remain.
- Recognition remains available after an earlier declined childhood offer, but the doubled-savings statement is omitted when no childhood savings remain.
- Three-year wording is narrative; there is no elapsed-year timer. Adult Link can withdraw.

## Save compatibility

No new save is required. Bank section version 1 is migrated to version 2 with balances and closure state intact. Existing child/adult customers are treated as already enrolled, preserving the behavior of accounts created before enrollment choices existed. New version-2 saves preserve Yes/No enrollment separately from meeting the banker.

After saving with this update, use this updated build: the earlier build does not have a version-2 bank loader. Keep a separate copy of your game saves if you want the option to return to that earlier build.

## Verification

The extracted C functions compiled with strict warnings and passed a focused interaction harness covering child/adult enrollment, declines, repeat introductions, greetings, recognition, restricted transactions, ordinary menus, deposits/withdrawals, receipts, errors, and farewells. The extracted C++ save callbacks passed checks for all 8 old flag combinations and all 32 new flag combinations, preserving balances and snapshot values. Message layout checks passed a four-line page limit and 216-pixel width budget, including a worst-case eight-character player name. Unrelated NPC and save code, including Local Dungeon Shuffle, was checked unchanged.

Full SoH compilation, native text rendering, and in-game validation of this update remain pending.

## Gameplay checks

1. On a fresh child test save, decline enrollment. Talk again and verify the introduction repeats. Accept, save/reload, and verify the ordinary named greeting replaces the introduction.
2. Deposit as a child and check the count, sound, receipt, and young-sir farewell. Select unavailable withdrawal and verify no money moves.
3. Choose Do nothing and verify the farewell.
4. Visit as an adult. Confirm one-time recognition, doubled childhood balance, and normal adult choices. Deposit and withdraw; check receipts and old-friend farewell.
5. Return as a child. Confirm closure is shown once and no deposit/withdrawal menu appears, including on later visits.
6. On a separate adult-first test save, decline then revisit to check the introduction repeats. Accept, save/reload, and confirm enrollment persists.
7. Load an existing working bank save. Confirm its balance and closed-child-deposit status remain intact and established customers are not asked to enroll again.
8. Verify the player name, four-digit balance, long introductory pages, and choice labels render without clipping. Check full wallet, empty wallet/account, and missing-wallet messages.
