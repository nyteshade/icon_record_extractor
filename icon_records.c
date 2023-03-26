#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef unsigned char bool;

typedef struct {
  uint32_t identifier;
  uint32_t size;
} RecordHeader;

typedef struct {
  RecordHeader header;
  void* data;
  bool isIcon;  
  bool isNestedIcon;
} Record;

char* Uint32String(uint32_t value)
{
  char* result = calloc(1, sizeof(char) * 5);
  
  result[0] = ((char*)&value)[3];
  result[1] = ((char*)&value)[2];
  result[2] = ((char*)&value)[1];
  result[3] = ((char*)&value)[0];
  
  return result;
}

uint32_t ReadUint32(FILE* file) {
  uint32_t read;
  if (fread(&read, sizeof(read), 1, file) != 1)
    return 0;

  return ntohl(read);
}

bool WriteUint32(uint32_t to_write, FILE* file) {
  uint32_t msb_first = htonl(to_write);
  return fwrite(&msb_first, sizeof(msb_first), 1, file) == 1;
}

void PrintRecordHeader(RecordHeader* header)
{
  char* idstring = NULL;
  
  idstring = Uint32String(header->identifier);

  printf("\nIdentifier: %s\n", idstring);
  printf("Size: %ld\n", (long)header->size);

  free(idstring);
}

bool WriteIcnsFileMetadata(FILE* file) {
  long size = ftell(file);
  return size >= 0 && fseek(file, 4L, SEEK_SET) == 0 && WriteUint32(size, file);
}

bool WriteRecord(const char *filename, Record *record)
{
  FILE* out = fopen(filename, "wb+");
  if (!out)
    return 0;

  if (record->isIcon || record->isNestedIcon) WriteUint32('icns', out);
  if (record->isIcon || record->isNestedIcon) WriteUint32(0, out);
  if (record->isIcon) WriteUint32(record->header.identifier, out);
  if (record->isIcon) WriteUint32(record->header.size, out);
  printf("[Writing%s...]\n", record->isIcon ? " icon" : "");
  fwrite(record->data, record->header.size - 8, 1, out);
  if (record->isIcon || record->isNestedIcon) WriteIcnsFileMetadata(out);
  fclose(out);
  
  return 1;
}


int main(int argc, char **argv) {
  FILE* file = fopen(argv[1], "r");
  FILE* out = NULL;
  
  Record record;
  char* idstring = NULL;
  char* sDesiredName = NULL;
  long position = 0L;
  long eof = 0L;
  int wroteCount = 0;
  int failure = 0;
  
  uint32_t kDark = '\xfd\xd9\x2f\xa8';
  uint32_t tDesired = 0;
  
  if (!file) 
  {
    printf("Cannot open file %s\n", argv[1]);
    return 1;
  }
  
  if (argc > 2) 
  {
    if (strcmp("dark", argv[2]) == 0)
      tDesired = kDark;
    else
      tDesired = (argv[2][0] << 24) | (argv[2][1] << 16) | (argv[2][2] << 8) | argv[2][3];
      
    if (argc > 3) 
    {
      sDesiredName = calloc(1, strlen(argv[3]));
      strcpy(sDesiredName, argv[3]);
    }
  }
  
  fseek(file, 0, SEEK_END);
  eof = ftell(file);
  fseek(file, 0, SEEK_SET);

  printf("[Position %ld]\n", ftell(file));
  record.header.identifier = ReadUint32(file);
  record.header.size = ReadUint32(file);
  PrintRecordHeader(&record.header);
  printf("[Position %ld]\n", ftell(file));
  
  while (position != eof) 
  {
    record.header.identifier = ReadUint32(file);
    record.header.size = ReadUint32(file);
    PrintRecordHeader(&record.header);
    printf("[Position %ld]\n", ftell(file));
    
    position = ftell(file);
    printf("[Reading...]\n");
    record.data = calloc(1, record.header.size - 8);
    fread(record.data, record.header.size - 8, 1, file);
    position = ftell(file);
    printf("[Position %ld]\n", ftell(file));
  
    idstring = Uint32String(record.header.identifier);
    strcat(idstring, ".icns");
    record.isIcon = 1;
    record.isNestedIcon = 0;
    
    if (record.header.identifier == kDark)
    {
      strcpy(idstring, "dark.icns");
      record.isIcon = 0;
      record.isNestedIcon = 1;
    }
      
    if (record.header.identifier == 'info')
    {
      strcpy(idstring, "info.plist");
      record.isIcon = 0;
    }

    if (!tDesired || (tDesired && tDesired == record.header.identifier))
    {
      failure = !WriteRecord(sDesiredName ? sDesiredName : idstring, &record); 
      if (!failure)
        wroteCount++;    
    }
    else
    {
      printf("[Skipping...]\n");
    } 
    free(idstring);
  }
  
  if (sDesiredName)
    free(sDesiredName);
    
  fclose(file);
    
  if (tDesired && wroteCount == 0)
    failure = 1;
    
  return failure;
}

