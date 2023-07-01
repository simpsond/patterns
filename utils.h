//
// Created by Dustin Simpson on 6/3/23.
//

#ifndef PATTERNS__UTILS_H_
#define PATTERNS__UTILS_H_

#include <exec/types.h>
struct Range {
  ULONG start;
  ULONG end;
};

int ReadFileIntoBuffer(STRPTR file, STRPTR *buffer, LONG* buffer_size_ptr);
int ReadDirListIntoBuffer(STRPTR file, STRPTR *buffer, LONG* buffer_size_ptr);
int ForEachReadDirList(STRPTR file, void(*cb)(struct FileInfoBlock* file_info));

void AppendChar(STRPTR buffer, char c);
BOOL GetLineRange(ULONG from, STRPTR text, struct Range* r);
void ExtractLine(STRPTR text_block, STRPTR buffer, struct Range r);
int CheckStringPattern(STRPTR pattern_string, STRPTR s_text);
void AppendControl(STRPTR buffer, APTR text_control);
void HighlightString(STRPTR highlighted_string, STRPTR source);
void ForEachLine(STRPTR search_string, void (*fnc)(STRPTR in_string, STRPTR out_string), APTR extra);

BOOL ReAllocMem(void** mem_ptr, ULONG cur_size, ULONG new_size);

#endif //PATTERNS__UTILS_H_
