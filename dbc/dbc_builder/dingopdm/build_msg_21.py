import cantools
from utils.signal_utils import create_duplicate_signals

def build_msg_21(base_id):
    message = cantools.database.Message(
        frame_id=base_id + 21,
        name="dingoPdmMsg21",
        length=8,
        is_extended_frame=False,
        signals=[]
    )

    can_in_val_sigs = create_duplicate_signals("CANInputValue", 2, 29, 0, 32, 1, 0)
    message.signals.extend(can_in_val_sigs)
    return message