import pytest

from application_client.ton_transaction import Transaction, SendMode, CommentPayload, Payload, JettonTransferPayload, NFTTransferPayload, CustomUnsafePayload, JettonBurnPayload, AddWhitelistPayload, SingleNominatorWithdrawPayload, ChangeValidatorPayload, TonstakersDepositPayload, JettonDAOVotePayload, ChangeDNSWalletPayload, ChangeDNSPayload, TokenBridgePaySwapPayload, TonWhalesPoolDepositPayload, TonWhalesPoolWithdrawPayload, VestingSendMsgCommentPayload
from application_client.ton_command_sender import BoilerplateCommandSender, Errors
from application_client.ton_response_unpacker import unpack_sign_tx_response
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns
from ledgered.devices import DeviceType
from utils import ROOT_SCREENSHOT_PATH, check_signature_validity
from tonsdk.utils import Address
from typing import List
from tonsdk.boc import Cell

# In this tests we check the behavior of the device when asked to sign a transaction


# In this test se send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
def test_sign_tx_no_payload(backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/607'/0'/0'/0'/0'"

    # First we need to get the public key of the device in order to build the transaction
    pubkey = client.get_public_key(path=path).data

    # Create the transaction that will be sent to the device for signing
    tx = Transaction(Address("0:" + "0" * 64), SendMode.PAY_GAS_SEPARATLY, 0, 1686176000, True, 100000000)
    tx_bytes = tx.to_request_bytes()

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_tx(path=path, transaction=tx_bytes):
        # Validate the on-screen request by performing the navigation appropriate for this device
        if backend.device.is_nano:
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Approve",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        else:
            navigator.navigate([
                                   NavInsID.SWIPE_CENTER_TO_LEFT,
                               ])
            navigator.navigate_until_text_and_compare(NavInsID.USE_CASE_VIEW_DETAILS_NEXT,
                                                      [NavInsID.USE_CASE_REVIEW_CONFIRM,
                                                       NavInsID.USE_CASE_STATUS_DISMISS],
                                                      "Hold to sign",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name,
                                                      screen_change_before_first_instruction=False)

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    sig, hash_b = unpack_sign_tx_response(response)
    assert hash_b == tx.transfer_cell().bytes_hash()
    assert check_signature_validity(pubkey, sig, hash_b)


def test_sign_tx_blind_error(backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/607'/0'/0'/0'/0'"

    payload = CustomUnsafePayload(Cell())

    tx = Transaction(Address("0:" + "0" * 64), SendMode.PAY_GAS_SEPARATLY, 0, 1686176000, True, 100000000, payload=payload)
    tx_bytes = tx.to_request_bytes()

    with pytest.raises(ExceptionRAPDU) as e:
        with client.sign_tx(path=path, transaction=tx_bytes):
            # Validate the on-screen request by performing the navigation appropriate for this device
            if backend.device.is_nano:
                if backend.device.type == DeviceType.NANOS:
                    navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                                    test_name,
                                                    [
                                                        NavIns(NavInsID.WAIT_FOR_TEXT_ON_SCREEN, ("Error", )),
                                                        NavInsID.RIGHT_CLICK,
                                                        NavInsID.BOTH_CLICK
                                                    ])
                else:
                    navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                                    test_name,
                                                    [
                                                        NavInsID.BOTH_CLICK
                                                    ])
            else:
                navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                                test_name,
                                                [
                                                    NavInsID.USE_CASE_CHOICE_CONFIRM
                                                ])

    assert e.value.status == Errors.SW_BLIND_SIGNING_DISABLED
    assert len(e.value.data) == 0


