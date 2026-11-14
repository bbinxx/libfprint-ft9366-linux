Experimental libfprint-ft9366-linux release, prepared as a draft for maintainer review.

Assets: native Arch x86_64 package, corresponding pinned/modified libfprint and
FT9366 reference sources, project source archive, and SHA256SUMS. The source
bundle supports investigation and native packaging on other Linux distributions;
DEB/RPM installers are not included. See docs/user-guide.md before installation.

Track A packages the attributed FT9366 reference implementation; original Track B
is incomplete. Hardware evidence covers USB 2808:a658 on one Arch machine with
one completed enrollment and one verify-match. CI container compilation does not
establish hardware support on Ubuntu, Debian, Fedora or other distributions.
Repeated matching, rejection and suspend/resume still need validation.

The Arch package replaces system libfprint and includes a USB-ID-specific rule
disabling autosuspend. Prepare verified rollback and keep password authentication.
No PAM changes are included. CI-built assets are unsigned and require review.

Before publication, resolve the original project's license, review upstream
LGPL notices, test results, source correspondence and checksums. Do not upload
private templates, images or proprietary binaries. The workflow does not publish
this draft automatically.
