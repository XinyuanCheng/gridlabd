/* $Id: file.h 4738 2014-07-03 00:55:39Z dchassin $
 *	Copyright (C) 2008 Battelle Memorial Institute
 */

#ifndef _FILE_H
#define _FILE_H

int file_open_player(struct player *my, const char *fname, const char *flags);
char *file_read_player(struct player *my,char *buffer,unsigned int size);
int file_rewind_player(struct player *my);
void file_close_player(struct player *my);

int file_open_shaper(struct shaper *my, const char *fname, const char *flags);
char *file_read_shaper(struct shaper *my,char *buffer,unsigned int size);
int file_rewind_shaper(struct shaper *my);
void file_close_shaper(struct shaper *my);

int file_open_recorder(struct recorder *my, const char *fname, const char *flags);
int file_write_recorder(struct recorder *my, const char *timestamp, const char *value);
void file_close_recorder(struct recorder *my);

int file_open_collector(struct collector *my, const char *fname, const char *flags);
int file_write_collector(struct collector *my, const char *timestamp, const char *value);
void file_close_collector(struct collector *my);

#endif
