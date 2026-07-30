# AGENTS.md

This directory contains the vendored VSCP protocol and client implementation used by VSCP Works. Treat it as a compatibility-sensitive library subtree rather than a place for broad architectural rewrites.

## Working guidelines
- Prefer small, targeted changes that preserve existing behavior and interfaces.
- Keep changes compatible with the surrounding C++ code and the upstream VSCP conventions used in this tree.
- When touching shared protocol/client code, check nearby implementations in the same subtree before introducing a new pattern.
- Avoid introducing new dependencies or large refactors unless they are clearly required.

## Useful context
- The application entry point and UI live in [src](../../src), while this subtree provides the protocol/backend implementation.
- Build and validation should still follow the repository root instructions in [../../AGENTS.md](../../AGENTS.md).
- The VSCP specification is available at https://grodansparadis.github.io/vscp-doc-spec/#/.
