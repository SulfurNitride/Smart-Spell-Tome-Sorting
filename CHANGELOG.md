# Changelog

## 1.0.0

- Replace the full TOML dependency with a small parser for the five boolean
  settings, substantially reducing the DLL size.
- Consolidate duplicated SkyUI sort-array setup and remove redundant runtime
  diagnostics.
- Harden the Magic-list callback wrapper so SkyUI's original callback still
  runs if row preprocessing is unavailable.
- Simplify versioned archive creation and remove the README.
- Add a reproducible GitHub Actions build that tests, packages, verifies, and
  uploads the Nexus-ready archive and checksum.

## 0.2.5

- Prepare Magic-menu spell rows inside SkyUI's list invalidation callback so
  native names and order never reach the renderer during favorite changes.
- Keep the interception DLL-only; no replacement Magic Menu SWF is required.

## 0.2.4

- Redraw Magic-menu rows synchronously after favorite changes, preventing the
  brief flash of native names and unsorted order.

## 0.2.3

- Reapply Magic-menu sorting after favoriting or unfavoriting a spell.
- Trigger a single SkyUI list invalidation only when row metadata or favorite
  state changes.

## 0.2.2

- Add configurable visible school and rank prefixes to ordinary Magic menu
  spells, enabled by default.

## 0.2.1

- Set SkyUI's row-level read fields for known-spell tomes instead of changing
  the underlying BOOK record.
- Wait for Magic menu form IDs before preparing sort metadata and refresh the
  list when late-arriving entries become ready.

## 0.2.0

- Add an optional Magic menu school, rank, and name sort for known spells.
- Treat known-spell tomes as read in memory so SkyUI shows its eye,
  including for starting spells whose tome was never physically read.
- Add a TOML configuration for compact naming, vendor sorting, Magic menu
  sorting, and known-tome read indicators.

## 0.1.1

- Apply smart sorting to every vendor category using plain alphabetical Name
  sorting instead of requiring SkyUI's standard Books category flag.
- Detect Name sorting directly from SkyUI's active sort attributes for better
  compatibility with customized inventory layouts.
- Add one-time log messages confirming that the menu hook and sorter are active.

## 0.1.0

- Initial implementation.
- Compact runtime-generated spell-tome labels.
- Automatic school, rank, spell-name, and learned-state sorting in the vendor
  Books list.
- Zero ESP/ESL and zero SWF files.
