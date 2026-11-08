"""The BOTW observation table and the real ARM64 gate; no player files required."""
import ast
import os
from pathlib import Path
import platform
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
GAME = ROOT / 'games/totk'
TOUCHNX = Path(os.environ.get('TOUCHNX_ROOT', ROOT.parent / 'touchnx')).resolve()
script = GAME / 'scripts/prepare_runtime.py'
tree = ast.parse(script.read_text())
namespace = {}
exec(compile(ast.Module(body=[n for n in tree.body if isinstance(n, ast.FunctionDef) and n.name == 'parse_gate'],
                        type_ignores=[]), str(script), 'exec'), namespace)
parse_gate = namespace['parse_gate']


class ObservationTableTests(unittest.TestCase):
    def test_distinct_kinds_and_observer_parameters(self):
        self.assertEqual([parse_gate(f'0x40 {kind} 29 0x80')['timer']
                          for kind in ('svc', 'cntpct', 'cntvct', 'spin', 'obs')], [0, 1, 1, 2, 3])
        gate = parse_gate('0x40 obs 298 0x80')  # OBS42, camera 1
        self.assertEqual(gate, dict(ret=64, timer=3, arg=298, original=128))
        for line in ('0x40 unknown 1 0x80', '0x40 obs 0 0x80', '0x40 obs 44 0x80',
                     '0x40 obs -1 0x80', '0x40 obs 4294967338 0x80'):
            with self.subTest(line=line), self.assertRaises(AssertionError):
                parse_gate(line)

    @unittest.skipUnless(platform.system() == 'Darwin' and platform.machine() == 'arm64' and shutil.which('xcrun'),
                         'requires native Apple ARM64 tools')
    def test_nested_observers_use_production_gate_and_dispatch(self):
        with tempfile.TemporaryDirectory() as temporary:
            binary = Path(temporary) / 'probe'
            command = ['xcrun', '--sdk', 'macosx', 'clang', '-std=gnu11', '-O1', '-g',
                       '-Wall', '-Wextra', '-Werror', '-Wno-deprecated-declarations',
                       '-DGAME_BOTW=1', '-DGAME_BOTW190=1', '-ffunction-sections', '-fdata-sections',
                       '-Wl,-dead_strip', '-I' + str(TOUCHNX), '-I' + str(GAME / 'runtime'), '-I' + str(GAME / 'third_party/d2r_gfx/gfx/nvn/include'),
                       str(ROOT / 'tests/botw_observation_probe.c'), str(ROOT / 'tests/botw_observation_probe.S')]
            command += [str(GAME / 'runtime' / name) for name in
                        ('gate.S', 'hle_stack.c', 'hle_stack.S', 'observation.c', 'botw_touch/hle_touch_clock.c')]
            build = subprocess.run(command + ['-o', str(binary)], capture_output=True, text=True, timeout=90)
            self.assertEqual(build.returncode, 0, build.stdout + build.stderr)
            run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
            self.assertIn('per-thread frames passed', run.stdout)


if __name__ == '__main__':
    unittest.main()
