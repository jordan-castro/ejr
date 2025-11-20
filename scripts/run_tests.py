
import unittest
import subprocess
import os


class Test(unittest.TestCase):

    def test_test_eval_module(self):
        code = subprocess.call(["libejr_test_eval_module"])
        self.assertEqual(code, 0, "test_eval_module failed")



    def test_test_eval_script(self):
        code = subprocess.call(["libejr_test_eval_script"])
        self.assertEqual(code, 0, "test_eval_script failed")



if __name__ == "__main__":
    subprocess.call(["py", "scripts/add_tests.py"])
    os.chdir("build/")
    unittest.main()
