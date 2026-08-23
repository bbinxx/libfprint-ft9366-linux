# Hardware

Observed November 13–14, 2026: ASUS DMI VivoBook_ASUSLaptop K3402ZA_S3402ZA;
USB 2808:a658, bcdDevice 1.12, Realtek USB2.0 Finger Print Bridge /
FocalTech Fingerprint Device. FT9366 and RTS5811 chip identities are reference-based.

Interface 0 has vendor-specific class ff, bulk OUT 0x01 and IN 0x82, both with
512-byte maximum packets. USB high speed, one configuration, maximum 100 mA.
`lsusb -v` could not open the device; some descriptor information was unavailable.

At inspection sysfs power/control was auto, runtime suspended, delay 2000 ms.
This does not prove a defect. Bus/device numbers and sysfs paths may change.
Recheck with `lsusb -d 2808:a658`; do not reset or claim interfaces without review.
