# Third-party license inventory

## Status and purpose

This document records the dependency-license decisions for the Windows-first desktop player. It is an engineering compliance baseline, not legal advice.

The repository currently bundles no Qt, libmpv, FFmpeg, codec, platform-runtime, or other third-party binaries. R0-04 pins the versions and immutable upstream identity used for acquisition, and R2-01 extends that identity baseline to the approved source-built libmpv dependency graph. Artifact SHA-256 values are recorded only after the exact source-built package exists; the actual binary manifest is verified again when the distributable package is assembled.

Distribution is prohibited if the shipped binaries, source archives, build options, notices, or license obligations cannot be matched exactly.

## Pinned dependency identities

The authoritative build-time values live in `cmake/DependencyVersions.cmake`:

| Component | Pinned identity |
|---|---|
| Qt | `6.8.3`, official MSVC 2022 64-bit open-source kit/source archive |
| mpv/libmpv | `0.41.0`, tag `v0.41.0`, commit `41f6a645068483470267271e1d09966ca3b9f413` |
| FFmpeg | `8.0.3`, ref `n8.0.3`, commit `8ae0b34901ba60a802f183ee75a250a9fc3e09a5` |
| libplacebo | `7.351.0`, ref `v7.351.0`, commit `3188549fba13bbdf3a5a98de2a38c2e71f04e21e` |
| libass | `0.17.4`, ref `0.17.4`, commit `bbb3c7f1570a4a021e52683f3fbdf74fe492ae84` |
| FreeType | `2.13.3`, ref `VER-2-13-3`, commit `42608f77f20749dd6ddc9e0536788eaad70ea4b5` |
| FriBidi | `1.0.16`, ref `v1.0.16`, commit `68162babff4f39c4e2dc164a5e825af93bda9983` |
| HarfBuzz | `10.2.0`, ref `10.2.0`, commit `7b27c8edd46c674e01dd226fa9e1aa7549f5c436` |

These values identify the approved source baseline; they do not approve an unverified prebuilt DLL or replace the release-time dependency scan.

## Selected distribution model

The first release uses the following baseline:

- the application may remain under its own license;
- Qt, libmpv, FFmpeg, and their approved dependencies are dynamically linked runtime components;
- LGPL-compatible builds are required for the open-source distribution path;
- static linking is not permitted without a separate reviewed decision that defines relinking materials and installation information;
- GPL-only, `--enable-gpl`, `--enable-nonfree`, unknown-license, and unredistributable components are release blockers;
- third-party binaries downloaded from an unverified build source are not accepted;
- every distributable build must carry a machine-readable version/hash manifest and human-readable notices that correspond to the exact shipped files.

A future Qt commercial-license path is possible only after an explicit user decision. Commercial and open-source Qt acquisition paths must not be mixed casually within the same product build history.

## Qt 6

### Approved source and license path

- Source authority: the official Qt `6.8.3` source archive and Qt Online Installer MSVC 2022 64-bit package.
- Approved application modules at the current scaffold stage: Qt Core, Qt Gui, Qt Qml, Qt Quick, and Qt Quick Controls 2.
- License route: GNU Lesser General Public License version 3 for the approved open-source Qt libraries, dynamically linked as Qt DLLs and plugins.
- Qt modules listed by Qt as GPL-only for open-source use are not approved for this product without a separate scope and licensing decision.

The current approved modules are not treated as proof of the complete shipping inventory. Exact plugins and embedded third-party components must be derived from the selected Qt package and SBOM.

### Distribution obligations

Before shipping a Qt-based package, the release process must:

1. include the applicable LGPLv3 and GPLv3 license texts supplied with Qt `6.8.3`;
2. state that the product uses Qt and identify the exact Qt version and modules shipped;
3. provide the complete corresponding Qt source for the shipped libraries, including applied modifications, through a project-controlled source archive or a legally sufficient written offer;
4. preserve the user's ability to replace or relink the dynamically linked Qt libraries and run the resulting application;
5. provide sufficient installation information when required by LGPLv3;
6. avoid EULA or technical restrictions that remove LGPL-granted reverse-engineering and modification rights for the covered Qt libraries;
7. include the notices for third-party code actually shipped inside Qt libraries or plugins, using the exact Qt-version license inventory or SPDX SBOM;
8. record any Qt source modification as a patch and make its corresponding source available.

