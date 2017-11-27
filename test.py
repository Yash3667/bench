with open("/dev/sdc", "w") as this_file:
    for i in range(16*1024*1024):
        this_file.write("1")


