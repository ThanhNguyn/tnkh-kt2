#!/usr/bin/env python3
"""Software-only BLE GATT client for the TH3 ESP-EMU BLE server."""

import asyncio
import logging
import sys

logging.basicConfig(level=logging.WARNING)

EXPECTED_DEVICE_NAME = "TH3-BLE"
WRITE_VALUE = b"BLE write verified"


async def main() -> int:
    try:
        from bumble.controller import Controller
        from bumble.link import LocalLink
        from bumble.device import Device, Peer
        from bumble.host import Host
        from bumble.transport import open_transport
        from bumble import hci
    except ImportError:
        print("ERROR: install the 'bumble' package in the active Python environment")
        return 2

    link = LocalLink()
    print("[*] Starting virtual BLE controller on TCP port 9544", flush=True)
    emu_transport = await open_transport("tcp-server:_:9544")
    emu_controller = Controller(
        "emu-controller",
        host_source=emu_transport.source,
        host_sink=emu_transport.sink,
        link=link,
        public_address="00:11:22:33:44:55",
    )

    def ack_ok(self, command):
        params = hci.HCI_GenericReturnParameters(data=bytes([0]))
        if isinstance(command, hci.HCI_SyncCommand):
            return params
        self.send_hci_packet(
            hci.HCI_Command_Complete_Event(
                num_hci_command_packets=1,
                command_opcode=command.op_code,
                return_parameters=params,
            )
        )
        return None

    for name in (
        "on_hci_le_set_privacy_mode_command",
        "on_hci_le_clear_resolving_list_command",
        "on_hci_le_add_device_to_resolving_list_command",
        "on_hci_le_remove_device_from_resolving_list_command",
    ):
        if not hasattr(Controller, name):
            setattr(Controller, name, ack_ok)

    emu_controller.random_address = hci.Address(
        "00:11:22:33:44:55", hci.Address.PUBLIC_DEVICE_ADDRESS
    )

    phone_controller = Controller(
        "phone-controller", link=link, public_address="AA:BB:CC:DD:EE:FF"
    )
    phone_host = Host()
    phone_host.controller = phone_controller
    phone_controller.host = phone_host
    phone = Device(name="Virtual GATT Client", host=phone_host)
    await phone.power_on()

    print("[*] Waiting for ESP-EMU firmware to advertise...", flush=True)
    await asyncio.sleep(15)

    discovered = {}

    def on_advertisement(advertisement):
        raw_name = advertisement.data.get(0x09, b"")
        if isinstance(raw_name, bytes):
            name = raw_name.decode("utf-8", errors="replace")
        else:
            name = str(raw_name)
        address = str(advertisement.address)
        if address not in discovered:
            discovered[address] = (advertisement, name)
            print(f"[+] Found BLE device: name='{name}', address={address}", flush=True)

    phone.on("advertisement", on_advertisement)
    await phone.start_scanning()
    print("[*] Scanning for 10 seconds...", flush=True)
    await asyncio.sleep(10)
    await phone.stop_scanning()

    target = next(
        (entry[0] for entry in discovered.values() if entry[1] == EXPECTED_DEVICE_NAME),
        None,
    )
    if target is None:
        print(f"[-] Device '{EXPECTED_DEVICE_NAME}' was not found", flush=True)
        return 1

    service_uuid = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
    characteristic_uuid = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
    connection = None

    try:
        print(f"[*] Connecting to {target.address}...", flush=True)
        connection = await phone.connect(target.address, timeout=10)
        print("[+] BLE connection established", flush=True)

        peer = Peer(connection)
        await peer.discover_services()
        service = next((s for s in peer.services if str(s.uuid).lower() == service_uuid), None)
        if service is None:
            print("[-] Expected GATT service was not found", flush=True)
            return 1

        print(f"[+] Service UUID: {service_uuid}", flush=True)
        await service.discover_characteristics()
        characteristic = next(
            (c for c in service.characteristics if str(c.uuid).lower() == characteristic_uuid),
            None,
        )
        if characteristic is None:
            print("[-] Expected GATT characteristic was not found", flush=True)
            return 1

        print(f"[+] Characteristic UUID: {characteristic_uuid}", flush=True)
        initial_value = await characteristic.read_value()
        initial_text = initial_value.decode("utf-8", errors="replace")
        print(f"[READ] Initial value: {initial_text}", flush=True)

        await characteristic.write_value(WRITE_VALUE)
        print(f"[WRITE] Sent value: {WRITE_VALUE.decode()}", flush=True)

        read_back = await characteristic.read_value()
        read_back_text = read_back.decode("utf-8", errors="replace")
        print(f"[READ-BACK] Value: {read_back_text}", flush=True)

        if read_back != WRITE_VALUE:
            print("[-] Read-back does not match the written value", flush=True)
            return 1

        print("[PASS] BLE GATT read/write verified", flush=True)
        return 0
    except Exception as exc:
        print(f"[-] BLE GATT test failed: {exc}", flush=True)
        return 1
    finally:
        if connection is not None:
            try:
                await connection.disconnect()
                print("[*] BLE client disconnected", flush=True)
            except Exception:
                pass


if __name__ == "__main__":
    try:
        raise SystemExit(asyncio.run(main()))
    except KeyboardInterrupt:
        print("\n[*] Interrupted", file=sys.stderr)
        raise SystemExit(130)
