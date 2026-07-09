
import cantools
import canboard

def build_db(base_id):
    db = cantools.database.Database()
    db.name = "CANBoard"
    db.version = "0.5.1" #FW version
    
    db.messages.append(canboard.build_msg_0(base_id + 2))
    db.messages.append(canboard.build_msg_1(base_id + 2))
    db.messages.append(canboard.build_msg_2(base_id + 2))
    db.messages.append(canboard.build_msg_3(base_id + 2))
    db.messages.append(canboard.build_msg_4(base_id + 2))
    db.messages.append(canboard.build_msg_5(base_id + 2))
    db.messages.append(canboard.build_msg_6(base_id + 2))
    db.messages.append(canboard.build_msg_7(base_id + 2))
    db.messages.append(canboard.build_msg_8(base_id + 2))

    return db