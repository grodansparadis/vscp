"""Run the real Mongoose TCP/TLS client against ephemeral loopback servers."""

import contextlib
import binascii
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
    @staticmethod
    def packet(frame):
        return len(frame).to_bytes(2, "big") + frame

    def binary(self, connection, pending, scenario):
        if scenario == b"reject":
            connection.sendall(b"-OK - Binary mode not supported\r\n")
            return False
        if scenario == b"silent":
            time.sleep(1)
            return True
        sample = b"\xe0\x00\x00\r\n\x00"
        sample += binascii.crc_hqx(sample[1:], 0xffff).to_bytes(2, "big")
        if scenario == b"coalesced":
            connection.sendall(b"+OK - Binary mode enabled\r\n" + self.packet(sample) * 2)
        elif scenario in (b"partial", b"partial-body"):
            split = 1 if scenario == b"partial" else 5
            connection.sendall(b"+OK\r\n" + self.packet(sample)[:split])
            time.sleep(0.15)
            connection.sendall(self.packet(sample)[split:])
        elif scenario == b"malformed-response":
            connection.sendall(b"UNKNOWN\r\n")
        elif scenario == b"zero":
            connection.sendall(b"+OK\r\n\x00\x00")
        elif scenario == b"truncated":
            connection.sendall(b"+OK\r\n\x00\x08\xe0\x00")
            return True
        elif scenario == b"bad-event":
            connection.sendall(b"+OK\r\n" + self.packet(b"\x10" + b"\x00" * 37))
        elif scenario == b"short-event":
            connection.sendall(b"+OK\r\n" + self.packet(b"\x10\x00"))
        elif scenario == b"oversized-event":
            event = bytearray(38)
            event[0] = 0x10
            event[34:36] = b"\xff\xff"
            event[-2:] = binascii.crc_hqx(event[1:-2], 0xffff).to_bytes(2, "big")
            connection.sendall(b"+OK\r\n" + self.packet(event))
        elif scenario == b"legacy-event":
            event = bytearray(40)
            event[3:7] = (123456).to_bytes(4, "big")
            event[7:14] = bytes((7, 234, 10, 15, 12, 34, 56))
            event[15] = 10
            event[17] = 6
            event[33] = 27
            event[35] = 2
            event[36:38] = b"\x00\xff"
            event[-2:] = binascii.crc_hqx(event[1:-2], 0xffff).to_bytes(2, "big")
            connection.sendall(b"+OK\r\n" + self.packet(event))
        elif scenario == b"error-confirmation":
            confirmation = b"\xf0\xff\xff\x00\x22" + b"\x00" * 6
            confirmation += binascii.crc_hqx(confirmation[1:], 0xffff).to_bytes(2, "big")
            connection.sendall(b"+OK\r\n" + self.packet(confirmation))
        else:
            connection.sendall(b"+OK - Binary mode enabled\r\n")

        while True:
            while len(pending) < 2:
                data = connection.recv(8192)
                if not data:
                    return True
                pending += data
            length = int.from_bytes(pending[:2], "big")
            while len(pending) < length + 2:
                data = connection.recv(8192)
                if not data:
                    return True
                pending += data
            frame, pending = pending[2:length + 2], pending[length + 2:]
            if (frame[0] & 0x0f) == 0 and frame[0] >> 4 in (0, 1):
                if binascii.crc_hqx(frame[1:], 0xffff):
                    raise ValueError("Invalid event CRC received from client")
                confirmation = b"\xf0\xff\xff\x00\x00\x00\x00" + frame[1:3] + frame[-2:]
                confirmation += binascii.crc_hqx(confirmation[1:], 0xffff).to_bytes(2, "big")
                connection.sendall(self.packet(confirmation) + self.packet(frame))
            elif frame[0] == 0xf0 and frame[1:3] == b"\xff\xff":
                if binascii.crc_hqx(frame[1:], 0xffff):
                    raise ValueError("Invalid event confirmation CRC")
            else:
                # Echo raw frames unchanged, including encrypted payloads.
                for fragment in (self.packet(frame)[:1], self.packet(frame)[1:4],
                                 self.packet(frame)[4:]):
                    connection.sendall(fragment)
                    time.sleep(0.01)

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
                scenario = b""
                while True:
                    data = connection.recv(8192)
                    if not data:
                        return
                    pending += data
                    while b"\n" in pending:
                        line, pending = pending.split(b"\n", 1)
                        command = line.strip().split(b" ", 1)[0].upper()
                        if command == b"SCENARIO":
                            scenario = line.strip().split(b" ", 1)[1]
                            connection.sendall(b"+OK\r\n")
                            continue
                        if command == b"BINARY":
                            if self.binary(connection, pending, scenario):
                                return
                            pending = b""
                            continue
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
        empty_ca_dir = root / "empty-ca-dir"
        empty_ca_dir.mkdir()

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
                           VSCP_TEST_KEY=str(encrypted_key), SSL_CERT_FILE=str(ca),
                           SSL_CERT_DIR=str(ca_dir))
        executable = Path(sys.argv[1]).resolve(strict=True)
        if executable.name not in ("unittest_tcp_transport", "unittest_tcp_transport.exe"):
            raise ValueError("Expected the CMake-built unittest_tcp_transport executable")
        if not executable.is_file() or not os.access(executable, os.X_OK):
            raise ValueError("The transport test executable is not runnable")
        result = subprocess.run([str(executable)], env=environment, timeout=60)
        if result.returncode:
            return result.returncode
        # A present CA file must not shadow trust anchors supplied only by a directory.
        directory_environment = dict(environment, SSL_CERT_FILE=str(wrong_ca),
                                     SSL_CERT_DIR=os.pathsep.join((str(empty_ca_dir), str(ca_dir))))
        result = subprocess.run([str(executable), "--gtest_filter=TcpTransport.DefaultTrustPaths"],
                                env=directory_environment, timeout=60)
        return result.returncode


if __name__ == "__main__":
    sys.exit(main())
