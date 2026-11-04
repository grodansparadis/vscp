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

## Why is logging opt-in in the underlying sources?

The helper and transport source files are also compiled directly by standalone
tests and other consumers. Those builds may not compile the logger or provide
spdlog headers. `VSCP_ENABLE_HELPER_LOGGING` explicitly enables calls into the
private logging interface for the `vscp_core` build; when it is not defined,
the affected diagnostics retain their stderr behavior and do not reference the
logger implementation.

The logger is built into `vscp_core`. Each separately linked copy of that
library has its own logging state. A helper-library binary that compiles its
own copy of the VSCP sources therefore has its own logger configuration as
well; configuring one binary does not configure another.

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

For configuration examples, level values, rotation behavior, and error
handling, see [Helper and TCP logging](library_reference.md#helper-and-tcp-logging).
