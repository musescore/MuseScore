/* fuzzer_metadata
 * Copyright (C) 2022-2023  Xiph.Org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <cstdio>
#include <cstring> /* for memcpy */
#include <unistd.h>
#include "FLAC++/metadata.h"
#include "common.h"

#define CONFIG_LENGTH 2

#define min(x,y) (x<y?x:y)

static void run_tests_with_level_0_interface(char filename[]);
static void run_tests_with_level_1_interface(char filename[], bool readonly, bool preservestats, const uint8_t *data, size_t size);
static void run_tests_with_level_2_interface(char filename[], bool ogg, bool use_padding, const uint8_t *data, size_t size);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	uint8_t command_length;
	char filename[] = "/tmp/tmpXXXXXX.flac";
	FLAC__bool init_bools[4];

	/* Use first byte for configuration, leave at least one byte of input */
	if(size < 1 + CONFIG_LENGTH){
		return 0;
	}

	/* First 4 bits for configuration bools, next 4 for length of command section */
	for(int i = 0; i < 4; i++)
		init_bools[i] = data[i/8] & (1 << (i % 8));

	command_length = data[0] >> 4;

	if(0)//data[1] < 128) /* Use MSB as on/off */
		alloc_check_threshold = data[1];
	else
		alloc_check_threshold = INT32_MAX;
	alloc_check_counter = 0;


	/* Leave at least one byte as input */
	if(command_length >= size - 1 - CONFIG_LENGTH)
		command_length = size - 1 - CONFIG_LENGTH;

	/* Dump input to file */
	{
		int file_to_fuzz = mkstemps(filename, 5);

		if (file_to_fuzz < 0)
			abort();
		write(file_to_fuzz,data+CONFIG_LENGTH+command_length,size-CONFIG_LENGTH-command_length);
		close(file_to_fuzz);
	}

	run_tests_with_level_0_interface(filename);
	run_tests_with_level_1_interface(filename, init_bools[1], init_bools[2], data+CONFIG_LENGTH, command_length/2);

	/* Dump input to file, to start fresh for level 2 */
	if(!init_bools[1]){
		FILE * file_to_fuzz = fopen(filename,"w");
		fwrite(data+CONFIG_LENGTH+command_length,1,size-CONFIG_LENGTH-command_length,file_to_fuzz);
		fclose(file_to_fuzz);
	}

	run_tests_with_level_2_interface(filename, init_bools[0], init_bools[3], data+command_length/2+CONFIG_LENGTH, command_length/2);

	remove(filename);

	return 0;
}

static void run_tests_with_level_0_interface(char filename[]) {
	FLAC::Metadata::StreamInfo streaminfo;
	FLAC::Metadata::VorbisComment vorbis_comment;
	FLAC::Metadata::CueSheet cue_sheet;
	FLAC::Metadata::Picture picture;

	FLAC::Metadata::get_streaminfo(filename,streaminfo);
	FLAC::Metadata::get_tags(filename,vorbis_comment);
	FLAC::Metadata::get_cuesheet(filename,cue_sheet);
	FLAC::Metadata::get_picture(filename,picture, (FLAC__StreamMetadata_Picture_Type)(1), NULL, NULL, -1, -1, -1, -1);
}

static void run_tests_with_level_1_interface(char filename[], bool readonly, bool preservestats, const uint8_t *data, size_t size) {
	FLAC::Metadata::SimpleIterator iterator;
	FLAC::Metadata::Prototype *metadata_block = nullptr;
	uint8_t id[4] = {0};

	if(!iterator.is_valid())
		return;

	if(!iterator.init(filename,readonly,preservestats))
		return;

	for(size_t i = 0; i < size && iterator.status() == FLAC__METADATA_SIMPLE_ITERATOR_STATUS_OK; i++) {
		switch(data[i] & 7) {
			case 0:
				iterator.get_block_type();
				iterator.get_block_offset();
				iterator.get_block_length();
				iterator.get_application_id(id);
				break;
			case 1:
				iterator.next();
				break;
			case 2:
				iterator.prev();
				break;
			case 3:
				iterator.delete_block(data[i] & 8);
				break;
			case 4:
				if(metadata_block != 0) {
					delete metadata_block;
					metadata_block = nullptr;
				}
				metadata_block = iterator.get_block();
				break;
			case 5:
				if(metadata_block != 0)
					iterator.set_block(metadata_block,data[i] & 8);
				break;
			case 6:
				if(metadata_block != 0)
					iterator.insert_block_after(metadata_block, data[i] & 8);
				break;
			case 7:
				iterator.status();
				iterator.is_last();
				iterator.is_writable();
				break;
		}
	}
	if(metadata_block != 0) {
		delete metadata_block;
		metadata_block = nullptr;
	}


}


