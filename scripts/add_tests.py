import glob
import subprocess
import os

test_skeleton = """
    # ------------------------------
    # 5. Test {name}
    # ------------------------------
    add_executable(libejr_{name} tests/{file_path})
    target_include_directories(libejr_{name} PRIVATE ${{PROJECT_SOURCE_DIR}}/include)
    target_link_libraries(libejr_{name} PRIVATE ejr)
"""
pyfunction_skeleton = """
    def test_{name}(self):
        code = subprocess.call(["libejr_{name}"])
        self.assertEqual(code, 1, "{name} failed")

"""
py_test = """
import unittest
import subprocess
import os


class Test(unittest.TestCase):
{tests}

if __name__ == "__main__":
    os.chdir("build/")
    unittest.main()
"""

# Open the CMakeLists.txt file and find where we do if defined
cmake_file_lines = []

with open("CMakeLists.txt", 'r') as file:
    cmake_file_lines = list(filter(lambda f: f if len(f.strip()) > 0 else None, file.readlines()))

start_pos = 0
for line in cmake_file_lines:
    if 'DEFINED ENV{EJR_TESTS}' in line:
        start_pos = cmake_file_lines.index(line)
        start_pos += 1

if start_pos == 0:
    print("Could not find start...")
    exit(1)

cmake_file_lines = cmake_file_lines[0:start_pos]
python_functions = []

# Now loop through tests
tests = glob.glob("tests/*.c")
for test in tests:
    test = test.replace("\\", "/")
    file_path = test.split("/")[-1]
    name = file_path.split(".")[0]

    pyfunc = pyfunction_skeleton.format(name=name)
    python_functions.append(pyfunc)

    test_stuff = test_skeleton.format(name=name,file_path=file_path)
    nlines = test_stuff.split('\n')
    for nline in nlines:
        cmake_file_lines.append(nline)

cmake_file_lines.append("endif()")

with open("CMakeLists.txt", "w") as f:
    f.write("\n".join(cmake_file_lines))

with open("scripts/run_tests.py", "w") as f:
    f.write(py_test.format(tests="\n".join(python_functions)))

# Now run this shit
os.environ["EJR_TESTS"] = "1"
os.chdir("build/")
cmd = subprocess.call(["cmake", "--build", "."])