DataCrypt - File encryption/decryption software.

Version 0.1

Copyright (C) 2016 Minho Jo <whitestone8214@openmailbox.org>

License: GNU General Public License version 3 (or any later version) (see license.txt)

Usage:
	datacrypt [encrypt | decrypt] [infile] [outfile] [keyfile?] :: Encrypt/decrypt file
	datacrypt create-key [outfile] :: Create key file

Option:
	
Shortcut keys:
	
Note:
	- This program uses:
		- LibFile (https://github.com/whitestone8214/libfile)
		- mbedTLS (https://github.com/ARMmbed/mbedtls)
	- Encryption method used in this software is AES-256-CBC.
