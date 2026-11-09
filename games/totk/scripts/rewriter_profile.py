"""Optional build-time rewrite inputs supplied by an integrating project."""
import hashlib
import os
from pathlib import Path
import runpy


def load():
    path = os.environ.get('TOUCHNX_REWRITE_PROFILE')
    return runpy.run_path(str(Path(path).resolve())) if path else None


def fingerprint(rewriter, profile=None):
    # Preserve the native rewriter's existing fingerprint when no integration is selected.
    source = Path(rewriter).read_bytes()
    if not profile:
        return hashlib.sha256(source).hexdigest()
    digest = hashlib.sha256(b'external rewrite profile v1\0')
    paths = [Path(__file__), Path(profile['__file__']), *map(Path, profile['inputs']())]
    for data in [source, *(path.read_bytes() for path in paths)]:
        digest.update(len(data).to_bytes(8, 'big'))
        digest.update(data)
    return digest.hexdigest()


def expected_rewritten(expected, version, profile):
    result = dict(expected)
    if profile:
        result.update(profile['expected_rewritten'](version))
    return result
