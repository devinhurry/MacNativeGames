"""Run the original portable BOTW touch checks against the copied headers only."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
TOUCHNX = Path(os.environ.get('TOUCHNX_ROOT', ROOT.parent / 'touchnx')).resolve()
CORE = TOUCHNX / 'games/botw/runtime'
INPUT = TOUCHNX / 'core/input'
ADAPTER = TOUCHNX / 'games/botw/backends/macnativegames/runtime'
INCLUDES = [item for path in (TOUCHNX, CORE, ADAPTER, ROOT/'games/totk/runtime') for item in ('-I', str(path))]


@unittest.skipUnless(shutil.which('xcrun'), 'needs Apple command line tools')
class BotwTouchCoreTests(unittest.TestCase):
    def test_runtime_input_adapter(self):
        with tempfile.TemporaryDirectory() as temporary:
            binary=Path(temporary)/'input-probe'
            sources=[str(ROOT/'tests/botw_touch_input_probe.c'),str(ADAPTER/'bridge.c'),
                     str(ROOT/'games/totk/runtime/observation.c')]
            sources += [str(path) for path in sorted(CORE.glob('hle_*.c'))]
            sources.append(str(INPUT/'hle_touch.c'))
            build=subprocess.run(['xcrun','--sdk','macosx','clang','-std=gnu11','-O1','-Wall','-Wextra','-Werror',
                                  '-Wno-unused-function','-Wno-unused-variable','-Wno-unused-parameter',
                                  '-DDM_TEST_HOOKS=1','-DGAME_BOTW190=1','-fsanitize=undefined',
                                  '-fno-sanitize-recover=all',*INCLUDES,*sources,'-o',str(binary)],capture_output=True,text=True,timeout=90)
            self.assertEqual(build.returncode,0,build.stdout+build.stderr)
            run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=30)
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)

    def test_original_behavior_checks(self):
        with tempfile.TemporaryDirectory() as temporary:
            for name in ('t_touchpad_geometry', 't_botw_touch_trackpad', 't_botw_portrait'):
                with self.subTest(check=name):
                    binary = Path(temporary) / name
                    build = subprocess.run(
                        ['xcrun', '--sdk', 'macosx', 'clang', '-std=gnu11', '-O1',
                         '-Wall', '-Wextra', '-Werror', '-fsanitize=undefined',
                         '-fno-sanitize-recover=all', *INCLUDES, str(CORE / 'tests' / (name + '.c')),
                         '-o', str(binary)], capture_output=True, text=True, timeout=60)
                    self.assertEqual(build.returncode, 0, build.stdout + build.stderr)
                    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
                    self.assertEqual(run.returncode, 0, run.stdout + run.stderr)

    def test_original_native_feature_checks(self):
        with tempfile.TemporaryDirectory() as temporary:
            for name in ('t_botw_touch_menu', 't_botw_touchhud', 't_botw_touch_actions', 't_look', 't_aspect', 't_touch'):
                with self.subTest(check=name):
                    binary = Path(temporary) / name
                    # Upstream fixtures have unused helpers/parameters. Preserve their bytes, including those helpers.
                    command = ['xcrun', '--sdk', 'macosx', 'clang', '-std=gnu11', '-O1',
                               '-Wall', '-Wextra', '-Werror', '-Wno-unused-function', '-Wno-unused-parameter',
                               '-DGAME_BOTW190=1', '-fsanitize=undefined', '-fno-sanitize-recover=all', *INCLUDES]
                    if name == 't_look':
                        command.append('-DDM_TEST_HOOKS=1')
                    if name == 't_touch':
                        command.append(str(INPUT / 'hle_touch.c'))
                    fixture = INPUT if name == 't_touch' else CORE
                    build = subprocess.run(command + [str(fixture / 'tests' / (name + '.c')), '-o', str(binary)],
                                           capture_output=True, text=True, timeout=90)
                    self.assertEqual(build.returncode, 0, build.stdout + build.stderr)
                    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=60)
                    self.assertEqual(run.returncode, 0, run.stdout + run.stderr)


if __name__ == '__main__':
    unittest.main()
