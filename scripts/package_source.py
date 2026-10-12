#!/usr/bin/env python3
"""Package the exact committed app and pinned dependencies, without .git/build files."""
from pathlib import Path
import io, json, subprocess, tarfile, re
root = Path(__file__).resolve().parent.parent
def git(path, *args):
    return subprocess.check_output(['git', '-C', str(path), *args])
if git(root, 'status', '--porcelain').strip():
    raise SystemExit('Commit the reviewed source before packaging.')
version = re.search(r'project\(Traka VERSION ([0-9.]+)', (root/'CMakeLists.txt').read_text()).group(1)
output = root/'release'; output.mkdir(exist_ok=True)
repos = [(root, ''), (root/'vendor/tracktion_engine', 'vendor/tracktion_engine/'),
         (root/'vendor/tracktion_engine/modules/juce', 'vendor/tracktion_engine/modules/juce/')]
manifest = {'version': version, 'repositories': []}
archive = output/f'Traka-{version}-source.tar.gz'
prefix = f'Traka-{version}/'
with tarfile.open(archive, 'w:gz') as target:
    for path, subdir in repos:
        if git(path, 'status', '--porcelain').strip():
            raise SystemExit(f'Dependency contains uncommitted changes: {subdir}')
        sha = git(path, 'rev-parse', 'HEAD').decode().strip()
        manifest['repositories'].append({'path': subdir or '.', 'commit': sha})
        data = git(path, 'archive', '--format=tar', f'--prefix={prefix}{subdir}', sha)
        with tarfile.open(fileobj=io.BytesIO(data)) as source:
            for member in source:
                if member.isdir() or member.isfile() or member.issym():
                    target.addfile(member, source.extractfile(member) if member.isfile() else None)
    data = json.dumps(manifest, indent=2).encode()
    member = tarfile.TarInfo(prefix+'SOURCE_MANIFEST.json'); member.size=len(data)
    target.addfile(member, io.BytesIO(data))
print(archive)
