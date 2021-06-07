#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <strings.h>

void help();
void copyMain(const char * fdSrc, const char * fdDst);

int main(int argc, char ** argv) {
  int opt;
  int optFlag = 0;
  while ((opt = getopt(argc, argv, "mh")) != -1)
    switch (opt) {
    case 'h':
      help();
      return 0;
      break;

    case 'm':
      optFlag = 1;
      break;

    }

  char * fdSrc, * fdDst;
  if (argc == 3 && optFlag == 0) {
    fdSrc = argv[1];
    fdDst = argv[2];
  } else if (argc == 4 && optFlag == 1) {
    fdSrc = argv[2];
    fdDst = argv[3];
  } else {
    help();
    return 0;
  }

  if (optFlag == 0) {
    printf("Content To be Copied with read/write \n");
    copyMain(fdSrc, fdDst);

  }
  if (optFlag == 1) {
    printf("Content To be Copied with memory map \n");
    copyMain(fdSrc, fdDst);
  }

  return 0;
}

void copyMain(const char * fdSrc,
  const char * fdDst) {
  int copyFrom = open(fdSrc, O_RDONLY);
  int copyTo = open(fdDst, O_CREAT | O_RDWR);
  if (copyFrom == -1 || copyTo == -1) {
    printf("File %s does not exist. Please provide a valid file. Copy failed.\n", fdSrc);
    return;
  }

  struct stat statbuf;
  if (fstat(copyFrom, & statbuf) < 0) {
    printf("File status failed. Copy failed.\n");
    return;
  }
  const unsigned size = statbuf.st_size;
  char buffer[size + 1];
  bzero(buffer, size);
  int fileread = read(copyFrom, buffer, size);
  write(copyTo, buffer, size);
  close(copyFrom);
  close(copyTo);
}

void help() {
  printf("\n A simple program to copy one file's content to another\n");
  printf("./copy -m [source_file] [destination_file] -- copies one file to another with memory mapping\n");
  printf("./copy [source_file] [destination_file] -- copies one file to another with read/write\n");
  printf("./copy -h -- display the help text.\n");
}
