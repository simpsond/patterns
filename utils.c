//
// Created by Dustin Simpson on 6/3/23.
//

#include "utils.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <string.h>

/**
 * Given a text, and a starting offset, provides range into the original text
 * returns bool, is there more
 */
BOOL GetLineRange(ULONG from, STRPTR text, struct Range* r) {
  STRPTR pos;
  ULONG text_length = strlen(text);
  if(from > text_length -1) {
    return FALSE;
  }
  r->start = from;
  if((pos = strstr(&text[from], "\n"))) {
    r->end = pos-text;
    return TRUE;
  } else {
    r->end = strlen(text);
    return FALSE;
  }
}

void ExtractLine(STRPTR text_block, STRPTR buffer, struct Range r) {
  ULONG char_count = r.end - r.start;
  CopyMem(&text_block[r.start], buffer, char_count);
  buffer[char_count] = '\0';
}

/**
 * Given a pattern_string, formatted as an Amiga DOS pattern, we look at s_text
 * returns
 *      -1 if pattern_string isn't valid
 *       0 if s_text doesn't match the pattern
 *       1 if s_test does match the pattern
 * @param pattern_string
 * @param s_text
 * @note is_wild inside the function tells is pattern_string has a wildcard, but isn't used
 * @return
 */
int CheckStringPattern(STRPTR pattern_string, STRPTR s_text) {
  LONG is_wild = -1;
  BOOL is_match = FALSE;
  char parsed_pattern[30] = "";

  is_wild = ParsePattern(pattern_string, parsed_pattern, 30);
  if(is_wild < 0) {
    return -1;
  }

  if(MatchPattern(parsed_pattern, s_text)) {
    return 1;
  }
  return 0;
}

void HighlightString(STRPTR highlighted_string, STRPTR source) {
  strcat(highlighted_string, "\033b");
  strcat(highlighted_string, source);
//  strcat(highlighted_string, "\033b");
  strcat(highlighted_string, "\033n");
}

void AppendChar(STRPTR buffer, char c) {
  ULONG l = strlen(buffer);
  buffer[l] = c;
  buffer[l+1] = '\0';
}

void ForEachLine(STRPTR search_string, void (*fnc)(STRPTR in_string, STRPTR out_string, APTR extra), APTR extra) {
  struct Range r;
  ULONG line_counter = 0;
  STRPTR buffer = AllocMem(300, MEMF_CLEAR);
  for(ULONG from = 0; from < strlen(search_string); from = r.end + 1) {
    GetLineRange(from, search_string, &r)
    ExtractLine(search_string, buffer, r);
    fnc(buffer, 0, extra);
    line_counter ++;
  }
  FreeMem(buffer, 300);
}

int ReadFileIntoBuffer(STRPTR file, STRPTR *buffer, LONG* buffer_size_ptr) {
  BPTR fh;
  LONG buffer_size;

  fh = Open(file, MODE_OLDFILE);
  if(fh == 0) {
    printf("file  couldn't be opened.\n");
    return FALSE;
  }

  Seek(fh, 0, OFFSET_END);
  buffer_size = Seek(fh, 0, OFFSET_BEGINNING);

  *buffer = AllocMem(buffer_size+1, MEMF_CLEAR);
  FRead(fh, *buffer, buffer_size, 1);
  Close(fh);

  *buffer_size_ptr = buffer_size+1;
  return TRUE;
}

int ForEachReadDirList(STRPTR file, void(*cb)(struct FileInfoBlock* file_info)) {
  BPTR dir_lock;
  BOOL examine_result;
  struct FileInfoBlock* fi = AllocMem(sizeof(struct FileInfoBlock), MEMF_CLEAR);

  dir_lock = Lock(file, ACCESS_READ);
  examine_result = Examine(dir_lock, fi); // gives the directory passed in
  examine_result = ExNext(dir_lock, fi); // first file, if exists
  while(examine_result) {
//    printf("file name: \"%s\"\n", fi->fib_FileName);
    cb(fi);
    examine_result = ExNext(dir_lock, fi);
  }
  UnLock(dir_lock);
}

int ReadDirListIntoBuffer(STRPTR file, STRPTR *buffer, LONG* buffer_size_ptr) {
  BPTR dir_lock;
  struct FileInfoBlock file_info;
  BOOL examine_result;
  printf("In ReadDirListIntoBuffer\n");

  ULONG tmp_buff_size = 100;
  ULONG tmp_buff_used = 0;

  *buffer = AllocMem(tmp_buff_size, MEMF_CLEAR);

  dir_lock = Lock(file, ACCESS_READ);
  examine_result = Examine(dir_lock, &file_info); // gives the directory passed in
  examine_result = ExNext(dir_lock, &file_info); // first file, if exists
  while(examine_result) {
//    printf("file: %s\n", file_info.fib_FileName);
    ULONG str_length = strlen(file_info.fib_FileName);
    if(tmp_buff_used + str_length > tmp_buff_size) {
//      printf("not enough room, must grow.\n");
      ReAllocMem(buffer, tmp_buff_size, tmp_buff_size + str_length + 100);
      tmp_buff_size += str_length + 100;
//      printf("buffer new address: %p, size: %ul", buffer, tmp_buff_size);
    }
    CopyMem(file_info.fib_FileName, *buffer + tmp_buff_used, str_length);
    tmp_buff_used += str_length;
    *(*buffer + tmp_buff_used) = '\n';
    tmp_buff_used++;
//    printf("main buffer is: %s\n", buffer);
    examine_result = ExNext(dir_lock, &file_info);
  }
  UnLock(dir_lock);
  *buffer_size_ptr = tmp_buff_size;
  return TRUE;
}

BOOL ReAllocMem(void** mem_ptr, ULONG cur_size, ULONG new_size) {
  void* new_buffer = AllocMem(new_size, MEMF_CLEAR);
  if(!new_buffer) {
    return FALSE;
  }
  CopyMem(*mem_ptr, new_buffer, cur_size);
  FreeMem(*mem_ptr, cur_size);
  *mem_ptr = new_buffer;
  return TRUE;
}

void GetHighlights(STRPTR* buffer, STRPTR text) {
//  STRPTR exported_text = (STRPTR)DoMethod(text_control, MUIM_TextEditor_ExportText);
  STRPTR mark = text;
  STRPTR start_mark;
  char the_marker[3] = "\033b";
  ULONG match_length = 0;
  ULONG match_count = 0;
  ULONG buffer_offset = 0;
  STRPTR new_buffer;

  new_buffer = AllocVec(strlen(text), MEMF_CLEAR);

  while((mark = (STRPTR)strstr(mark, the_marker))) {
    mark+=2;
    match_count++
    if(match_count % 2 == 0) {
      match_length = mark - start_mark - 2;
      CopyMem(start_mark, new_buffer+ buffer_offset, match_length);
      the_marker[1] = 'b';
      buffer_offset += match_length;
      new_buffer[buffer_offset] = '\n';
      buffer_offset++;
    } else {
      start_mark = mark;
      the_marker[1] = 'n';
    }
  }
  FreeVec(text);
  *(buffer) = new_buffer;
}
