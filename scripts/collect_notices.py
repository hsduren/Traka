#!/usr/bin/env python3
"""Keep upstream notice texts in the app without removing source headers."""
from pathlib import Path
import re
root = Path(__file__).resolve().parent.parent
vendor = root / 'vendor/tracktion_engine'
blocks = []
seen = set()
for path in sorted(vendor.rglob('*')):
    if not path.is_file() or '.git' in path.parts:
        continue
    relative = path.relative_to(vendor)
    name = path.name.lower()
    full_license = name.startswith(('license', 'licence', 'copying', 'notice'))
    if not full_license and not ('3rd_party' in path.parts and path.suffix in ('.h', '.hpp', '.cpp', '.c')):
        continue
    text = path.read_text(errors='replace')
    if not full_license:
        match = re.match(r'\s*(/\*.*?\*/|(?://[^\n]*\n)+)', text, re.S)
        if not match:
            continue
        text = match.group(0)
        if not re.search(r'copyright|licen[cs]e|permission|public domain', text, re.I):
            continue
    if text in seen:
        continue
    seen.add(text)
    blocks.append(f'\n===== {relative.as_posix()} =====\n{text}\n')
(root / 'Resources/OpenSource/ThirdPartyLicenseTexts.txt').write_text(
    'Traka third-party copyright and license texts\n'
    'Collected unchanged from pinned upstream sources. SDK/example notices may refer to code not linked into Traka.\n'
    + ''.join(blocks))
print(f'Collected {len(blocks)} distinct upstream notices')
