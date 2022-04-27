#include <stdio.h>
#include "opusenc.h"

#define READ_SIZE 256

int main(int argc, char **argv) {
  FILE *fin;
  OggOpusEnc *enc;
  OggOpusComments *comments;
  int error;
  if (argc != 3) {
    fprintf(stderr, "usage: %s <raw pcm input> <Ogg Opus output>\n", argv[0]);
    return 1;
  }
  fin = fopen(argv[1], "rb");
  if (!fin) {
    fprintf(stderr, "cannot open input file: %s\n", argv[1]);
    return 1;
  }
  comments = ope_comments_create();
  ope_comments_add(comments, "ARTIST", "Someone");
  ope_comments_add(comments, "TITLE", "Some track");
  enc = ope_encoder_create_file(argv[2], comments, 44100, 2, 0, &error);
  if (!enc) {
    fprintf(stderr, "error encoding to file %s: %s\n", argv[2], ope_strerror(error));
    ope_comments_destroy(comments);
    fclose(fin);
    return 1;
  }
  while (1) {
    short buf[2*READ_SIZE];
    int ret = fread(buf, 2*sizeof(short), READ_SIZE, fin);
    if (ret > 0) {
      ope_encoder_write(enc, buf, ret);
    } else break;
  }
  ope_encoder_drain(enc);
  ope_encoder_destroy(enc);
  ope_comments_destroy(comments);
  fclose(fin);
  return 0;
}
