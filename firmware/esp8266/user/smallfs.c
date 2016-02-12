#include "smallfs.h"
#include <string.h>
#ifdef __linux__

#include <fcntl.h>
#include <unistd.h>
#include <endian.h>
#include <stdio.h>
#include <strings.h>

#define BE32(x) be32toh(x)

#else

#define BE32(x) ((((x) & 0x000000ff) << 24) |      \
                      (((x) & 0x0000ff00) << 8)  |      \
                      (((x) & 0x00ff0000) >> 8)  |      \
                      (((x) & 0xff000000) >> 24))

#endif

#undef SMALLFSDEBUG

extern int smallfs_start;

int ICACHE_FLASH_ATTR smallfs__begin(struct smallfs *f, unsigned offset)
{
    f->fsstart = offset;
    f->offset = 0xffffffff;
    flash_control_init(&f->fc);
    smallfs__read(f, f->fsstart, &f->hdr,sizeof(f->hdr));
    if ( BE32(f->hdr.magic) == SMALLFS_MAGIC ) {
        return 0;
    } else {
        flash_control_release(&f->fc);
    }
    return -1;
}

void ICACHE_FLASH_ATTR smallfs__end(struct smallfs *f)
{
    flash_control_release(&f->fc);
}

void ICACHE_FLASH_ATTR smallfs__seek_if_needed(struct smallfs *f, unsigned address)
{
    if (address!=f->offset)
    {
        f->offset = address;
    }
}

uint8_t ICACHE_FLASH_ATTR smallfs__readByte(struct smallfs *fs, unsigned address)
{
    uint8_t byte;
    smallfs__seek_if_needed(fs, address);

    spiflash_read_cached(&fs->fc, fs->offset, &byte, 1);
    fs->offset++;
    return byte;
}


void ICACHE_FLASH_ATTR smallfs__read(struct smallfs *fs, unsigned address, void *target, unsigned size)
{
    smallfs__seek_if_needed(fs, address);
    spiflash_read_cached(&fs->fc, fs->offset, target, size);
}

int ICACHE_FLASH_ATTR smallfs__getFirstEntry(struct smallfs *fs, struct smallfsentry *e)
{
    int r = -1;
    unsigned o = fs->fsstart + sizeof(struct smallfs_header);

    if (BE32(fs->hdr.numfiles)==0) {
        smallfsentry__init(e,fs);
    }
    else {
        smallfsentry__initadv(e,o,0);
        r=0;
    }
    return r;
}

bool ICACHE_FLASH_ATTR smallfsentry__hasNext(const struct smallfsentry *e)
{
    if (!(smallfsentry__valid(e)))
        return false;
    return (e->m_index+1)<smallfs__getCount(e->fs);
}

bool ICACHE_FLASH_ATTR smallfsentry__equals(const struct smallfsentry *e, const char *name)
{
    if (!(smallfsentry__valid(e)))
        return false;

    struct smallfs_entry he;
    char buf[256];
    smallfs__read(e->fs, e->m_offset, &he,sizeof(struct smallfs_entry));
    smallfs__read(e->fs, e->m_offset + sizeof(struct smallfs_entry), buf, he.namesize);
    buf[he.namesize] = '\0';
    return (strcmp(name,buf)==0);
}

bool ICACHE_FLASH_ATTR smallfsentry__endsWith(const struct smallfsentry *e, const char *name)
{
    struct smallfs_entry he;
    char buf[256];
    smallfs__read(e->fs, e->m_offset, &he,sizeof(struct smallfs_entry));
    smallfs__read(e->fs, e->m_offset + sizeof(struct smallfs_entry), buf, he.namesize);
    buf[he.namesize] = '\0';
    unsigned l = strlen(name);
    if (l>he.namesize)
        return false;
    char *p = buf + he.namesize - l;
    return (strcmp(name,p)==0);
}

void ICACHE_FLASH_ATTR smallfsentry__getName(const struct smallfsentry *e, char *name)
{
    struct smallfs_entry he;
    smallfs__read( e->fs, e->m_offset, &he,sizeof(struct smallfs_entry));
    smallfs__read( e->fs, e->m_offset + sizeof(struct smallfs_entry), name, he.namesize);
    name[he.namesize] = '\0';
}

bool ICACHE_FLASH_ATTR smallfsentry__startsWith(const struct smallfsentry *e, const char *name)
{
    struct smallfs_entry he;
    char buf[256];
    smallfs__read( e->fs, e->m_offset, &he, sizeof(struct smallfs_entry));
    smallfs__read( e->fs, e->m_offset + sizeof(struct smallfs_entry), buf, he.namesize);
    buf[he.namesize] = '\0';
    return (strncmp(name,buf,strlen(name))==0);
}

