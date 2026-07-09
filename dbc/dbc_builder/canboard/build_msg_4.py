import cantools
from utils.signal_utils import create_duplicate_signals
from cantools.database.conversion import LinearConversion

def build_msg_4(base_id):
    message = cantools.database.Message(
        frame_id=base_id + 4,
        name="CANBoardMsg4",
        length=8,
        is_extended_frame=False,
        signals=[]
    )

    counter_val_sigs = create_duplicate_signals("CounterValue", 4, 1, 0, 8, 1, 0)
    message.signals.extend(counter_val_sigs)
    
    condition_result_sigs = create_duplicate_signals("ConditionResult", 32, 1, 32, 1, 1, 0)
    message.signals.extend(condition_result_sigs)
    
    return message