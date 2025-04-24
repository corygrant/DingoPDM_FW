import cantools
import dingopdm
import dingopdm_max

base_id = 2000

if __name__ == "__main__":
    dbs = []
    dbs.append(dingopdm.build_db(base_id))
    dbs.append(dingopdm_max.build_db(base_id))
    
    for db in dbs:
        with open("./dbc/" + db.name + "_" + db.version + ".dbc", 'w') as f:
            f.write(db.as_dbc_string()) 
    