def test_sign_tx_with_payload(backend, navigator, test_name):
    import os

    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/607'/0'/0'/0'/0'"

    # First we need to get the public key of the device in order to build the transaction
    pubkey = client.get_public_key(path=path).data

    payloads: List[Payload] = [
        CustomUnsafePayload(Cell()),
        CommentPayload("test"),
        JettonTransferPayload(100, Address("0:" + "0" * 64), forward_amount=1),
        NFTTransferPayload(Address("0:" + "0" * 64), forward_amount=1),
        JettonBurnPayload(100, Address("0:" + "0" * 64), custom_payload=Cell()),
        AddWhitelistPayload(Address("0:" + "0" * 64)),
        SingleNominatorWithdrawPayload(1_000_000_000),
        ChangeValidatorPayload(Address("0:" + "0" * 64)),
        TonstakersDepositPayload(app_id=123),
        JettonDAOVotePayload(Address("0:" + "0" * 64), 1686176000, True, True),
        ChangeDNSWalletPayload(Address("0:" + "0" * 64), True, True),
        ChangeDNSPayload(bytes([0] * 32), None),
        TokenBridgePaySwapPayload(bytes([0] * 32)),
        JettonBurnPayload(100, Address("0:" + "0" * 64), custom_payload=bytes([1, 32] * 10)),
        TonWhalesPoolDepositPayload(gas_limit=100_000_000, query_id=123),
        TonWhalesPoolWithdrawPayload(withdrawal_amount=1_000_000_000, gas_limit=100_000_000, query_id=123),
        TonWhalesPoolWithdrawPayload(gas_limit=100_000_000, query_id=123),
        VestingSendMsgCommentPayload("Deposit", SendMode.IGNORE_ERRORS, Address("0:" + "0" * 64), 10_000_000_000),
        VestingSendMsgCommentPayload("A very very long comment that should be so long that it should be stored in a separate cell", SendMode.IGNORE_ERRORS, Address("0:" + "0" * 64), 10_000_000_000)
    ]

    # Enable blind signing and expert mode
    if backend.device.is_nano:
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                        test_name + "/pretest",
                                        [
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                        ],
                                        screen_change_before_first_instruction=False)
    else:
        if backend.device.type == DeviceType.APEX_P:
            touch_pos_1 = (265, 95)
            touch_pos_2 = (265, 215)
        else:
            touch_pos_1 = (354, 125)
            touch_pos_2 = (354, 272)
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                        test_name + "/pretest",
                                        [
                                            NavInsID.USE_CASE_HOME_INFO,
                                            NavIns(NavInsID.TOUCH, touch_pos_1),
                                            NavIns(NavInsID.TOUCH, touch_pos_2),
                                            NavInsID.USE_CASE_SETTINGS_MULTI_PAGE_EXIT,
                                        ],
                                        screen_change_before_first_instruction=False)

    for (i, payload) in list(enumerate(payloads))[int(os.environ.get("PAYLOAD_START_INDEX", default="0")):]:
        # Create the transaction that will be sent to the device for signing
        tx = Transaction(Address("0:" + "0" * 64), SendMode.PAY_GAS_SEPARATLY, 0, 1686176000, True, 100000000, payload=payload)
        tx_bytes = tx.to_request_bytes()

        # Send the sign device instruction.
        # As it requires on-screen validation, the function is asynchronous.
        # It will yield the result when the navigation is done
        with client.sign_tx(path=path, transaction=tx_bytes):
            # Validate the on-screen request by performing the navigation appropriate for this device
            if backend.device.is_nano:
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                            [NavInsID.BOTH_CLICK],
                                                            "Approve",
                                                            ROOT_SCREENSHOT_PATH,
                                                            test_name + f"/part{i}")
            else:
                instructions = [
                    NavInsID.SWIPE_CENTER_TO_LEFT,
                ]
                if i == 0:
                    pre = [
                        NavInsID.USE_CASE_CHOICE_REJECT,
                        NavInsID.USE_CASE_CHOICE_CONFIRM,
                    ]
                    instructions = pre + instructions
                navigator.navigate(instructions)
                navigator.navigate_until_text_and_compare(NavInsID.USE_CASE_VIEW_DETAILS_NEXT,
                                                            [NavInsID.USE_CASE_REVIEW_CONFIRM,
                                                            NavInsID.USE_CASE_STATUS_DISMISS],
                                                            "Hold to sign",
                                                            ROOT_SCREENSHOT_PATH,
                                                            test_name + f"/part{i}",
                                                            screen_change_before_first_instruction=False)

        # The device as yielded the result, parse it and ensure that the signature is correct
        response = client.get_async_response().data
        sig, hash_b = unpack_sign_tx_response(response)
        assert hash_b == tx.transfer_cell().bytes_hash()
        assert check_signature_validity(pubkey, sig, hash_b)


