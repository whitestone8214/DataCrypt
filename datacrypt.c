/*
	gcc -O3 $(pkg-config --cflags --libs libfile) -I/usr/include/mbedtls -lmbed{crypto,tls,x509} datacrypt.c -o datacrypt
*/

#include <libfile.h>
#include <aes.h>
#include <termios.h>
#include <time.h>

struct termios _blob1, _blob2;
#define MAX_PASSWORD 16

int file_size (char *address) {
	struct stat _blob; if (stat (address, &_blob) != 0) return 0;
	return (int) _blob.st_size;
}
void free_if_not_null (void *whatever) {if (whatever != NULL) free (whatever);}

void die (int type) {
	printf ("\n");
	if (type == 0) {
		printf ("Usage:\n");
		printf ("datacrypt [encrypt | decrypt] [infile] [outfile] [keyfile?]\n");
		printf ("datacrypt create-key [outfile]\n");
	}
	else if (type == 1) {
		printf ("[datacrypt] ERROR: Cannot create key file. Check the permission, disk status, etc.\n");
	}
	printf ("\n");
	exit (1);
}

char _work; char *_from = NULL, *_to = NULL;
int _nth; char *_key = NULL; char *_fileA = NULL;
mbedtls_aes_context _blob; char *_result = NULL;

int main (int argc, char **argv) {
	/* Arguments. */
	if (argc < 3) die (0);
	_work = strcmp (argv[1], "encrypt") == 0 ? 0 : (strcmp (argv[1], "decrypt") == 0 ? 1 : (strcmp (argv[1], "create-key") == 0 ? 2 : -1)); if (_work == -1) die (0);
	if ((_work != 2) && (argc < 4)) die (0);
	_from = _work == 2 ? NULL : libfile_refine_address (argv[2]); if ((_work != 2) && (_from == NULL)) die (0);
	_to = libfile_refine_address (argv[_work == 2 ? 2 : 3]); if (_to == NULL) die (0);
	if ((_work != 2) && (argc >= 5)) _key = libfile_refine_address (argv[4]);
	
	/* Create key. */
	if (_work == 2) {
		int _file = open (_to, O_RDWR | O_CREAT | O_TRUNC, 0600); if (_file == -1) die (1);
		lseek (_file, 0, SEEK_SET); int _nth; for (_nth = 0; _nth < 16; _nth++) {srand (time (NULL)); unsigned char _byte[1] = {rand () / 256}; write (_file, &_byte[0], sizeof (unsigned char));}
		close (_file); printf ("[datacrypt] Done.\n"); return 0;
	}
	
	/* Key. */
	unsigned char _key1[16] = {0x00, 0x09, 0x5f, 0x34, 0x23, 0x84, 0xf0, 0xd9, 0xcc, 0xb0, 0xf1, 0x32, 0xaf, 0xef, 0xbb, 0xd2}; /* Default key. */
	if (_key != NULL) {
		int _file = open (_key, O_RDONLY); if (_file == -1) {printf ("[datacrypt] WARNING: Cannot open selected key file. Will use default key.\n");}
		if (file_size (_key) < 16) {printf ("[datacrypt] WARNING: Selected key file has size lesser than 16, so irrelevant to use. Will use default key.\n");}
		else {lseek (_file, 0, SEEK_SET); int _nth; for (_nth = 0; _nth < 16; _nth++) read (_file, &_key1[_nth], sizeof (unsigned char));}
		close (_file);
	}
	else printf ("[datacrypt] WARNING: No key file selected. Will use default key.\n");
	
	/* Password. */
	char *_password = (char *) malloc (sizeof (char) * MAX_PASSWORD); memset (_password, 0, MAX_PASSWORD);
	tcgetattr (fileno (stdin), &_blob1); tcgetattr (fileno (stdin), &_blob2); _blob2.c_lflag &= ~ECHO;
	tcsetattr (fileno (stdin), TCSANOW, &_blob2);
	printf ("Password (up to %d character(s)): ", MAX_PASSWORD); fgets (_password, MAX_PASSWORD, stdin); printf ("\n");
	tcsetattr (fileno (stdin), TCSANOW, &_blob1);
	
	/* Prepare. */
	int _size = file_size (_from); unsigned char _content[_size]; memset (&_content[0], 0, _size);
	int _from1 = open (_from, O_RDONLY); if (_from1 == -1) die (0);
	for (_nth = 0; _nth < _size; _nth++) pread (_from1, &_content[_nth], sizeof (unsigned char), _nth); close (_from1);
	int _size1 = _size; if (_work == 0) _size1 = _size + (16 - (_size % 16));
	unsigned char _content1[_size1]; for (_nth = 0; _nth < _size; _nth++) _content1[_nth] = _content[_nth];
	if (_size < _size1) {_content1[_nth] = 0x03; _nth++;} if (_size < _size1 - 1) while (_nth < _size1) {_content1[_nth] = 0; _nth++;}
	memset (&_blob, 0, sizeof (mbedtls_aes_context)); mbedtls_aes_init (&_blob);
	if (_work == 0) mbedtls_aes_setkey_enc (&_blob, &_password[0], 256); else mbedtls_aes_setkey_dec (&_blob, &_password[0], 256);
	unsigned char _result[_size1];
	
	/* Encrypt/decrypt and write into file. */
	mbedtls_aes_crypt_cbc (&_blob, _work == 1 ? 0 : 1, _size1, _key1, &_content1[0], &_result[0]);
	int _to1 = open (_to, O_RDWR | O_CREAT | O_TRUNC, 0600); if (_to1 == -1) {free (_from); free (_password); free (_to); die (0);}
	if (_work == 1) {
		if (_result[_size1 - 1] == 0x03) _size1--;
		else if (_result[_size1 - 1] == 0) {_nth = _size1 - 1; while (_result[_nth] != 0x03) _nth--; _size1 = _nth;}
	}
	for (_nth = 0; _nth < _size1; _nth++) {
		if ((_work == 1) && (_result[_nth] == 0)) break;
		pwrite (_to1, &_result[_nth], sizeof (unsigned char), _nth);
	}
	printf ("_nth = %d\n", _nth);
	close (_to1);
	
	/* Dispose. */
	free_if_not_null (_from); free_if_not_null (_password); free_if_not_null (_to); return 0;
}
