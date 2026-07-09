
import cantools
import dingopdm

def build_db(base_id):
    db = cantools.database.Database()
    db.name = "dingoPdm"
    db.version = "0.5.1"
    
    db.messages.append(dingopdm.build_msg_0(base_id + 2))
    db.messages.append(dingopdm.build_msg_1(base_id + 2))
    db.messages.append(dingopdm.build_msg_2(base_id + 2))
    db.messages.append(dingopdm.build_msg_3(base_id + 2))
    db.messages.append(dingopdm.build_msg_4(base_id + 2))
    db.messages.append(dingopdm.build_msg_5(base_id + 2))
    db.messages.append(dingopdm.build_msg_6(base_id + 2))
    db.messages.append(dingopdm.build_msg_7(base_id + 2))
    db.messages.append(dingopdm.build_msg_8(base_id + 2))
    db.messages.append(dingopdm.build_msg_9(base_id + 2))
    db.messages.append(dingopdm.build_msg_10(base_id + 2))
    db.messages.append(dingopdm.build_msg_11(base_id + 2))
    db.messages.append(dingopdm.build_msg_12(base_id + 2))
    db.messages.append(dingopdm.build_msg_13(base_id + 2))
    db.messages.append(dingopdm.build_msg_14(base_id + 2))
    db.messages.append(dingopdm.build_msg_15(base_id + 2))
    db.messages.append(dingopdm.build_msg_16(base_id + 2))
    db.messages.append(dingopdm.build_msg_17(base_id + 2))
    db.messages.append(dingopdm.build_msg_18(base_id + 2))
    db.messages.append(dingopdm.build_msg_19(base_id + 2))
    db.messages.append(dingopdm.build_msg_20(base_id + 2))
    db.messages.append(dingopdm.build_msg_21(base_id + 2))
    db.messages.append(dingopdm.build_msg_22(base_id + 2))
    db.messages.append(dingopdm.build_msg_23(base_id + 2))
    db.messages.append(dingopdm.build_msg_24(base_id + 2))
    db.messages.append(dingopdm.build_msg_25(base_id + 2))
    db.messages.append(dingopdm.build_msg_26(base_id + 2))

    return db