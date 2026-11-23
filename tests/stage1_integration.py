"""Run against a running stage 1 server with a pre-existing test account."""

import argparse
import secrets
import socket
from pathlib import Path


def exact(connection, size):
    data = bytearray()
    while len(data) < size:
        part = connection.recv(size - len(data))
        if not part:
            raise EOFError("Incomplete OBEP frame")
        data.extend(part)
    return bytes(data)


def exchange(connection, command, data="", status="SUCCESS"):
    body = data.encode() + (b"\0" if data else b"")
    connection.sendall(f"{len(body):04X}|{command:7}|RE|       #".encode() + body)
    header = exact(connection, 24).decode("ascii")
    if header[4:] != f"|{command:7}|AN|{status}#":
        raise AssertionError(f"{command}: unexpected response {header!r}")
    body = exact(connection, int(header[:4], 16))
    if not body:
        return ""
    if body[-1] != 0 or b"\0" in body[:-1]:
        raise AssertionError(f"{command}: invalid response terminator")
    return body[:-1].decode()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("host")
    parser.add_argument("port", type=int)
    parser.add_argument("credentials", type=Path, help="Local file with login and password on separate lines")
    parser.add_argument("--insert", action="store_true", help="Add three fixture rows and retain them")
    args = parser.parse_args()
    login, password = args.credentials.read_text().splitlines()
    wrong = ("0" if password.startswith("9") else "9") + password[1:]

    def connect():
        return socket.create_connection((args.host, args.port), timeout=5)

    def sign_in(connection, value=password, status="SUCCESS"):
        exchange(connection, "LOGIN", f"ID:0|login:{login}|password:{value}", status)

    with connect() as connection:
        exchange(connection, "GETAUTH", status="FAILURE")
        sign_in(connection, wrong, "FAILURE")
        exchange(connection, "GETSUBJ", status="FAILURE")
        sign_in(connection)
        exchange(connection, "GETAUTH")
        exchange(connection, "GETSUBJ")
        if args.insert:
            suffix = secrets.token_hex(4)
            name = f"DACSC{suffix}"
            isbn = f"99{secrets.randbelow(10**11):011d}"
            author = f"ID:0|lastname:{name}|firstname:Test|birthdate:1990-11-23"
            subject = f"ID:0|name:{name}"
            aid = int(exchange(connection, "ADDAUTH", author))
            exchange(connection, "ADDAUTH", author, "FAILURE")
            sid = int(exchange(connection, "ADDSUBJ", subject))
            exchange(connection, "ADDSUBJ", subject, "FAILURE")
            book = (
                f"ID:0|auth_id:{aid}|subj_id:{sid}|pagecount:123|stock:2"
                f"|price:12.37|publishyear:2026|title:{name}|isbn:{isbn}"
            )
            bid = int(exchange(connection, "ADDBOOK", book))
            exchange(connection, "ADDBOOK", book, "FAILURE")
            if min(aid, sid, bid) <= 0:
                raise AssertionError("Expected positive IDs")
            if f"ID:{aid}|lastname:{name}|" not in exchange(connection, "GETAUTH"):
                raise AssertionError("Author missing from list")
            if f"ID:{sid}|name:{name}@" not in exchange(connection, "GETSUBJ"):
                raise AssertionError("Subject missing from list")
            print(f"Fixture {name}, ISBN {isbn}, IDs {aid}/{sid}/{bid}")
        exchange(connection, "LOGOUT")
        exchange(connection, "GETAUTH", status="FAILURE")
        sign_in(connection)

    for _ in range(8):
        with connect() as connection:
            exchange(connection, "GETSUBJ", status="FAILURE")
            sign_in(connection)
    print("OBEP session, lists and eight reconnects passed")


if __name__ == "__main__":
    main()
