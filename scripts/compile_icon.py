#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2026 hsduren and Traka contributors
"""Compile the Icon Composer source and merge actool's icon metadata into the bundle."""
from pathlib import Path
import plistlib, shutil, subprocess, sys, tempfile
bundle, icon = map(Path, sys.argv[1:3])
resources = bundle/'Contents/Resources'
resources.mkdir(parents=True, exist_ok=True)
with tempfile.TemporaryDirectory(prefix='traka-icon-') as temp:
    metadata=Path(temp)/'icon-info.plist'
    subprocess.run(['xcrun','actool','--compile',str(resources),'--platform','macosx',
                    '--minimum-deployment-target','13.0','--output-partial-info-plist',str(metadata),
                    '--app-icon','AppIcon',str(icon)],check=True)
    path=bundle/'Contents/Info.plist'
    info=plistlib.loads(path.read_bytes())
    info.update(plistlib.loads(metadata.read_bytes()))
    path.write_bytes(plistlib.dumps(info))
    if not (resources/'AppIcon.icns').exists() or info.get('CFBundleIconName')!='AppIcon':
        raise SystemExit('Icon compiler did not produce the expected application icon')

# Keep the bundled editable source in sync, removing obsolete draft layers.
destination=resources/'AppIcon.icon'
if destination.exists():
    shutil.rmtree(destination)
shutil.copytree(icon,destination)
