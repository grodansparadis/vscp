"""Run the TCP/TLS client against ephemeral loopback servers."""

import contextlib
import hashlib
import hmac
import binascii
import os
from pathlib import Path
import shutil
import socket
import socketserver
import ssl
import struct
import subprocess
import sys
import tempfile
import threading

try:
    from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
    HAVE_AES = True
except ImportError:  # Authenticated binary tests are skipped without it
    HAVE_AES = False
import time


def openssl(*args):
    executable = shutil.which("openssl")
    if executable is None:
        raise FileNotFoundError("The openssl command is required for TLS integration tests")
    subprocess.run([str(Path(executable).resolve()), *map(str, args)], check=True, capture_output=True)


# Authenticated binary frames (version 1, AES-CBC + HMAC-SHA-256), written
# independently of the C++ code with the test keys from
# docs/vscpremotetcpif-binary.md. Used by the "wrap-auth*" scenarios.
AUTH_AES_KEY = bytes(range(32))
AUTH_HMAC_KEY = bytes(range(0x20, 0x40))
AUTH_ALGORITHM = 3  # AES-256


def v1_tag(frame):
    word = (1 << 13) | (len(frame) + 16)
    digest = hmac.new(AUTH_HMAC_KEY, b"\x55" + word.to_bytes(2, "big") + frame, hashlib.sha256).digest()
    return digest[:16]


def v1_encrypt(plain):
    body = plain[1:]
    pad = 16 - len(body) % 16
    body += bytes([pad]) * pad
    iv = os.urandom(16)
    encryptor = Cipher(algorithms.AES(AUTH_AES_KEY), modes.CBC(iv)).encryptor()
    frame = bytes([(plain[0] & 0xf0) | AUTH_ALGORITHM]) + encryptor.update(body) + encryptor.finalize() + iv
    return frame + v1_tag(frame)


def v1_decrypt(inner):
    if len(inner) < 49 or (len(inner) - 33) % 16 or (inner[0] & 0x0f) != AUTH_ALGORITHM:
        raise ValueError("Not an authenticated frame")
    frame, tag = inner[:-16], inner[-16:]
    if not hmac.compare_digest(v1_tag(frame), tag):
        raise ValueError("Bad authentication tag")
    decryptor = Cipher(algorithms.AES(AUTH_AES_KEY), modes.CBC(frame[-16:])).decryptor()
    body = decryptor.update(frame[1:-16]) + decryptor.finalize()
    pad = body[-1]
    if not 1 <= pad <= 16 or body[-pad:] != bytes([pad]) * pad:
        raise ValueError("Bad padding")
    plain = bytes([frame[0] & 0xf0]) + body[:-pad]
    if binascii.crc_hqx(plain[1:], 0xffff):
        raise ValueError("Bad CRC in authenticated frame")
    return plain


