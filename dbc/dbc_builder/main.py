import os
import canboard
import dingopdm
import dingopdm_max

if __name__ == "__main__":
    dbs = []
    
    dbs.append(dingopdm.build_db(0x0DE))
    dbs.append(dingopdm_max.build_db(0x0DE))
    dbs.append(canboard.build_db(0x640))
    
    for db in dbs:
        script_dir = os.path.dirname(os.path.realpath(__file__)) #<-- absolute dir the script is in
        print(script_dir)
        rel_path = "../" + db.name + "_" + db.version + ".dbc"
        abs_file_path = os.path.abspath(os.path.realpath(os.path.join(script_dir, rel_path)))
        print(abs_file_path)
        with open(abs_file_path, 'w') as f:
            f.write(db.as_dbc_string()) 
      