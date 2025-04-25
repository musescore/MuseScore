#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sndfile.h>

typedef struct
{
  sf_count_t offset;
  sf_count_t length;
  const unsigned char *data;
} VIO_DATA;

static sf_count_t vfget_filelen (void *user_data)
{
  VIO_DATA *vf = (VIO_DATA *)user_data;
  return vf->length;
}

static sf_count_t vfseek (sf_count_t offset, int whence, void *user_data)
{
  VIO_DATA *vf = (VIO_DATA *)user_data;

  switch (whence)
  {
    case SEEK_SET:
      vf->offset = offset;
      break ;

    case SEEK_CUR:
      vf->offset = vf->offset + offset;
      break ;

    case SEEK_END:
      vf->offset = vf->length + offset;
      break;

    default:
      break;
  }

  return vf->offset;
}

static sf_count_t vfread (void *ptr, sf_count_t count, void *user_data)
{
  VIO_DATA *vf = (VIO_DATA *)user_data;

  if (vf->offset + count > vf->length)
  {
    count = vf->length - vf->offset;
  }

  memcpy(ptr, vf->data + vf->offset, count);
  vf->offset += count;

  return count;
}

static sf_count_t vfwrite (const void *ptr, sf_count_t count, void *user_data)
{
  (void)ptr;
  (void)count;
  (void)user_data;

  // Cannot write to this virtual file.
  return 0;
}

static sf_count_t vftell (void *user_data)
{ VIO_DATA *vf = (VIO_DATA *)user_data;

  return vf->offset;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  VIO_DATA vio_data;
  SF_VIRTUAL_IO vio;
  SF_INFO sndfile_info;
  SNDFILE *sndfile = NULL;
  float* read_buffer = NULL;

  // Initialize the virtual IO structure.
  vio.get_filelen = vfget_filelen;
  vio.seek = vfseek;
  vio.read = vfread;
  vio.write = vfwrite;
  vio.tell = vftell;

  // Initialize the VIO user data.
  vio_data.data = data;
  vio_data.length = size;
  vio_data.offset = 0;

  memset(&sndfile_info, 0, sizeof(SF_INFO));

  // Try and open the virtual file.
  sndfile = sf_open_virtual(&vio, SFM_READ, &sndfile_info, &vio_data);

  if (sndfile_info.channels == 0)
  {
    // No sound channels in file.
    goto EXIT_LABEL;
  }
  else if (sndfile_info.channels > 1024 * 1024)
  {
    // Too many channels to handle.
    goto EXIT_LABEL;
  }

  // Just the right number of channels. Create some buffer space for reading.
  read_buffer = (float*)malloc(sizeof(float) * sndfile_info.channels);
  if (read_buffer == NULL)
  {
    abort();
  }

  while (sf_readf_float(sndfile, read_buffer, 1))
  {
    // Do nothing with the data.
  }

EXIT_LABEL:

  if (sndfile != NULL)
  {
    sf_close(sndfile);
  }

  free(read_buffer);

  return 0;
}
