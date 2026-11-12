# J.O.O.N.A.S. Map Commands: behavior and implementation

Describes the retained payload **0.3.0**. The behavior prose describes the retained payload; current candidate source and metadata are identified below. Gameplay qualification is a separate requirement.


<!-- source-only-sync -->
## Current source candidate

Source version **0.3.0** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.3.0** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.

The authored behavior and lifecycle descriptions below are retained from the published payload. They are not new qualification evidence for the source candidate. The integration table below follows the current candidate metadata.
<!-- /source-only-sync -->

## Behavior

Adds a J.O.O.N.A.S. row to Map Control Framework’s native top section. Select a loaded dock-backed robot by stable dock ID. P moves to a native right-click ground pick; G replaces its work sector; S stops, C resumes and R enables roaming. Move normalizes the picked direction onto the moon, invokes native PlanRoute and sets native target/Travel state; native robot logic owns subsequent movement.

## State and lifecycle

Discovery scans at most 256 docks and offers up to 64 selectable robots. Host commands revalidate world, loaded dock and input context, update native power/work-area revision and request normal saves. Selected rows and retained map pick are session state. Map/world changes drop UI/robot roots; Map Control Framework owns pointer capture and layout.

## Integration and configuration

Runtime dependencies: [map-control-framework](https://github.com/swooping-mods/map-control-framework).

The [module metadata](../source/mods/joonas-map-commands/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

| Service | ABI | Table size |
|---|---:|---|
| Requires `map-control-framework.map-controls` | 1 | at least 32 bytes |

## Implementation sources

- [mod.c](../source/mods/joonas-map-commands/mod.c)
- [runtime.h](../source/mods/joonas-map-commands/runtime.h)
- [store.h](../source/mods/joonas-map-commands/store.h)

## Verification boundary

No route queue, custom pathfinder or far/unloaded robot command channel. Ground-height semantics, actual route completion, work-area persistence and guest/co-op behavior require game qualification.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
