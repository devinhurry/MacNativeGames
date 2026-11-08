# TouchNX integration

The feature implementations have moved to the independent `touchnx` checkout. This directory keeps C/Objective-C forwarding files for the runtime's existing include and source lists; it does not own the touch behavior anymore.

Set `TOUCHNX_ROOT` to that checkout (the default is the adjacent `../../../touchnx` from `games/totk`), or run `python3 touchnx.py build --backend macnativegames` from TouchNX. Shared touch, launcher and fullscreen behavior is maintained there; engine-specific code lives in `touchnx/adapters/macnativegames`.

The original `ORIGIN.tsv` records the pre-extraction migration snapshot, not the contents of these forwarding files. Current provenance, backend commits, integration patches and standalone checks are in TouchNX.
