
import unittest
import subprocess
import os


class Test(unittest.TestCase):

    def test_test_eval(self):
        code = subprocess.call(["libejr_test_eval"])
        self.assertEqual(code, 0, "test_eval failed")



if __name__ == "__main__":
    os.chdir("build/")
    unittest.main()
