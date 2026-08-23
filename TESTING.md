# In-game test checklist

1. Install the ZIP after SKSE, Address Library, and SkyUI.
2. Disable other spell-tome renaming mods during compatibility testing.
3. Open a merchant who stocks tomes.
4. Select **Name** and verify the order is school, then Novice through Master,
   then spell name.
5. Verify a tome for a spell the player already knows appears below ordinary
   items and all unlearned tomes.
6. Select **Value** or **Weight** and verify SkyUI's native ordering returns.
7. Switch to the Sell tab and verify its sorting remains native.
8. Check a mod-added tome and verify its school, rank, and spell name were read
   automatically.
9. Check `SmartSpellTomeSorting.log` for the successful hook message and errors.
10. Open **Magic > All** and each spell school with Name active; verify spells
    have visible compact prefixes, are grouped by school in All, and progress
    from Novice through Master (for example, `Dest. Nov. Spell: Flames`).
11. Find a vendor tome for a starting spell such as Healing and verify SkyUI
    displays its read-eye even if that physical book was never opened.
12. Favorite and unfavorite spells in a school list and verify the list remains
    ordered by rank and original spell name after each change.