int ICACHE_FLASH_ATTR smallfsentry__open(struct smallfsentry *e, struct smallfsfile *file)
{
    return smallfs__openByOffset(e->fs, file, e->m_offset);
};

void ICACHE_FLASH_ATTR smallfsentry__advance(struct smallfsentry *e,int amount) {
    if (smallfsentry__hasNext(e)) {
        struct smallfs_entry he;
        e->m_index++;
        /* Recompute offset */
        smallfs__read(e->fs, e->m_offset, &he,sizeof(struct smallfs_entry));
        e->m_offset+=sizeof(struct smallfs_entry);
        e->m_offset+=he.namesize;
    }
}

int ICACHE_FLASH_ATTR smallfs__openByOffset(struct smallfs*fs,struct smallfsfile *file, unsigned offset)
{
    struct smallfs_entry e;
    smallfs__read(fs, offset, &e,sizeof(struct smallfs_entry));
    smallfs__seek_if_needed(fs, BE32(e.offset) + fs->fsstart);
    smallfsfile__init(file, fs, BE32(e.offset) + fs->fsstart, BE32(e.size));
    return 0;
}

int ICACHE_FLASH_ATTR smallfs__open(struct smallfs *fs, struct smallfsfile *file, const char *name)
{
    /* Start at root offset */
    unsigned o = fs->fsstart + sizeof(struct smallfs_header);
    unsigned char buf[256];
    struct smallfs_entry he;

    int c;

    for (c=BE32(fs->hdr.numfiles); c; c--) {

        smallfs__read(fs, o, &he,sizeof(struct smallfs_entry));
        o+=sizeof(struct smallfs_entry);

        smallfs__read(fs, o, buf, he.namesize);
        o+=he.namesize;

        buf[he.namesize] = '\0';
        /* Compare */
        if (strcmp((const char*)buf,name)==0) {

            // Seek and readahead
            smallfs__seek_if_needed(fs, BE32(he.offset) + fs->fsstart);
            smallfsfile__init(file, fs, BE32(he.offset) + fs->fsstart, BE32(he.size));
            return 0;
        }
    }
    // Reset offset.
    fs->offset=(unsigned)-1;
    smallfsfile__initempty(file);
    return -1;
}

int ICACHE_FLASH_ATTR smallfsfile__read(struct smallfsfile *file, void *buf, int s)
{
	if (!smallfsfile__valid(file))
		return -1;

	if (file->seekpos==file->filesize)
		return 0; /* EOF */

	if (s + file->seekpos > file->filesize) {
		s = file->filesize-file->seekpos;
	}
	smallfs__read(file->fs, file->seekpos + file->flashoffset, buf, s);

	file->seekpos+=s;
	return s;
}

int ICACHE_FLASH_ATTR smallfsfile__readCallback(struct smallfsfile *file, int s, void (*callback)(unsigned char, void*), void *data)
{
	unsigned char c;
	int save_s;

	if (!smallfsfile__valid(file))
		return -1;

        if (file->seekpos==file->filesize)
		return 0; /* EOF */

	if (s + file->seekpos > file->filesize) {
		s = file->filesize-file->seekpos;
	}
#ifdef SMALLFSDEBUG
	Serial.print("Will read ");
	Serial.print(s);
	Serial.print(" bytes from file at offset ");
	Serial.print(flashoffset);
	Serial.print(" seekpos is ");

	Serial.println(seekpos);
#endif

	//SmallFS.spi_enable();

	//SmallFS.startread( seekpos + flashoffset );
	save_s = s;
	unsigned tpos = file->seekpos + file->flashoffset;
	file->seekpos += s;

	while (s--) {
		c=smallfs__readByte(file->fs,tpos++);
		callback(c,data);
	}

	return save_s;
}

void ICACHE_FLASH_ATTR smallfsfile__seek(struct smallfsfile *file, int pos, int whence)
{
	int newpos;

	if (whence==SEEK_SET)
		newpos = pos;
	else if (whence==SEEK_CUR)
		newpos = file->seekpos + pos;
	else
		newpos = file->filesize + pos;

	if (newpos>file->filesize)
		newpos=file->filesize;

	if (newpos<0)
		newpos=0;

	file->seekpos=newpos;
	smallfs__seek(file->fs, file->seekpos + file->flashoffset);
}

//struct smallfs_header SmallFS_class::hdr;
//unsigned SmallFS_class::fsstart=0;
//unsigned SmallFS_class::offset=0;