static void run_tests_with_level_2_interface(char filename[], bool ogg, bool use_padding, const uint8_t *data, size_t size) {
	FLAC::Metadata::Chain chain;
	FLAC::Metadata::Iterator iterator;
	FLAC::Metadata::Prototype *metadata_block_get = nullptr;
	FLAC::Metadata::Prototype *metadata_block_transfer = nullptr;
	FLAC::Metadata::Prototype *metadata_block_put = nullptr;

	if(!chain.is_valid())
		return;

	if(!chain.read(filename, ogg))
		return;

	iterator.init(chain);

	for(size_t i = 0; i < size; i++) {
		switch(data[i] & 15) {
			case 0:
				iterator.get_block_type();
				break;
			case 1:
				iterator.next();
				break;
			case 2:
				iterator.prev();
				break;
			case 3:
				iterator.delete_block(data[i] & 16);
				break;
			case 4:
				metadata_block_get = iterator.get_block();
				if(metadata_block_get != 0 && metadata_block_get->is_valid()) {
					if(metadata_block_transfer != 0 && metadata_block_transfer->is_valid()) {
						if(metadata_block_transfer != metadata_block_get) {
							delete metadata_block_transfer;
							metadata_block_transfer = nullptr;
							metadata_block_transfer = FLAC::Metadata::clone(metadata_block_get);
						}
					}
					else {
						metadata_block_transfer = FLAC::Metadata::clone(metadata_block_get);
					}
				}
				delete metadata_block_get;
				break;
			case 5:
				if(metadata_block_transfer != 0 && metadata_block_transfer->is_valid()) {
					metadata_block_put = FLAC::Metadata::clone(metadata_block_transfer);
					if(metadata_block_put != 0 && metadata_block_put->is_valid()) {
						if(!iterator.insert_block_before(metadata_block_put))
							delete metadata_block_put;
					}
					else
						if(metadata_block_put != 0)
							delete metadata_block_put;
				}
				break;
			case 6:
				if(metadata_block_transfer != 0 && metadata_block_transfer->is_valid()) {
					metadata_block_put = FLAC::Metadata::clone(metadata_block_transfer);
					if(metadata_block_put != 0 && metadata_block_put->is_valid()) {
						if(!iterator.insert_block_after(metadata_block_put))
							delete metadata_block_put;
					}
					else
						if(metadata_block_put != 0)
							delete metadata_block_put;
				}
				break;
			case 7:
				if(metadata_block_transfer != 0 && metadata_block_transfer->is_valid()) {
					metadata_block_put = FLAC::Metadata::clone(metadata_block_transfer);
					if(metadata_block_put != 0 && metadata_block_put->is_valid()) {
						if(!iterator.set_block(metadata_block_put))
							delete metadata_block_put;
					}
					else
						if(metadata_block_put != 0)
							delete metadata_block_put;
				}
				break;
			case 8: /* Examine block */
				if(metadata_block_transfer != 0 && metadata_block_transfer->is_valid()) {
					switch(metadata_block_transfer->get_type()) {
						case FLAC__METADATA_TYPE_VORBIS_COMMENT:
						{
							uint32_t num_comments;
							::FLAC__StreamMetadata_VorbisComment_Entry entry;
							FLAC::Metadata::VorbisComment::Entry entry_cpp;
							FLAC::Metadata::VorbisComment * vorbiscomment = dynamic_cast<FLAC::Metadata::VorbisComment *>(metadata_block_transfer);
							const ::FLAC__StreamMetadata * metadata_c = *metadata_block_transfer;
							if(vorbiscomment == 0)
								abort();
							vorbiscomment->get_vendor_string();
							num_comments = vorbiscomment->get_num_comments();
							if(num_comments > 0) {
								entry = metadata_c->data.vorbis_comment.comments[min(data[i]>>4,num_comments-1)];
								if(entry.entry == 0)
									abort();
								if(vorbiscomment->get_comment(min(data[i]>>4,num_comments-1)).is_valid()) {
									entry_cpp = vorbiscomment->get_comment(min(data[i]>>4,num_comments-1));
									if(entry_cpp.is_valid() && entry_cpp.get_field() == 0)
										abort();
									vorbiscomment->find_entry_from(0,"TEST");
								}
							}

						}
						break;
						case FLAC__METADATA_TYPE_CUESHEET:
						{
							uint32_t num_tracks, num_indices;
							FLAC::Metadata::CueSheet * cuesheet = dynamic_cast<FLAC::Metadata::CueSheet *>(metadata_block_transfer);
							if(cuesheet == 0 || !cuesheet->is_legal())
								break;
							cuesheet->is_legal(true); /* check CDDA subset */
							cuesheet->calculate_cddb_id();
							cuesheet->get_media_catalog_number();
							cuesheet->get_lead_in();
							cuesheet->get_is_cd();
							num_tracks = cuesheet->get_num_tracks();
							if(num_tracks > 0) {
								FLAC::Metadata::CueSheet::Track track = cuesheet->get_track(min(data[i]>>4,num_tracks-1));
								track.get_offset();
								track.get_number();
								track.get_isrc();
								track.get_pre_emphasis();
								num_indices = track.get_num_indices();
								if(num_indices > 0) {
									FLAC__StreamMetadata_CueSheet_Index index = track.get_index(min(data[i]>>4,num_indices-1));
									(void)index;
								}
							}
						}
						break;
						case FLAC__METADATA_TYPE_PICTURE:
						{
							char * violation = nullptr;
							FLAC::Metadata::Picture * picture = dynamic_cast<FLAC::Metadata::Picture *>(metadata_block_transfer);
							if(picture == 0 || !picture->is_legal((const char **)&violation))
								break;
							picture->get_data();
						}
						break;
						default:
						break;
					}

				}
				break;
			case 9: /* Replace or add in block */
				if(metadata_block_transfer != 0 && metadata_block_transfer->is_valid()) {
					switch(metadata_block_transfer->get_type()) {
						case FLAC__METADATA_TYPE_SEEKTABLE:
						{
							uint32_t num_seekpoints;
							FLAC__StreamMetadata_SeekPoint seekpoint;
							FLAC::Metadata::SeekTable * seektable = dynamic_cast<FLAC::Metadata::SeekTable *>(metadata_block_transfer);
							if(seektable == 0)
								break;
							if(seektable->is_valid() && seektable->is_legal()) {
								num_seekpoints = seektable->get_num_points();
								if(num_seekpoints > 0) {
									seekpoint = seektable->get_point(min(data[i]>>5,num_seekpoints-1));
									seektable->set_point(0,seekpoint);
									seektable->insert_point(min(data[i]>>5,num_seekpoints-1),seekpoint);
								}
								seektable->template_append_placeholders(4);
								seektable->template_append_point(111111);
								seektable->template_append_points((FLAC__uint64[]){222222, 333333, 444444}, 3);
								seektable->template_append_spaced_points(data[i]>>5, 1234567);
								seektable->template_append_spaced_points_by_samples(data[i]>>5, 2468000);
								seektable->template_sort(data[i] & 16);
							}
						}
						case FLAC__METADATA_TYPE_VORBIS_COMMENT:
						{
							uint32_t num_comments;
							FLAC::Metadata::VorbisComment::Entry entry;
							FLAC::Metadata::VorbisComment * vorbiscomment = dynamic_cast<FLAC::Metadata::VorbisComment *>(metadata_block_transfer);
							if(vorbiscomment == 0)
								break;
							num_comments = vorbiscomment->get_num_comments();
							if(num_comments > 0 && entry.is_valid()) {
								if(vorbiscomment->get_comment(min(data[i]>>5,num_comments-1)).is_valid()) {
									entry = vorbiscomment->get_comment(min(data[i]>>5,num_comments-1));
									if(entry.is_valid()) {
										vorbiscomment->replace_comment(entry,data[i] & 16);
										vorbiscomment->set_comment(0,entry);
										vorbiscomment->append_comment(entry);
										vorbiscomment->insert_comment(0,entry);
									}
								}
							}
						}
						break;
						case FLAC__METADATA_TYPE_CUESHEET:
						{
							uint32_t num_tracks, num_indices;
							FLAC::Metadata::CueSheet * cuesheet = dynamic_cast<FLAC::Metadata::CueSheet *>(metadata_block_transfer);
							if(cuesheet == 0 || !cuesheet->is_legal())
								break;
							num_tracks = cuesheet->get_num_tracks();
							if(num_tracks > 0) {
								FLAC::Metadata::CueSheet::Track track = cuesheet->get_track(min(data[i]>>4,num_tracks-1));
								num_indices = track.get_num_indices();
								if(num_indices > 0) {
									FLAC__StreamMetadata_CueSheet_Index index = track.get_index(min(data[i]>>4,num_indices-1));
									track.set_index(0,index);
									cuesheet->insert_index(0,0,index);
									cuesheet->insert_blank_index(0,0);
								}
								cuesheet->insert_blank_track(0);
								cuesheet->insert_track(0,track);
								cuesheet->resize_indices(min(data[i]>>4,num_tracks-1),data[i]>>4);
							}
						}
						break;
						case FLAC__METADATA_TYPE_PICTURE:
						{
							FLAC::Metadata::Picture * picture = dynamic_cast<FLAC::Metadata::Picture *>(metadata_block_transfer);
							const char testtext[] = "TEST";
							if(picture == 0 || !picture->is_legal(NULL))
								break;
							picture->set_description((FLAC__byte *)&testtext);
							picture->set_mime_type((const char *)&testtext);
							picture->set_data((FLAC__byte *)&testtext,4);
						}
						break;
						default:
						break;
					}

				}
				break;
			case 10: /* Delete from block */
				if(metadata_block_transfer != 0 && metadata_block_transfer->is_valid()) {
					switch(metadata_block_transfer->get_type()) {
						case FLAC__METADATA_TYPE_SEEKTABLE:
						{
							uint32_t num_seekpoints;
							FLAC::Metadata::SeekTable * seektable = dynamic_cast<FLAC::Metadata::SeekTable *>(metadata_block_transfer);
							if(seektable == 0)
								break;
							if(seektable->is_valid() && seektable->is_legal()) {
								num_seekpoints = seektable->get_num_points();
								if(num_seekpoints > 0)
									seektable->delete_point(min(data[i]>>4,num_seekpoints-1));
							}
						}
						case FLAC__METADATA_TYPE_VORBIS_COMMENT:
						{
							uint32_t num_comments;
							FLAC::Metadata::VorbisComment * vorbiscomment = dynamic_cast<FLAC::Metadata::VorbisComment *>(metadata_block_transfer);
							if(vorbiscomment == 0)
								break;
							num_comments = vorbiscomment->get_num_comments();
							if(num_comments > 0)
								vorbiscomment->delete_comment(min(data[i]>>4,num_comments-1));
							vorbiscomment->remove_entry_matching("TEST");
							vorbiscomment->remove_entries_matching("TEST");
						}
						break;
						case FLAC__METADATA_TYPE_CUESHEET:
						{
							uint32_t num_tracks;
							FLAC::Metadata::CueSheet * cuesheet = dynamic_cast<FLAC::Metadata::CueSheet *>(metadata_block_transfer);
							if(cuesheet == 0 || !cuesheet->is_legal())
								break;
							num_tracks = cuesheet->get_num_tracks();
							if(num_tracks > 0) {
								FLAC::Metadata::CueSheet::Track track = cuesheet->get_track(min(data[i]>>4,num_tracks-1));
								if(track.get_num_indices() > 0)
									cuesheet->delete_index(min(data[i]>>4,num_tracks-1),0);
								cuesheet->delete_track(0);
							}
						}
						break;
						default:
						break;
					}

				}
				break;
			case 11: /* Resize block */
				if(metadata_block_transfer != 0 && metadata_block_transfer->is_valid()) {
					switch(metadata_block_transfer->get_type()) {
						case FLAC__METADATA_TYPE_PADDING:
						{
							FLAC::Metadata::Padding * padding = dynamic_cast<FLAC::Metadata::Padding *>(metadata_block_transfer);
							if(padding == 0)
								break;
							padding->set_length(data[i]>>4);
						}
						break;
						case FLAC__METADATA_TYPE_SEEKTABLE:
						{
							FLAC::Metadata::SeekTable * seektable = dynamic_cast<FLAC::Metadata::SeekTable *>(metadata_block_transfer);
							if(seektable == 0)
								break;
							seektable->resize_points(data[i]>>4);
						}
						break;
						case FLAC__METADATA_TYPE_VORBIS_COMMENT:
						{
							FLAC::Metadata::VorbisComment * vorbiscomment = dynamic_cast<FLAC::Metadata::VorbisComment *>(metadata_block_transfer);
							if(vorbiscomment == 0)
								break;
							vorbiscomment->resize_comments(data[i]>>4);
						}
						break;
						case FLAC__METADATA_TYPE_CUESHEET:
						{
							uint32_t num_tracks;
							FLAC::Metadata::CueSheet * cuesheet = dynamic_cast<FLAC::Metadata::CueSheet *>(metadata_block_transfer);
							if(cuesheet == 0 || !cuesheet->is_legal())
								break;
							num_tracks = cuesheet->get_num_tracks();
							if(num_tracks > 0) {
								cuesheet->resize_indices(min(data[i]>>4,num_tracks-1),data[i]>>4);
							}
							cuesheet->resize_tracks(data[i]<<4);
						}
						break;
						default:
						break;
					}

				}
				break;
			case 12: /* Prototype functions */
				if(metadata_block_transfer != 0 && metadata_block_transfer->is_valid()) {
					const ::FLAC__StreamMetadata * metadata_compare = *metadata_block_transfer;
					metadata_block_transfer->get_is_last();
					metadata_block_transfer->get_length();
					metadata_block_transfer->set_is_last(data[i] & 16);
					FLAC__metadata_object_is_equal(metadata_compare, metadata_compare);
				}
				break;
		}
	}
	if(metadata_block_transfer != 0) {
		delete metadata_block_transfer;
		metadata_block_transfer = nullptr;
	}

	chain.status();
	chain.sort_padding();
	chain.merge_padding();

	chain.check_if_tempfile_needed(!use_padding);
	chain.write(use_padding);

}
