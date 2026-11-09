"""An external profile must reach the TOTK package step with its selected identity."""
from contextlib import ExitStack
from pathlib import Path
import os
import tempfile
import unittest
from unittest.mock import patch

import build_app


class ExternalIosProfile(unittest.TestCase):
    def test_update_profile_does_not_change_base_game_rewrite(self):
        with tempfile.TemporaryDirectory() as tmp:
            profile = Path(tmp)/'profile.py'
            profile.write_text("def expected_rewritten(version):\n    assert version == 'v143'\n    return {'main.patched.bin': 'custom'}\n")
            with patch.dict(os.environ, MNG_REWRITE_PROFILE=str(profile), MNG_SELECTED_VERSION='v143'):
                native = {'main.patched.bin': 'native'}
                helper = build_app.rewriter_profile
                self.assertEqual(helper.expected_rewritten(native, 'v0', helper.load('v0')), native)
                self.assertEqual(helper.expected_rewritten(native, 'v143', helper.load('v143')),
                                 {'main.patched.bin': 'custom'})

    def test_totk_builder_identity_and_version_boundary(self):
        with tempfile.TemporaryDirectory() as tmp, ExitStack() as stack:
            root = Path(tmp)
            base, update, keys = [root/name for name in ('base.nsp', 'update.nsp', 'prod.keys')]
            for path in (base, update, keys):
                path.touch()
            profile = {'IOS_BUILDER': '/external/package.py', 'ios_arguments': lambda args: ['--external-option']}
            stack.enter_context(patch.dict(os.environ, MNG_SELECTED_GAME='totk', MNG_SELECTED_VERSION='v143',
                                           MNG_APP_IDENTITY='["Custom title", "org.example.custom", "Custom"]'))
            stack.enter_context(patch.dict(build_app.APPS))
            for name in ('font_report', 'step_code', 'step_data', 'step_update_data', 'step_shaders',
                         'step_binary', 'build_device_translator', 'say', 'say_device_pipelines'):
                stack.enter_context(patch.object(build_app, name))
            stack.enter_context(patch.object(build_app, 'build_profile', return_value=profile))
            stack.enter_context(patch.object(build_app, 'input_kind', return_value='nsp'))
            stack.enter_context(patch.object(build_app, 'prerequisites', return_value=Path('/python')))
            stack.enter_context(patch.object(build_app, 'prepare_work', return_value=root/'totk'))
            stack.enter_context(patch.object(build_app, 'identify_game', return_value='totk'))
            stack.enter_context(patch.object(build_app.shutil, 'disk_usage',
                                           return_value=build_app.shutil.disk_usage(root)._replace(free=100*2**30)))
            identify = stack.enter_context(patch.object(build_app, 'identify_update', return_value=('v143', update)))
            run = stack.enter_context(patch.object(build_app, 'run'))
            argv = [str(base), '-k', str(keys), '--update', str(update), '--shared-fonts', str(root),
                    '--platform', 'ios', '--work', str(root/'work'), '--out', str(root/'custom.ipa'),
                    '--no-shader-reminder']
            build_app.main(argv)
            command = run.call_args.args[0]
            self.assertEqual(command[1], '/external/package.py')
            self.assertEqual(command[command.index('--app-name')+1], 'Custom')
            self.assertEqual(command[command.index('--bundle-id')+1], 'org.example.custom')
            self.assertIn('--external-option', command)
            identify.return_value = ('v0', update)
            run.reset_mock()
            with self.assertRaisesRegex(build_app.BuildError, 'Selected version v143, but the input is v0'):
                build_app.main(argv)
            run.assert_not_called()
