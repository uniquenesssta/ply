# Third-party license inventory

## Status and purpose

This document records the R0-03 dependency-license decisions for the Windows-first desktop player. It is an engineering compliance baseline, not legal advice.

The repository currently bundles no Qt, libmpv, FFmpeg, codec, platform-runtime, or other third-party binaries. Exact versions and hashes are pinned in R0-04; the actual binary manifest is verified again when libmpv is introduced and when the distributable package is assembled.

Distribution is prohibited if the shipped binaries, source archives, build options, notices, or license obligations cannot be matched exactly.

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

- Source authority: the official Qt source archive and Qt Online Installer packages corresponding to the patch version pinned in R0-04.
- Release family: Qt 6.8 series unless a later Atomic Task deliberately changes the baseline.
- Approved application modules at the current scaffold stage: Qt Core, Qt Gui, Qt Qml, Qt Quick, and Qt Quick Controls 2.
- License route: GNU Lesser General Public License version 3 for the approved open-source Qt libraries, dynamically linked as Qt DLLs and plugins.
- Qt modules listed by Qt as GPL-only for open-source use are not approved for this product without a separate scope and licensing decision.

The current approved modules are not on Qt 6.8's GPL-only module list. Their exact shipped plugins and embedded third-party components must still be derived from the selected Qt package and SBOM rather than assumed from the module name.

### Distribution obligations

Before shipping a Qt-based package, the release process must:

1. include the applicable LGPLv3 and GPLv3 license texts supplied with the selected Qt distribution;
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

- Source authority: an official signed or otherwise verifiable release tag from `mpv-player/mpv`.
- Exact tag and commit: pinned in R0-04 before dependency download or compilation.
- Build ownership: project-controlled, reproducible build; arbitrary prebuilt libmpv DLLs are not accepted.
- License mode: LGPLv2.1-or-later build using Meson's `-Dgpl=false` option.
- Linkage: the application links dynamically to the libmpv DLL and import library.

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

- Source authority: an official FFmpeg release tag or commit matched to the selected mpv build.
- Exact tag and commit: pinned with the libmpv dependency record.
- License route: LGPLv2.1-or-later configuration.
- Prohibited configure options: `--enable-gpl` and `--enable-nonfree`.
- Preferred linkage: separate FFmpeg DLLs dynamically linked by libmpv, so replacement and license boundaries remain explicit.

The expected runtime family may include `libavcodec`, `libavformat`, `libavutil`, `libswresample`, `libswscale`, and `libavfilter`, but this list is not a shipping manifest. Only libraries proven by the final binary dependency scan may be packaged or documented as shipped.

GPL libraries and wrappers such as x264/x265, or any component that forces FFmpeg into GPL mode, are not approved for the first-release dependency baseline. `--enable-nonfree` produces a build that FFmpeg identifies as unredistributable and is always prohibited.

### Distribution obligations

The release record for FFmpeg must include:

- exact source tag/commit and corresponding source archive;
- FFmpeg license and copying files from that source;
- full configure command, compiler identification, and enabled external-library list;
- `git diff` or an equivalent changes patch;
- hashes for every FFmpeg DLL shipped;
- corresponding source hosted through a project-controlled distribution channel;
- an application notice identifying FFmpeg and its applicable LGPL version;
- EULA wording that does not claim ownership of FFmpeg or restrict rights granted by its license.

Every external library compiled into or loaded by FFmpeg requires its own license review. Passing the top-level FFmpeg check does not approve its optional dependencies automatically.

## Transitive and platform dependencies

No transitive dependency is approved merely because mpv, FFmpeg, or Qt can discover it during configuration.

Before a dependency can enter a release build, its record must state:

- component name and purpose;
- authoritative source and exact version/commit;
- binary and source hashes;
- license identifier and complete notice text;
- whether it is linked dynamically or statically;
- whether it changes the effective license of mpv or FFmpeg;
- corresponding-source and modification-delivery obligations;
- redistribution restrictions and required attributions.

This applies to rendering, color-management, subtitle, audio, network, compression, font, image, and codec libraries, as well as Qt plugins and their embedded third-party code.

The Microsoft Visual C++ runtime may be distributed only through Microsoft's permitted redistributable mechanism. Its exact packaging method and terms are verified during the Windows packaging stage.

## Required package records

Do not create empty placeholder license files. When the first actual runtime package is assembled, generate the `LICENSES/` payload from the selected source distributions and dependency manifest.

The package must contain or provide access to:

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

- Qt modules and plugins are from the approved license path and exact version;
- Qt remains dynamically replaceable and the corresponding source is retained under project control;
- libmpv was built from the approved official source with `-Dgpl=false`;
- FFmpeg was built without `--enable-gpl` and without `--enable-nonfree`;
- binary dependency scanning matches the declared manifest;
- every transitive component has an approved license record;
- exact license and notice files are included;
- corresponding source, modifications, and build instructions are available as required;
- installer/EULA terms preserve the rights required by the applicable LGPL licenses;
- codec-patent and jurisdiction-specific distribution risks have received separate review.

Copyright-license compliance does not itself grant patent rights. Codec patent exposure and regional distribution requirements remain separate release decisions.

## R0-03 boundary

R0-03 selects the license route, authoritative source locations, linkage policy, prohibited configurations, required notices, and release gates. R0-04 must pin exact dependency versions and hashes. R2-01 and packaging tasks must verify the real binaries against this policy.

No third-party runtime binary is approved or considered shipped by this document alone.
