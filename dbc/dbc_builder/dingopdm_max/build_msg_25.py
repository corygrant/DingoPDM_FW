import cantools
from utils.signal_utils import create_duplicate_signals

def build_msg_25(base_id):
    message = cantools.database.Message(
        frame_id=base_id + 25,
        name="dingoPdmMaxMsg25",
        length=8,
        is_extended_frame=False,
        signals=[]
    )

    dial_sigs = create_duplicate_signals("Keypad0Dial", 4, 1, 0, 16, 1, 0)
    message.signals.extend(dial_sigs)

    return message