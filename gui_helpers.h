//
// Created by Dustin Simpson on 6/3/23.
//

#ifndef PATTERNS__GUI_HELPERS_H_
#define PATTERNS__GUI_HELPERS_H_

#include <exec/types.h>

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

void HighlightAndAppendControl(STRPTR buffer, APTR text_control);
void AppendControl(STRPTR buffer, APTR text_control);
void SetTextEditorContent(STRPTR buffer, APTR text_control);
STRPTR ClearTextEditorMarkup(APTR editor);

#endif //PATTERNS__GUI_HELPERS_H_