A link only to a third-party-hosted Qt source page is not treated as the project's complete compliance archive. The project must retain control of the corresponding-source delivery mechanism for each released build.

## libmpv / mpv

### Approved source and build mode

- Source authority: the official mpv release tag `v0.41.0` at commit `41f6a645068483470267271e1d09966ca3b9f413`.
- Build ownership: project-controlled, reproducible build; arbitrary prebuilt libmpv DLLs are not accepted.
- License mode: LGPLv2.1-or-later build using Meson's `-Dgpl=false` option.
- Linkage: the application links dynamically to the libmpv DLL and generated MSVC import library.

mpv is GPLv2-or-later by default. A build without verifiable `-Dgpl=false` configuration is therefore not an approved dependency for this distribution route.

### Distribution obligations

The release record for libmpv must include:

- the exact mpv tag, commit, source archive, and source-archive SHA-256;
- `LICENSE.LGPL`, the upstream copyright file, and all license notices supplied by the selected source;
- the complete Meson configure options and build environment summary;
- a patch or clean-tree statement describing project modifications;
- the exact produced DLL/import-library hashes;
- a complete list of linked runtime and static dependencies;
- the corresponding source and build instructions needed for the covered library and any LGPL-covered dependency.

If the selected mpv feature set requires GPL-only code, the feature must be removed or a new product-license decision must be approved before implementation. It must not be enabled silently.

## FFmpeg

### Approved source and configuration

- Source authority: FFmpeg `8.0.3`, ref `n8.0.3`, commit `8ae0b34901ba60a802f183ee75a250a9fc3e09a5`.
- License route: LGPLv2.1-or-later configuration.
- Build policy for the R2 package explicitly uses `--disable-autodetect`, `--disable-gpl`, `--disable-nonfree`, `--enable-shared`, and `--disable-static`.
- Preferred linkage: separate FFmpeg DLLs dynamically linked by libmpv, so replacement and license boundaries remain explicit.

The expected runtime family may include `libavcodec`, `libavformat`, `libavutil`, `libswresample`, `libswscale`, and `libavfilter`, but this list is not a shipping manifest. Only libraries proven by the final binary dependency scan may be packaged or documented as shipped.

GPL libraries and wrappers such as x264/x265, or any component that forces FFmpeg into GPL mode, are not approved for the first-release dependency baseline. `--enable-nonfree` produces a build that FFmpeg identifies as unredistributable and is always prohibited.

### Distribution obligations

The release record for FFmpeg must include:

- exact source release and corresponding source archive hash;
- FFmpeg license and copying files from that source;
- full configure command, compiler identification, and enabled external-library list;
- `git diff` or an equivalent changes patch;
- hashes for every FFmpeg DLL shipped;
- corresponding source hosted through a project-controlled distribution channel;
- an application notice identifying FFmpeg and its applicable LGPL version;
- EULA wording that does not claim ownership of FFmpeg or restrict rights granted by its license.

Every external library compiled into or loaded by FFmpeg requires its own license review. Passing the top-level FFmpeg check does not approve its optional dependencies automatically.

## Approved R2 source-build dependencies

The user explicitly approved the following production transitive dependency set for the R2 project-controlled libmpv build. These components are built from the pinned source identities above rather than installed as prebuilt MSYS2 runtime packages:

| Component | Purpose | Current license route | R2 linkage policy |
|---|---|---|---|
| libplacebo | mpv GPU/rendering infrastructure dependency | LGPL-2.1-or-later | dynamic |
| libass | ASS/SSA subtitle renderer required by mpv | ISC | dynamic |
| FreeType | font rasterization required by libass | FreeType License route | dynamic |
| FriBidi | bidirectional text processing required by libass | LGPL-2.1-or-later | dynamic |
| HarfBuzz | text shaping required by libass | permissive HarfBuzz license route | dynamic |

