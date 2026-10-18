// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 hsduren and Traka contributors
#import <AppKit/AppKit.h>

void trakaRefreshDockIcon()
{
    // Read the current bundle bytes directly: Launch Services may retain an older
    // icon for a rebuilt app at the same path and bundle identifier.
    NSString* path = [[NSBundle mainBundle] pathForResource:@"AppIcon" ofType:@"icns"];
    if (path == nil) return;
    NSImage* icon = [[NSImage alloc] initWithContentsOfFile:path];
    if (icon == nil) return;
    [NSApp setApplicationIconImage:icon];
    [[NSApp dockTile] display];
#if !__has_feature(objc_arc)
    [icon release];
#endif
}