def test_sign_tx_subwallet_id(backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/607'/0'/0'/0'/0'"

    # First we need to get the public key of the device in order to build the transaction
    pubkey = client.get_public_key(path=path).data

    payloads: List[Payload] = [
        CommentPayload("test"),
        CustomUnsafePayload(Cell()),
    ]

    # Enable blind signing and expert mode
    if backend.device.is_nano:
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                        test_name + "/pretest",
                                        [
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                        ],
                                        screen_change_before_first_instruction=False)
    else:
        if backend.device.type == DeviceType.APEX_P:
            touch_pos_1 = (265, 95)
            touch_pos_2 = (265, 215)
        else:
            touch_pos_1 = (354, 125)
            touch_pos_2 = (354, 272)
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                        test_name + "/pretest",
                                        [
                                            NavInsID.USE_CASE_HOME_INFO,
                                            NavIns(NavInsID.TOUCH, touch_pos_1),
                                            NavIns(NavInsID.TOUCH, touch_pos_2),
                                            NavInsID.USE_CASE_SETTINGS_MULTI_PAGE_EXIT,
                                        ],
                                        screen_change_before_first_instruction=False)

    for (i, payload) in enumerate(payloads):
        # Create the transaction that will be sent to the device for signing
        tx = Transaction(Address("0:" + "0" * 64), SendMode.PAY_GAS_SEPARATLY, 0, 1686176000, True, 100000000, payload=payload, subwallet_id=0x10c, include_wallet_op=False)
        tx_bytes = tx.to_request_bytes()

        # Send the sign device instruction.
        # As it requires on-screen validation, the function is asynchronous.
        # It will yield the result when the navigation is done
        with client.sign_tx(path=path, transaction=tx_bytes):
            # Validate the on-screen request by performing the navigation appropriate for this device
            if backend.device.is_nano:
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                            [NavInsID.BOTH_CLICK],
                                                            "Approve",
                                                            ROOT_SCREENSHOT_PATH,
                                                            test_name + f"/part{i}")
            else:
                instructions = [
                    NavInsID.SWIPE_CENTER_TO_LEFT,
                ]
                if i == 1:
                    pre = [
                        NavInsID.USE_CASE_CHOICE_REJECT,
                        NavInsID.USE_CASE_CHOICE_CONFIRM,
                    ]
                    instructions = pre + instructions
                navigator.navigate(instructions)
                navigator.navigate_until_text_and_compare(NavInsID.USE_CASE_VIEW_DETAILS_NEXT,
                                                            [NavInsID.USE_CASE_REVIEW_CONFIRM,
                                                            NavInsID.USE_CASE_STATUS_DISMISS],
                                                            "Hold to sign",
                                                            ROOT_SCREENSHOT_PATH,
                                                            test_name + f"/part{i}",
                                                            screen_change_before_first_instruction=False)

        # The device as yielded the result, parse it and ensure that the signature is correct
        response = client.get_async_response().data
        sig, hash_b = unpack_sign_tx_response(response)
        assert hash_b == tx.transfer_cell().bytes_hash()
        assert check_signature_validity(pubkey, sig, hash_b)


# Transaction signature refused test
# The test will ask for a transaction signature that will be refused on screen
def test_sign_tx_refused(backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44'/607'/0'/0'/0'/0'"

    tx = Transaction(Address("0:" + "0" * 64), SendMode.PAY_GAS_SEPARATLY, 0, 1686176000, True, 100000000)
    tx_bytes = tx.to_request_bytes()

    if backend.device.is_nano:
        with pytest.raises(ExceptionRAPDU) as e:
            with client.sign_tx(path=path, transaction=tx_bytes):
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                          [NavInsID.BOTH_CLICK],
                                                          "Reject",
                                                          ROOT_SCREENSHOT_PATH,
                                                          test_name)

        # Assert that we have received a refusal
        assert e.value.status == Errors.SW_DENY
        assert len(e.value.data) == 0
    else:
        for i in range(3):
            instructions = []
            if i > 0:
                instructions += [NavInsID.SWIPE_CENTER_TO_LEFT]
                instructions += [NavInsID.USE_CASE_VIEW_DETAILS_NEXT] * (i-1)
            instructions += [NavInsID.USE_CASE_REVIEW_REJECT,
                             NavInsID.USE_CASE_CHOICE_CONFIRM,
                             NavInsID.USE_CASE_STATUS_DISMISS]
            with pytest.raises(ExceptionRAPDU) as e:
                with client.sign_tx(path=path, transaction=tx_bytes):
                    navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                                   test_name + f"/part{i}",
                                                   instructions)
            # Assert that we have received a refusal
            assert e.value.status == Errors.SW_DENY
            assert len(e.value.data) == 0


