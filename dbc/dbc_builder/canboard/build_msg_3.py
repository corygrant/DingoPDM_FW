import cantools
from utils.signal_utils import create_duplicate_signals
from cantools.database.conversion import LinearConversion

def build_msg_3(base_id):
    message = cantools.database.Message(
        frame_id=base_id + 3,
        name="CANBoardMsg3",
        length=8,
        is_extended_frame=False,
        signals=[]
    )

    can_input_sigs = create_duplicate_signals("CANInput", 32, 1, 0, 1, 1, 0)
    message.signals.extend(can_input_sigs)
    
    virt_input_sigs = create_duplicate_signals("VirtualInput", 16, 1, 32, 1, 1, 0)
    message.signals.extend(virt_input_sigs)
    
    return message