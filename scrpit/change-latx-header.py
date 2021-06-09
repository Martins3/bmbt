#!/bin/python3

import os

work_dir = "/home/maritns3/core/ld/DuckBuBi"
latx_dir = work_dir + "/src/i386/LATX/"


def scan_files(suffix):
    files = []
    for r, d, f in os.walk(latx_dir):
        for file in f:
            if file.endswith(suffix):
                files.append(os.path.join(r, file).split('LATX/')[1])

    return files


c_files = scan_files(".c")
c_headers = scan_files(".h")


def check_identical_header(files):
    # every header is different so can be included correctly
    if len(files) != len(set(files)):
        print("identical header found, this script is assumption failed")
        exit(1)


check_identical_header(c_headers)


def get_locations(files):
    locations = dict()
    for h in files:
        path = h.split("/", 1)
        if(len(path) == 1):
            locations[path[0]] = None
        elif(len(path) == 2):
            locations[path[1]] = path[0]
        else:
            print("Impossible, maximum dir depth is one")
            exit(1)
    return locations


c_header_locations = get_locations(c_headers)
#  c_file_locations = get_locations(c_files)


def change_header_style(abs_path):
    with open(abs_path) as f:
        content = f.readlines()
    # you may also want to remove whitespace characters like `\n` at the end of each line
    #  content = [x.strip() for x in content]
    prefix = "#include \""
    suffix = "\"\n"

    print(abs_path)
    rel_path = abs_path.split("LATX/")[1]
    print(rel_path)
    in_dir = rel_path.find("/") != -1
    print(in_dir)

    external_dir = set(["qemu", "disas", "fpu", "exec"])

    new_contents = [ ]
    for line in content:
        if(not line.startswith(prefix)):
            new_contents.append(line)
        else:
            orig_inc = line[len(prefix):-2]
            header = orig_inc.split("/")[-1]

            header_path = orig_inc.split("/")[0]
            if header_path in external_dir:
                print("skip : " + line)
                continue


            try:
                print(c_header_locations[header])
            except Exception as e:
                print("key not find!")
                print(line)
                print(filename)
                raise e

            new_header = ""
            if in_dir:
                new_header = "../"

            if c_header_locations[header] != None:
                new_header = new_header + c_header_locations[header] + "/"

            new_header = prefix + new_header + header + suffix
            new_contents.append(new_header)
            print(line + " --> " + new_header)
    
    target = open(abs_path, 'w')
    target.writelines(new_contents)
    target.close()


source_files = [latx_dir + f for f in c_files + c_headers]
for filename in source_files:
    change_header_style(filename)
    #  break
