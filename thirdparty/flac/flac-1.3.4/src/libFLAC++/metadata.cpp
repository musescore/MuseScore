/* libFLAC++ - Free Lossless Audio Codec library
 * Copyright (C) 2002-2009  Josh Coalson
 * Copyright (C) 2011-2016  Xiph.Org Foundation
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

#define __STDC_LIMIT_MACROS 1 /* otherwise SIZE_MAX is not defined for c++ */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "share/alloc.h"
#include "FLAC++/metadata.h"
#include "FLAC/assert.h"
#include <cstdlib> // for malloc(), free()
#include <cstring> // for memcpy() etc.

#ifdef _MSC_VER
// warning C4800: 'int' : forcing to bool 'true' or 'false' (performance warning)
#pragma warning ( disable : 4800 )
#endif

namespace FLAC {
	namespace Metadata {

		// local utility routines

		namespace local {

			Prototype *construct_block(::FLAC__StreamMetadata *object)
			{
				if (0 == object)
					return 0;

				Prototype *ret = 0;
				switch(object->type) {
					case FLAC__METADATA_TYPE_STREAMINFO:
						ret = new StreamInfo(object, /*copy=*/false);
						break;
					case FLAC__METADATA_TYPE_PADDING:
						ret = new Padding(object, /*copy=*/false);
						break;
					case FLAC__METADATA_TYPE_APPLICATION:
						ret = new Application(object, /*copy=*/false);
						break;
					case FLAC__METADATA_TYPE_SEEKTABLE:
						ret = new SeekTable(object, /*copy=*/false);
						break;
					case FLAC__METADATA_TYPE_VORBIS_COMMENT:
						ret = new VorbisComment(object, /*copy=*/false);
						break;
					case FLAC__METADATA_TYPE_CUESHEET:
						ret = new CueSheet(object, /*copy=*/false);
						break;
					case FLAC__METADATA_TYPE_PICTURE:
						ret = new Picture(object, /*copy=*/false);
						break;
					default:
						ret = new Unknown(object, /*copy=*/false);
						break;
				}
				return ret;
			}

		} // namespace local

		FLACPP_API Prototype *clone(const Prototype *object)
		{
			FLAC__ASSERT(0 != object);

			const StreamInfo *streaminfo = dynamic_cast<const StreamInfo *>(object);
			const Padding *padding = dynamic_cast<const Padding *>(object);
			const Application *application = dynamic_cast<const Application *>(object);
			const SeekTable *seektable = dynamic_cast<const SeekTable *>(object);
			const VorbisComment *vorbiscomment = dynamic_cast<const VorbisComment *>(object);
			const CueSheet *cuesheet = dynamic_cast<const CueSheet *>(object);
			const Picture *picture = dynamic_cast<const Picture *>(object);
			const Unknown *unknown = dynamic_cast<const Unknown *>(object);

			if(0 != streaminfo)
				return new StreamInfo(*streaminfo);
			if(0 != padding)
				return new Padding(*padding);
			if(0 != application)
				return new Application(*application);
			if(0 != seektable)
				return new SeekTable(*seektable);
			if(0 != vorbiscomment)
				return new VorbisComment(*vorbiscomment);
			if(0 != cuesheet)
				return new CueSheet(*cuesheet);
			if(0 != picture)
				return new Picture(*picture);
			if(0 != unknown)
				return new Unknown(*unknown);

			FLAC__ASSERT(0);
			return 0;
		}

		//
		// Prototype
		//

		Prototype::Prototype(const Prototype &object):
		object_(::FLAC__metadata_object_clone(object.object_)),
		is_reference_(false)
		{
			FLAC__ASSERT(object.is_valid());
		}

		Prototype::Prototype(const ::FLAC__StreamMetadata &object):
		object_(::FLAC__metadata_object_clone(&object)),
		is_reference_(false)
		{
		}

		Prototype::Prototype(const ::FLAC__StreamMetadata *object):
		object_(::FLAC__metadata_object_clone(object)),
		is_reference_(false)
		{
			FLAC__ASSERT(0 != object);
		}

		Prototype::Prototype(::FLAC__StreamMetadata *object, bool copy):
		object_(copy? ::FLAC__metadata_object_clone(object) : object),
		is_reference_(false)
		{
			FLAC__ASSERT(0 != object);
		}

		Prototype::~Prototype()
		{
			clear();
		}

		void Prototype::clear()
		{
			if(0 != object_ && !is_reference_)
				FLAC__metadata_object_delete(object_);
			object_ = 0;
		}

		Prototype &Prototype::operator=(const Prototype &object)
		{
			FLAC__ASSERT(object.is_valid());
			clear();
			is_reference_ = false;
			object_ = ::FLAC__metadata_object_clone(object.object_);
			return *this;
		}

		Prototype &Prototype::operator=(const ::FLAC__StreamMetadata &object)
		{
			clear();
			is_reference_ = false;
			object_ = ::FLAC__metadata_object_clone(&object);
			return *this;
		}

		Prototype &Prototype::operator=(const ::FLAC__StreamMetadata *object)
		{
			FLAC__ASSERT(0 != object);
			clear();
			is_reference_ = false;
			object_ = ::FLAC__metadata_object_clone(object);
			return *this;
		}

		Prototype &Prototype::assign_object(::FLAC__StreamMetadata *object, bool copy)
		{
			FLAC__ASSERT(0 != object);
			clear();
			object_ = (copy? ::FLAC__metadata_object_clone(object) : object);
			is_reference_ = false;
			return *this;
		}

		bool Prototype::get_is_last() const
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(object_->is_last);
		}

		FLAC__MetadataType Prototype::get_type() const
		{
			FLAC__ASSERT(is_valid());
			return object_->type;
		}

