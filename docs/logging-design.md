# Logging design

VSCP's helper and TCP transport code uses a private, synchronous logger for
diagnostics. This design provides configurable console and rotating-file
logging without making spdlog a dependency of the public interface or
interfering with the logging configuration of an application that embeds
VSCP.

## Why use a private logger?

An application may use spdlog itself, potentially with a different version or
configuration. Relying on spdlog's process-wide default logger, registry,
global levels, thread pool, or shutdown state could change application
behavior or cause collisions. VSCP therefore owns its logger instance and
routes messages directly to it; it does not register a named logger or change
the host's default logger.

The public configuration interface is C-compatible. It exchanges ordinary
values, strings, and error codes rather than spdlog objects, C++ callbacks, or
other implementation details. Applications can configure console and
rotating-file thresholds without including spdlog headers or linking to its
types. This boundary also keeps the helper library's existing C logging API
independent of the implementation.

## Why synchronous logging?

The logger writes directly to its configured sinks. It starts no worker
threads, so applications do not need to manage a logger thread pool or
coordinate its lifetime during shutdown. Logging, configuration, flushing, and
disabling are synchronized. A slow console or file sink can delay the calling
VSCP operation, so applications should choose an appropriate log path and
consider the cost of synchronous output.

## Sharing logging with inline helpers

The logger backend lives in `vscp_util`, which is also used by lower-level
libraries that call inline functions in `vscphelper.h`. The public logging
entry points are available through both `vscp_util` and `vscp_core`, but
forward to that single backend. This avoids a dependency cycle between
`vscp_core` and the lower-level libraries while keeping the core's existing
logging symbols available.

`vscphelperlogging-forward.cpp` implements the public C logging functions as
thin forwarders to the shared backend functions. It is compiled into both
libraries: `vscp_util` also contains the backend implementation, while
`vscp_core` contains only the forwarders. This keeps the public logging API
available to consumers of either library without creating a second logger
instance or duplicating its configuration state.

Standalone builds that use these inline functions without linking `vscp_util`
can define `VSCP_DISABLE_HELPER_LOGGING` consistently for their translation
units. Their inline debug traces are disabled; standalone helper failures
are reported to stderr. Direct builds of the TCP source enable
shared logging with `VSCP_ENABLE_HELPER_LOGGING` and the logger library;
otherwise their errors continue to use stderr.

Each independently linked copy of `vscp_util` owns its own logging state, so
configuring one copy does not configure another.

## Configuration and diagnostics

Use the C-compatible declarations in `vscphelperlogging.h` to configure the
console threshold, file threshold, and optional rotating file. By default,
the existing TCP transport errors continue to use stderr. Invalid
configuration returns a VSCP error code and leaves the previous logger
configuration active. Logging failures are reported to stderr and can be
retrieved from the calling thread with `vscphelper_getLastLoggingError()`.
Explicit flush and disable functions are available for applications that need
to control logger lifetime.

The logger records diagnostics routed by the integrated helper and transport
sources. It does not capture arbitrary stderr or stdout output from the
application, external drivers, or other dependencies.
Function traces contain names and relevant status codes, not arguments or
event data. Transport error diagnostics can include hostnames and TLS paths.

For configuration examples, level values, rotation behavior, and error
handling, see [Helper and TCP logging](library_reference.md#helper-and-tcp-logging).
