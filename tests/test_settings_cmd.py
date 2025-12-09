from application_client.ton_command_sender import BoilerplateCommandSender
from utils import ROOT_SCREENSHOT_PATH
from ledgered.devices import DeviceType
from ragger.navigator import NavInsID, NavIns

# In this test we check the behavior of the device when asked to provide the app version
def test_app_settings(backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    rapdu = client.get_app_settings()
    assert(len(rapdu.data) == 1 and rapdu.data[0] == 0x00)

        # Enable blind signing and expert mode
    if backend.device.is_nano:
        navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                        test_name,
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
                                        test_name,
                                        [
                                            NavInsID.USE_CASE_HOME_INFO,
                                            NavIns(NavInsID.TOUCH, touch_pos_1),
                                            NavIns(NavInsID.TOUCH, touch_pos_2),
                                            NavInsID.USE_CASE_SETTINGS_NEXT,
                                            NavInsID.USE_CASE_SETTINGS_MULTI_PAGE_EXIT,
                                        ],
                                        screen_change_before_first_instruction=False)

    rapdu = client.get_app_settings()
    assert(len(rapdu.data) == 1 and rapdu.data[0] == (0x01 | 0x02))

