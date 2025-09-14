# TODO: change to EJ
from glob import glob
from random import randint

delimiter = ""
CHARS = "abcdefghijklmnopqrstuvwxyz"
LENGTH = 5
for x in range(LENGTH):
    pos = randint(0, len(CHARS) - 1)
    delimiter += CHARS[pos]

files = [x.replace('\\', '/') for x in glob("js/*.js")]

for file in files:
    file_name = file.split('/')[-1].split('.')[0]
    print("Creating header for ", file_name)
    header_file_name = "js/include_" + file_name + ".h"
    contents_2_write = ""
    definition = f"INCLUDE_{file_name.upper()}"
    with open(file, 'r') as f:
        file_contents = f.read()
        contents_2_write = f'''
#ifndef {definition} 
#define {definition}
        
inline constexpr const char* {file_name}_contents = R"{delimiter}({file_contents}){delimiter}";
        
#endif // {definition}'''
    with open(header_file_name, 'w') as f:
        f.write(contents_2_write)