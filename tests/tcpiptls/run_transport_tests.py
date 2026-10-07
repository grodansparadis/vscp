"""Run the real Mongoose TCP/TLS client against ephemeral loopback servers."""

import contextlib
import os
from pathlib import Path
import shutil
import socketserver
import ssl
import subprocess
import sys
import tempfile
import threading
import time


def openssl(*args):
    executable = shutil.which("openssl")
    if executable is None:
        raise FileNotFoundError("The openssl command is required for TLS integration tests")
    subprocess.run([str(Path(executable).resolve()), *map(str, args)], check=True, capture_output=True)


class Handler(socketserver.BaseRequestHandler):
    def handle(self):
        connection = self.request
        connection.settimeout(3)
        try:
            if self.server.stall:
                time.sleep(2)
                return
            if self.server.context:
                connection = self.server.context.wrap_socket(connection, server_side=True)
            with connection:
                connection.sendall(b"+OK - VSCP test server\r\n")
                pending = b""
                while True:
                    data = connection.recv(8192)
                    if not data:
                        return
                    pending += data
                    while b"\n" in pending:
                        line, pending = pending.split(b"\n", 1)
                        command = line.strip().split(b" ", 1)[0].upper()
                        if command == b"DROP":
                            return
                        if command == b"SILENT":
                            continue
                        if command == b"VERS":
                            connection.sendall(b"26,10,1\r\n+OK\r\n")
                        elif command == b"RCVLOOP":
                            connection.sendall(b"+OK\r\n")
                            time.sleep(0.02)
                            connection.sendall(b"0,10,6,0,2026-10-07T00:00:00,0,-,1,2,3\r\n")
                        else:
                            reply = b"+ERR - rejected\r\n" if command == b"ERROR" else b"+OK\r\n"
                            for fragment in (reply[:2], reply[2:-1], reply[-1:]):
                                connection.sendall(fragment)
                                time.sleep(0.01)
                        if command == b"QUIT":
                            return
        except (ssl.SSLError, ConnectionError, TimeoutError):
            # Rejected handshakes and client timeout tests deliberately disconnect.
            return


class Server(socketserver.ThreadingTCPServer):
    daemon_threads = True

    def __init__(self, context=None, stall=False):
        self.context = context
        self.stall = stall
        super().__init__(("127.0.0.1", 0), Handler)
        self.thread = threading.Thread(target=self.serve_forever, daemon=True)
        self.thread.start()

    def close(self):
        self.shutdown()
        self.server_close()
        self.thread.join()

    def endpoint(self, secure=False):
        return f"{'stcp' if secure else 'tcp'}://localhost:{self.server_address[1]}"


def main():
    with tempfile.TemporaryDirectory(prefix="vscp-tls-") as temporary, contextlib.ExitStack() as stack:
        root = Path(temporary)
        ca = root / "ca.pem"
        ca_key = root / "ca.key"
        wrong_ca = root / "wrong-ca.pem"
        server_cert = root / "server.pem"
        server_key = root / "server.key"
        client_cert = root / "client.pem"
        client_key = root / "client.key"
        openssl("req", "-x509", "-newkey", "rsa:2048", "-nodes", "-days", "1",
                "-subj", "/CN=VSCP test CA", "-keyout", ca_key, "-out", ca)
        openssl("req", "-x509", "-newkey", "rsa:2048", "-nodes", "-days", "1",
                "-subj", "/CN=Untrusted test CA", "-keyout", root / "wrong.key", "-out", wrong_ca)
        extensions = root / "extensions"
        extensions.write_text("subjectAltName=DNS:localhost\nextendedKeyUsage=serverAuth,clientAuth\n")
        for name, cert, key in (("localhost", server_cert, server_key), ("client", client_cert, client_key)):
            csr = root / f"{name}.csr"
            openssl("req", "-newkey", "rsa:2048", "-nodes", "-subj", f"/CN={name}",
                    "-keyout", key, "-out", csr)
            openssl("x509", "-req", "-in", csr, "-CA", ca, "-CAkey", ca_key,
                    "-CAcreateserial", "-days", "1", "-extfile", extensions, "-out", cert)
        encrypted_key = root / "encrypted.key"
        openssl("pkey", "-in", client_key, "-aes-256-cbc", "-passout", "pass:test-password",
                "-out", encrypted_key)
        ca_dir = root / "ca-dir"
        ca_dir.mkdir()
        (ca_dir / "ca.pem").write_bytes(ca.read_bytes())

        tls = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        tls.load_cert_chain(server_cert, server_key)
        untrusted = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        untrusted.load_cert_chain(wrong_ca, root / "wrong.key")
        mtls = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        mtls.load_cert_chain(server_cert, server_key)
        mtls.load_verify_locations(ca)
        mtls.verify_mode = ssl.CERT_REQUIRED
        servers = [Server(), Server(tls), Server(mtls), Server(stall=True), Server(untrusted)]
        for server in servers:
            stack.callback(server.close)
        environment = dict(os.environ, VSCP_TEST_PLAIN=servers[0].endpoint(),
                           VSCP_TEST_TLS=servers[1].endpoint(True),
                           VSCP_TEST_MTLS=servers[2].endpoint(True),
                           VSCP_TEST_STALL=servers[3].endpoint(True),
                           VSCP_TEST_UNTRUSTED=servers[4].endpoint(True),
                           VSCP_TEST_CA=str(ca), VSCP_TEST_WRONG_CA=str(wrong_ca),
                           VSCP_TEST_CA_DIR=str(ca_dir), VSCP_TEST_CERT=str(client_cert),
                           VSCP_TEST_KEY=str(encrypted_key), SSL_CERT_FILE=str(ca))
        executable = Path(sys.argv[1]).resolve(strict=True)
        if executable.name not in ("unittest_tcp_transport", "unittest_tcp_transport.exe"):
            raise ValueError("Expected the CMake-built unittest_tcp_transport executable")
        if not executable.is_file() or not os.access(executable, os.X_OK):
            raise ValueError("The transport test executable is not runnable")
        result = subprocess.run([str(executable)], env=environment, timeout=60)
        return result.returncode


if __name__ == "__main__":
    sys.exit(main())