def test_sign_tx_clear_jetton(backend, navigator, test_name):
    import os

    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/607'/0'/0'/0'/0'"

    # test that nano S refuses
    if backend.device.type == DeviceType.NANOX:
        tx = Transaction(Address("0:" + "0" * 64), SendMode.PAY_GAS_SEPARATLY, 0, 1686176000, True, 100000000,
                         payload=JettonTransferPayload(100, Address("0:" + "0" * 64), forward_amount=1, jetton_id=0))
        tx_bytes = tx.to_request_bytes()

        with pytest.raises(ExceptionRAPDU) as e:
            with client.sign_tx(path=path, transaction=tx_bytes):
                pass

        assert e.value.status == Errors.SW_TX_PARSING_FAIL
        assert len(e.value.data) == 0

        return

    # test that trying a wrong jetton wallet address refuses
    tx = Transaction(Address("0:" + "0" * 64), SendMode.PAY_GAS_SEPARATLY, 0, 1686176000, True, 100000000,
                     payload=JettonTransferPayload(100, Address("0:" + "0" * 64), forward_amount=1, jetton_id=0))
    tx_bytes = tx.to_request_bytes()

    with pytest.raises(ExceptionRAPDU) as e:
        with client.sign_tx(path=path, transaction=tx_bytes):
            pass

    assert e.value.status == Errors.SW_TX_PARSING_FAIL
    assert len(e.value.data) == 0

    # First we need to get the public key of the device in order to build the transaction
    pubkey = client.get_public_key(path=path).data

    # jetton wallet addresses for the owner EQDWey_FGPhd3phmerdVXi-zUIujfyO4-29y_VT1yD0medCi
    addresses = [
        "EQD0sKn8DbS12U015TWOSpYmyJYYDC_7sxg1upaMxnBvTiX8",
        "EQANxfGN1EgFPawYB1fhPqebKe1Nb6FIsaiekEecJ6R-3kYF",
        "EQCJngWcgOzi9kV2vwKTbOcJc5AoDiLj6u0CMperigl1Dul-",
        "EQBvh5tyLrxE7CyeNFAUvHCQn50bmGK9iNPTSoXXcuNzHAsO",
        "EQD5Hgvvs8iyeH_mwcgAdtz9eap3de8EBdn3dgv6NQaMSFxZ",
        "EQC5LlcR2UXlqVrOJUqrF4YVTHVweuYavC_u2ia9VKDNIPEa",
        "EQBR8_qGiqF2KhYwPnwGU6kMjA0P53bRhdNX4UH_YJUYhUDL",
        "EQBCgyCGsBV5mQetCF3NWcRfW_uI11KT1zJUwRoXLWk8N-LG",
        "EQDXTn8IvuNJTkDnTN9XQUZOqEYuYNhit1LgBTBnkJk55Vjj",
        "EQA3M981CTXeFtkeurrOs8Z-jcFwlHpkJO5DrwMC7zn0U-La",
        "EQDWy3LycZWF-NlwGoFPVBoew1A89jIiUYiMAZ3ErEvSuezj",
    ]

    # Enable blind signing and expert mode
    if backend.device.is_nano:
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                        test_name + "/pretest",
                                        [
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                        ],
                                        screen_change_before_first_instruction=False)
    else:
        if backend.device.type == DeviceType.APEX_P:
            touch_pos_1 = (265, 95)
            touch_pos_2 = (265, 215)
        else:
            touch_pos_1 = (354, 125)
            touch_pos_2 = (354, 272)
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                        test_name + "/pretest",
                                        [
                                            NavInsID.USE_CASE_HOME_INFO,
                                            NavIns(NavInsID.TOUCH, touch_pos_1),
                                            NavIns(NavInsID.TOUCH, touch_pos_2),
                                            NavInsID.USE_CASE_SETTINGS_MULTI_PAGE_EXIT,
                                        ],
                                        screen_change_before_first_instruction=False)

    for (i, addr) in list(enumerate(addresses)):
        # Create the transaction that will be sent to the device for signing
        tx = Transaction(Address(addr), SendMode.PAY_GAS_SEPARATLY, 0, 1686176000, True, 100000000,
                         payload=JettonTransferPayload(100, Address("0:" + "0" * 64), forward_amount=1, jetton_id=i))
        tx_bytes = tx.to_request_bytes()

        # Send the sign device instruction.
        # As it requires on-screen validation, the function is asynchronous.
        # It will yield the result when the navigation is done
        with client.sign_tx(path=path, transaction=tx_bytes):
            # Validate the on-screen request by performing the navigation appropriate for this device
            if backend.device.is_nano:
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                            [NavInsID.BOTH_CLICK],
                                                            "Approve",
                                                            ROOT_SCREENSHOT_PATH,
                                                            test_name + f"/part{i}")
            else:
                navigator.navigate([
                                       NavInsID.SWIPE_CENTER_TO_LEFT,
                                   ])
                navigator.navigate_until_text_and_compare(NavInsID.USE_CASE_VIEW_DETAILS_NEXT,
                                                            [NavInsID.USE_CASE_REVIEW_CONFIRM,
                                                            NavInsID.USE_CASE_STATUS_DISMISS],
                                                            "Hold to sign",
                                                            ROOT_SCREENSHOT_PATH,
                                                            test_name + f"/part{i}",
                                                            screen_change_before_first_instruction=False)

        # The device as yielded the result, parse it and ensure that the signature is correct
        response = client.get_async_response().data
        sig, hash_b = unpack_sign_tx_response(response)
        assert hash_b == tx.transfer_cell().bytes_hash()
        assert check_signature_validity(pubkey, sig, hash_b)


