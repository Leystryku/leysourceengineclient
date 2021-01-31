#include "../leychandefs.h"

typedef struct dataFragments_s
{
	FileHandle_t	file;			// open file handle
	char			filename[MAX_OSPATH]; // filename
	char* buffer;			// if NULL it's a file
	unsigned int	bytes;			// size in bytes
	unsigned int	bits;			// size in bits
	unsigned int	transferID;		// only for files
	bool			isCompressed;	// true if data is bzip compressed
	unsigned int	nUncompressedSize; // full size in bytes
	bool			asTCP;			// send as TCP stream
	int				numFragments;	// number of total fragments
	int				ackedFragments; // number of fragments send & acknowledged
	int				pendingFragments; // number of fragments send, but not acknowledged yet
	int fragmentOffsets[0xFFFF];
	int fragmentOffsets_num;

} dataFragments_t;