//
// Created by Dustin Simpson on 6/3/23.
//

#include "gui_helpers.h"
#include "utils.h"

#include <stdio.h>

#include <proto/exec.h>
#include <libraries/mui.h>
#include <proto/muimaster.h>
#include <BetterString_mcc.h>
#include <TextEditor_mcc.h>

void AppendControl(STRPTR buffer, APTR text_control) {
  AppendChar(buffer, '\n');
  DoMethod(text_control, MUIM_TextEditor_InsertText, buffer, MUIV_TextEditor_InsertText_Bottom);
}

void HighlightAndAppendControl(STRPTR buffer, APTR text_control) {
  STRPTR highlighted_buffer = AllocMem(300, MEMF_CLEAR);
  HighlightString(highlighted_buffer, buffer);
  AppendControl(highlighted_buffer, text_control)
  FreeMem(highlighted_buffer, 300);
}

void SetTextEditorContent(STRPTR buffer, APTR text_control) {
  DoMethod(text_control, MUIM_TextEditor_ClearText);
  DoMethod(text_control, MUIM_TextEditor_InsertText, buffer, MUIV_TextEditor_InsertText_Bottom);
}

/**
 * Given a TextEditorObject this function exports the content w/o markup, clears the editor and
 * inserts the cleaned up text
 * @param editor
 * @return a clean (w/o markup) version if the editor content
 */
STRPTR ClearTextEditorMarkup(APTR editor) {
  STRPTR search_string = (STRPTR)DoMethod(editor, MUIM_TextEditor_ExportText);
  DoMethod(editor, MUIM_TextEditor_ClearText);
  DoMethod(editor, MUIM_TextEditor_InsertText, search_string, MUIV_TextEditor_InsertText_Top);
  return search_string;
}