class Handler(socketserver.BaseRequestHandler):
    @staticmethod
    def packet(frame):
        return len(frame).to_bytes(2, "big") + frame

    @staticmethod
    def binary_reply(code, error=0, payload=b""):
        frame = b"\xf0" + code.to_bytes(2, "big") + error.to_bytes(2, "big") + payload
        return frame + binascii.crc_hqx(frame[1:], 0xffff).to_bytes(2, "big")

    @staticmethod
    def binary_event():
        event = bytearray(41)
        event[0] = 0x10
        event[3:11] = (123456789).to_bytes(8, "big")
        event[14:16] = (10).to_bytes(2, "big")
        event[16:18] = (6).to_bytes(2, "big")
        event[33] = 27
        event[34:36] = (3).to_bytes(2, "big")
        event[36:39] = b"\x01\x02\x03"
        event[-2:] = binascii.crc_hqx(event[1:-2], 0xffff).to_bytes(2, "big")
        return bytes(event)

    def binary_client(self, connection, pending, scenario):
        server = self.server
        if not isinstance(server, Server):
            raise TypeError("TCP binary handler requires a test server")
        peer = (connection, threading.Lock())
        auth = scenario.startswith(b"wrap-auth")
        events = [self.binary_event()]
        if scenario in (b"wrap-oversized-event", b"wrap-retr-oversized"):
            event = bytearray(events[0][:36] + b"\x00" * 513 + b"\x00\x00")
            event[34:36] = (513).to_bytes(2, "big")
            event[-2:] = binascii.crc_hqx(event[1:-2], 0xffff).to_bytes(2, "big")
            events[0] = bytes(event)

        def send(peer, *frames):
            if auth:
                frames = [v1_encrypt(frame) for frame in frames]
            with peer[1]:
                peer[0].sendall(b"".join(self.packet(frame) for frame in frames))

        connection.sendall(b"+OK - Binary mode enabled\r\n")
        try:
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
                if not frame:
                    raise ValueError("Empty binary frame received from client")
                if auth:
                    # Unauthenticated or forged frames end the connection
                    frame = v1_decrypt(frame)

                if frame[0] & 0x0f:
                    send(peer, frame)
                elif frame[0] >> 4 in (0, 1):
                    if binascii.crc_hqx(frame[1:], 0xffff):
                        raise ValueError("Invalid event CRC received from client")
                    error = 34 if scenario == b"wrap-confirm-error" else 0
                    confirmation = self.binary_reply(0xffff, error, b"\x00\x00" + frame[1:3] + frame[-2:])
                    send(peer, confirmation)
                    with server.binary_lock:
                        recipients = list(server.binary_open_peers)
                    if recipients:
                        for recipient in recipients:
                            try:
                                send(recipient, frame)
                            except (ConnectionError, TimeoutError, ssl.SSLError):
                                with server.binary_lock:
                                    if recipient in server.binary_open_peers:
                                        server.binary_open_peers.remove(recipient)
                    else:
                        events.append(frame)
                elif frame[0] == 0xf0 and frame[1:3] == b"\xff\xff":
                    if binascii.crc_hqx(frame[1:], 0xffff):
                        raise ValueError("Invalid event confirmation CRC")
                elif frame[0] == 0xe0 and len(frame) >= 5:
                    if binascii.crc_hqx(frame[1:], 0xffff):
                        raise ValueError("Invalid binary command CRC")
                    code = int.from_bytes(frame[1:3], "big")
                    if code == 7:
                        if scenario == b"wrap-open-timeout":
                            time.sleep(0.7)
                            return True
                        if scenario == b"wrap-open-reject":
                            send(peer, self.binary_reply(code, 17))
                            continue
                        if scenario == b"wrap-open-malformed":
                            reply = bytearray(self.binary_reply(code))
                            reply[-1] ^= 1
                            send(peer, reply)
                            continue
                        with server.binary_lock:
                            server.binary_open_peers.append(peer)
                        event = events.pop(0)
                        if scenario == b"wrap-bad-event":
                            event = event[:-1] + bytes((event[-1] ^ 1,))
                        if scenario == b"wrap-open-early":
                            send(peer, event, self.binary_reply(code))
                        else:
                            send(peer, self.binary_reply(code), event)
                        if scenario == b"wrap-close":
                            return True
                    elif code == 6:
                        if scenario == b"wrap-retr-delay":
                            time.sleep(0.2)
                        if scenario == b"wrap-retr-reject":
                            reply = self.binary_reply(code, 17)
                        elif scenario == b"wrap-retr-short":
                            reply = self.binary_reply(code, payload=b"\x00" * 10)
                        elif scenario == b"wrap-retr-empty-success":
                            reply = self.binary_reply(code)
                        elif events:
                            event = events.pop(0)
                            if scenario == b"wrap-retr-separate":
                                send(peer, event)
                                reply = self.binary_reply(code)
                            else:
                                reply = self.binary_reply(code, payload=event[1:-2])
                        else:
                            reply = self.binary_reply(code, 19)
                        if scenario == b"wrap-retr-malformed":
                            reply = reply[:-1] + bytes((reply[-1] ^ 1,))
                        send(peer, reply)
                    elif code == 10:
                        if scenario == b"wrap-clear-delay":
                            time.sleep(0.2)
                        if scenario == b"wrap-clear-timeout":
                            time.sleep(0.7)
                            return True
                        error = 17 if scenario == b"wrap-clear-reject" else 0
                        reply = self.binary_reply(code, error)
                        if scenario == b"wrap-clear-malformed":
                            reply = reply[:-1] + bytes((reply[-1] ^ 1,))
                        if not error:
                            events.clear()
                        send(peer, reply)
                    else:
                        send(peer, self.binary_reply(code, payload=frame[3:-2]))
                else:
                    send(peer, frame)
        finally:
            with server.binary_lock:
                if peer in server.binary_open_peers:
                    server.binary_open_peers.remove(peer)

    def binary(self, connection, pending, scenario):
        if scenario.startswith(b"wrap-"):
            return self.binary_client(connection, pending, scenario)
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
        elif scenario == b"hold":
            connection.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 4096)
            connection.sendall(b"+OK\r\n")
            time.sleep(2)
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
        server = self.server
        if not isinstance(server, Server):
            raise TypeError("TCP transport handler requires a test server")
        connection = self.request
        connection.settimeout(3)
        acquired = False
        if server.single_client:
            acquired = server.client_slot.acquire(blocking=False)
            if not acquired:
                return
        try:
            if server.stall:
                time.sleep(2)
                return
            if server.context:
                connection = server.context.wrap_socket(connection, server_side=True)
            with connection:
                connection.sendall(b"+OK - VSCP test server\r\n")
                pending = b""
                scenario = b""
                receive_loop = False
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
                        if command == b"USER":
                            user = line.strip().split(b" ", 1)[-1]
                            if user.startswith(b"wrap-"):
                                scenario = user
                        if command == b"BINARY":
                            if self.binary(connection, pending, scenario):
                                return
                            pending = b""
                            continue
                        if command == b"DROP":
                            return
                        if command == b"RESET":
                            connection.sendall(b"+OK\r\n")
                            time.sleep(0.1)
                            linger = struct.pack("HH" if os.name == "nt" else "ii", 1, 0)
                            connection.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, linger)
                            return
                        if command == b"SILENT":
                            continue
                        if command == b"VERS":
                            connection.sendall(b"26,10,1\r\n+OK\r\n")
                        elif command == b"CHID":
                            connection.sendall(b"1\r\n+OK\r\n")
                        elif command in (b"RETR", b"OTHER") and scenario == b"empty":
                            connection.sendall(b"-OK - No event(s) available\r\n")
                        elif command == b"RETR" and scenario == b"reject-retr":
                            connection.sendall(b"-OK - No event(s) available\r\n+ERR - rejected\r\n")
                        elif command == b"RETR":
                            connection.sendall(b"-OK - No event(s) available\r\n")
                        elif command == b"RCVLOOP":
                            if server.reject_receive_loop:
                                connection.sendall(b"+ERR - Receive loop rejected\r\n")
                            else:
                                receive_loop = True
                                event = b"0,10,6,0,2026-10-07T00:00:00,0,-,1,2,3\r\n"
                                if scenario == b"oversized-text":
                                    event = b"0,10,6,0,,0,-" + b",1" * 513 + b"\r\n"
                                if server.coalesced_receive:
                                    connection.sendall(b"+OK\r\n" + event)
                                else:
                                    connection.sendall(b"+OK\r\n")
                                    time.sleep(0.02)
                                    connection.sendall(event)
                        elif command == b"QUITLOOP":
                            receive_loop = False
                            connection.sendall(b"+OK - Success\r\n")
                        elif receive_loop and command == b"SEND":
                            if server.bare_receive_ack:
                                connection.sendall(b"+OK\r\n")
                            else:
                                connection.sendall(b"+OK\r\n+OK - Success\r\n")
                                connection.sendall(b"0,10,6,0,2026-10-07T00:00:01,0,-,4,5,6\r\n")
                        elif receive_loop and command in (b"NOOP", b"CLRA"):
                            connection.sendall(b"+OK\r\n+OK - Success\r\n")
                        elif command == b"QUIT":
                            if acquired:
                                server.client_slot.release()
                                acquired = False
                            connection.sendall(b"+OK\r\n")
                            return
                        else:
                            rejected = command == b"ERROR" or (command == b"NOOP" and server.reject_noop)
                            reply = b"+ERR - rejected\r\n" if rejected else b"+OK\r\n"
                            for fragment in (reply[:2], reply[2:-1], reply[-1:]):
                                connection.sendall(fragment)
                                time.sleep(0.01)
        except (ssl.SSLError, ConnectionError, TimeoutError):
            # Rejected handshakes and client timeout tests deliberately disconnect.
            return
        finally:
            if acquired:
                server.client_slot.release()


