"""TouchNX-owned build helper; select its external checkout with TOUCHNX_ROOT."""
import os
from pathlib import Path
import runpy
_TNX=Path(os.environ.get('TOUCHNX_ROOT',Path(__file__).resolve().parents[4]/'touchnx')).resolve()
globals().update(runpy.run_path(str(_TNX/'games/botw/backends/macnativegames/tools/resource_packages.py'),run_name=__name__))