R2 also uses MSYS2 CLANG64, LLVM/Clang, Meson, Ninja, pkg-config, NASM, Python, Git, and base build utilities as **build tools**. They are not automatically product runtime dependencies. If the produced DLL graph imports compiler runtime DLLs from CLANG64, those exact runtime files become package dependencies and must be captured by the generated manifest and reviewed again at the R13 binary/license gate.

The R2 source-build scripts deliberately disable optional external libraries that are not part of this approved set where practical. Submodules used by libplacebo for source generation or built-in implementation remain subject to the same final source and license inventory if their code is incorporated into a shipped DLL.

## Transitive and platform dependencies

No additional transitive dependency is approved merely because mpv, FFmpeg, libplacebo, libass, or Qt can discover it during configuration.

Before any additional dependency can enter a release build, its record must state:

- component name and purpose;
- authoritative source and exact version/commit;
- binary and source hashes;
- license identifier and complete notice text;
- whether it is linked dynamically or statically;
- whether it changes the effective license of mpv or FFmpeg;
- corresponding-source and modification-delivery obligations;
- redistribution restrictions and required attributions.

This applies to rendering, color-management, subtitle, audio, network, compression, font, image, codec and compiler-runtime libraries, as well as Qt plugins and their embedded third-party code.

The Microsoft Visual C++ runtime may be distributed only through Microsoft's permitted redistributable mechanism. Its exact packaging method and terms are verified during the Windows packaging stage.

## Required package records

Do not create empty placeholder license files. The R2 libmpv dependency package stages license files sourced from the exact source checkouts, while the final distributable `LICENSES/` payload is assembled and verified during packaging.

The distributable package must contain or provide access to:

```text
LICENSES/
├─ README.md                         # this policy and shipped-component index
├─ manifest.json                     # exact versions, hashes, linkage, source locations
├─ qt/                               # Qt license texts, notices, SBOM subset, source offer
├─ mpv/                              # mpv license, copyright, source/build record
├─ ffmpeg/                           # FFmpeg license, configure record, source/build record
└─ third-party/                      # notices for every additional shipped component
```

The installer, About dialog, download page, and EULA must use the same dependency names and license versions as the package manifest.

## Release compliance gate

A distributable build fails the license gate unless all of the following are true:

- Qt modules and plugins are from the approved `6.8.3` license path;
- Qt remains dynamically replaceable and the corresponding source is retained under project control;
- libmpv was built from mpv `v0.41.0` commit `41f6a645068483470267271e1d09966ca3b9f413` with `-Dgpl=false`;
- FFmpeg `8.0.3` was built without `--enable-gpl` and without `--enable-nonfree`;
- the source commits of FFmpeg, libplacebo, libass, FreeType, FriBidi, and HarfBuzz match `cmake/DependencyVersions.cmake`;
- binary dependency scanning matches the declared manifest;
- every shipped transitive/compiler-runtime component has an approved license record;
- exact license and notice files are included;
- corresponding source, modifications, and build instructions are available as required;
- installer/EULA terms preserve the rights required by the applicable LGPL licenses;
- codec-patent and jurisdiction-specific distribution risks have received separate review.

Copyright-license compliance does not itself grant patent rights. Codec patent exposure and regional distribution requirements remain separate release decisions.

## Atomic Task boundary

R0-03 selected the license route, authoritative source locations, linkage policy, prohibited configurations, required notices, and release gates. R0-04 pinned the original Qt/mpv/FFmpeg identity baseline. R2-01 extends the approved source identity to the actual libmpv dependency graph and builds the dependency package under `../libmpv`; R13 remains responsible for the final clean-machine binary dependency/license scan and distributable compliance payload.

No third-party runtime binary is considered release-approved by this document alone. The real R2 package must first pass its generated identity/hash manifest, and the final shipping set must pass the R13 release gate.
