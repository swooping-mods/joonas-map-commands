# J.O.O.N.A.S. Map Commands

<img src="media/icon.png" alt="J.O.O.N.A.S. Map Commands icon" width="128" height="128">

Shared map controls: select robots, move, assign sectors, stop, resume and roam.


<!-- source-only-sync -->
## Current source candidate

Source version **0.3.0** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.3.0** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.
<!-- /source-only-sync -->

## Install

Install [Moon Sweepers Mod Loader](https://github.com/swooping-mods/moon-sweepers-mod-loader) first.

Download [joonas-map-commands-0.3.0.msmod](content/joonas-map-commands-0.3.0.msmod), then drag and drop the `.msmod` file onto the [Moon Sweepers Mod Loader](https://github.com/swooping-mods/moon-sweepers-mod-loader) window. You can also use **Add mods…** (older loader builds: **Install mod package**).

Select the mod and its required frameworks, click **Prepare files**, then **Launch**.

Required mods: [map-control-framework](https://github.com/swooping-mods/map-control-framework).

## Compatibility

Version 0.3.0. Targets the pinned Steam Windows x64 IL2CPP build of Moon Sweepers (Steam app 5335510). On Linux, the game runs through Proton. Experimental: back up saves before using save-affecting mods.

## Build

Run `python3 build.py --check` to verify source hashes, then `python3 build.py` to write a build under `.local/build/`. Requires installed Clang, lld-link and llvm-dlltool.

## Credits

See [CREDITS.txt](CREDITS.txt) and [third-party notices](vendored/THIRD_PARTY_NOTICES.md).