class Server(socketserver.ThreadingTCPServer):
    daemon_threads = True

    def __init__(self, context=None, stall=False, ipv6=False, reject_noop=False,
                 single_client=False, reject_receive_loop=False, bare_receive_ack=False,
                 coalesced_receive=False):
        self.context = context
        self.stall = stall
        self.reject_noop = reject_noop
        self.single_client = single_client
        self.reject_receive_loop = reject_receive_loop
        self.bare_receive_ack = bare_receive_ack
        self.coalesced_receive = coalesced_receive
        self.client_slot = threading.Lock()
        self.binary_lock = threading.Lock()
        self.binary_open_peers = []
        self.address_family = socket.AF_INET6 if ipv6 else socket.AF_INET
        super().__init__(("::1" if ipv6 else "127.0.0.1", 0), Handler)
        self.thread = threading.Thread(target=self.serve_forever, daemon=True)
        self.thread.start()

    def close(self):
        self.shutdown()
        self.server_close()
        self.thread.join()

    def endpoint(self, secure=False):
        host = "[::1]" if self.address_family == socket.AF_INET6 else "localhost"
        return f"{'stcp' if secure else 'tcp'}://{host}:{self.server_address[1]}"


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
        single = Server(single_client=True)
        single_tls = Server(tls, single_client=True, coalesced_receive=True)
        rejected_loop = Server(reject_receive_loop=True)
        bare_ack = Server(bare_receive_ack=True)
        servers.extend((single, single_tls, rejected_loop, bare_ack))
        for server in servers:
            stack.callback(server.close)
        ipv6_server = None
        if socket.has_ipv6:
            try:
                ipv6_server = Server(ipv6=True)
                stack.callback(ipv6_server.close)
            except OSError as error:
                print(f"IPv6 loopback unavailable: {error}", file=sys.stderr)
        environment = dict(os.environ, VSCP_TEST_PLAIN=servers[0].endpoint(),
                           VSCP_TEST_TLS=servers[1].endpoint(True),
                           VSCP_TEST_MTLS=servers[2].endpoint(True),
                           VSCP_TEST_STALL=servers[3].endpoint(True),
                           VSCP_TEST_UNTRUSTED=servers[4].endpoint(True),
                           VSCP_TEST_SINGLE=single.endpoint(),
                           VSCP_TEST_SINGLE_TLS=single_tls.endpoint(True),
                           VSCP_TEST_REJECT_LOOP=rejected_loop.endpoint(),
                           VSCP_TEST_BARE_ACK=bare_ack.endpoint(),
                           VSCP_TEST_CA=str(ca), VSCP_TEST_WRONG_CA=str(wrong_ca),
                           VSCP_TEST_CA_DIR=str(ca_dir), VSCP_TEST_CERT=str(client_cert),
                           VSCP_TEST_KEY=str(encrypted_key), SSL_CERT_FILE=str(ca),
                           SSL_CERT_DIR=str(ca_dir))
        if HAVE_AES:
            environment["VSCP_TEST_AUTH"] = "1"
        else:
            environment.pop("VSCP_TEST_AUTH", None)
        if ipv6_server is not None:
            environment["VSCP_TEST_IPV6"] = ipv6_server.endpoint()
        else:
            environment.pop("VSCP_TEST_IPV6", None)
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