def test_sign_tx_jetton_high_query_id(backend, navigator, test_name):
    import os

    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/607'/0'/0'/0'/0'"

    # First we need to get the public key of the device in order to build the transaction
    pubkey = client.get_public_key(path=path).data

    # Enable blind signing and expert mode
    if backend.device.is_nano:
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                        test_name + "/pretest",
                                        [
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                            NavInsID.RIGHT_CLICK,
                                            NavInsID.BOTH_CLICK,
                                        ],
                                        screen_change_before_first_instruction=False)
    else:
        if backend.device.type == DeviceType.APEX_P:
            touch_pos_1 = (265, 95)
            touch_pos_2 = (265, 215)
        else:
            touch_pos_1 = (354, 125)
            touch_pos_2 = (354, 272)
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                        test_name + "/pretest",
                                        [
                                            NavInsID.USE_CASE_HOME_INFO,
                                            NavIns(NavInsID.TOUCH, touch_pos_1),
                                            NavIns(NavInsID.TOUCH, touch_pos_2),
                                            NavInsID.USE_CASE_SETTINGS_MULTI_PAGE_EXIT,
                                        ],
                                        screen_change_before_first_instruction=False)

    # the actual address does not really matter
    usdt_wallet_address = "EQD0sKn8DbS12U015TWOSpYmyJYYDC_7sxg1upaMxnBvTiX8"

    # Create the transaction that will be sent to the device for signing
    tx = Transaction(Address(usdt_wallet_address), SendMode.PAY_GAS_SEPARATLY, 0, 1686176000, True, 100000000,
                        payload=JettonTransferPayload(100, Address("0:" + "0" * 64), forward_amount=1, query_id=((1 << 64) - 1)))
    tx_bytes = tx.to_request_bytes()

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_tx(path=path, transaction=tx_bytes):
        # Validate the on-screen request by performing the navigation appropriate for this device
        if backend.device.is_nano:
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                        [NavInsID.BOTH_CLICK],
                                                        "Approve",
                                                        ROOT_SCREENSHOT_PATH,
                                                        test_name + "/part0")
        else:
            navigator.navigate([
                                    NavInsID.SWIPE_CENTER_TO_LEFT,
                                ])
            navigator.navigate_until_text_and_compare(NavInsID.USE_CASE_VIEW_DETAILS_NEXT,
                                                        [NavInsID.USE_CASE_REVIEW_CONFIRM,
                                                        NavInsID.USE_CASE_STATUS_DISMISS],
                                                        "Hold to sign",
                                                        ROOT_SCREENSHOT_PATH,
                                                        test_name + "/part0",
                                                        screen_change_before_first_instruction=False)

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    sig, hash_b = unpack_sign_tx_response(response)
    assert hash_b == tx.transfer_cell().bytes_hash()
    assert check_signature_validity(pubkey, sig, hash_b)