		uint32_t Prototype::get_length() const
		{
			FLAC__ASSERT(is_valid());
			return object_->length;
		}

		void Prototype::set_is_last(bool value)
		{
			FLAC__ASSERT(is_valid());
			object_->is_last = value;
		}


		//
		// StreamInfo
		//

		StreamInfo::StreamInfo():
		Prototype(FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO), /*copy=*/false)
		{ }

		StreamInfo::~StreamInfo()
		{ }

		uint32_t StreamInfo::get_min_blocksize() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.stream_info.min_blocksize;
		}

		uint32_t StreamInfo::get_max_blocksize() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.stream_info.max_blocksize;
		}

		uint32_t StreamInfo::get_min_framesize() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.stream_info.min_framesize;
		}

		uint32_t StreamInfo::get_max_framesize() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.stream_info.max_framesize;
		}

		uint32_t StreamInfo::get_sample_rate() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.stream_info.sample_rate;
		}

		uint32_t StreamInfo::get_channels() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.stream_info.channels;
		}

		uint32_t StreamInfo::get_bits_per_sample() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.stream_info.bits_per_sample;
		}

		FLAC__uint64 StreamInfo::get_total_samples() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.stream_info.total_samples;
		}

		const FLAC__byte *StreamInfo::get_md5sum() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.stream_info.md5sum;
		}

		void StreamInfo::set_min_blocksize(uint32_t value)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(value >= FLAC__MIN_BLOCK_SIZE);
			FLAC__ASSERT(value <= FLAC__MAX_BLOCK_SIZE);
			object_->data.stream_info.min_blocksize = value;
		}

		void StreamInfo::set_max_blocksize(uint32_t value)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(value >= FLAC__MIN_BLOCK_SIZE);
			FLAC__ASSERT(value <= FLAC__MAX_BLOCK_SIZE);
			object_->data.stream_info.max_blocksize = value;
		}

		void StreamInfo::set_min_framesize(uint32_t value)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(value < (1u << FLAC__STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN));
			object_->data.stream_info.min_framesize = value;
		}

		void StreamInfo::set_max_framesize(uint32_t value)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(value < (1u << FLAC__STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN));
			object_->data.stream_info.max_framesize = value;
		}

		void StreamInfo::set_sample_rate(uint32_t value)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(FLAC__format_sample_rate_is_valid(value));
			object_->data.stream_info.sample_rate = value;
		}

		void StreamInfo::set_channels(uint32_t value)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(value > 0);
			FLAC__ASSERT(value <= FLAC__MAX_CHANNELS);
			object_->data.stream_info.channels = value;
		}

		void StreamInfo::set_bits_per_sample(uint32_t value)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(value >= FLAC__MIN_BITS_PER_SAMPLE);
			FLAC__ASSERT(value <= FLAC__MAX_BITS_PER_SAMPLE);
			object_->data.stream_info.bits_per_sample = value;
		}

		void StreamInfo::set_total_samples(FLAC__uint64 value)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(value < (((FLAC__uint64)1) << FLAC__STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN));
			object_->data.stream_info.total_samples = value;
		}

		void StreamInfo::set_md5sum(const FLAC__byte value[16])
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(0 != value);
			std::memcpy(object_->data.stream_info.md5sum, value, 16);
		}


		//
		// Padding
		//

		Padding::Padding():
		Prototype(FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING), /*copy=*/false)
		{ }

		Padding::Padding(uint32_t length):
		Prototype(FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING), /*copy=*/false)
		{
			set_length(length);
		}

		Padding::~Padding()
		{ }

		void Padding::set_length(uint32_t length)
		{
			FLAC__ASSERT(is_valid());
			object_->length = length;
		}


		//
		// Application
		//

		Application::Application():
		Prototype(FLAC__metadata_object_new(FLAC__METADATA_TYPE_APPLICATION), /*copy=*/false)
		{ }

		Application::~Application()
		{ }

		const FLAC__byte *Application::get_id() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.application.id;
		}

		const FLAC__byte *Application::get_data() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.application.data;
		}

		void Application::set_id(const FLAC__byte value[4])
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(0 != value);
			std::memcpy(object_->data.application.id, value, 4);
		}

		bool Application::set_data(const FLAC__byte *data, uint32_t length)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_application_set_data(object_, const_cast<FLAC__byte*>(data), length, true));
		}

		bool Application::set_data(FLAC__byte *data, uint32_t length, bool copy)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_application_set_data(object_, data, length, copy));
		}


		//
		// SeekTable
		//

		SeekTable::SeekTable():
		Prototype(FLAC__metadata_object_new(FLAC__METADATA_TYPE_SEEKTABLE), /*copy=*/false)
		{ }

		SeekTable::~SeekTable()
		{ }

		uint32_t SeekTable::get_num_points() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.seek_table.num_points;
		}

		::FLAC__StreamMetadata_SeekPoint SeekTable::get_point(uint32_t indx) const
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(indx < object_->data.seek_table.num_points);
			return object_->data.seek_table.points[indx];
		}

		bool SeekTable::resize_points(uint32_t new_num_points)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_seektable_resize_points(object_, new_num_points));
		}

		void SeekTable::set_point(uint32_t indx, const ::FLAC__StreamMetadata_SeekPoint &point)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(indx < object_->data.seek_table.num_points);
			::FLAC__metadata_object_seektable_set_point(object_, indx, point);
		}

		bool SeekTable::insert_point(uint32_t indx, const ::FLAC__StreamMetadata_SeekPoint &point)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(indx <= object_->data.seek_table.num_points);
			return static_cast<bool>(::FLAC__metadata_object_seektable_insert_point(object_, indx, point));
		}

		bool SeekTable::delete_point(uint32_t indx)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(indx < object_->data.seek_table.num_points);
			return static_cast<bool>(::FLAC__metadata_object_seektable_delete_point(object_, indx));
		}

		bool SeekTable::is_legal() const
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_seektable_is_legal(object_));
		}

		bool SeekTable::template_append_placeholders(uint32_t num)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_seektable_template_append_placeholders(object_, num));
		}

		bool SeekTable::template_append_point(FLAC__uint64 sample_number)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_seektable_template_append_point(object_, sample_number));
		}

		bool SeekTable::template_append_points(FLAC__uint64 sample_numbers[], uint32_t num)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_seektable_template_append_points(object_, sample_numbers, num));
		}

		bool SeekTable::template_append_spaced_points(uint32_t num, FLAC__uint64 total_samples)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_seektable_template_append_spaced_points(object_, num, total_samples));
		}

		bool SeekTable::template_append_spaced_points_by_samples(uint32_t samples, FLAC__uint64 total_samples)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_seektable_template_append_spaced_points_by_samples(object_, samples, total_samples));
		}

		bool SeekTable::template_sort(bool compact)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_seektable_template_sort(object_, compact));
		}


		//
		// VorbisComment::Entry
		//

		VorbisComment::Entry::Entry() :
			is_valid_(true),
			entry_(),
			field_name_(0),
			field_name_length_(0),
			field_value_(0),
			field_value_length_(0)
		{
			zero();
		}

		VorbisComment::Entry::Entry(const char *field, uint32_t field_length) :
			is_valid_(true),
			entry_(),
			field_name_(0),
			field_name_length_(0),
			field_value_(0),
			field_value_length_(0)
		{
			zero();
			construct(field, field_length);
		}

		VorbisComment::Entry::Entry(const char *field) :
			is_valid_(true),
			entry_(),
			field_name_(0),
			field_name_length_(0),
			field_value_(0),
			field_value_length_(0)
		{
			zero();
			construct(field);
		}

		VorbisComment::Entry::Entry(const char *field_name, const char *field_value, uint32_t field_value_length) :
			is_valid_(true),
			entry_(),
			field_name_(0),
			field_name_length_(0),
			field_value_(0),
			field_value_length_(0)
		{
			zero();
			construct(field_name, field_value, field_value_length);
		}

		VorbisComment::Entry::Entry(const char *field_name, const char *field_value) :
			is_valid_(true),
			entry_(),
			field_name_(0),
			field_name_length_(0),
			field_value_(0),
			field_value_length_(0)
		{
			zero();
			construct(field_name, field_value);
		}

		VorbisComment::Entry::Entry(const Entry &entry) :
			is_valid_(true),
			entry_(),
			field_name_(0),
			field_name_length_(0),
			field_value_(0),
			field_value_length_(0)
		{
			FLAC__ASSERT(entry.is_valid());
			zero();
			construct(reinterpret_cast<const char *>(entry.entry_.entry), entry.entry_.length);
		}

		VorbisComment::Entry &VorbisComment::Entry::operator=(const Entry &entry)
		{
			FLAC__ASSERT(entry.is_valid());
			clear();
			construct(reinterpret_cast<const char *>(entry.entry_.entry), entry.entry_.length);
			return *this;
		}

		VorbisComment::Entry::~Entry()
		{
			clear();
		}

		bool VorbisComment::Entry::is_valid() const
		{
			return is_valid_;
		}

		uint32_t VorbisComment::Entry::get_field_length() const
		{
			FLAC__ASSERT(is_valid());
			return entry_.length;
		}

		uint32_t VorbisComment::Entry::get_field_name_length() const
		{
			FLAC__ASSERT(is_valid());
			return field_name_length_;
		}

		uint32_t VorbisComment::Entry::get_field_value_length() const
		{
			FLAC__ASSERT(is_valid());
			return field_value_length_;
		}

		::FLAC__StreamMetadata_VorbisComment_Entry VorbisComment::Entry::get_entry() const
		{
			FLAC__ASSERT(is_valid());
			return entry_;
		}

		const char *VorbisComment::Entry::get_field() const
		{
			FLAC__ASSERT(is_valid());
			return reinterpret_cast<const char *>(entry_.entry);
		}

		const char *VorbisComment::Entry::get_field_name() const
		{
			FLAC__ASSERT(is_valid());
			return field_name_;
		}

		const char *VorbisComment::Entry::get_field_value() const
		{
			FLAC__ASSERT(is_valid());
			return field_value_;
		}

		bool VorbisComment::Entry::set_field(const char *field, uint32_t field_length)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(0 != field);

			if(!::FLAC__format_vorbiscomment_entry_is_legal(reinterpret_cast<const ::FLAC__byte*>(field), field_length))
				return is_valid_ = false;

			clear_entry();

			if(0 == (entry_.entry = static_cast<FLAC__byte*>(safe_malloc_add_2op_(field_length, /*+*/1)))) {
				is_valid_ = false;
			}
			else {
				entry_.length = field_length;
				std::memcpy(entry_.entry, field, field_length);
				entry_.entry[field_length] = '\0';
				(void) parse_field();
			}

			return is_valid_;
		}

		bool VorbisComment::Entry::set_field(const char *field)
		{
			return set_field(field, std::strlen(field));
		}

		bool VorbisComment::Entry::set_field_name(const char *field_name)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(0 != field_name);

			if(!::FLAC__format_vorbiscomment_entry_name_is_legal(field_name))
				return is_valid_ = false;

			clear_field_name();

			if(0 == (field_name_ = strdup(field_name))) {
				is_valid_ = false;
			}
			else {
				field_name_length_ = std::strlen(field_name_);
				compose_field();
			}

			return is_valid_;
		}

		bool VorbisComment::Entry::set_field_value(const char *field_value, uint32_t field_value_length)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(0 != field_value);

			if(!::FLAC__format_vorbiscomment_entry_value_is_legal(reinterpret_cast<const FLAC__byte*>(field_value), field_value_length))
				return is_valid_ = false;

			clear_field_value();

			if(0 == (field_value_ = static_cast<char *>(safe_malloc_add_2op_(field_value_length, /*+*/1)))) {
				is_valid_ = false;
			}
			else {
				field_value_length_ = field_value_length;
				std::memcpy(field_value_, field_value, field_value_length);
				field_value_[field_value_length] = '\0';
				compose_field();
			}

			return is_valid_;
		}

		bool VorbisComment::Entry::set_field_value(const char *field_value)
		{
			return set_field_value(field_value, std::strlen(field_value));
		}

		void VorbisComment::Entry::zero()
		{
			is_valid_ = true;
			entry_.length = 0;
			entry_.entry = 0;
			field_name_ = 0;
			field_name_length_ = 0;
			field_value_ = 0;
			field_value_length_ = 0;
		}

		void VorbisComment::Entry::clear()
		{
			clear_entry();
			clear_field_name();
			clear_field_value();
			is_valid_ = true;
		}

		void VorbisComment::Entry::clear_entry()
		{
			if(0 != entry_.entry) {
				std::free(entry_.entry);
				entry_.entry = 0;
				entry_.length = 0;
			}
		}

		void VorbisComment::Entry::clear_field_name()
		{
			if(0 != field_name_) {
				std::free(field_name_);
				field_name_ = 0;
				field_name_length_ = 0;
			}
		}

		void VorbisComment::Entry::clear_field_value()
		{
			if(0 != field_value_) {
				std::free(field_value_);
				field_value_ = 0;
				field_value_length_ = 0;
			}
		}

		void VorbisComment::Entry::construct(const char *field, uint32_t field_length)
		{
			if(set_field(field, field_length))
				parse_field();
		}

		void VorbisComment::Entry::construct(const char *field)
		{
			construct(field, std::strlen(field));
		}

		void VorbisComment::Entry::construct(const char *field_name, const char *field_value, uint32_t field_value_length)
		{
			if(set_field_name(field_name) && set_field_value(field_value, field_value_length))
				compose_field();
		}

		void VorbisComment::Entry::construct(const char *field_name, const char *field_value)
		{
			construct(field_name, field_value, std::strlen(field_value));
		}

		void VorbisComment::Entry::compose_field()
		{
			clear_entry();

			if(0 == (entry_.entry = static_cast<FLAC__byte*>(safe_malloc_add_4op_(field_name_length_, /*+*/1, /*+*/field_value_length_, /*+*/1)))) {
				is_valid_ = false;
			}
			else {
				std::memcpy(entry_.entry, field_name_, field_name_length_);
				entry_.length += field_name_length_;
				std::memcpy(entry_.entry + entry_.length, "=", 1);
				entry_.length += 1;
				if (field_value_length_ > 0)
					std::memcpy(entry_.entry + entry_.length, field_value_, field_value_length_);
				entry_.length += field_value_length_;
				entry_.entry[entry_.length] = '\0';
				is_valid_ = true;
			}
		}

		void VorbisComment::Entry::parse_field()
		{
			clear_field_name();
			clear_field_value();

			const char *p = static_cast<const char *>(std::memchr(entry_.entry, '=', entry_.length));

			if(0 == p)
				p = reinterpret_cast<const char *>(entry_.entry) + entry_.length;

			field_name_length_ = static_cast<uint32_t>(p - reinterpret_cast<const char *>(entry_.entry));
			if(0 == (field_name_ = static_cast<char *>(safe_malloc_add_2op_(field_name_length_, /*+*/1)))) { // +1 for the trailing \0
				is_valid_ = false;
				return;
			}
			std::memcpy(field_name_, entry_.entry, field_name_length_);
			field_name_[field_name_length_] = '\0';

			if(entry_.length - field_name_length_ == 0) {
				field_value_length_ = 0;
				if(0 == (field_value_ = static_cast<char *>(safe_malloc_(0)))) {
					is_valid_ = false;
					return;
				}
			}
			else {
				field_value_length_ = entry_.length - field_name_length_ - 1;
				if(0 == (field_value_ = static_cast<char *>(safe_malloc_add_2op_(field_value_length_, /*+*/1)))) { // +1 for the trailing \0
					is_valid_ = false;
					return;
				}
				std::memcpy(field_value_, ++p, field_value_length_);
				field_value_[field_value_length_] = '\0';
			}

			is_valid_ = true;
		}


		//
		// VorbisComment
		//

		VorbisComment::VorbisComment():
		Prototype(FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT), /*copy=*/false)
		{ }

		VorbisComment::~VorbisComment()
		{ }

		uint32_t VorbisComment::get_num_comments() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.vorbis_comment.num_comments;
		}

		const FLAC__byte *VorbisComment::get_vendor_string() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.vorbis_comment.vendor_string.entry;
		}

		VorbisComment::Entry VorbisComment::get_comment(uint32_t indx) const
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(indx < object_->data.vorbis_comment.num_comments);
			return Entry(reinterpret_cast<const char *>(object_->data.vorbis_comment.comments[indx].entry), object_->data.vorbis_comment.comments[indx].length);
		}

		bool VorbisComment::set_vendor_string(const FLAC__byte *string)
		{
			FLAC__ASSERT(is_valid());
			// vendor_string is a special kind of entry
			const ::FLAC__StreamMetadata_VorbisComment_Entry vendor_string = { static_cast<FLAC__uint32>(std::strlen(reinterpret_cast<const char *>(string))), const_cast<FLAC__byte*>(string) }; // we can cheat on const-ness because we make a copy below:
			return static_cast<bool>(::FLAC__metadata_object_vorbiscomment_set_vendor_string(object_, vendor_string, /*copy=*/true));
		}

		bool VorbisComment::resize_comments(uint32_t new_num_comments)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_vorbiscomment_resize_comments(object_, new_num_comments));
		}

		bool VorbisComment::set_comment(uint32_t indx, const VorbisComment::Entry &entry)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(indx < object_->data.vorbis_comment.num_comments);
			return static_cast<bool>(::FLAC__metadata_object_vorbiscomment_set_comment(object_, indx, entry.get_entry(), /*copy=*/true));
		}

		bool VorbisComment::insert_comment(uint32_t indx, const VorbisComment::Entry &entry)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(indx <= object_->data.vorbis_comment.num_comments);
			return static_cast<bool>(::FLAC__metadata_object_vorbiscomment_insert_comment(object_, indx, entry.get_entry(), /*copy=*/true));
		}

		bool VorbisComment::append_comment(const VorbisComment::Entry &entry)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_vorbiscomment_append_comment(object_, entry.get_entry(), /*copy=*/true));
		}

		bool VorbisComment::replace_comment(const VorbisComment::Entry &entry, bool all)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_vorbiscomment_replace_comment(object_, entry.get_entry(), static_cast<FLAC__bool>(all), /*copy=*/true));
		}

		bool VorbisComment::delete_comment(uint32_t indx)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(indx < object_->data.vorbis_comment.num_comments);
			return static_cast<bool>(::FLAC__metadata_object_vorbiscomment_delete_comment(object_, indx));
		}

		int VorbisComment::find_entry_from(uint32_t offset, const char *field_name)
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__metadata_object_vorbiscomment_find_entry_from(object_, offset, field_name);
		}

		int VorbisComment::remove_entry_matching(const char *field_name)
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__metadata_object_vorbiscomment_remove_entry_matching(object_, field_name);
		}

		int VorbisComment::remove_entries_matching(const char *field_name)
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__metadata_object_vorbiscomment_remove_entries_matching(object_, field_name);
		}


		//
		// CueSheet::Track
		//

		CueSheet::Track::Track():
		object_(::FLAC__metadata_object_cuesheet_track_new())
		{ }

		CueSheet::Track::Track(const ::FLAC__StreamMetadata_CueSheet_Track *track):
		object_(::FLAC__metadata_object_cuesheet_track_clone(track))
		{ }

		CueSheet::Track::Track(const Track &track):
		object_(::FLAC__metadata_object_cuesheet_track_clone(track.object_))
		{ }

		CueSheet::Track &CueSheet::Track::operator=(const Track &track)
		{
			if(0 != object_)
				::FLAC__metadata_object_cuesheet_track_delete(object_);
			object_ = ::FLAC__metadata_object_cuesheet_track_clone(track.object_);
			return *this;
		}

		CueSheet::Track::~Track()
		{
			if(0 != object_)
				::FLAC__metadata_object_cuesheet_track_delete(object_);
		}

		bool CueSheet::Track::is_valid() const
		{
			return(0 != object_);
		}

		::FLAC__StreamMetadata_CueSheet_Index CueSheet::Track::get_index(uint32_t i) const
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(i < object_->num_indices);
			return object_->indices[i];
		}

		void CueSheet::Track::set_isrc(const char value[12])
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(0 != value);
			std::memcpy(object_->isrc, value, 12);
			object_->isrc[12] = '\0';
		}

		void CueSheet::Track::set_type(uint32_t value)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(value <= 1);
			object_->type = value;
		}

 		void CueSheet::Track::set_index(uint32_t i, const ::FLAC__StreamMetadata_CueSheet_Index &indx)
 		{
 			FLAC__ASSERT(is_valid());
 			FLAC__ASSERT(i < object_->num_indices);
 			object_->indices[i] = indx;
 		}


		//
		// CueSheet
		//

		CueSheet::CueSheet():
		Prototype(FLAC__metadata_object_new(FLAC__METADATA_TYPE_CUESHEET), /*copy=*/false)
		{ }

		CueSheet::~CueSheet()
		{ }

		const char *CueSheet::get_media_catalog_number() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.cue_sheet.media_catalog_number;
		}

		FLAC__uint64 CueSheet::get_lead_in() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.cue_sheet.lead_in;
		}

		bool CueSheet::get_is_cd() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.cue_sheet.is_cd? true : false;
		}

		uint32_t CueSheet::get_num_tracks() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.cue_sheet.num_tracks;
		}

		CueSheet::Track CueSheet::get_track(uint32_t i) const
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(i < object_->data.cue_sheet.num_tracks);
			return Track(object_->data.cue_sheet.tracks + i);
		}

		void CueSheet::set_media_catalog_number(const char value[128])
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(0 != value);
			std::memcpy(object_->data.cue_sheet.media_catalog_number, value, 128);
			object_->data.cue_sheet.media_catalog_number[128] = '\0';
		}

		void CueSheet::set_lead_in(FLAC__uint64 value)
		{
			FLAC__ASSERT(is_valid());
			object_->data.cue_sheet.lead_in = value;
		}

		void CueSheet::set_is_cd(bool value)
		{
			FLAC__ASSERT(is_valid());
			object_->data.cue_sheet.is_cd = value;
		}

		void CueSheet::set_index(uint32_t track_num, uint32_t index_num, const ::FLAC__StreamMetadata_CueSheet_Index &indx)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(track_num < object_->data.cue_sheet.num_tracks);
			FLAC__ASSERT(index_num < object_->data.cue_sheet.tracks[track_num].num_indices);
			object_->data.cue_sheet.tracks[track_num].indices[index_num] = indx;
		}

		bool CueSheet::resize_indices(uint32_t track_num, uint32_t new_num_indices)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(track_num < object_->data.cue_sheet.num_tracks);
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_track_resize_indices(object_, track_num, new_num_indices));
		}

		bool CueSheet::insert_index(uint32_t track_num, uint32_t index_num, const ::FLAC__StreamMetadata_CueSheet_Index &indx)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(track_num < object_->data.cue_sheet.num_tracks);
			FLAC__ASSERT(index_num <= object_->data.cue_sheet.tracks[track_num].num_indices);
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_track_insert_index(object_, track_num, index_num, indx));
		}

		bool CueSheet::insert_blank_index(uint32_t track_num, uint32_t index_num)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(track_num < object_->data.cue_sheet.num_tracks);
			FLAC__ASSERT(index_num <= object_->data.cue_sheet.tracks[track_num].num_indices);
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_track_insert_blank_index(object_, track_num, index_num));
		}

		bool CueSheet::delete_index(uint32_t track_num, uint32_t index_num)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(track_num < object_->data.cue_sheet.num_tracks);
			FLAC__ASSERT(index_num < object_->data.cue_sheet.tracks[track_num].num_indices);
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_track_delete_index(object_, track_num, index_num));
		}

		bool CueSheet::resize_tracks(uint32_t new_num_tracks)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_resize_tracks(object_, new_num_tracks));
		}

		bool CueSheet::set_track(uint32_t i, const CueSheet::Track &track)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(i < object_->data.cue_sheet.num_tracks);
			// We can safely const_cast since copy=true
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_set_track(object_, i, const_cast< ::FLAC__StreamMetadata_CueSheet_Track*>(track.get_track()), /*copy=*/true));
		}

		bool CueSheet::insert_track(uint32_t i, const CueSheet::Track &track)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(i <= object_->data.cue_sheet.num_tracks);
			// We can safely const_cast since copy=true
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_insert_track(object_, i, const_cast< ::FLAC__StreamMetadata_CueSheet_Track*>(track.get_track()), /*copy=*/true));
		}

		bool CueSheet::insert_blank_track(uint32_t i)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(i <= object_->data.cue_sheet.num_tracks);
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_insert_blank_track(object_, i));
		}

		bool CueSheet::delete_track(uint32_t i)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(i < object_->data.cue_sheet.num_tracks);
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_delete_track(object_, i));
		}

		bool CueSheet::is_legal(bool check_cd_da_subset, const char **violation) const
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_cuesheet_is_legal(object_, check_cd_da_subset, violation));
		}

		FLAC__uint32 CueSheet::calculate_cddb_id() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__metadata_object_cuesheet_calculate_cddb_id(object_);
		}


		//
		// Picture
		//

		Picture::Picture():
		Prototype(FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE), /*copy=*/false)
		{ }

		Picture::~Picture()
		{ }

		::FLAC__StreamMetadata_Picture_Type Picture::get_type() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.picture.type;
		}

		const char *Picture::get_mime_type() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.picture.mime_type;
		}

		const FLAC__byte *Picture::get_description() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.picture.description;
		}

		FLAC__uint32 Picture::get_width() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.picture.width;
		}

		FLAC__uint32 Picture::get_height() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.picture.height;
		}

		FLAC__uint32 Picture::get_depth() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.picture.depth;
		}

		FLAC__uint32 Picture::get_colors() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.picture.colors;
		}

		FLAC__uint32 Picture::get_data_length() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.picture.data_length;
		}

		const FLAC__byte *Picture::get_data() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.picture.data;
		}

		void Picture::set_type(::FLAC__StreamMetadata_Picture_Type type)
		{
			FLAC__ASSERT(is_valid());
			object_->data.picture.type = type;
		}

		bool Picture::set_mime_type(const char *string)
		{
			FLAC__ASSERT(is_valid());
			// We can safely const_cast since copy=true
			return static_cast<bool>(::FLAC__metadata_object_picture_set_mime_type(object_, const_cast<char*>(string), /*copy=*/true));
		}

		bool Picture::set_description(const FLAC__byte *string)
		{
			FLAC__ASSERT(is_valid());
			// We can safely const_cast since copy=true
			return static_cast<bool>(::FLAC__metadata_object_picture_set_description(object_, const_cast<FLAC__byte*>(string), /*copy=*/true));
		}

		void Picture::set_width(FLAC__uint32 value) const
		{
			FLAC__ASSERT(is_valid());
			object_->data.picture.width = value;
		}

		void Picture::set_height(FLAC__uint32 value) const
		{
			FLAC__ASSERT(is_valid());
			object_->data.picture.height = value;
		}

		void Picture::set_depth(FLAC__uint32 value) const
		{
			FLAC__ASSERT(is_valid());
			object_->data.picture.depth = value;
		}

		void Picture::set_colors(FLAC__uint32 value) const
		{
			FLAC__ASSERT(is_valid());
			object_->data.picture.colors = value;
		}

		bool Picture::set_data(const FLAC__byte *data, FLAC__uint32 data_length)
		{
			FLAC__ASSERT(is_valid());
			// We can safely const_cast since copy=true
			return static_cast<bool>(::FLAC__metadata_object_picture_set_data(object_, const_cast<FLAC__byte*>(data), data_length, /*copy=*/true));
		}

		bool Picture::is_legal(const char **violation)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_picture_is_legal(object_, violation));
		}


		//
		// Unknown
		//

		Unknown::Unknown():
		Prototype(FLAC__metadata_object_new(FLAC__METADATA_TYPE_APPLICATION), /*copy=*/false)
		{ }

		Unknown::~Unknown()
		{ }

		const FLAC__byte *Unknown::get_data() const
		{
			FLAC__ASSERT(is_valid());
			return object_->data.application.data;
		}

		bool Unknown::set_data(const FLAC__byte *data, uint32_t length)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_application_set_data(object_, const_cast<FLAC__byte*>(data), length, true));
		}

		bool Unknown::set_data(FLAC__byte *data, uint32_t length, bool copy)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_object_application_set_data(object_, data, length, copy));
		}


		// ============================================================
		//
		//  Level 0
		//
		// ============================================================

		FLACPP_API bool get_streaminfo(const char *filename, StreamInfo &streaminfo)
		{
			FLAC__ASSERT(0 != filename);

			::FLAC__StreamMetadata object;

			if(::FLAC__metadata_get_streaminfo(filename, &object)) {
				streaminfo = object;
				return true;
			}
			else
				return false;
		}

		FLACPP_API bool get_tags(const char *filename, VorbisComment *&tags)
		{
			FLAC__ASSERT(0 != filename);

			::FLAC__StreamMetadata *object;

			tags = 0;

			if(::FLAC__metadata_get_tags(filename, &object)) {
				tags = new VorbisComment(object, /*copy=*/false);
				return true;
			}
			else
				return false;
		}

		FLACPP_API bool get_tags(const char *filename, VorbisComment &tags)
		{
			FLAC__ASSERT(0 != filename);

			::FLAC__StreamMetadata *object;

			if(::FLAC__metadata_get_tags(filename, &object)) {
				tags.assign(object, /*copy=*/false);
				return true;
			}
			else
				return false;
		}

		FLACPP_API bool get_cuesheet(const char *filename, CueSheet *&cuesheet)
		{
			FLAC__ASSERT(0 != filename);

			::FLAC__StreamMetadata *object;

			cuesheet = 0;

			if(::FLAC__metadata_get_cuesheet(filename, &object)) {
				cuesheet = new CueSheet(object, /*copy=*/false);
				return true;
			}
			else
				return false;
		}

		FLACPP_API bool get_cuesheet(const char *filename, CueSheet &cuesheet)
		{
			FLAC__ASSERT(0 != filename);

			::FLAC__StreamMetadata *object;

			if(::FLAC__metadata_get_cuesheet(filename, &object)) {
				cuesheet.assign(object, /*copy=*/false);
				return true;
			}
			else
				return false;
		}

		FLACPP_API bool get_picture(const char *filename, Picture *&picture, ::FLAC__StreamMetadata_Picture_Type type, const char *mime_type, const FLAC__byte *description, uint32_t max_width, uint32_t max_height, uint32_t max_depth, uint32_t max_colors)
		{
			FLAC__ASSERT(0 != filename);

			::FLAC__StreamMetadata *object;

			picture = 0;

			if(::FLAC__metadata_get_picture(filename, &object, type, mime_type, description, max_width, max_height, max_depth, max_colors)) {
				picture = new Picture(object, /*copy=*/false);
				return true;
			}
			else
				return false;
		}

		FLACPP_API bool get_picture(const char *filename, Picture &picture, ::FLAC__StreamMetadata_Picture_Type type, const char *mime_type, const FLAC__byte *description, uint32_t max_width, uint32_t max_height, uint32_t max_depth, uint32_t max_colors)
		{
			FLAC__ASSERT(0 != filename);

			::FLAC__StreamMetadata *object;

			if(::FLAC__metadata_get_picture(filename, &object, type, mime_type, description, max_width, max_height, max_depth, max_colors)) {
				picture.assign(object, /*copy=*/false);
				return true;
			}
			else
				return false;
		}


		// ============================================================
		//
		//  Level 1
		//
		// ============================================================

		SimpleIterator::SimpleIterator():
		iterator_(::FLAC__metadata_simple_iterator_new())
		{ }

		SimpleIterator::~SimpleIterator()
		{
			clear();
		}

		void SimpleIterator::clear()
		{
			if(0 != iterator_)
				FLAC__metadata_simple_iterator_delete(iterator_);
			iterator_ = 0;
		}

		bool SimpleIterator::init(const char *filename, bool read_only, bool preserve_file_stats)
		{
			FLAC__ASSERT(0 != filename);
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_simple_iterator_init(iterator_, filename, read_only, preserve_file_stats));
		}

		bool SimpleIterator::is_valid() const
		{
			return 0 != iterator_;
		}

		SimpleIterator::Status SimpleIterator::status()
		{
			FLAC__ASSERT(is_valid());
			return Status(::FLAC__metadata_simple_iterator_status(iterator_));
		}

		bool SimpleIterator::is_writable() const
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_simple_iterator_is_writable(iterator_));
		}

		bool SimpleIterator::next()
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_simple_iterator_next(iterator_));
		}

		bool SimpleIterator::prev()
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_simple_iterator_prev(iterator_));
		}

		//@@@@ add to tests
		bool SimpleIterator::is_last() const
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_simple_iterator_is_last(iterator_));
		}

		//@@@@ add to tests
		off_t SimpleIterator::get_block_offset() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__metadata_simple_iterator_get_block_offset(iterator_);
		}

		::FLAC__MetadataType SimpleIterator::get_block_type() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__metadata_simple_iterator_get_block_type(iterator_);
		}

		//@@@@ add to tests
		uint32_t SimpleIterator::get_block_length() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__metadata_simple_iterator_get_block_length(iterator_);
		}

		//@@@@ add to tests
		bool SimpleIterator::get_application_id(FLAC__byte *id)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_simple_iterator_get_application_id(iterator_, id));
		}

		Prototype *SimpleIterator::get_block()
		{
			FLAC__ASSERT(is_valid());
			return local::construct_block(::FLAC__metadata_simple_iterator_get_block(iterator_));
		}

		bool SimpleIterator::set_block(Prototype *block, bool use_padding)
		{
			FLAC__ASSERT(0 != block);
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_simple_iterator_set_block(iterator_, block->object_, use_padding));
		}

		bool SimpleIterator::insert_block_after(Prototype *block, bool use_padding)
		{
			FLAC__ASSERT(0 != block);
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_simple_iterator_insert_block_after(iterator_, block->object_, use_padding));
		}

		bool SimpleIterator::delete_block(bool use_padding)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_simple_iterator_delete_block(iterator_, use_padding));
		}


		// ============================================================
		//
		//  Level 2
		//
		// ============================================================

		Chain::Chain():
		chain_(::FLAC__metadata_chain_new())
		{ }

		Chain::~Chain()
		{
			clear();
		}

		void Chain::clear()
		{
			if(0 != chain_)
				FLAC__metadata_chain_delete(chain_);
			chain_ = 0;
		}

		bool Chain::is_valid() const
		{
			return 0 != chain_;
		}

		Chain::Status Chain::status()
		{
			FLAC__ASSERT(is_valid());
			return Status(::FLAC__metadata_chain_status(chain_));
		}

		bool Chain::read(const char *filename, bool is_ogg)
		{
			FLAC__ASSERT(0 != filename);
			FLAC__ASSERT(is_valid());
			return is_ogg?
				static_cast<bool>(::FLAC__metadata_chain_read_ogg(chain_, filename)) :
				static_cast<bool>(::FLAC__metadata_chain_read(chain_, filename))
			;
		}

		bool Chain::read(FLAC__IOHandle handle, ::FLAC__IOCallbacks callbacks, bool is_ogg)
		{
			FLAC__ASSERT(is_valid());
			return is_ogg?
				static_cast<bool>(::FLAC__metadata_chain_read_ogg_with_callbacks(chain_, handle, callbacks)) :
				static_cast<bool>(::FLAC__metadata_chain_read_with_callbacks(chain_, handle, callbacks))
			;
		}

		bool Chain::check_if_tempfile_needed(bool use_padding)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_chain_check_if_tempfile_needed(chain_, use_padding));
		}

		bool Chain::write(bool use_padding, bool preserve_file_stats)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_chain_write(chain_, use_padding, preserve_file_stats));
		}

		bool Chain::write(bool use_padding, ::FLAC__IOHandle handle, ::FLAC__IOCallbacks callbacks)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_chain_write_with_callbacks(chain_, use_padding, handle, callbacks));
		}

		bool Chain::write(bool use_padding, ::FLAC__IOHandle handle, ::FLAC__IOCallbacks callbacks, ::FLAC__IOHandle temp_handle, ::FLAC__IOCallbacks temp_callbacks)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_chain_write_with_callbacks_and_tempfile(chain_, use_padding, handle, callbacks, temp_handle, temp_callbacks));
		}

		void Chain::merge_padding()
		{
			FLAC__ASSERT(is_valid());
			::FLAC__metadata_chain_merge_padding(chain_);
		}

		void Chain::sort_padding()
		{
			FLAC__ASSERT(is_valid());
			::FLAC__metadata_chain_sort_padding(chain_);
		}


		Iterator::Iterator():
		iterator_(::FLAC__metadata_iterator_new())
		{ }

		Iterator::~Iterator()
		{
			clear();
		}

		void Iterator::clear()
		{
			if(0 != iterator_)
				FLAC__metadata_iterator_delete(iterator_);
			iterator_ = 0;
		}

		bool Iterator::is_valid() const
		{
			return 0 != iterator_;
		}

		void Iterator::init(Chain &chain)
		{
			FLAC__ASSERT(is_valid());
			FLAC__ASSERT(chain.is_valid());
			::FLAC__metadata_iterator_init(iterator_, chain.chain_);
		}

		bool Iterator::next()
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_iterator_next(iterator_));
		}

		bool Iterator::prev()
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_iterator_prev(iterator_));
		}

		::FLAC__MetadataType Iterator::get_block_type() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__metadata_iterator_get_block_type(iterator_);
		}

		Prototype *Iterator::get_block()
		{
			FLAC__ASSERT(is_valid());
			Prototype *block = local::construct_block(::FLAC__metadata_iterator_get_block(iterator_));
			if(0 != block)
				block->set_reference(true);
			return block;
		}

		bool Iterator::set_block(Prototype *block)
		{
			FLAC__ASSERT(0 != block);
			FLAC__ASSERT(is_valid());
			bool ret = static_cast<bool>(::FLAC__metadata_iterator_set_block(iterator_, block->object_));
			if(ret) {
				block->set_reference(true);
				delete block;
			}
			return ret;
		}

		bool Iterator::delete_block(bool replace_with_padding)
		{
			FLAC__ASSERT(is_valid());
			return static_cast<bool>(::FLAC__metadata_iterator_delete_block(iterator_, replace_with_padding));
		}

		bool Iterator::insert_block_before(Prototype *block)
		{
			FLAC__ASSERT(0 != block);
			FLAC__ASSERT(is_valid());
			bool ret = static_cast<bool>(::FLAC__metadata_iterator_insert_block_before(iterator_, block->object_));
			if(ret) {
				block->set_reference(true);
				delete block;
			}
			return ret;
		}

		bool Iterator::insert_block_after(Prototype *block)
		{
			FLAC__ASSERT(0 != block);
			FLAC__ASSERT(is_valid());
			bool ret = static_cast<bool>(::FLAC__metadata_iterator_insert_block_after(iterator_, block->object_));
			if(ret) {
				block->set_reference(true);
				delete block;
			}
			return ret;
		}

	} // namespace Metadata
} // namespace FLAC
