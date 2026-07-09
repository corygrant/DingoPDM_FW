import cantools
from utils.signal_utils import create_duplicate_signals
from cantools.database.conversion import LinearConversion

def build_msg_7(base_id):
    message = cantools.database.Message(
        frame_id=base_id + 7,
        name="CANBoardMsg7",
        length=8,
        is_extended_frame=False,
        signals=[]
    )

    can_in_val_sigs = create_duplicate_signals("CANInputValue", 2, 5, 0, 32, 1, 0)
    message.signals.extend(can_in_val_sigs)
    
    return message