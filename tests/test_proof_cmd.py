import pytest

from application_client.ton_command_sender import BoilerplateCommandSender, Errors, AddressDisplayFlags
from application_client.ton_response_unpacker import unpack_proof_response
from application_client.ton_utils import build_ton_proof_message, build_ton_proof_message_v3r2
from ledgered.devices import DeviceType
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns
from utils import ROOT_SCREENSHOT_PATH, check_signature_validity


def test_get_proof_accepted(backend, navigator, test_name):
    client = BoilerplateCommandSender(backend)
    path = "m/44'/607'/0'/0'/0'/0'"
    pubkey = client.get_public_key(path).data
    domain = "example.com"
    timestamp = 123
    payload = b"test"
    proof_msg = build_ton_proof_message(0, pubkey, domain, timestamp, payload)
    with client.get_address_proof(path, AddressDisplayFlags.NONE, domain, timestamp, payload):
        if backend.device.is_nano:
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Approve",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        else:
            instructions = []
            if backend.device.type == DeviceType.FLEX:
                instructions = [
                    NavInsID.SWIPE_CENTER_TO_LEFT,
                    NavIns(NavInsID.TOUCH, (80, 440)),
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
                ]
            elif backend.device.type == DeviceType.STAX:
                instructions = [
                    NavInsID.SWIPE_CENTER_TO_LEFT,
                    NavIns(NavInsID.TOUCH, (60, 520)),
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
                ]
            elif backend.device.type == DeviceType.APEX_P:
                instructions = [
                    NavInsID.SWIPE_CENTER_TO_LEFT,
                    NavIns(NavInsID.TOUCH, (52, 300)),
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
                ]
            else:
                raise ValueError(f"Unsupported device: {backend.device}")

            navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                           test_name,
                                           instructions)
    response = client.get_async_response().data
    sig, hash_b = unpack_proof_response(response)
    assert hash_b == proof_msg
    assert check_signature_validity(pubkey, sig, hash_b)

def test_get_proof_accepted_v3r2(backend, navigator, test_name):
    client = BoilerplateCommandSender(backend)
    path = "m/44'/607'/0'/0'/0'/0'"
    pubkey = client.get_public_key(path).data
    domain = "example.com"
    timestamp = 123
    payload = b"test"
    proof_msg = build_ton_proof_message_v3r2(0, pubkey, domain, timestamp, payload)
    with client.get_address_proof(path, AddressDisplayFlags.NONE, domain, timestamp, payload, is_v3r2=True):
        if backend.device.is_nano:
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Approve",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        else:
            instructions = []
            if backend.device.type == DeviceType.FLEX:
                instructions = [
                    NavInsID.SWIPE_CENTER_TO_LEFT,
                    NavIns(NavInsID.TOUCH, (80, 440)),
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
                ]
            elif backend.device.type == DeviceType.STAX:
                instructions = [
                    NavInsID.SWIPE_CENTER_TO_LEFT,
                    NavIns(NavInsID.TOUCH, (60, 520)),
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
                ]
            elif backend.device.type == DeviceType.APEX_P:
                instructions = [
                    NavInsID.SWIPE_CENTER_TO_LEFT,
                    NavIns(NavInsID.TOUCH, (52, 300)),
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
                ]
            else:
                raise ValueError(f"Unsupported device: {backend.device}")

            navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                           test_name,
                                           instructions)
    response = client.get_async_response().data
    sig, hash_b = unpack_proof_response(response)
    assert hash_b == proof_msg
    assert check_signature_validity(pubkey, sig, hash_b)

def test_get_proof_refused(backend, navigator, test_name):
    client = BoilerplateCommandSender(backend)
    path = "m/44'/607'/0'/0'/0'/0'"
    domain = "example.com"
    timestamp = 123
    payload = b"test"

    if backend.device.is_nano:
        with pytest.raises(ExceptionRAPDU) as e:
            with client.get_address_proof(path, AddressDisplayFlags.NONE, domain, timestamp, payload):
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                          [NavInsID.BOTH_CLICK],
                                                          "Reject",
                                                          ROOT_SCREENSHOT_PATH,
                                                          test_name)
        # Assert that we have received a refusal
        assert e.value.status == Errors.SW_DENY
        assert len(e.value.data) == 0
    else:
        instructions_set = [
            [
                NavInsID.USE_CASE_REVIEW_REJECT,
            ],
            [
                NavInsID.SWIPE_CENTER_TO_LEFT,
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CANCEL,
            ],
        ]
        if backend.device.type == DeviceType.FLEX:
            instructions_set.append([
                NavInsID.SWIPE_CENTER_TO_LEFT,
                NavIns(NavInsID.TOUCH, (80, 440)),
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CANCEL,
            ])
        elif backend.device.type == DeviceType.STAX:
            instructions_set.append([
                NavInsID.SWIPE_CENTER_TO_LEFT,
                NavIns(NavInsID.TOUCH, (60, 520)),
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CANCEL,
            ])
        elif backend.device.type == DeviceType.APEX_P:
            instructions_set.append([
                NavInsID.SWIPE_CENTER_TO_LEFT,
                NavIns(NavInsID.TOUCH, (52, 300)),
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CANCEL,
            ])
        else:
            raise ValueError(f"Unsupported device: {backend.device}")

        for i, instructions in enumerate(instructions_set):
            with pytest.raises(ExceptionRAPDU) as e:
                with client.get_address_proof(path, AddressDisplayFlags.NONE, domain, timestamp, payload):
                    navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                                   test_name + f"/part{i}",
                                                   instructions)
            # Assert that we have received a refusal
            assert e.value.status == Errors.SW_DENY
            assert len(e.value.data) == 0
