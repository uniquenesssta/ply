# Third-party license inventory

This scaffold does not bundle Qt, libmpv, FFmpeg, or platform runtime binaries.

Before the first distributable build, R0-03 must record:

- the selected Qt distribution and license path;
- the exact libmpv build source and enabled options;
- the FFmpeg components included by that build;
- dynamic-linking and redistribution obligations;
- the license and notice files copied into the final package.

Do not add guessed or incomplete license texts. The packaged notices must match the binaries that are actually shipped.
