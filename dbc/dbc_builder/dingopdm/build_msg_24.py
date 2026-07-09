import cantools
from utils.signal_utils import create_duplicate_signals

def build_msg_24(base_id):
    message = cantools.database.Message(
        frame_id=base_id + 24,
        name="dingoPdmMsg24",
        length=8,
        is_extended_frame=False,
        signals=[]
    )

    button_sigs = create_duplicate_signals("Keypad0Button", 20, 1, 0, 1, 1, 0)
    message.signals.extend(button_sigs)

    button_sigs = create_duplicate_signals("Keypad1Button", 20, 1, 24, 1, 1, 0)
    message.signals.extend(button_sigs)

    return message